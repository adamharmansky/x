#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "toolkit.h"

#define SCROLL_AMOUNT 20

static int _toolkit_default_width, _toolkit_default_height;
static char* _toolkit_default_name;
static pthread_t _toolkit_thread;

static int pointer_x, pointer_y, pointer_down;

int toolkit_width, toolkit_height;

static char ________toolkit_ready = 0;
static char _toolkit_has_to_redraw = 0;

color_t toolkit_bg;
color_t toolkit_text_color;
color_t toolkit_disabled_text_color;
font_t  toolkit_font;
color_t toolkit_shade_dark;
color_t toolkit_shade_light;
color_t toolkit_outline;
color_t toolkit_outline_corner;
color_t toolkit_text_field_bg;
color_t toolkit_selected_bg;

static _toolkit_widget* widgets;
static int widget_count = 0;

static void
refresh_text_field(TextField* t)
{
	int x;
	int y;
	int i;
	int x_limit;

	t->_text_length = draw_string_width(toolkit_font, t->text);

	if(t->scrollable) {
		x = 3;
		y = 3 + toolkit_font.xftfont->ascent;
		x_limit = t->w - 3 - t->scrollable*16;

		for(i = 0; t->text[i]; i++) {
			if(t->text[i] == '\n' || x >= x_limit) {
				x = 3;
				y += toolkit_font.xftfont->height;
			} else if(t->text[i] == '\t') {
				x += 24;
			} else {
				x += draw_char_width(toolkit_font, t->text[i]);
			}
		}
		t->max_scroll = y-t->h/2;
	}
	if(t->scroll < 0) t->scroll = 0;
	if(t->scroll > t->max_scroll) t->scroll = t->max_scroll;
}

static void
draw_button(Button b)
{
	int i;
	int xx, yy;

	if(b.toggle) {
		draw_rectangle(b.x, b.y, b.w, b.h, toolkit_bg);
		xx = b.x + 9;
		yy = b.y + b.h/2;
		draw_text(b.x + 18, b.y + b.h/2 + toolkit_font.xftfont->ascent/2-2, toolkit_font, b.text, toolkit_text_color);
		draw_rectangle(xx-6,yy-6,13,13,toolkit_outline);
		draw_rectangle(xx-5,yy-5,11,11,toolkit_text_field_bg);
		if(b.pressed) {
			for(i = 0; i < 6; i++) {
				draw_rectangle(xx-i,yy-i,1,1,toolkit_outline);
				draw_rectangle(xx+i,yy-i,1,1,toolkit_outline);
				draw_rectangle(xx-i,yy+i,1,1,toolkit_outline);
				draw_rectangle(xx+i,yy+i,1,1,toolkit_outline);
			}
		}
	} else {
		draw_rectangle(b.x-1, b.y-1, b.w+2, b.h+2, toolkit_outline);

		draw_rectangle(b.x-1  , b.y-1,   1, 1, toolkit_outline_corner);
		draw_rectangle(b.x+b.w, b.y-1,   1, 1, toolkit_outline_corner);
		draw_rectangle(b.x-1  , b.y+b.h, 1, 1, toolkit_outline_corner);
		draw_rectangle(b.x+b.w, b.y+b.h, 1, 1, toolkit_outline_corner);
		if(b.pressed) {
			draw_rectangle(b.x, b.y, b.w, b.h, toolkit_shade_dark);
			draw_rectangle(b.x+2, b.y+2, b.w-2, b.h-2, toolkit_shade_light);
			draw_rectangle(b.x+2, b.y+2, b.w-3, b.h-3, toolkit_bg);
		} else {
			draw_rectangle(b.x, b.y, b.w, b.h, toolkit_shade_light);
			draw_rectangle(b.x+1, b.y+1, b.w-1, b.h-1, toolkit_shade_dark);
			draw_rectangle(b.x+1, b.y+1, b.w-3, b.h-3, toolkit_bg);
		}
		draw_text(b.x+b.w/2-b._text_length/2 + b.pressed, b.y + b.h/2 + toolkit_font.xftfont->ascent/2-3 + b.pressed, toolkit_font, b.text, toolkit_text_color);
	}
	draw_flush(b.x-1, b.y-1, b.w+2, b.h+2);
}

