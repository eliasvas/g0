#include "vn.h"
#include "gui/gui.h"

// TODO: Make the game.json loaded from disk ok?
static u8 game_json[] = {
#embed "../../data/game.json"
};

void vn_extract(VN_System *vns, Json_Element *node) {
  vns->current_node = node;
  vns->transition_sec = buf_to_float(json_lookup(node, MAKE_STR("trans_sec"))->value);
  vns->max_transition_sec = vns->transition_sec;
  vns->text = json_lookup(node, MAKE_STR("text"))->value;
  vns->name = json_lookup(node, MAKE_STR("name"))->value;
  vns->img_idx = buf_to_int(json_lookup(node, MAKE_STR("img_idx"))->value);
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
  vns->transition_sec -= dt;


  // Dialog box on down tile of screen
  Gui_Box *down = gui_box_lookup_from_key(0, gui_key_from_str(MAKE_STR("panel_p_down")));
  // Poor man's scrolling
  buf string_to_display = buf_make(vns->text.data, maximum(1, vns->text.count * lerp(0.0, 1.0, (vns->max_transition_sec - maximum(0,vns->transition_sec))/vns->max_transition_sec)));
  assert(!gui_box_is_nil(down));
  gui_set_next_parent(down);
  Gui_Dialog_State ds = gui_dialog(MAKE_STR("dialog1"), vns->name, string_to_display);  

  if (vns->transition_sec <= 0.0 && ds == GUI_DIALOG_STATE_NEXT_PRESSED) {
    u32 next_id = buf_to_float(json_lookup(vns->current_node, MAKE_STR("nextID"))->value);
    Json_Element *next = vn_goto(vns, next_id);
    vn_extract(vns, next);
  } else if (ds == GUI_DIALOG_STATE_NEXT_PRESSED) {
    vns->transition_sec = 0;
  }

}

