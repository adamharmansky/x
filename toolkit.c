#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "toolkit.h"

#define SCROLL_AMOUNT 20

static TWindow* _i_am_too_lazy_to_do_arguments_right;

int toolkit_running = 0;

const XRectangle RECTS_RESET[] = {
	(XRectangle){.x = 0, .y = 0, .width = ~0, .height = ~0}
};

static void recalculate_size();

static void
refresh_text_field(TWindow* tw,TextField* t)
{
	int x;
	int y;
	int i;
	int x_limit;
	char* text;

	t->_text_length = draw_string_width(tw->c,tw->font, t->text);

	
	if(t->scrollable) {
		text = (t->input && *t->typed_text) ? t->typed_text : t->text;
		x = 3;
		y = 3 + tw->font.xftfont->ascent;
		x_limit = t->w - 3 - t->scrollable*16;

		for(i = 0; text[i]; i++) {
			if(text[i] == '\n' || x >= x_limit) {
				x = 3;
				y += tw->font.xftfont->height;
			} else if(text[i] == '\t') {
				x += 24;
			} else {
				x += draw_char_width(tw->c,tw->font, text[i]);
			}
		}
		t->max_scroll = y-t->h;
	}
	if(t->scroll < 0) t->scroll = 0;
	if(t->scroll > t->max_scroll) t->scroll = t->max_scroll;
}

static void
draw_button(TWindow* tw,Button b)
{
	int i;
	int xx, yy;

	if(b.toggle) {
		draw_rectangle(tw->c,b.x, b.y, b.w, b.h, tw->bg);
		xx = b.x + 9;
		yy = b.y + b.h/2;
		draw_text(tw->c,b.x + 18, b.y + b.h/2 + tw->font.xftfont->ascent/2-2, tw->font, b.text, tw->text_color);
		draw_rectangle(tw->c,xx-6,yy-6,13,13,tw->outline);
		draw_rectangle(tw->c,xx-5,yy-5,11,11,tw->text_field_bg);
		if(b.pressed) {
			for(i = 0; i < 6; i++) {
				draw_rectangle(tw->c,xx-i,yy-i,1,1,tw->outline);
				draw_rectangle(tw->c,xx+i,yy-i,1,1,tw->outline);
				draw_rectangle(tw->c,xx-i,yy+i,1,1,tw->outline);
				draw_rectangle(tw->c,xx+i,yy+i,1,1,tw->outline);
			}
		}
	} else {
		draw_rectangle(tw->c,b.x-1, b.y-1, b.w+2, b.h+2, tw->outline);

		draw_rectangle(tw->c,b.x-1  , b.y-1,   1, 1, tw->outline_corner);
		draw_rectangle(tw->c,b.x+b.w, b.y-1,   1, 1, tw->outline_corner);
		draw_rectangle(tw->c,b.x-1  , b.y+b.h, 1, 1, tw->outline_corner);
		draw_rectangle(tw->c,b.x+b.w, b.y+b.h, 1, 1, tw->outline_corner);
		if(b.pressed) {
			draw_rectangle(tw->c,b.x, b.y, b.w, b.h, tw->shade_dark);
			draw_rectangle(tw->c,b.x+2, b.y+2, b.w-2, b.h-2, tw->shade_light);
			draw_rectangle(tw->c,b.x+2, b.y+2, b.w-3, b.h-3, tw->bg);
		} else {
			draw_rectangle(tw->c,b.x, b.y, b.w, b.h, tw->shade_light);
			draw_rectangle(tw->c,b.x+1, b.y+1, b.w-1, b.h-1, tw->shade_dark);
			draw_rectangle(tw->c,b.x+1, b.y+1, b.w-3, b.h-3, tw->bg);
		}
		draw_text(tw->c,b.x+b.w/2-b._text_length/2 + b.pressed, b.y + b.h/2 + tw->font.xftfont->ascent/2-3 + b.pressed, tw->font, b.text, tw->text_color);
	}
	draw_flush(tw->c,b.x-1, b.y-1, b.w+2, b.h+2);
}

