#include "vn.h"
#include "gui/gui.h"
#include "core/core_inc.h"

// TODO: Make the game.json loaded from disk ok?
static u8 game_json[] = {
#embed "../../data/game.json"
};

void vn_extract(VN_System *vns, Json_Element *node) {
  vns->current_node = node;
  Json_Element *dialog = json_lookup(node, MAKE_STR("Dialog"));
  if (dialog) {
    vns->active_dialog.text = json_lookup(dialog, MAKE_STR("text"))->value;
    vns->active_dialog.name = json_lookup(dialog, MAKE_STR("name"))->value;
    vns->active_dialog.img_idx = buf_to_int(json_lookup(dialog, MAKE_STR("img_idx"))->value);
  }

  vns->duration_target = 0; 
  vns->duration = 0; // This is running duration 
  Json_Element *stat = json_lookup(node, MAKE_STR("Static"));
  if (stat) {
    vns->duration_target = buf_to_float(json_lookup(stat, MAKE_STR("duration"))->value);
    vns->next_node_idx = buf_to_int(json_lookup(stat, MAKE_STR("nextID"))->value);
  }

  vns->choice_count = 0;
  Json_Element *choices = json_lookup(node, MAKE_STR("Choices"));
  if (choices) {
    u32 itr = 0;
    // for each choice found in the array add it to available ones in the VN
    for (Json_Element *choice = choices->first; choice != nullptr; choice = choice->next) {
      buf choice_text = json_lookup(choice, MAKE_STR("choice"))->value;
      u32 next_id = buf_to_int(json_lookup(choice, MAKE_STR("nextID"))->value);
      vns->choices[itr] = (VN_Choice){choice_text, next_id};
      itr+=1;
    }
    vns->choice_count = itr;
  }

  effect_destroy(&vns->active_effect.e);
  Json_Element *effect = json_lookup(node, MAKE_STR("Effect"));
  if (effect) {
    buf kind = json_lookup(effect, MAKE_STR("kind"))->value;
    if (buf_eq(kind, MAKE_STR("EFFECT_KIND_FILL"))) {
      vns->active_effect.e = effect_make(EFFECT_KIND_FILL);
    } else if (buf_eq(kind, MAKE_STR("EFFECT_KIND_VORTEX"))) {
      vns->active_effect.e = effect_make(EFFECT_KIND_VORTEX);
    }
    Json_Element *param0 = json_lookup(effect, MAKE_STR("param0"));
    if (param0) {
      param0 = param0->first;
      for (u32 i = 0; i < 4; ++i) {
        vns->active_effect.param0.raw[i] = buf_to_float(param0->value);
        param0 = param0->next;
      }
    }
    Json_Element *param1 = json_lookup(effect, MAKE_STR("param1"));
    if (param1) {
      param1 = param1->first;
      for (u32 i = 0; i < 4; ++i) {
        vns->active_effect.param1.raw[i] = buf_to_float(param1->value);
        param1 = param1->next;
      }
    }
    Json_Element *duration  = json_lookup(effect, MAKE_STR("duration"));
    vns->active_effect.duration = (duration) ? buf_to_float(duration->value) : F64_MAX;
  }
}

// TODO: Make the VN_System have its own 'game' arena ok?
VN_System vn_load_new_game(Arena *arena) {
  game_json[sizeof(game_json)-1] = '\0'; // null teminate the embedded json
  Json_Element *root = json_parse(arena, (char*)game_json);
  assert(root);

  // First Json array element, start of the game
  Json_Element *current = root->first;
  assert(current);

  VN_System vns = (VN_System){
    .root_node = root,
  };

  vn_extract(&vns, current);

  return vns;
}


Json_Element * vn_goto(VN_System *vns, u32 idx) {
  Json_Element *current = vns->root_node->first;
  while(idx--) {
    current = current->next;
  }
  return current;
}

void vn_simulate(VN_System *vns, v2 screen_dim, f64 dt) {
  Gui_Box *up_right = gui_box_lookup_from_key(0, gui_key_from_str(MAKE_STR("panel_p_up_right")));
  assert(!gui_box_is_nil(up_right));
 
  // 1. First render the effect
  // I don't like this too much, effects kinda suck rn
  if (vns->active_effect.duration > 0 && vns->active_effect.e.kind != EFFECT_KIND_NONE) {
    rect v = ogl_to_gl_rect(up_right->r, screen_dim.y);
    Effect_Data ed = {
      //.screen_dim = v2m(v.w, v.h),
      .screen_dim = screen_dim,
      .time_sec = platform_get_time(),
      .framerate = 1.0f/dt,
      .param0 = vns->active_effect.param0,
      .param1 = vns->active_effect.param1,
    };
    effect_render(&vns->active_effect.e, &ed, screen_dim, v);
    vns->active_effect.duration = maximum(0, vns->active_effect.duration - dt);
  }

  // 2. Then render the dialog box
  // Dialog box on down tile of screen
  Gui_Box *down = gui_box_lookup_from_key(0, gui_key_from_str(MAKE_STR("panel_p_down")));
  assert(!gui_box_is_nil(down));
  VN_Active_Dialog *dialog = &vns->active_dialog;
  buf s_cut = buf_make(dialog->text.data, lerp(1, dialog->text.count, (vns->duration_target) ? vns->duration / vns->duration_target : 1));
  gui_set_next_parent(down);
  Gui_Dialog_State ds = gui_dialog(MAKE_STR("dialog1"), dialog->name, s_cut);  

  if ( (vns->duration_target - vns->duration) <= 0.0 && ds == GUI_DIALOG_STATE_NEXT_PRESSED) {
    Json_Element *next = vn_goto(vns, vns->next_node_idx);
    vn_extract(vns, next);
  } else if (ds == GUI_DIALOG_STATE_NEXT_PRESSED) {
    vns->duration = vns->duration_target;
  } else if (vns->duration_target - vns->duration > 0.0) {
    vns->duration = clamp(vns->duration+dt, 0, vns->duration_target);
  }

  // 3. Then any choices the player needs to take
  // HORRIBLE HORRIBLE HORRIBLE
  gui_push_parent(up_right);
  if (vns->choice_count > 0) {
    buf choices[MAX_VN_CHOICE];
    for (int i =0; i < vns->choice_count; ++i)choices[i] = vns->choices[i].text;
    int idx = gui_choice_box(MAKE_STR("choice_box"), choices, vns->choice_count);
    if (idx != -1) {
      Json_Element *next = vn_goto(vns, vns->choices[idx].next_node_idx);
      vn_extract(vns, next);
    }
  }
  gui_pop_parent();
}



