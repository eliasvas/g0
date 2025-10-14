#include "gui.h"
#include "core/core_inc.h"

static Gui_Context g_gui_ctx;

// Self-referential box used instead of nullptr to denote end/invalid
static Gui_Box g_nil_box = {
  .first = &g_nil_box,
  .last = &g_nil_box,
  .next = &g_nil_box,
  .prev = &g_nil_box,
  .parent = &g_nil_box,
};

void gui_context_init(Arena *temp_arena, Font_Info *font) {
  g_gui_ctx.font = font;
  g_gui_ctx.font_scale = 0.3;

  g_gui_ctx.temp_arena = temp_arena;
  g_gui_ctx.persistent_arena = arena_make(MB(256));

  g_gui_ctx.slot_count = GUI_SLOT_COUNT;
  g_gui_ctx.slots = arena_push_array(g_gui_ctx.persistent_arena, Gui_Box_Hash_Slot, g_gui_ctx.slot_count);
}

Gui_Context* gui_get_ui_state() {
  return &g_gui_ctx;
}

Arena *gui_get_build_arena() {
	return gui_get_ui_state()->temp_arena;
}
///////////////////////////////////
// Gui Keying mechanism
///////////////////////////////////

Gui_Key gui_key_make(u64 val){
	Gui_Key res = (Gui_Key){val};
	return res;
}

Gui_Key gui_key_zero(){
	return gui_key_make(0);
}

Gui_Key gui_key_from_str(char *s) {
	Gui_Key res = gui_key_zero();
	if (s && str_len(s)) {
		res = gui_key_make(djb2((u8*)s));
	}
	return res;
}

b32 gui_key_match(Gui_Key a, Gui_Key b) {
	return ((u64)a == (u64)b);
}

///////////////////////////////////
// Gui Box Stuff
///////////////////////////////////

Gui_Box *gui_box_nil_id() {
	return &g_nil_box;
}

b32 gui_box_is_nil(Gui_Box *box) {
	return (box == 0 || box == gui_box_nil_id());
}

Gui_Box* gui_box_lookup_from_key(Gui_Box_Flags flags, Gui_Key key) {
	Gui_Box *res = gui_box_nil_id();

	if (!gui_key_match(key, gui_key_zero())) {
		u64 slot = key % gui_get_ui_state()->slot_count;
		for (Gui_Box *box = gui_get_ui_state()->slots[slot].hash_first; !gui_box_is_nil(box); box = box->hash_next) {
			if (gui_key_match(box->key, key)) {
				res = box;
				break;
			}
		}
	}
	return res;
}