static void
draw_label(TWindow* tw,Label l)
{
	int x = 3, y = tw->font.xftfont->ascent;
	int i;
	draw_rectangle(tw->c,l.x, l.y, l.w, l.h, tw->bg);
	for(i = 0; l.text[i]; i++) {
		if(x >= l.w - 5) {
			x = 3;
			y += tw->font.xftfont->height;
			if(y>=l.h) break;
		}
		if(l.text[i] == '\n') {
			x = 3;
			y += tw->font.xftfont->height;
			if(y>=l.h) break;
		} else if(l.text[i] == '\t') {
			x += 24;
		} else {
			draw_char(tw->c,x+ l.x, y + l.y, tw->font, l.text[i], tw->text_color);
			x += draw_char_width(tw->c,tw->font, l.text[i]);
		}
	}
	draw_flush(tw->c,l.x, l.y, l.w, l.h);
}

static void
draw_text_field(TWindow* tw,TextField b)
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
	color_t color = tw->text_color;
	XRectangle recs[] = {
		(XRectangle){.x = b.x, .y = b.y, .width = b.w, .height = b.h}
	};

	/* draw the frame and the background */
	draw_rectangle(tw->c,b.x-1, b.y-1, b.w+2, 1, tw->outline);
	draw_rectangle(tw->c,b.x-1, b.y-1, 1, b.h+2, tw->outline);
	draw_rectangle(tw->c,b.x+b.w, b.y-1, 1, b.h+2, tw->outline);
	draw_rectangle(tw->c,b.x-1, b.y+b.h, b.w+2, 1, tw->outline);

	draw_rectangle(tw->c,b.x-1  , b.y-1,   1, 1, tw->outline_corner);
	draw_rectangle(tw->c,b.x+b.w, b.y-1,   1, 1, tw->outline_corner);
	draw_rectangle(tw->c,b.x-1  , b.y+b.h, 1, 1, tw->outline_corner);
	draw_rectangle(tw->c,b.x+b.w, b.y+b.h, 1, 1, tw->outline_corner);
	//draw_rectangle(tw->c,b.x, b.y, b.w - b.scrollable*16, b.h, tw->text_field_bg);

	x = 3;
	y = tw->font.xftfont->ascent - b.scroll;
	x_limit = b.w - 3 - b.scrollable*16 - draw_char_width(tw->c,tw->font, 'n');
	y_limit_low = -tw->font.xftfont->ascent;
	y_limit_high = b.h+tw->font.xftfont->descent;

	/* draw the text */

	if(b.input) {
		if(b.selected || *b.typed_text) text = b.typed_text;
		else{ text = b.text; color = tw->disabled_text_color;}
	} else text = b.text;

	XftDrawSetClipRectangles(tw->c.draw, 0, 0, recs, 1);
	draw_rectangle(tw->c,b.x, b.y, b.w, b.h, tw->text_field_bg);
	for(i = 0; text[i] && y<y_limit_low; i++) {
		if(x >= x_limit) {
			x = 3;
			y += tw->font.xftfont->height;
		}
		if(text[i] == '\n') {
			x = 3;
			y += tw->font.xftfont->height;
		} else if(text[i] == '\t') x += 24;
		else x += draw_char_width(tw->c,tw->font, text[i]);
	}
	for(; text[i]; i++) {
		if(x >= x_limit) {
			x = 3;
			y += tw->font.xftfont->height;
			if(y>=y_limit_high) break;
		}
		if(text[i] == '\n') {
			x = 3;
			y += tw->font.xftfont->height;
			if(y>=y_limit_high) break;
		} else if(text[i] == '\t') {
			x += 24;
		} else {
			draw_char(tw->c,x+ b.x, y + b.y, tw->font, text[i], color);
			x += draw_char_width(tw->c,tw->font, text[i]);
		}
	}

	if(b.input && b.selected)draw_rectangle(tw->c,x+b.x, y+b.y-tw->font.xftfont->ascent, 2,tw->font.xftfont->ascent,tw->text_color);

	/* draw the scrollbar */
	if(b.scrollable) {
		scrollbar_y = b.y+b.scroll*(b.h-50)/(b.max_scroll+1);
		draw_rectangle(tw->c,b.x+b.w-16, b.y-1, 1, b.h+2,  tw->outline);
		draw_rectangle(tw->c,b.x+b.w-15, b.y, 15, b.h,  tw->bg);
		draw_rectangle(tw->c,b.x+b.w-15, scrollbar_y-1, 15, 52,  tw->outline);
		draw_rectangle(tw->c,b.x+b.w-15, scrollbar_y, 15, 50,  tw->shade_light);
		draw_rectangle(tw->c,b.x+b.w-14, scrollbar_y+1, 14, 49,  tw->shade_dark);
		draw_rectangle(tw->c,b.x+b.w-14, scrollbar_y+1, 12, 47,  tw->bg);
	}
	XftDrawSetClipRectangles(tw->c.draw, 0, 0, RECTS_RESET, 1);
	draw_flush(tw->c,b.x-1, b.y-1, b.w+2, b.h+2);
}

