#ifndef GUI_H__
#define GUI_H__

#include "base/base_inc.h"
#include "core/core_inc.h"

//rect font_util_calc_text_rect(Font_Info *font_info, char *text, v2 baseline_pos, f32 scale);
//f32 font_util_measure_text_width(Font_Info *font_info, char *text, f32 scale);
//f32 font_util_measure_text_height(Font_Info *font_info, char *text, f32 scale);
//void font_util_debug_draw_text(Font_Info *font_info, Arena *arena, v2 screen_dim, char *text, v2 baseline_pos, f32 scale);
//
typedef enum {
  GUI_AXIS_X,
  GUI_AXIS_Y,
} Gui_Axis;

typedef enum {
  GUI_SIZE_KIND_NULL,
  GUI_SIZE_KIND_PIXELS,
  GUI_SIZE_KIND_TEXT_CONTENT,
  GUI_SIZE_KIND_PARENT_PCT,
  GUI_SIZE_KIND_CHILDREN_SUM,
} Gui_Size_Kind;

typedef struct {
  Gui_Size_Kind kind;
  f32 value;
  f32 strictness;
} Gui_Size;

typedef enum {
  GB_FLAG_CLICKABLE             = (1 << 0),
  GB_FLAG_FOCUS_HOT             = (1 << 1),
  GB_FLAG_FOCUS_ACTIVE          = (1 << 2),
  GB_FLAG_FIXED_WIDTH           = (1 << 3),
  GB_FLAG_FIXED_HEIGHT          = (1 << 4),
  GB_FLAG_FIXED_X               = (1 << 5),
  GB_FLAG_FIXED_Y               = (1 << 6),
  GB_FLAG_DRAW_HOT_ANIMATION    = (1 << 7),
  GB_FLAG_DRAW_ACTIVE_ANIMATION = (1 << 8),
  GB_FLAG_DRAW_BACKGROUND       = (1 << 9),
  GB_FLAG_DRAW_TEXT             = (1 << 10),
  GB_FLAG_CLIP                  = (1 << 11),
  GB_FLAG_HOVERING              = (1 << 12),
  GB_FLAG_OVERFLOW_X            = (1 << 13),
  GB_FLAG_OVERFLOW_Y            = (1 << 14),
} Gui_Box_Flags;

typedef u64 Gui_Key;

typedef struct Gui_Box Gui_Box;
struct Gui_Box {
  // tree links
  Gui_Box* first;
  Gui_Box* last;
  Gui_Box* next;
  Gui_Box* prev;
  Gui_Box* parent;

  // hash links
  Gui_Box* hash_next;
  Gui_Box* hash_prev;

  // keying
  Gui_Key key;
  char str[128];

  // layouting state
  v2 fixed_pos;
  v2 fixed_size;
  Gui_Size pref_size[2];
  Gui_Axis child_layout_axis;
  rect r; // used for immediate mode calculation AND for final drawing

  Gui_Box_Flags flags;
  f32 transparency;
  u64 last_used_frame_idx;
  v4 color;
  //v4 roundness;

  f32 hot_t;
	f32 active_t;
	u32 child_count;
	v2 view_off;
	v2 view_off_target;
};

typedef struct Gui_Box_Hash_Slot Gui_Box_Hash_Slot;
struct Gui_Box_Hash_Slot {
  Gui_Box *hash_first;
  Gui_Box *hash_last;
};


typedef struct Gui_ParentNode Gui_ParentNode; struct Gui_ParentNode{Gui_ParentNode *next; Gui_Box *v; };
typedef struct Gui_PrefWidthNode Gui_PrefWidthNode; struct Gui_PrefWidthNode {Gui_PrefWidthNode *next; Gui_Size v;};
typedef struct Gui_PrefHeightNode Gui_PrefHeightNode; struct Gui_PrefHeightNode {Gui_PrefHeightNode *next; Gui_Size v;};
typedef struct Gui_FixedXNode Gui_FixedXNode; struct Gui_FixedXNode {Gui_FixedXNode *next; f32 v;};
typedef struct Gui_FixedYNode Gui_FixedYNode; struct Gui_FixedYNode {Gui_FixedYNode *next; f32 v;};
typedef struct Gui_FixedWidthNode Gui_FixedWidthNode; struct Gui_FixedWidthNode {Gui_FixedWidthNode *next; f32 v;};
typedef struct Gui_FixedHeightNode Gui_FixedHeightNode; struct Gui_FixedHeightNode {Gui_FixedHeightNode *next; f32 v;};
typedef struct Gui_BgColorNode Gui_BgColorNode; struct Gui_BgColorNode {Gui_BgColorNode *next; v4 v;};
typedef struct Gui_TextColorNode Gui_TextColorNode; struct Gui_TextColorNode {Gui_TextColorNode *next; v4 v;};
typedef struct Gui_ChildLayoutAxisNode Gui_ChildLayoutAxisNode; struct Gui_ChildLayoutAxisNode {Gui_ChildLayoutAxisNode*next; Gui_Axis v;};