Gui_Box *gui_box_build_from_key(Gui_Box_Flags flags, Gui_Key key) {
  Gui_Context *state = gui_get_ui_state();
	Gui_Box *parent = gui_top_parent();

  // Look up to slot based cache to find the box
	Gui_Box *box = gui_box_lookup_from_key(flags, key);

	b32 box_first_time = gui_box_is_nil(box);
	b32 box_is_transient = gui_key_match(key,gui_key_zero());

  // If we find the box to have updated frame_idx, its a double creation of same idx..
  if(!box_first_time && box->last_used_frame_idx == gui_get_ui_state()->frame_idx) {
		box = gui_box_nil_id();
		key = gui_key_zero();
		box_first_time = 1;
		printf("Key [%lu] has been detected twice, which means some box's hash to same ID!\n", key);
	}

  // If box was created this frame, allocate it, try to reuse Gui_Box's or allocate a new one
  if (box_first_time) {
		box = box_is_transient ? 0 : gui_get_ui_state()->box_freelist;
		if (!gui_box_is_nil(box)) {
			sll_stack_pop(gui_get_ui_state()->box_freelist);
		}
		else {
			box = arena_push_array_nz(box_is_transient? gui_get_build_arena() : gui_get_ui_state()->persistent_arena, Gui_Box, 1);
		}
		M_ZERO_STRUCT(box);
	}

  // zero out per-frame data for box (will be recalculated)
	{
		box->first = box->last = box->next = box->prev = box->parent = gui_box_nil_id();
		box->child_count = 0;
		box->flags = 0;
		box->last_used_frame_idx = gui_get_ui_state()->frame_idx;
		M_ZERO_ARRAY(box->pref_size);
	}

  // hook into persistent table
	if (box_first_time && !box_is_transient) {
		u64 hash_slot = (u64)key % gui_get_ui_state()->slot_count;
		dll_insert_NPZ(&g_nil_box, gui_get_ui_state()->slots[hash_slot].hash_first, gui_get_ui_state()->slots[hash_slot].hash_last, gui_get_ui_state()->slots[hash_slot].hash_last, box, hash_next, hash_prev);
	}

  // hook into tree structure
	if (!gui_box_is_nil(parent)) {
		dll_push_back_NPZ(gui_box_nil_id(), parent->first, parent->last, box, next, prev);
		parent->child_count += 1;
		box->parent = parent;
	}

  // fill the box's info stuff
	{
		box->key = key;
		box->flags |= flags;
		box->child_layout_axis = gui_top_child_layout_axis();
		// We are doing all layouting here, we should probably just traverse the hierarchy like Ryan says
		if (state->fixed_x_stack.top != &state->fixed_x_nil_stack_top) {
			box->fixed_pos.raw[GUI_AXIS_X] = state->fixed_x_stack.top->v;
			box->flags |= GB_FLAG_FIXED_X;
		}
		if (state->fixed_y_stack.top != &state->fixed_y_nil_stack_top) {
			box->fixed_pos.raw[GUI_AXIS_Y] = state->fixed_y_stack.top->v;
			box->flags |= GB_FLAG_FIXED_Y;
		}
		// FIXED_WIDTH/HEIGHT have NO pref size (GUI_SIZEKIND_NULL) so their fixed_size will stay the same
		if (state->fixed_width_stack.top != &state->fixed_width_nil_stack_top) {
			box->fixed_size.raw[GUI_AXIS_X] = state->fixed_width_stack.top->v;
			box->flags |= GB_FLAG_FIXED_WIDTH;
		}else {
			box->pref_size[GUI_AXIS_X] = gui_top_pref_width();
		}
		if (state->fixed_height_stack.top != &state->fixed_height_nil_stack_top) {
			box->fixed_size.raw[GUI_AXIS_Y] = state->fixed_height_stack.top->v;
			box->flags |= GB_FLAG_FIXED_HEIGHT;
		}else {
			box->pref_size[GUI_AXIS_Y] = gui_top_pref_height();
		}
		box->color = gui_top_bg_color();
	}

	gui_autopop_all_stacks();

  return box;
}


Gui_Box *gui_box_build_from_str(Gui_Box_Flags flags, char *str) {
	Gui_Key key = gui_key_from_str(str);
	Gui_Box *box = gui_box_build_from_key(flags, key);
	if (str){
    // TODO: remove c standard library please
		strcpy(box->str, str);
	}
	return box;
}

Gui_Key gui_get_hot_box_key() {
	return gui_get_ui_state()->hot_box_key;
}

Gui_Key gui_get_active_box_key(Input_Mouse_Button b) {
	return gui_get_ui_state()->active_box_keys[b];
}


Gui_Signal gui_button(char *str) {
	Gui_Box *w = gui_box_build_from_str( GB_FLAG_CLICKABLE |
									GB_FLAG_DRAW_TEXT |
									GB_FLAG_DRAW_BACKGROUND |
									GB_FLAG_DRAW_HOT_ANIMATION |
									GB_FLAG_DRAW_ACTIVE_ANIMATION,
									str);
	Gui_Signal signal = gui_get_signal_for_box(w);
	//if (signal.box->flags & GB_FLAG_HOVERING) { w->flags |= GB_FLAG_DRAW_BORDER; }
	return signal;
}