static void
draw_combo_box(TWindow* tw,ComboBox comb)
{
	int i, count;
	int xx, yy;

	xx = comb.x + comb.w - comb.h/2;
	yy = comb.y + comb.h/2;

	/* draw the button for the combo box */
	draw_rectangle(tw->c,comb.x-1, comb.y-1, comb.w+2, comb.h+2, tw->outline);
	draw_rectangle(tw->c,comb.x-1  , comb.y-1,   1, 1, tw->outline_corner);
	draw_rectangle(tw->c,comb.x+comb.w, comb.y-1,   1, 1, tw->outline_corner);
	draw_rectangle(tw->c,comb.x-1  , comb.y+comb.h, 1, 1, tw->outline_corner);
	draw_rectangle(tw->c,comb.x+comb.w, comb.y+tw->c.h, 1, 1, tw->outline_corner);

	if(comb.open) {
		draw_rectangle(tw->c,comb.x, comb.y, comb.w, comb.h, tw->shade_dark);
		draw_rectangle(tw->c,comb.x+2, comb.y+2, comb.w-2, comb.h-2, tw->shade_light);
		draw_rectangle(tw->c,comb.x+2, comb.y+2, comb.w-3, comb.h-3, tw->bg);
	} else {
		draw_rectangle(tw->c,comb.x, comb.y, comb.w, comb.h, tw->shade_light);
		draw_rectangle(tw->c,comb.x+1, comb.y+1, comb.w-1, comb.h-1, tw->shade_dark);
		draw_rectangle(tw->c,comb.x+1, comb.y+1, comb.w-3, comb.h-3, tw->bg);
	}
	if(comb.selected_option) draw_text(tw->c,comb.x+7 + comb.open, comb.y + comb.h/2 + tw->font.xftfont->ascent/2-3, tw->font, comb.options[comb.selected_option-1], tw->text_color);
	else draw_text(tw->c,comb.x+7 + comb.open, comb.y + comb.h/2 + tw->font.xftfont->ascent/2-3, tw->font, comb.text, tw->text_color);

	draw_rectangle(tw->c,xx-5, yy-4, 11, 1, tw->text_color);
	draw_rectangle(tw->c,xx-4, yy-3, 9,  1, tw->text_color);
	draw_rectangle(tw->c,xx-3, yy-2, 7,  1, tw->text_color);
	draw_rectangle(tw->c,xx-2, yy-1, 5,  1, tw->text_color);
	draw_rectangle(tw->c,xx-1, yy  , 3,  1, tw->text_color);
	draw_rectangle(tw->c,xx  , yy+1, 1,  1, tw->text_color);

	count = 0;
	/* draw the options */
	if(comb.open) {
		for(i = 0; comb.options[i]; i++) {
			draw_rectangle(tw->c,comb.x,   comb.y + (comb.expand_up ? -i*comb.h : i*comb.h)   , comb.w,   comb.h,   tw->text_field_bg);
			if(i == comb.selected_option-1)
				draw_rectangle(tw->c,comb.x+1,   comb.y + (comb.expand_up ? -i*comb.h : i*comb.h)  +1 , comb.w-2,   comb.h-2,   tw->selected_bg);
			draw_text(tw->c,comb.x+3, comb.y + comb.h/2 + tw->font.xftfont->ascent/2-3 + (comb.expand_up ? -i*comb.h : i*comb.h), tw->font, comb.options[i], tw->text_color);
			count++;
		}
		draw_rectangle(tw->c,comb.x-1, comb.y-1 , comb.w+2, 1, tw->outline);
		draw_rectangle(tw->c,comb.x-1, comb.y+ (comb.h*(count)), comb.w+2, 1, tw->outline);
		draw_rectangle(tw->c,comb.x-1, comb.y-1 , 1, (comb.h*(count))+2, tw->outline);
		draw_rectangle(tw->c,comb.x+comb.w, comb.y-1 , 1, (comb.h*(count))+2, tw->outline);
	}
	draw_flush(tw->c,comb.x-1, comb.y-1, comb.w+2, comb.h+2 + comb.open * (comb.h*(count-1)));
}

