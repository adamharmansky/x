#ifndef TOOLKIT_H
#define TOOLKIT_H

#include "../draw.h"

extern color_t toolkit_bg;
extern font_t  toolkit_font;

typedef struct {
	int x, y;
	int w, h;
} AnyWidget;

typedef struct {
	int x, y;
	int w, h;
	char* text;
	int _text_length;
} Label;

typedef struct {
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