Gui_Signal gui_get_signal_for_box(Gui_Box *box) {
	Gui_Signal signal = {0};
	signal.box = box;
  v2 mp = input_get_mouse_pos();

	rect r = box->r;
  /*
	// if a parent has FLAG_CLIP, we intersect its childrens rects to clip them
  for(Gui_Box *p = box->parent; !gui_box_is_nil(p); p = p->parent) {
		if (p->flags & GB_FLAG_CLIP) {
			r = gui_intersect_rects(box->r,p->r);
			break;
		}
	}
  */

	b32 mouse_inside_box = rect_isect_point(r, mp);

	// perform scrolling via scroll wheel if widget in focus
	//if (mouse_inside_box && (box->flags & GB_FLAG_SCROLL)) { signal.flags |= GUI_SIGNAL_FLAG_SCROLLED; }

	// FIXME -- What the FUck? ////////
	if (!(box->flags & GB_FLAG_CLICKABLE))return signal;
	///////////////////////////////////

	// if mouse inside box, the box is HOT
	if (mouse_inside_box && (box->flags & GB_FLAG_CLICKABLE)) {
		gui_get_ui_state()->hot_box_key = box->key;
		box->flags |= GB_FLAG_HOVERING;
	}
	// if mouse inside box AND mouse button pressed, box is ACTIVE, PRESS event
	for (each_enumv(Input_Mouse_Button, INPUT_MOUSE, mk)) {
		if (mouse_inside_box && input_mkey_pressed(mk)) {
			gui_get_ui_state()->active_box_keys[mk] = box->key;
			// TODO -- This is pretty crappy logic, fix someday
			signal.flags |= (GUI_SIGNAL_FLAG_LMB_PRESSED << mk);
			//gui_drag_set_current_mp();
		}
	}
	// if current box is active, set is as dragging
	for (each_enumv(Input_Mouse_Button, INPUT_MOUSE, mk)) {
		if (gui_key_match(gui_get_active_box_key(mk), box->key)) {
			signal.flags |= (GUI_SIGNAL_FLAG_DRAGGING);
		}
	}
	// if mouse inside box AND mouse button released and box was ACTIVE, reset hot/active RELEASE signal
	for (each_enumv(Input_Mouse_Button, INPUT_MOUSE, mk)) {
		if (mouse_inside_box && input_mkey_released(mk) && gui_key_match(gui_get_active_box_key(mk), box->key)) {
			gui_get_ui_state()->hot_box_key = gui_key_zero();
			gui_get_ui_state()->active_box_keys[mk]= gui_key_zero();
			signal.flags |= (GUI_SIGNAL_FLAG_LMB_RELEASED << mk);
		}
	}
	// if mouse outside box AND mouse button released and box was ACTIVE, reset hot/active
	for (each_enumv(Input_Mouse_Button, INPUT_MOUSE, mk)) {
		if (!mouse_inside_box && input_mkey_released(mk) && gui_key_match(gui_get_active_box_key(mk), box->key)) {
			gui_get_ui_state()->hot_box_key = gui_key_zero();
			gui_get_ui_state()->active_box_keys[mk] = gui_key_zero();
		}
	}
	return signal;
}

Gui_Signal gui_spacer(Gui_Size size) {
	Gui_Box *parent = gui_top_parent();
	gui_set_next_pref_size(parent->child_layout_axis, size);
	Gui_Box *w = gui_box_build_from_str(0, NULL);
	Gui_Signal signal = gui_get_signal_for_box(w);
	return signal;
}

///////////////////////////////////
// Gui Build
///////////////////////////////////

void gui_build_begin(void) {
	//Gui_Context *state = gui_get_ui_state();
	// We init all stacks here because they are STRICTLY per-frame data structures
	gui_init_stacks();


	// build top level's root guiBox
	gui_set_next_child_layout_axis(GUI_AXIS_Y);
	Gui_Box *root = gui_box_build_from_str(0, "ImRootPlsDontPutSameHashSomewhereElse");
	gui_push_parent(root);
  gui_get_ui_state()->root = root;

	// reset hot if box pruned
	{
		Gui_Key hot_key = gui_get_hot_box_key();
		Gui_Box *box = gui_box_lookup_from_key(0, hot_key);
		b32 box_not_found = gui_box_is_nil(box);
		if (box_not_found) {
			gui_get_ui_state()->hot_box_key = gui_key_zero();
		}
	}

	// reset active if box pruned
	b32 active_exists = false;
	for (each_enumv(Input_Mouse_Button, INPUT_MOUSE, mk)) {
		Gui_Key active_key = gui_get_active_box_key(mk);
		Gui_Box *box = gui_box_lookup_from_key(0, active_key);
		b32 box_not_found = gui_box_is_nil(box);
		if (box_not_found) {
			gui_get_ui_state()->active_box_keys[mk] = gui_key_zero();
		}else {
			active_exists = true;
		}
	}

	// reset hot if there is no active
	if (!active_exists) {
		gui_get_ui_state()->hot_box_key = gui_key_zero();
	}

}