static void
redraw_widgets(TWindow* tw)
{
	int i;

	for(i = 0; i < tw->widget_count; i++) {
		if     (tw->widgets[i].type == BUTTON    ) draw_button    (tw, *tw->widgets[i].widget.b);
		else if(tw->widgets[i].type == TEXT_FIELD) draw_text_field(tw, *tw->widgets[i].widget.t);
		else if(tw->widgets[i].type == LABEL)      draw_label     (tw, *tw->widgets[i].widget.l);
	}
	/* draw combo boxes on top of fixed-size tw->widgets */
	for(i = 0; i < tw->widget_count; i++) {
		if(tw->widgets[i].type == COMBO_BOX ) draw_combo_box(tw, *tw->widgets[i].widget.c);
	}
}

static void
full_redraw(TWindow* tw)
{
	draw_rectangle(tw->c,0,0,tw->c.w,tw->c.h,tw->bg);

	redraw_widgets(tw);
	draw_flush_all(tw->c);
}

static void
call_users_function(TWindow* tw,void(*fun)())
{
	int i;

	if(!fun)return;
	(*fun)();
	recalculate_size(tw);
	/* the content of some tw->widgets has probably changed, recalculate all the
	 * special parameters! */
	for(i = 0; i < tw->widget_count; i++) {
		if(tw->widgets[i].type == TEXT_FIELD) {
			refresh_text_field(tw,tw->widgets[i].widget.t);
		}
	}
	redraw_widgets(tw);
}