typedef enum {
	GUI_SIGNAL_FLAG_LMB_PRESSED  = (1<<0),
	GUI_SIGNAL_FLAG_MMB_PRESSED  = (1<<1),
	GUI_SIGNAL_FLAG_RMB_PRESSED  = (1<<2),
	GUI_SIGNAL_FLAG_LMB_RELEASED = (1<<3),
	GUI_SIGNAL_FLAG_MMB_RELEASED = (1<<4),
	GUI_SIGNAL_FLAG_RMB_RELEASED = (1<<5),
	GUI_SIGNAL_FLAG_MOUSE_HOVER  = (1<<7),
	GUI_SIGNAL_FLAG_SCROLLED     = (1<<7),
	// TODO -- maybe we need one dragging for each mouse key
	GUI_SIGNAL_FLAG_DRAGGING     = (1<<8),
	// ...
} Gui_Signal_Flag;

typedef struct {
	Gui_Box *box;
	v2 mouse;
	v2 drag_delta;
	Gui_Signal_Flag flags;
} Gui_Signal;

Gui_Signal gui_get_signal_for_box(Gui_Box *box);


typedef struct {
  Arena *temp_arena;
  Arena *persistent_arena;
  Font_Info *font;

  v2 screen_dim;
  f64 dt;
  float font_scale;

  Gui_Key hot_box_key;
  Gui_Key active_box_keys[INPUT_MOUSE_COUNT];

  u64 frame_idx;

  Gui_Box* root;
  Gui_Box* box_freelist;

#define GUI_SLOT_COUNT 64
	Gui_Box_Hash_Slot *slots;
  u32 slot_count;
  
  // The Stacks!
	Gui_ParentNode parent_nil_stack_top;
	struct { Gui_ParentNode *top; Gui_Box * bottom_val; Gui_ParentNode *free; b32 auto_pop; } parent_stack;
	Gui_FixedXNode fixed_x_nil_stack_top;
	struct { Gui_FixedXNode *top; f32 bottom_val; Gui_FixedXNode *free; b32 auto_pop; } fixed_x_stack;
	Gui_FixedYNode fixed_y_nil_stack_top;
	struct { Gui_FixedYNode *top; f32 bottom_val; Gui_FixedYNode *free; b32 auto_pop; } fixed_y_stack;
	Gui_FixedWidthNode fixed_width_nil_stack_top;
	struct { Gui_FixedWidthNode *top; f32 bottom_val; Gui_FixedWidthNode *free; b32 auto_pop; } fixed_width_stack;
	Gui_FixedHeightNode fixed_height_nil_stack_top;
	struct { Gui_FixedHeightNode *top; f32 bottom_val; Gui_FixedHeightNode *free; b32 auto_pop; } fixed_height_stack;
	Gui_PrefWidthNode pref_width_nil_stack_top;
	struct { Gui_PrefWidthNode *top; Gui_Size bottom_val; Gui_PrefWidthNode *free; b32 auto_pop; } pref_width_stack;
	Gui_PrefHeightNode pref_height_nil_stack_top;
	struct { Gui_PrefHeightNode *top; Gui_Size bottom_val; Gui_PrefHeightNode *free; b32 auto_pop; } pref_height_stack;
	Gui_BgColorNode bg_color_nil_stack_top;
	struct { Gui_BgColorNode *top; v4 bottom_val; Gui_BgColorNode *free; b32 auto_pop; } bg_color_stack;
	Gui_TextColorNode text_color_nil_stack_top;
	struct { Gui_TextColorNode *top; v4 bottom_val; Gui_TextColorNode *free; b32 auto_pop; } text_color_stack;
	Gui_ChildLayoutAxisNode child_layout_axis_nil_stack_top;
	struct { Gui_ChildLayoutAxisNode *top; Gui_Axis bottom_val; Gui_ChildLayoutAxisNode *free; b32 auto_pop; } child_layout_axis_stack;

} Gui_Context;