static void
draw_label(Label l)
{
	int x = 3, y = toolkit_font.xftfont->ascent;
	int i;
	draw_rectangle(l.x, l.y, l.w, l.h, toolkit_bg);
	//draw_text(l.x+l.w/2-l._text_length/2, l.y + l.h/2 + toolkit_font.xftfont->ascent/2-3, toolkit_font, l.text, toolkit_text_color);
	for(i = 0; l.text[i]; i++) {
		if(x >= l.w - 5) {
			x = 3;
			y += toolkit_font.xftfont->height;
			if(y>=l.h) break;
		}
		if(l.text[i] == '\n') {
			x = 3;
			y += toolkit_font.xftfont->height;
			if(y>=l.h) break;
		} else if(l.text[i] == '\t') {
			x += 24;
		} else {
			draw_char(x+ l.x, y + l.y, toolkit_font, l.text[i], toolkit_text_color);
			x += draw_char_width(toolkit_font, l.text[i]);
		}
	}
	draw_flush(l.x, l.y, l.w, l.h);
}

static void
draw_text_field(TextField b)
{
	int x;
	int y;
	int i;
	char* text;
	int x_limit;
	int y_limit_low;
	int y_limit_high;
	int scrollbar_y;
	int s1 = b.w - b.scrollable*16;
	color_t color = toolkit_text_color;
	XRectangle recs[] = {
		(XRectangle){.x = b.x, .y = b.y, .width = b.w, .height = b.h}
	};
	XRectangle recs_reset[] = {
		(XRectangle){.x = 0, .y = 0, .width = ~0, .height = ~0}
	};

	/* draw the frame and the background */
	draw_rectangle(b.x-1, b.y-1, b.w+2, 1, toolkit_outline);
	draw_rectangle(b.x-1, b.y-1, 1, b.h+2, toolkit_outline);
	draw_rectangle(b.x+b.w, b.y-1, 1, b.h+2, toolkit_outline);
	draw_rectangle(b.x-1, b.y+b.h, b.w+2, 1, toolkit_outline);

	draw_rectangle(b.x-1  , b.y-1,   1, 1, toolkit_outline_corner);
	draw_rectangle(b.x+b.w, b.y-1,   1, 1, toolkit_outline_corner);
	draw_rectangle(b.x-1  , b.y+b.h, 1, 1, toolkit_outline_corner);
	draw_rectangle(b.x+b.w, b.y+b.h, 1, 1, toolkit_outline_corner);
	//draw_rectangle(b.x, b.y, b.w - b.scrollable*16, b.h, toolkit_text_field_bg);

	x = 3;
	y = toolkit_font.xftfont->ascent - b.scroll;
	x_limit = b.w - 3 - b.scrollable*16 - draw_char_width(toolkit_font, 'n');
	y_limit_low = -toolkit_font.xftfont->ascent;
	y_limit_high = b.h+toolkit_font.xftfont->descent;

	/* draw the text */

	if(b.input) {
		if(b.selected || *b.typed_text) text = b.typed_text;
		else{ text = b.text; color = toolkit_disabled_text_color;}
	} else text = b.text;

	XftDrawSetClipRectangles(_draw_draw, 0, 0, recs, 1);
	draw_rectangle(b.x, b.y, b.w, b.h, toolkit_text_field_bg);
	for(i = 0; text[i] && y<y_limit_low; i++) {
		if(x >= x_limit) {
			x = 3;
			y += toolkit_font.xftfont->height;
		}
		if(text[i] == '\n') {
			x = 3;
			y += toolkit_font.xftfont->height;
		} else if(text[i] == '\t') x += 24;
		else x += draw_char_width(toolkit_font, text[i]);
	}
	for(; text[i]; i++) {
		if(x >= x_limit) {
			x = 3;
			y += toolkit_font.xftfont->height;
			if(y>=y_limit_high) break;
		}
		if(text[i] == '\n') {
			x = 3;
			y += toolkit_font.xftfont->height;
			if(y>=y_limit_high) break;
		} else if(text[i] == '\t') {
			x += 24;
		} else {
			draw_char(x+ b.x, y + b.y, toolkit_font, text[i], color);
			x += draw_char_width(toolkit_font, text[i]);
		}
	}

	if(b.input && b.selected)draw_rectangle(x+b.x, y+b.y-toolkit_font.xftfont->ascent, 2,toolkit_font.xftfont->ascent,toolkit_text_color);

	/* draw the scrollbar */
	if(b.scrollable) {
		scrollbar_y = b.y+b.scroll*(b.h-50)/(b.max_scroll+1);
		draw_rectangle(b.x+b.w-16, b.y-1, 1, b.h+2,  toolkit_outline);
		draw_rectangle(b.x+b.w-15, b.y, 15, b.h,  toolkit_bg);
		draw_rectangle(b.x+b.w-15, scrollbar_y-1, 15, 52,  toolkit_outline);
		draw_rectangle(b.x+b.w-15, scrollbar_y, 15, 50,  toolkit_shade_light);
		draw_rectangle(b.x+b.w-14, scrollbar_y+1, 14, 49,  toolkit_shade_dark);
		draw_rectangle(b.x+b.w-14, scrollbar_y+1, 12, 47,  toolkit_bg);
	}
	XftDrawSetClipRectangles(_draw_draw, 0, 0, recs_reset, 1);
	draw_flush(b.x-1, b.y-1, b.w+2, b.h+2);
}