static void
recalculate_size(TWindow* tw)
{
	int i;
	for(i = 0; i < tw->widget_count; i++) {
		tw->widgets[i].widget.a->x = tw->widgets[i].widget.a->position.abs.x + tw->widgets[i].widget.a->position.lin.x*tw->c.w;
		tw->widgets[i].widget.a->y = tw->widgets[i].widget.a->position.abs.y + tw->widgets[i].widget.a->position.lin.y*tw->c.h;
		tw->widgets[i].widget.a->w = tw->widgets[i].widget.a->size.abs.w     + tw->widgets[i].widget.a->    size.lin.w*tw->c.w;
		tw->widgets[i].widget.a->h = tw->widgets[i].widget.a->size.abs.h     + tw->widgets[i].widget.a->    size.lin.h*tw->c.h;
	}
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
	TWindow* tw = _i_am_too_lazy_to_do_arguments_right;

	tw->c = draw_init(tw->default_width, tw->default_height, tw->default_name,
		ExposureMask |
		PointerMotionMask |
		ButtonPressMask |
		ButtonReleaseMask |
		KeyPressMask |
		Button1MotionMask |
		KeyReleaseMask);
	Atom stop_atom = XInternAtom(tw->c.disp, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(tw->c.disp, tw->c.win, &stop_atom, 1);
	tw->width  = tw->default_width;
	tw->height = tw->default_height;

	tw->bg                  = create_color(tw->c,0xc0,0xc0,0xc0);
	tw->text_color          = create_color(tw->c,0   ,0   ,0   );
	tw->disabled_text_color = create_color(tw->c,128 ,128 ,128 );
	tw->shade_light         = create_color(tw->c,255 ,255 ,255 );
	tw->shade_dark          = create_color(tw->c,128 ,128 ,128 );
	tw->outline             = create_color(tw->c,0   ,0   ,0   );
	tw->outline_corner      = create_color(tw->c,128 ,128 ,128 );
	tw->text_field_bg       = create_color(tw->c,255 ,255 ,255 );
	tw->selected_bg         = create_color(tw->c,0x76,0x9e,0xc5);

	tw->font = load_font(tw->c,"Sans-serif:size=11");
	tw->widgets = malloc(tw->widget_count=0);

	toolkit_running++;
	tw->ready = 1;


	for(;;) {
		meh:
		if(XPending(tw->c.disp)) XNextEvent(tw->c.disp, &e);
		else if(tw->has_to_redraw) {
			full_redraw(tw);
			tw->has_to_redraw = 0;
		}
		else {
			usleep(30000);
			goto meh;
		}
		if(e.type == Expose) {
			if(e.xexpose.count == 0) {
				draw_resize(&tw->c,draw_width(tw->c), draw_height(tw->c));
				recalculate_size(tw);
				full_redraw(tw);
			}
		} else if(e.type == MotionNotify) {
			tw->pointer_x = e.xmotion.x;
			tw->pointer_y = e.xmotion.y;
			/* this part of code scrolls text fields when we're dragging the
			 * scroll handle thing */
			if(e.xmotion.state & Button1Mask) {
				for(i = 0; i < tw->widget_count; i++) {
					if(tw->widgets[i].type == TEXT_FIELD) {
						if(tw->widgets[i].widget.t->scrollable && grasp && grasped_window == tw->widgets[i].widget.t) {
								tw->widgets[i].widget.t->scroll = (tw->pointer_y - grasp - tw->widgets[i].widget.t->y)*(tw->widgets[i].widget.t->max_scroll+1)/(tw->widgets[i].widget.t->h-50);
								tw->widgets[i].widget.t->scroll = tw->widgets[i].widget.t->scroll < 0 ? 0 : tw->widgets[i].widget.t->scroll;
								tw->widgets[i].widget.t->scroll = tw->widgets[i].widget.t->scroll > tw->widgets[i].widget.t->max_scroll ? tw->widgets[i].widget.t->max_scroll : tw->widgets[i].widget.t->scroll;
								draw_text_field(tw,*tw->widgets[i].widget.t);
						}
					}
				}
			}
			/* if there is a combo box open, see which option is selected */
			if(open_combo_box) {
				for(j = 0; open_combo_box->options[j]; j++) {
					if(tw->pointer_x >= open_combo_box->x && tw->pointer_y >= open_combo_box->y + j*open_combo_box->h && tw->pointer_y <= open_combo_box->y + open_combo_box->h * (j+1) && tw->pointer_x <= open_combo_box->x + open_combo_box->w) {
						open_combo_box->selected_option = j+1;
						draw_combo_box(tw,*open_combo_box);
					}
				}
			}
		/* if a mouse button is pressed */
		} else if(e.type == ButtonPress) {
			tw->pointer_down = 1;

			/* for every widget */
			for(i = 0;i < tw->widget_count;i++) {

				/* if clicked, deselect all text fields
				 * if we click on the same field, it will
				 * get selected later in this cycle */
				if(tw->widgets[i].type == TEXT_FIELD){
					if(tw->widgets[i].widget.t->selected && e.xbutton.button == 1) {
						tw->widgets[i].widget.t->selected = 0;
						draw_text_field(tw,*tw->widgets[i].widget.t);
					}
				/* the same for combo boxes, except that we
				 * don't want to select anything back, so just
				 * say we're done with this event*/
				} else if(tw->widgets[i].type == COMBO_BOX) {
					if(tw->widgets[i].widget.c->open) {
						tw->widgets[i].widget.c->open = 0;
						open_combo_box = NULL;
						call_users_function(tw,tw->widgets[i].widget.c->on_select_option);
						full_redraw(tw);
						goto meh;
					}
				}


				/* if we click on a widget */
				if(tw->pointer_x >= tw->widgets[i].widget.b->x && tw->pointer_x <= tw->widgets[i].widget.b->x + tw->widgets[i].widget.b->w && tw->pointer_y >= tw->widgets[i].widget.b->y && tw->pointer_y <= tw->widgets[i].widget.b->y+tw->widgets[i].widget.b->h) {


					/* if we click on a button */
					if(tw->widgets[i].type == BUTTON) {
						/* we need to see if we have changed it's state */
						old = tw->widgets[i].widget.b->pressed;
						if(tw->widgets[i].widget.b->toggle) tw->widgets[i].widget.b->pressed = !tw->widgets[i].widget.b->pressed;
						else tw->widgets[i].widget.b->pressed = 1;
						/* it the button's state has changed form 0 to 1, call on_press */
						if(tw->widgets[i].widget.b->pressed && !old) call_users_function(tw,tw->widgets[i].widget.b->on_press);
						/* if a toggle button is released */
						if(!tw->widgets[i].widget.b->pressed && old) call_users_function(tw,tw->widgets[i].widget.b->on_release);
						/* draw the button because it's state has changed */
						draw_button(tw,*tw->widgets[i].widget.b);


					/* if we click on a text field */
					} else if (tw->widgets[i].type == TEXT_FIELD) {
						if(tw->widgets[i].widget.t->scrollable && e.xbutton.button == 1) {
							/* if we click on the scroll hanle thingy on the right */
							if(tw->pointer_x >= tw->widgets[i].widget.t->x+tw->widgets[i].widget.t->w-15 && tw->pointer_y >= tw->widgets[i].widget.t->y+tw->widgets[i].widget.t->scroll*(tw->widgets[i].widget.t->h-50)/(tw->widgets[i].widget.t->max_scroll+1) && tw->pointer_x <= tw->widgets[i].widget.t->x+tw->widgets[i].widget.t->w && tw->pointer_y <= tw->widgets[i].widget.t->y+tw->widgets[i].widget.t->scroll*(tw->widgets[i].widget.t->h-50)/(tw->widgets[i].widget.t->max_scroll+1)+50) {
								/* calculate the relative position of the scroll handle to the pinter */
								grasp = tw->pointer_y -( tw->widgets[i].widget.t->y+tw->widgets[i].widget.t->scroll*(tw->widgets[i].widget.t->h-50)/(tw->widgets[i].widget.t->max_scroll+1));
								grasped_window = tw->widgets[i].widget.t;
							}
						}

						/* select an input field if we click on it */
						if(tw->widgets[i].widget.t->input && e.xbutton.button == 1) {
							tw->widgets[i].widget.t->selected = 1;
							draw_text_field(tw,*tw->widgets[i].widget.t);
						}

						/* scroll a field with buttons 4 and 5 */
						if(tw->widgets[i].widget.t->scrollable) {
							if(e.xbutton.button == 4 && tw->widgets[i].widget.t->scroll >= SCROLL_AMOUNT) {
								tw->widgets[i].widget.t->scroll-=SCROLL_AMOUNT;
								draw_text_field(tw,*tw->widgets[i].widget.t);
							} else if(e.xbutton.button == 5 && tw->widgets[i].widget.t->scroll <= tw->widgets[i].widget.t->max_scroll-SCROLL_AMOUNT) {
								tw->widgets[i].widget.t->scroll+=SCROLL_AMOUNT;
								draw_text_field(tw,*tw->widgets[i].widget.t);
							}
						}
					} else if(tw->widgets[i].type == COMBO_BOX) {
						/* selecting a combo box */
						if(e.xbutton.button == 1) {
							tw->widgets[i].widget.c->open = 1;
							open_combo_box = tw->widgets[i].widget.c;
						} else if(e.xbutton.button == 4 && tw->widgets[i].widget.c->selected_option > 1) {
							tw->widgets[i].widget.c->selected_option--;
							call_users_function(tw,tw->widgets[i].widget.c->on_select_option);
						} else if(e.xbutton.button == 5 && tw->widgets[i].widget.c->options[tw->widgets[i].widget.c->selected_option]) {
							tw->widgets[i].widget.c->selected_option++;
							call_users_function(tw,tw->widgets[i].widget.c->on_select_option);
						}
						draw_combo_box(tw,*tw->widgets[i].widget.c);
					}
				}
			}
		} else if(e.type == ButtonRelease) {
			tw->pointer_down = 0;
			grasp = 0;
			/* deselect every button */
			for(i = 0;i < tw->widget_count;i++) {
				if(tw->widgets[i].type == BUTTON) {
					if(!tw->widgets[i].widget.b->toggle) {
						if(tw->widgets[i].widget.b->pressed) {
							tw->widgets[i].widget.b->pressed = 0;
							draw_button(tw,*tw->widgets[i].widget.b);
							call_users_function(tw,tw->widgets[i].widget.b->on_release);
						}
					}
				}
			}
		} else if(e.type == KeyPress) {
			/* typing into input fields */
			XLookupString(&e.xkey, s, 64, &k, NULL);
			for(i = 0; i < tw->widget_count; i++) {
				if(tw->widgets[i].type == TEXT_FIELD) {
					if(tw->widgets[i].widget.t->input && tw->widgets[i].widget.t->selected){
						if(!s[1] && *s) {
							l = strlen(tw->widgets[i].widget.t->typed_text);
							if(k == XK_BackSpace) {
								if(l) {
									tw->widgets[i].widget.t->typed_text = realloc(tw->widgets[i].widget.t->typed_text, l);
									tw->widgets[i].widget.t->typed_text[l-1] = 0;
								}
							} else {
								if(*s == 0x0d) *s = '\n';
								tw->widgets[i].widget.t->typed_text = realloc(tw->widgets[i].widget.t->typed_text, ++l+1);
								tw->widgets[i].widget.t->typed_text[l] = 0;
								tw->widgets[i].widget.t->typed_text[l-1] = *s;
							}
							refresh_text_field(tw, tw->widgets[i].widget.t);
							if(tw->widgets[i].widget.t->max_scroll > 0) {
								tw->widgets[i].widget.t->scroll = tw->widgets[i].widget.t->max_scroll;
							}
							call_users_function(tw,tw->widgets[i].widget.t->on_content_changed);
							draw_text_field(tw,*tw->widgets[i].widget.t);
						}
					}
				}
			}
		} else if(e.type == ClientMessage) {
			if(e.xclient.data.l[0] == stop_atom) break;
		}
	}
	XDestroyWindow(tw->c.disp, tw->c.win);
	XCloseDisplay(tw->c.disp);
	toolkit_running--;
}

/* starts a separate thread for all the widget logic */
TWindow*
toolkit_init(int w, int h, char* name) {
	TWindow* tw = malloc(sizeof(TWindow));
	_i_am_too_lazy_to_do_arguments_right = tw;
	tw->ready = 0;
	tw->default_width = w;
	tw->default_height = h;
	tw->default_name = name;
	pthread_create(&tw->thread, NULL, _toolkit_thread_func, NULL);
	while(!tw->ready) usleep(50000); 
	usleep(50000);
	return tw;
}

/* these funtions add elements to the widget list and initialize some fields */
void
toolkit_show_button(TWindow* tw, Button* b)
{
	b->pressed = 0;
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = BUTTON,
		.widget.b = b
	};
	tw->widgets[tw->widget_count-1].widget.b->_text_length = draw_string_width(tw->c,tw->font, b->text);
	recalculate_size(tw);
	redraw_widgets(tw);
}