void gui_context_init(Arena *temp_arena, Font_Info *font);
Arena* gui_get_build_arena();
void gui_frame_begin(v2 screen_dim, f64 dt);
void gui_frame_end();
void gui_render_hierarchy(Gui_Box *box);

Gui_Key gui_key_zero(void);
Gui_Key gui_key_from_str(char *s);
b32 gui_key_match(Gui_Key a, Gui_Key b);

Gui_Context* gui_get_ui_state();
Gui_Box *gui_box_nil_id();
b32 gui_box_is_nil(Gui_Box *box);
Gui_Box *gui_box_make(Gui_Box_Flags flags, char *str);
Gui_Box *gui_box_lookup_from_key(Gui_Box_Flags flags, Gui_Key key);
Gui_Box *gui_box_build_from_str(Gui_Box_Flags flags, char *str);
Gui_Box *gui_box_build_from_key(Gui_Box_Flags flags, Gui_Key key);

void gui_autopop_all_stacks();
void gui_init_stacks();

Gui_Box *gui_push_parent(Gui_Box *box);
Gui_Box *gui_set_next_parent(Gui_Box *box);
Gui_Box *gui_pop_parent(void);
Gui_Box *gui_top_parent(void);

f32 gui_push_fixed_x(f32 v);
f32 gui_set_next_fixed_x(f32 v);
f32 gui_pop_fixed_x(void);
f32 gui_top_fixed_x(void);

f32 gui_push_fixed_y(f32 v);
f32 gui_set_next_fixed_y(f32 v);
f32 gui_pop_fixed_y(void);
f32 gui_top_fixed_y(void);

f32 gui_push_fixed_width(f32 v);
f32 gui_set_next_fixed_width(f32 v);
f32 gui_pop_fixed_width(void);
f32 gui_top_fixed_width(void);

f32 gui_push_fixed_height(f32 v);
f32 gui_set_next_fixed_height(f32 v);
f32 gui_pop_fixed_height(void);
f32 gui_top_fixed_height(void);

Gui_Size gui_push_pref_width(Gui_Size v);
Gui_Size gui_set_next_pref_width(Gui_Size v);
Gui_Size gui_pop_pref_width(void);
Gui_Size gui_top_pref_width(void);

Gui_Size gui_push_pref_height(Gui_Size v);
Gui_Size gui_set_next_pref_height(Gui_Size v);
Gui_Size gui_pop_pref_height(void);
Gui_Size gui_top_pref_height(void);

v4 gui_top_bg_color(void);
v4 gui_set_next_bg_color(v4 v);
v4 gui_push_bg_color(v4 v);
v4 gui_pop_bg_color(void);

v4 gui_top_text_color(void);
v4 gui_set_next_text_color(v4 v);
v4 gui_push_text_color(v4 v);
v4 gui_pop_text_color(void);

void gui_layout_root(Gui_Box *root, Gui_Axis axis);
Gui_Axis gui_top_child_layout_axis(void);
Gui_Axis gui_set_next_child_layout_axis(Gui_Axis v);
Gui_Axis gui_push_child_layout_axis(Gui_Axis v);
Gui_Axis gui_pop_child_layout_axis(void);

// pushes fixed widths heights (TODO -- i should probably add all the lower level stack functions in future)
void gui_push_rect(rect r);
void gui_set_next_rect(rect r);
void gui_pop_rect(void);

Gui_Size gui_push_pref_size(Gui_Axis axis, Gui_Size v);
Gui_Size gui_set_next_pref_size(Gui_Axis axis, Gui_Size v);
Gui_Size gui_pop_pref_size(Gui_Axis axis);

// widgets
Gui_Signal gui_button(char *str);
Gui_Signal gui_pane(char *str);
Gui_Signal gui_spacer(Gui_Size size);

#endif
