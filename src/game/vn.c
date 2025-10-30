#include "vn.h"
#include "gui/gui.h"

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

  // MORE to be added
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

void vn_simulate(VN_System *vns, f64 dt) {
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

  // HORRIBLE HORRIBLE HORRIBLE
  Gui_Box *up_right = gui_box_lookup_from_key(0, gui_key_from_str(MAKE_STR("panel_p_up_right")));
  assert(!gui_box_is_nil(up_right));
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





