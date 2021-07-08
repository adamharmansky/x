#ifndef TOOLKIT_H
#define TOOLKIT_H

#include "draw.h"
#include <pthread.h>

extern int toolkit_running;


typedef struct {
	/* the linear coefficient is multiplied by the width of the window
	 * and added to the absolute coefficient */
	struct {
		struct { int   x,y; } abs;
		struct { float x,y; } lin;
	} position;
	struct {
		struct { int   w,h; } abs;
		struct { float w,h; } lin;
	} size;

	/* DO NOT USE, these are recalculated when the windows recieves an Expose event: */
	int x, y;
	int w, h;
} AnyWidget;

typedef struct {
	/* the linear coefficient is multiplied by the width of the window
	 * and added to the absolute coefficient */
	struct {
		struct { int   x,y; } abs;
		struct { float x,y; } lin;
	} position;
	struct {
		struct { int   w,h; } abs;
		struct { float w,h; } lin;
	} size;

	/* DO NOT USE, these are recalculated when the windows recieves an Expose event: */
	int x, y;
	int w, h;
	char* text;
	int _text_length;
} Label;

typedef struct {
	/* the linear coefficient is multiplied by the width of the window
	 * and added to the absolute coefficient */
	struct {
		struct { int   x,y; } abs;
		struct { float x,y; } lin;
	} position;
	struct {
		struct { int   w,h; } abs;
		struct { float w,h; } lin;
	} size;

	/* DO NOT USE, these are recalculated when the windows recieves an Expose event: */
	int x, y;
	int w, h;
	char* text;
	int _text_length;
	char pressed;
	void (*on_press)();
	void (*on_release)();
	char toggle;
} Button;

typedef struct {
	/* the linear coefficient is multiplied by the width of the window
	 * and added to the absolute coefficient */
	struct {
		struct { int   x,y; } abs;
		struct { float x,y; } lin;
	} position;
	struct {
		struct { int   w,h; } abs;
		struct { float w,h; } lin;
	} size;

	/* DO NOT USE, these are recalculated when the windows recieves an Expose event: */
	int x, y;
	int w, h;
	char* text;
	int _text_length;
	char* typed_text;
	void (*on_content_changed)();
	char input;
	char selected;
	char scrollable;
	int scroll;
	int max_scroll;
	int cursor_position;
	int cursor_y;
} TextField;

typedef struct {
	/* the linear coefficient is multiplied by the width of the window
	 * and added to the absolute coefficient */
	struct {
		struct { int   x,y; } abs;
		struct { float x,y; } lin;
	} position;
	struct {
		struct { int   w,h; } abs;
		struct { float w,h; } lin;
	} size;

	/* DO NOT USE, these are recalculated when the windows recieves an Expose event: */
	int x, y;
	int w, h;
	char* text;
	int _text_length;
	void (*on_select_option)();
	char** options;
	int selected_option;
	char open;
	char expand_up;
} ComboBox;

/* do not set theese up yourself ↓ */
typedef struct {
	enum {
		BUTTON,
		TEXT_FIELD,
		COMBO_BOX,
		LABEL,
	} type;
	union {
		AnyWidget* a;
		Button* b;
		TextField* t;
		ComboBox* c;
		Label* l;
	} widget;
} _toolkit_widget;


typedef struct {
	int width, height;
	int default_width, default_height;
	char* default_name;
	pthread_t thread;
	draw_context c;
	int pointer_x, pointer_y, pointer_down;
	char ready;
	char has_to_redraw;
	font_t font;
	color_t bg;
	color_t text_color;
	color_t disabled_text_color;
	color_t shade_dark;
	color_t shade_light;
	color_t outline;
	color_t outline_corner;
	color_t text_field_bg;
	color_t selected_bg;
	_toolkit_widget* widgets;
	int widget_count;
} TWindow;

extern TWindow*
toolkit_init(int,int,char*);
extern void
toolkit_show_button(TWindow*, Button*);
extern void
toolkit_show_label(TWindow*,Label*);
extern void
toolkit_show_text_field(TWindow*,TextField*);
extern void
toolkit_show_combo_box(TWindow*,ComboBox* c);
extern void
toolkit_full_redraw(TWindow*);
extern void
toolkit_redraw(TWindow*);
extern int
toolkit_remove_widget(TWindow*,void*);

#endif
