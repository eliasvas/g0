#include "gui.h"

#define gui_stack_top_impl(state, name_upper, name_lower) return state->name_lower##_stack.top->v;

#define gui_stack_bottom_impl(state, name_upper, name_lower) return state->name_lower##_stack.bottom_val;

#define gui_stack_push_impl(state, name_upper, name_lower, type, new_value) \
Gui_##name_upper##_Node *node = state->name_lower##_stack.free;\
if(node != 0) {sll_stack_pop(state->name_lower##_stack.free);}\
else {node = arena_push_array(gui_get_build_arena(), Gui_##name_upper##_Node, 1);}\
type old_value = state->name_lower##_stack.top->v;\
node->v = new_value;\
sll_stack_push(state->name_lower##_stack.top, node);\
if(node->next == &state->name_lower##_nil_stack_top)\
{\
state->name_lower##_stack.bottom_val = (new_value);\
}\
state->name_lower##_stack.auto_pop = 0;\
return old_value;

#define gui_stack_pop_impl(state, name_upper, name_lower) \
Gui_##name_upper##_Node *popped = state->name_lower##_stack.top;\
if(popped != &state->name_lower##_nil_stack_top)\
{\
sll_stack_pop(state->name_lower##_stack.top);\
sll_stack_push(state->name_lower##_stack.free, popped);\
state->name_lower##_stack.auto_pop = 0;\
}\
return popped->v;\

#define gui_stack_set_next_impl(state, name_upper, name_lower, type, new_value) \
Gui_##name_upper##_Node *node = state->name_lower##_stack.free;\
if(node != 0) {sll_stack_pop(state->name_lower##_stack.free);}\
else {node = arena_push_array(gui_get_build_arena(), Gui_##name_upper##_Node, 1);}\
type old_value = state->name_lower##_stack.top->v;\
node->v = new_value;\
sll_stack_push(state->name_lower##_stack.top, node);\
state->name_lower##_stack.auto_pop = 1;\
return old_value;\

#define gui_stack_empty_impl(state, name_upper, name_lower) \
Gui_##name_upper##_Node *top = state->name_lower##_stack.top;\
return (top == &state->name_lower##_nil_stack_top);\
//-----------------------------------------------------------------------------



// This function should be huge and initialize ALL the UI stacks to empty
// TODO -- MAYBE this should be just some macro (maybe), declarations too?
void gui_init_stacks() {
	Gui_Context *state = gui_get_ctx();
	// -- parent stack initialization
	state->parent_nil_stack_top.v = gui_box_nil_id();
	state->parent_stack.top = &state->parent_nil_stack_top;
	state->parent_stack.bottom_val = gui_box_nil_id();
	state->parent_stack.free = 0;
	state->parent_stack.auto_pop = 0;
	// -- fixed_x stack initialization
	state->fixed_x_nil_stack_top.v = 0;
	state->fixed_x_stack.top = &state->fixed_x_nil_stack_top;
	state->fixed_x_stack.bottom_val = 0;
	state->fixed_x_stack.free = 0;
	state->fixed_x_stack.auto_pop = 0;
	// -- fixed_y stack initialization
	state->fixed_y_nil_stack_top.v = 0;
	state->fixed_y_stack.top = &state->fixed_y_nil_stack_top;
	state->fixed_y_stack.bottom_val = 0;
	state->fixed_y_stack.free = 0;
	state->fixed_y_stack.auto_pop = 0;
	// -- fixed_width stack initialization
	state->fixed_width_nil_stack_top.v = 0;
	state->fixed_width_stack.top = &state->fixed_width_nil_stack_top;
	state->fixed_width_stack.bottom_val = 0;
	state->fixed_width_stack.free = 0;
	state->fixed_width_stack.auto_pop = 0;
	// -- fixed_height stack initialization
	state->fixed_height_nil_stack_top.v = 0;
	state->fixed_height_stack.top = &state->fixed_height_nil_stack_top;
	state->fixed_height_stack.bottom_val = 0;
	state->fixed_height_stack.free = 0;
	state->fixed_height_stack.auto_pop = 0;
	// -- pref_width stack initialization
	state->pref_width_nil_stack_top.v = (Gui_Size){GUI_SIZE_KIND_PIXELS,250.0f,1.0};
	state->pref_width_stack.top = &state->pref_width_nil_stack_top;
	state->pref_width_stack.bottom_val = state->pref_width_nil_stack_top.v;
	state->pref_width_stack.free = 0;
	state->pref_width_stack.auto_pop = 0;
	// -- pref_height stack initialization
	state->pref_height_nil_stack_top.v = (Gui_Size){GUI_SIZE_KIND_PIXELS,40.0f,1.0};
	state->pref_height_stack.top = &state->pref_height_nil_stack_top;
	state->pref_height_stack.bottom_val = state->pref_height_nil_stack_top.v;
	state->pref_height_stack.free = 0;
	state->pref_height_stack.auto_pop = 0;
	// -- bg_color stack initialization
	state->bg_color_nil_stack_top.v = v4m(0,0,0,0);
	state->bg_color_stack.top = &state->bg_color_nil_stack_top;
	state->bg_color_stack.bottom_val = state->bg_color_nil_stack_top.v;
	state->bg_color_stack.free = 0;
	state->bg_color_stack.auto_pop = 0;
	// -- text_color stack initialization
	state->text_color_nil_stack_top.v = v4m(1,1,1,1);
	state->text_color_stack.top = &state->text_color_nil_stack_top;
	state->text_color_stack.bottom_val = state->text_color_nil_stack_top.v;
	state->text_color_stack.free = 0;
	state->text_color_stack.auto_pop = 0;
	// -- child_layout_axis stack initialization
	state->child_layout_axis_nil_stack_top.v = GUI_AXIS_X;
	state->child_layout_axis_stack.top = &state->child_layout_axis_nil_stack_top;
	state->child_layout_axis_stack.bottom_val = state->child_layout_axis_nil_stack_top.v;
	state->child_layout_axis_stack.free = 0;
	state->child_layout_axis_stack.auto_pop = 0;

	// -- panel iterator stack initialization (NOT part of box state)
  // TODO: pull out to another place, or just, don't support set next? maybe ok compromise
  state->panel_itr_nil_stack_top.v = (Gui_Panel_Itr) { nullptr, nullptr };
	state->panel_itr_stack.top= &state->panel_itr_nil_stack_top;
	state->panel_itr_stack.bottom_val = state->panel_itr_nil_stack_top.v;
	state->panel_itr_stack.free = 0;
	state->panel_itr_stack.auto_pop = 0;
}

void gui_autopop_all_stacks() {
	Gui_Context *state = gui_get_ctx();
	if (state->parent_stack.auto_pop) { gui_pop_parent();state->parent_stack.auto_pop = 0; }
	if (state->fixed_x_stack.auto_pop) { gui_pop_fixed_x();state->fixed_x_stack.auto_pop = 0; }
	if (state->fixed_y_stack.auto_pop) { gui_pop_fixed_y();state->fixed_y_stack.auto_pop = 0; }
	if (state->fixed_width_stack.auto_pop) { gui_pop_fixed_width();state->fixed_width_stack.auto_pop = 0; }
	if (state->fixed_height_stack.auto_pop) { gui_pop_fixed_height();state->fixed_height_stack.auto_pop = 0; }
	if (state->pref_width_stack.auto_pop) { gui_pop_pref_width();state->pref_width_stack.auto_pop = 0; }
	if (state->pref_height_stack.auto_pop) { gui_pop_pref_height();state->pref_height_stack.auto_pop = 0; }
	if (state->bg_color_stack.auto_pop) { gui_pop_bg_color();state->bg_color_stack.auto_pop = 0; }
	if (state->text_color_stack.auto_pop) { gui_pop_text_color();state->text_color_stack.auto_pop = 0; }
	if (state->child_layout_axis_stack.auto_pop) { gui_pop_child_layout_axis();state->child_layout_axis_stack.auto_pop = 0; }
	if (state->panel_itr_stack.auto_pop) { gui_pop_panel_itr();state->panel_itr_stack.auto_pop = 0; }
}

Gui_Box *gui_top_parent(void) { gui_stack_top_impl(gui_get_ctx(), Parent, parent); }
Gui_Box *gui_set_next_parent(Gui_Box *box) { gui_stack_set_next_impl(gui_get_ctx(), Parent, parent, Gui_Box*, box); }
Gui_Box *gui_push_parent(Gui_Box *box) { gui_stack_push_impl(gui_get_ctx(), Parent, parent, Gui_Box*, box); }
Gui_Box *gui_pop_parent(void) { gui_stack_pop_impl(gui_get_ctx(), Parent, parent); }

f32 gui_top_fixed_x(void) { gui_stack_top_impl(gui_get_ctx(), Fixed_X, fixed_x); }
f32 gui_set_next_fixed_x(f32 v) { gui_stack_set_next_impl(gui_get_ctx(), Fixed_X, fixed_x, f32, v); }
f32 gui_push_fixed_x(f32 v) { gui_stack_push_impl(gui_get_ctx(), Fixed_X, fixed_x, f32, v); }
f32 gui_pop_fixed_x(void) { gui_stack_pop_impl(gui_get_ctx(), Fixed_X, fixed_x); }

f32 gui_top_fixed_y(void) { gui_stack_top_impl(gui_get_ctx(), Fixed_Y, fixed_y); }
f32 gui_set_next_fixed_y(f32 v) { gui_stack_set_next_impl(gui_get_ctx(), Fixed_Y, fixed_y, f32, v); }
f32 gui_push_fixed_y(f32 v) { gui_stack_push_impl(gui_get_ctx(), Fixed_Y, fixed_y, f32, v); }
f32 gui_pop_fixed_y(void) { gui_stack_pop_impl(gui_get_ctx(), Fixed_Y, fixed_y); }

f32 gui_top_fixed_width(void) { gui_stack_top_impl(gui_get_ctx(), Fixed_Width, fixed_width); }
f32 gui_set_next_fixed_width(f32 v) { gui_stack_set_next_impl(gui_get_ctx(), Fixed_Width, fixed_width, f32, v); }
f32 gui_push_fixed_width(f32 v) { gui_stack_push_impl(gui_get_ctx(), Fixed_Width, fixed_width, f32, v); }
f32 gui_pop_fixed_width(void) { gui_stack_pop_impl(gui_get_ctx(), Fixed_Width, fixed_width); }

f32 gui_top_fixed_height(void) { gui_stack_top_impl(gui_get_ctx(), Fixed_Height, fixed_height); }
f32 gui_set_next_fixed_height(f32 v) { gui_stack_set_next_impl(gui_get_ctx(), Fixed_Height, fixed_height, f32, v); }
f32 gui_push_fixed_height(f32 v) { gui_stack_push_impl(gui_get_ctx(), Fixed_Height, fixed_height, f32, v); }
f32 gui_pop_fixed_height(void) { gui_stack_pop_impl(gui_get_ctx(), Fixed_Height, fixed_height); }

Gui_Size gui_top_pref_width(void) { gui_stack_top_impl(gui_get_ctx(), Pref_Width, pref_width); }
Gui_Size gui_set_next_pref_width(Gui_Size v) { gui_stack_set_next_impl(gui_get_ctx(), Pref_Width, pref_width, Gui_Size, v); }
Gui_Size gui_push_pref_width(Gui_Size v) { gui_stack_push_impl(gui_get_ctx(), Pref_Width, pref_width, Gui_Size, v); }
Gui_Size gui_pop_pref_width(void) { gui_stack_pop_impl(gui_get_ctx(), Pref_Width, pref_width); }

Gui_Size gui_top_pref_height(void) { gui_stack_top_impl(gui_get_ctx(), Pref_Height, pref_height); }
Gui_Size gui_set_next_pref_height(Gui_Size v) { gui_stack_set_next_impl(gui_get_ctx(), Pref_Height, pref_height, Gui_Size, v); }
Gui_Size gui_push_pref_height(Gui_Size v) { gui_stack_push_impl(gui_get_ctx(), Pref_Height, pref_height, Gui_Size, v); }
Gui_Size gui_pop_pref_height(void) { gui_stack_pop_impl(gui_get_ctx(), Pref_Height, pref_height); }

v4 gui_top_bg_color(void) { gui_stack_top_impl(gui_get_ctx(), Bg_Color, bg_color); }
v4 gui_set_next_bg_color(v4 v) { gui_stack_set_next_impl(gui_get_ctx(), Bg_Color, bg_color, v4, v); }
v4 gui_push_bg_color(v4 v) { gui_stack_push_impl(gui_get_ctx(), Bg_Color, bg_color, v4, v); }
v4 gui_pop_bg_color(void) { gui_stack_pop_impl(gui_get_ctx(), Bg_Color, bg_color); }

v4 gui_top_text_color(void) { gui_stack_top_impl(gui_get_ctx(), Text_Color, text_color); }
v4 gui_set_next_text_color(v4 v) { gui_stack_set_next_impl(gui_get_ctx(), Text_Color, text_color, v4, v); }
v4 gui_push_text_color(v4 v) { gui_stack_push_impl(gui_get_ctx(), Text_Color, text_color, v4, v); }
v4 gui_pop_text_color(void) { gui_stack_pop_impl(gui_get_ctx(), Text_Color, text_color); }

Gui_Axis gui_top_child_layout_axis(void) { gui_stack_top_impl(gui_get_ctx(), Child_Layout_Axis, child_layout_axis); }
Gui_Axis gui_set_next_child_layout_axis(Gui_Axis v) { gui_stack_set_next_impl(gui_get_ctx(), Child_Layout_Axis, child_layout_axis, Gui_Axis, v); }
Gui_Axis gui_push_child_layout_axis(Gui_Axis v) { gui_stack_push_impl(gui_get_ctx(), Child_Layout_Axis, child_layout_axis, Gui_Axis, v); }
Gui_Axis gui_pop_child_layout_axis(void) { gui_stack_pop_impl(gui_get_ctx(), Child_Layout_Axis, child_layout_axis); }

Gui_Size gui_push_pref_size(Gui_Axis axis, Gui_Size v) {
  Gui_Size result;
  switch(axis)
  {
    case GUI_AXIS_X: {result = gui_push_pref_width(v);}break;
    case GUI_AXIS_Y: {result = gui_push_pref_height(v);}break;
    default: break;
  }
  return result;
}

Gui_Size gui_pop_pref_size(Gui_Axis axis) {
  Gui_Size result;
  switch(axis)
  {
    case GUI_AXIS_X: {result = gui_pop_pref_width();}break;
    case GUI_AXIS_Y: {result = gui_pop_pref_height();}break;
    default: break;
  }
  return result;
}

Gui_Size gui_set_next_pref_size(Gui_Axis axis, Gui_Size v) {
  if (axis == GUI_AXIS_X){
    return gui_set_next_pref_width(v);
  }
  return gui_set_next_pref_height(v);
}


void gui_push_rect(rect r) {
  gui_push_fixed_x(r.p0.x);
  gui_push_fixed_y(r.p0.y);
  gui_push_fixed_width(r.dim.x);
  gui_push_fixed_height(r.dim.y);
}

void gui_pop_rect(void) {
  gui_pop_fixed_x();
  gui_pop_fixed_y();
  gui_pop_fixed_width();
  gui_pop_fixed_height();
}

void gui_set_next_rect(rect r) {
  gui_set_next_fixed_x(r.p0.x);
  gui_set_next_fixed_y(r.p0.y);
  gui_set_next_fixed_width(r.dim.x);
  gui_set_next_fixed_height(r.dim.y);
}

Gui_Panel_Itr gui_top_panel_itr(void) { gui_stack_top_impl(gui_get_ctx(), Panel_Itr, panel_itr); }
Gui_Panel_Itr gui_set_next_panel_itr(Gui_Panel_Itr itr) { gui_stack_set_next_impl(gui_get_ctx(), Panel_Itr, panel_itr, Gui_Panel_Itr, itr); }
Gui_Panel_Itr gui_push_panel_itr(Gui_Panel_Itr itr) { gui_stack_push_impl(gui_get_ctx(), Panel_Itr, panel_itr, Gui_Panel_Itr, itr); }
Gui_Panel_Itr gui_pop_panel_itr(void) { gui_stack_pop_impl(gui_get_ctx(), Panel_Itr, panel_itr); }
bool gui_empty_panel_itr(void) { gui_stack_empty_impl(gui_get_ctx(), Panel_Itr, panel_itr); }