void
toolkit_show_label(TWindow* tw, Label* l)
{
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = LABEL,
		.widget.l = l
	};
	recalculate_size(tw);
	redraw_widgets(tw);
}

void
toolkit_show_text_field(TWindow* tw, TextField* b)
{
	b->selected = 0;
	b->scroll = 0;
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = TEXT_FIELD,
		.widget.t = b
	};
	if(b->input) {
		b->typed_text = malloc(1);
		*b->typed_text = 0;
	}
	refresh_text_field(tw,b);
	recalculate_size(tw);
	redraw_widgets(tw);
}

void
toolkit_show_combo_box(TWindow* tw, ComboBox* c)
{
	c->open = 0;
	c->selected_option = 0;
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = COMBO_BOX,
		.widget.c = c
	};
	recalculate_size(tw);
	redraw_widgets(tw);
}

int
toolkit_remove_widget(TWindow* tw, void* widget)
{
	int i;
	for(i = 0; i < tw->widget_count; i++) {
		if(tw->widgets[i].widget.a == widget) {
			memmove(&tw->widgets[i], &tw->widgets[i+1], (--tw->widget_count-i)*sizeof(_toolkit_widget));
			toolkit_redraw(tw);
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
toolkit_redraw(TWindow* tw)
{
	tw->has_to_redraw = 1;
}