static void
draw_combo_box(ComboBox c)
{
	int i, count;
	int xx, yy;

	xx = c.x + c.w - c.h/2;
	yy = c.y + c.h/2;

	/* draw the button for the combo box */
	draw_rectangle(c.x-1, c.y-1, c.w+2, c.h+2, toolkit_outline);
	draw_rectangle(c.x-1  , c.y-1,   1, 1, toolkit_outline_corner);
	draw_rectangle(c.x+c.w, c.y-1,   1, 1, toolkit_outline_corner);
	draw_rectangle(c.x-1  , c.y+c.h, 1, 1, toolkit_outline_corner);
	draw_rectangle(c.x+c.w, c.y+c.h, 1, 1, toolkit_outline_corner);

	if(c.open) {
		draw_rectangle(c.x, c.y, c.w, c.h, toolkit_shade_dark);
		draw_rectangle(c.x+2, c.y+2, c.w-2, c.h-2, toolkit_shade_light);
		draw_rectangle(c.x+2, c.y+2, c.w-3, c.h-3, toolkit_bg);
	} else {
		draw_rectangle(c.x, c.y, c.w, c.h, toolkit_shade_light);
		draw_rectangle(c.x+1, c.y+1, c.w-1, c.h-1, toolkit_shade_dark);
		draw_rectangle(c.x+1, c.y+1, c.w-3, c.h-3, toolkit_bg);
	}
	if(c.selected_option) draw_text(c.x+7 + c.open, c.y + c.h/2 + toolkit_font.xftfont->ascent/2-3, toolkit_font, c.options[c.selected_option-1], toolkit_text_color);
	else draw_text(c.x+7 + c.open, c.y + c.h/2 + toolkit_font.xftfont->ascent/2-3, toolkit_font, c.text, toolkit_text_color);

	draw_rectangle(xx-5, yy-4, 11, 1, toolkit_text_color);
	draw_rectangle(xx-4, yy-3, 9,  1, toolkit_text_color);
	draw_rectangle(xx-3, yy-2, 7,  1, toolkit_text_color);
	draw_rectangle(xx-2, yy-1, 5,  1, toolkit_text_color);
	draw_rectangle(xx-1, yy  , 3,  1, toolkit_text_color);
	draw_rectangle(xx  , yy+1, 1,  1, toolkit_text_color);

	count = 0;
	/* draw the options */
	if(c.open) {
		for(i = 0; c.options[i]; i++) {
			draw_rectangle(c.x,   c.y + (c.expand_up ? -i*c.h : i*c.h)   , c.w,   c.h,   toolkit_text_field_bg);
			if(i == c.selected_option-1)
				draw_rectangle(c.x+1,   c.y + (c.expand_up ? -i*c.h : i*c.h)  +1 , c.w-2,   c.h-2,   toolkit_selected_bg);
			draw_text(c.x+3, c.y + c.h/2 + toolkit_font.xftfont->ascent/2-3 + (c.expand_up ? -i*c.h : i*c.h), toolkit_font, c.options[i], toolkit_text_color);
			count++;
		}
		draw_rectangle(c.x-1, c.y-1 , c.w+2, 1, toolkit_outline);
		draw_rectangle(c.x-1, c.y+ (c.h*(count)), c.w+2, 1, toolkit_outline);
		draw_rectangle(c.x-1, c.y-1 , 1, (c.h*(count))+2, toolkit_outline);
		draw_rectangle(c.x+c.w, c.y-1 , 1, (c.h*(count))+2, toolkit_outline);
	}
	draw_flush(c.x-1, c.y-1, c.w+2, c.h+2 + c.open * (c.h*(count-1)));
}