void gui_build_end(void) {
	Gui_Context *state = gui_get_ui_state();
	gui_pop_parent();

	// prune unused boxes
	for (u32 hash_slot = 0; hash_slot < state->slot_count; hash_slot+=1) {
		for (Gui_Box *box = state->slots[hash_slot].hash_first; !gui_box_is_nil(box); box = box->hash_next){
			if (box->last_used_frame_idx < state->frame_idx) {
				dll_remove_NPZ(gui_box_nil_id(), state->slots[hash_slot].hash_first, state->slots[hash_slot].hash_last,box,hash_next,hash_prev);
				sll_stack_push(state->box_freelist, box);
			}
		}
	}

	// do layout pass
	gui_layout_root(state->root, GUI_AXIS_X);
	gui_layout_root(state->root, GUI_AXIS_Y);

	// print hierarchy if need-be
	// if (gui_input_mb_pressed(GUI_RMB)) {
	// 	print_gui_hierarchy();
	// }

	//gui_drag_set_current_mp();

	// do animations
	for (u32 hash_slot = 0; hash_slot < state->slot_count; hash_slot+=1) {
		for (Gui_Box *box = state->slots[hash_slot].hash_first; !gui_box_is_nil(box); box = box->hash_next){
			// TODO -- do some logarithmic curve here, this is not very responsive!
			f32 trans_rate = 10 * state->dt;

			b32 is_box_hot = gui_key_match(box->key,gui_get_hot_box_key());
			b32 is_box_active = gui_key_match(box->key,gui_get_active_box_key(0));

			box->hot_t += trans_rate * (is_box_hot - box->hot_t);
			box->active_t += trans_rate * (is_box_active - box->active_t);
		}
	}

	// render eveything
  gui_render_hierarchy(gui_get_ui_state()->root);

	// clear previous frame's arena + advance frame_index
	arena_clear(gui_get_build_arena());
	state->frame_idx += 1;
}


void gui_frame_begin(v2 screen_dim, f64 dt) {
  g_gui_ctx.screen_dim = screen_dim;
  g_gui_ctx.dt = dt;
  gui_build_begin();
}

// Do NOT call this multiple times because (!!!)
void gui_frame_end() {
  // TBA: Rendering will be done here, actually.
  gui_build_end();
}

///////////////////////////////////
// Gui Layouting 
///////////////////////////////////

void gui_layout_root(Gui_Box *root, Gui_Axis axis)  {
	for(Gui_Box *child = root->first; !gui_box_is_nil(child); child = child->next) {
    child->r.p0.raw[axis] = root->r.p0.raw[axis] + child->fixed_pos.raw[axis] - root->view_off.raw[axis];
    child->r.dim.raw[axis] = child->fixed_size.raw[axis];
	}

  // do the same for all nodes and their children in hierarchy
  for(Gui_Box *child = root->first; !gui_box_is_nil(child); child = child->next) {
    gui_layout_root(child, axis);
  }
}

///////////////////////////////////
// Gui Rendering
///////////////////////////////////


void gui_draw_rect(rect r, v4 c) {
  Gui_Context *state = gui_get_ui_state();
  R2D* text_rend = r2d_begin(state->temp_arena, &(R2D_Cam){ .offset = v2m(0,0), .origin = v2m(0,0), .zoom = 1.0, .rot_deg = 0.0, }, state->screen_dim);
  r2d_push_quad(text_rend, (R2D_Quad) {
      .dst_rect = *(R2D_Rect*)&r,
      .color = *(R2D_Color*)&c,
  });
  r2d_end(text_rend);
}

void gui_draw_text(rect r, v4 c, char *s) {
  Gui_Context *state = gui_get_ui_state();

  rect label_rect = font_util_calc_text_rect(g_gui_ctx.font, s, v2m(0,0), state->font_scale);
  rect fitted_rect = rect_try_fit_inside(label_rect, r);

  v2 top_left = v2m(fitted_rect.x, fitted_rect.y);
  v2 baseline = v2_sub(top_left, label_rect.p0);

  font_util_debug_draw_text(state->font, state->temp_arena, state->screen_dim, s, baseline, state->font_scale, false);
}


void gui_render_hierarchy(Gui_Box *box) {
	// Visualize hot_t, active_t values to plug to renderer
	if (box->flags & GB_FLAG_DRAW_ACTIVE_ANIMATION) {
		box->color.r += box->active_t/6.0f;
	}
	if (box->flags & GB_FLAG_DRAW_HOT_ANIMATION) {
		box->color.r += box->hot_t/6.0f;
	}

	// render current box
  // TODO: clipping
	if (box->flags & GB_FLAG_DRAW_BACKGROUND) {
    gui_draw_rect(box->r, box->color);
    gui_draw_text(box->r, box->color, box->str);
	}

	// iterate through hierarchy
	for(Gui_Box *child = box->first; !gui_box_is_nil(child); child = child->next) {
		gui_render_hierarchy(child);
	}
}

