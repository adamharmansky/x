#ifndef TOOLKIT_H
#define TOOLKIT_H

#include "draw.h"

extern color_t toolkit_bg;
extern font_t  toolkit_font;

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

extern int
toolkit_init(int,int,char*);

extern void
toolkit_show_button(Button*);
extern void
toolkit_show_label(Label*);
extern void
toolkit_show_text_field(TextField*);
extern void
toolkit_show_combo_box(ComboBox* c);

extern void
toolkit_full_redraw();
extern void
toolkit_redraw();

extern int
toolkit_remove_widget(void* widget);

extern int toolkit_width, toolkit_height;
#endif