void
toolkit_redraw_widgets()
{
	int i;

	for(i = 0; i < widget_count; i++) {
		if     (widgets[i].type == BUTTON    ) draw_button    (*widgets[i].widget.b);
		else if(widgets[i].type == TEXT_FIELD) draw_text_field(*widgets[i].widget.t);
		else if(widgets[i].type == LABEL)      draw_label     (*widgets[i].widget.l);
	}
	/* draw combo boxes on top of fixed-size widgets */
	for(i = 0; i < widget_count; i++) {
		if(widgets[i].type == COMBO_BOX ) draw_combo_box (*widgets[i].widget.c);
	}
}

void
toolkit_full_redraw()
{
	int w, h;

	w = draw_width();
	h = draw_height();

	draw_rectangle(0,0,w,h,toolkit_bg);

	toolkit_redraw_widgets();
	draw_flush_all();
}

static void
call_users_function(void(*fun)())
{
	int i;

	if(!fun)return;
	(*fun)();
	/* the content of some widgets has probably changed, recalculate all the
	 * special parameters! */
	for(i = 0; i < widget_count; i++) {
		if(widgets[i].type == TEXT_FIELD) {
			refresh_text_field(widgets[i].widget.t);
		}
	}
	toolkit_redraw_widgets();
}

static void*
_toolkit_thread_func(void* args)
{
	XEvent e;
	int i, j;
	char s[64];
	KeySym k;
	int l;
	char old;
	int grasp = 0;
	TextField* grasped_window;
	ComboBox* open_combo_box = NULL;

	draw_init(_toolkit_default_width, _toolkit_default_height, _toolkit_default_name,
		ExposureMask |
		PointerMotionMask |
		ButtonPressMask |
		ButtonReleaseMask |
		KeyPressMask |
		Button1MotionMask |
		KeyReleaseMask);
	toolkit_width  = _toolkit_default_width;
	toolkit_height = _toolkit_default_height;

	toolkit_bg                  = create_color(0xc0,0xc0,0xc0);
	toolkit_text_color          = create_color(0   ,0   ,0   );
	toolkit_disabled_text_color = create_color(128 ,128 ,128 );
	toolkit_shade_light         = create_color(255 ,255 ,255 );
	toolkit_shade_dark          = create_color(128 ,128 ,128 );
	toolkit_outline             = create_color(0   ,0   ,0   );
	toolkit_outline_corner      = create_color(128 ,128 ,128 );
	toolkit_text_field_bg       = create_color(255 ,255 ,255 );
	toolkit_selected_bg         = create_color(0x76,0x9e,0xc5);

	toolkit_font = load_font("Sans-serif:size=11");

	widgets = malloc(0);

	________toolkit_ready = 1;

	for(;;) {
		meh:
		if(XPending(_draw_disp)) XNextEvent(_draw_disp, &e);
		else if(_toolkit_has_to_redraw) {
			toolkit_full_redraw();
			_toolkit_has_to_redraw = 0;
		}
		else {
			usleep(30000);
			goto meh;
		}
		if(e.type == Expose) {
			if(e.xexpose.count == 0) {
				toolkit_full_redraw();
				/* toolkit_width = draw_width();  */
				/* toolkit_height = draw_height();*/
			}

		} else if(e.type == MotionNotify) {
			pointer_x = e.xmotion.x;
			pointer_y = e.xmotion.y;
			/* this part of code scrolls text fields when we're dragging the
			 * scroll handle thing */
			if(e.xmotion.state & Button1Mask) {
				for(i = 0; i < widget_count; i++) {
					if(widgets[i].type == TEXT_FIELD) {
						if(widgets[i].widget.t->scrollable && grasp && grasped_window == widgets[i].widget.t) {
								widgets[i].widget.t->scroll = (pointer_y - grasp - widgets[i].widget.t->y)*(widgets[i].widget.t->max_scroll+1)/(widgets[i].widget.t->h-50);
								widgets[i].widget.t->scroll = widgets[i].widget.t->scroll < 0 ? 0 : widgets[i].widget.t->scroll;
								widgets[i].widget.t->scroll = widgets[i].widget.t->scroll > widgets[i].widget.t->max_scroll ? widgets[i].widget.t->max_scroll : widgets[i].widget.t->scroll;
								draw_text_field(*widgets[i].widget.t);
						}
					}
				}
			}
			/* if there is a combo box open, see which option is selected */
			if(open_combo_box) {
				for(j = 0; open_combo_box->options[j]; j++) {
					if(pointer_x >= open_combo_box->x && pointer_y >= open_combo_box->y + j*open_combo_box->h && pointer_y <= open_combo_box->y + open_combo_box->h * (j+1) && pointer_x <= open_combo_box->x + open_combo_box->w) {
						open_combo_box->selected_option = j+1;
						draw_combo_box(*open_combo_box);
					}
				}
			}
		/* if a mouse button is pressed */
		} else if(e.type == ButtonPress) {
			pointer_down = 1;

			/* for every widget */
			for(i = 0;i < widget_count;i++) {

				/* if clicked, deselect all text fields
				 * if we click on the same field, it will
				 * get selected later in this cycle */
				if(widgets[i].type == TEXT_FIELD){
					if(widgets[i].widget.t->selected) {
						widgets[i].widget.t->selected = 0;
						draw_text_field(*widgets[i].widget.t);
					}
				/* the same for combo boxes, except that we
				 * don't want to select anything back, so just
				 * say we're done with this event*/
				} else if(widgets[i].type == COMBO_BOX) {
					if(widgets[i].widget.c->open) {
						widgets[i].widget.c->open = 0;
						open_combo_box = NULL;
						call_users_function(widgets[i].widget.c->on_select_option);
						toolkit_full_redraw();
						goto meh;
					}
				}


				/* if we click on a widget */
				if(pointer_x >= widgets[i].widget.b->x && pointer_x <= widgets[i].widget.b->x + widgets[i].widget.b->w && pointer_y >= widgets[i].widget.b->y && pointer_y <= widgets[i].widget.b->y+widgets[i].widget.b->h) {


					/* if we click on a button */
					if(widgets[i].type == BUTTON) {
						/* we need to see if we have changed it's state */
						old = widgets[i].widget.b->pressed;
						if(widgets[i].widget.b->toggle) widgets[i].widget.b->pressed = !widgets[i].widget.b->pressed;
						else widgets[i].widget.b->pressed = 1;
						/* it the button's state has changed form 0 to 1, call on_press */
						if(widgets[i].widget.b->pressed && !old) call_users_function(widgets[i].widget.b->on_press);
						/* if a toggle button is released */
						if(!widgets[i].widget.b->pressed && old) call_users_function(widgets[i].widget.b->on_release);
						/* draw the button because it's state has changed */
						draw_button(*widgets[i].widget.b);


					/* if we click on a text field */
					} else if (widgets[i].type == TEXT_FIELD) {
						if(widgets[i].widget.t->scrollable && e.xbutton.button == 1) {
							/* if we click on the scroll hanle thingy on the right */
							if(pointer_x >= widgets[i].widget.t->x+widgets[i].widget.t->w-15 && pointer_y >= widgets[i].widget.t->y+widgets[i].widget.t->scroll*(widgets[i].widget.t->h-50)/(widgets[i].widget.t->max_scroll+1) && pointer_x <= widgets[i].widget.t->x+widgets[i].widget.t->w && pointer_y <= widgets[i].widget.t->y+widgets[i].widget.t->scroll*(widgets[i].widget.t->h-50)/(widgets[i].widget.t->max_scroll+1)+50) {
								/* calculate the relative position of the scroll handle to the pinter */
								grasp = pointer_y -( widgets[i].widget.t->y+widgets[i].widget.t->scroll*(widgets[i].widget.t->h-50)/(widgets[i].widget.t->max_scroll+1));
								grasped_window = widgets[i].widget.t;
							}
						}

						/* select an input field if we click on it */
						if(widgets[i].widget.t->input && e.xbutton.button == 1) {
							widgets[i].widget.t->selected = 1;
							draw_text_field(*widgets[i].widget.t);
						}

						/* scroll a field with buttons 4 and 5 */
						if(widgets[i].widget.t->scrollable) {
							if(e.xbutton.button == 4 && widgets[i].widget.t->scroll >= SCROLL_AMOUNT) {
								widgets[i].widget.t->scroll-=SCROLL_AMOUNT;
								draw_text_field(*widgets[i].widget.t);
							} else if(e.xbutton.button == 5 && widgets[i].widget.t->scroll <= widgets[i].widget.t->max_scroll-SCROLL_AMOUNT) {
								widgets[i].widget.t->scroll+=SCROLL_AMOUNT;
								draw_text_field(*widgets[i].widget.t);
							}
						}
					} else if(widgets[i].type == COMBO_BOX) {
						/* selecting a combo box */
						if(e.xbutton.button == 1) {
							widgets[i].widget.c->open = 1;
							open_combo_box = widgets[i].widget.c;
						} else if(e.xbutton.button == 4 && widgets[i].widget.c->selected_option > 1) {
							widgets[i].widget.c->selected_option--;
							call_users_function(widgets[i].widget.c->on_select_option);
						} else if(e.xbutton.button == 5 && widgets[i].widget.c->options[widgets[i].widget.c->selected_option]) {
							widgets[i].widget.c->selected_option++;
							call_users_function(widgets[i].widget.c->on_select_option);
						}
						draw_combo_box(*widgets[i].widget.c);
					}
				}
			}
		} else if(e.type == ButtonRelease) {
			pointer_down = 0;
			grasp = 0;
			/* deselect every button */
			for(i = 0;i < widget_count;i++) {
				if(widgets[i].type == BUTTON) {
					if(!widgets[i].widget.b->toggle) {
						if(widgets[i].widget.b->pressed) {
							widgets[i].widget.b->pressed = 0;
							draw_button(*widgets[i].widget.b);
							call_users_function(widgets[i].widget.b->on_release);
						}
					}
				}
			}
		} else if(e.type == KeyPress) {
			/* typing into input fields */
			XLookupString(&e.xkey, s, 64, &k, NULL);
			for(i = 0; i < widget_count; i++) {
				if(widgets[i].type == TEXT_FIELD) {
					if(widgets[i].widget.t->input && widgets[i].widget.t->selected){
						if(!s[1] && *s) {
							l = strlen(widgets[i].widget.t->typed_text);
							if(k == XK_BackSpace) {
								if(l) {
									widgets[i].widget.t->typed_text = realloc(widgets[i].widget.t->typed_text, l);
									widgets[i].widget.t->typed_text[l-1] = 0;
								}
							} else {
								if(*s == 0x0d) *s = '\n';
								widgets[i].widget.t->typed_text = realloc(widgets[i].widget.t->typed_text, ++l+1);
								widgets[i].widget.t->typed_text[l] = 0;
								widgets[i].widget.t->typed_text[l-1] = *s;
							}
							call_users_function(widgets[i].widget.t->on_content_changed);
							draw_text_field(*widgets[i].widget.t);
						}
					}
				}
			}
		}
	}
}

/* starts a separate thread for all the widget logic */
int
toolkit_init(int w, int h, char* name) {
	_toolkit_default_width = w;
	_toolkit_default_height = h;
	_toolkit_default_name = name;
	pthread_create(&_toolkit_thread, NULL, _toolkit_thread_func, NULL);
	while(!________toolkit_ready) usleep(50000); 
	usleep(50000);
	return 0;
}

/* these funtions add elements to the widget list and initialize some fields */
void
toolkit_show_button(Button* b)
{
	b->pressed = 0;
	widgets = realloc(widgets, (++widget_count)*sizeof(_toolkit_widget));
	widgets[widget_count-1] = (_toolkit_widget) {
		.type = BUTTON,
		.widget.b = b
	};
	widgets[widget_count-1].widget.b->_text_length = draw_string_width(toolkit_font, b->text);
	toolkit_redraw();
}

void
toolkit_show_label(Label* l)
{
	widgets = realloc(widgets, (++widget_count)*sizeof(_toolkit_widget));
	widgets[widget_count-1] = (_toolkit_widget) {
		.type = LABEL,
		.widget.l = l
	};
	toolkit_redraw();
}

void
toolkit_show_text_field(TextField* b)
{
	b->selected = 0;
	b->scroll = 0;
	widgets = realloc(widgets, (++widget_count)*sizeof(_toolkit_widget));
	widgets[widget_count-1] = (_toolkit_widget) {
		.type = TEXT_FIELD,
		.widget.t = b
	};
	if(b->input) {
		b->typed_text = malloc(1);
		*b->typed_text = 0;
	}
	refresh_text_field(b);
	toolkit_redraw();
}

void
toolkit_show_combo_box(ComboBox* c)
{
	c->open = 0;
	c->selected_option = 0;
	widgets = realloc(widgets, (++widget_count)*sizeof(_toolkit_widget));
	widgets[widget_count-1] = (_toolkit_widget) {
		.type = COMBO_BOX,
		.widget.c = c
	};
	toolkit_redraw();
}

int
toolkit_remove_widget(void* widget)
{
	int i;
	for(i = 0; i < widget_count; i++) {
		if(widgets[i].widget.a == widget) {
			memmove(&widgets[i], &widgets[i+1], (--widget_count-i)*sizeof(_toolkit_widget));
			toolkit_redraw();
			return 0;
		}
	}
	return 1;
}

/* we need to redraw in the main thread, so this just issuses the other thread
 * to redraw (no, I can't use signals ) */
/* at least we don't have to redraw multiple times when nothing's had time to
 * change */
void
toolkit_redraw()
{
	_toolkit_has_to_redraw = 1;
}
