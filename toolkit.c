#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>
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
		y = tw->font.xftfont->ascent;
		x_limit = t->w - 3 - t->scrollable*16 - draw_char_width(tw->c,tw->font, 'n');

		if(0 == t->cursor_position) {t->cursor_y = y; t->cursor_x = x;}
		for(i = 0; text[i]; i++) {
			if(text[i] == '\n' || x >= x_limit) {
				x = 3;
				y += tw->font.xftfont->height;
			} else if(text[i] == '\t') {
				x += 24;
			} else {
				x += draw_char_width(tw->c,tw->font, text[i]);
			}
			if(i+1 == t->cursor_position) {t->cursor_y = y; t->cursor_x = x;}
		}
		t->max_scroll = y-tw->font.xftfont->ascent;
	}
	if(t->scroll < 0) t->scroll = 0;
	if(t->scroll > t->max_scroll) t->scroll = t->max_scroll;
}

static void
text_field_find_click(TWindow* tw, TextField* t, int pointer_x, int pointer_y)
{
	int x;
	int y;
	int i;
	int x_limit;
	char* text;

	t->_text_length = draw_string_width(tw->c,tw->font, t->text);

	
	text = t->typed_text;
	x = 3;
	y = tw->font.xftfont->ascent - t->scroll;
	x_limit = t->w - 3 - t->scrollable*16 - draw_char_width(tw->c,tw->font, 'n');

	if(0 == t->cursor_position) t->cursor_y = y;
	for(i = 0; text[i] && y < pointer_y-t->y; i++) {
		if(text[i] == '\n' || x >= x_limit) {
			x = 3;
			y += tw->font.xftfont->height;
		} else if(text[i] == '\t') {
			x += 24;
		} else {
			x += draw_char_width(tw->c,tw->font, text[i]);
		}
	}
	for(; text[i] && x < pointer_x - t->x; i++) {
		if(text[i] == '\n' || x >= x_limit || !text[i+1]) {
			t->cursor_position = !text[i+1]?i+1:i;
			t->cursor_y = y; t->cursor_x = x;
			return;
		} else if(text[i] == '\t') {
			x += 24;
		} else {
			x += draw_char_width(tw->c,tw->font, text[i]);
		}
	}
	t->cursor_position = text[i]?i-1<0?0:i-1:i;
	t->cursor_y = y; t->cursor_x = x;
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
draw_progress_bar(TWindow* tw, ProgressBar p)
{
	draw_rectangle(tw->c, p.x-1,p.y-1,p.w+2,p.h+2,tw->outline);
	draw_rectangle(tw->c, p.x,p.y,p.w,p.h,tw->text_field_bg);
	draw_rectangle(tw->c, p.x,p.y,(float)p.w*p.progress,p.h,tw->selected_bg);
	draw_flush(tw->c, p.x-1,p.y-1,p.w+2,p.h+2);
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
	if(b.input && b.selected && 0 == b.cursor_position) draw_rectangle(tw->c,x+b.x, y+b.y-tw->font.xftfont->ascent, 2,tw->font.xftfont->ascent,tw->text_color);
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
		if(b.input && b.selected && i+1 == b.cursor_position) draw_rectangle(tw->c,x+b.x, y+b.y-tw->font.xftfont->ascent, 2,tw->font.xftfont->ascent,tw->text_color);
	}


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
draw_icon(TWindow* tw, Icon i)
{
	draw_image(tw->c, i.x, i.y, i.image);
	draw_flush(tw->c, i.x, i.y, i.w,i.h);
}

static void
redraw_widgets(TWindow* tw)
{
	int i;

	for(i = 0; i < tw->widget_count; i++) {                       
		if     (tw->widgets[i].type == BUTTON    )  draw_button          (tw, *tw->widgets[i].widget.b);
		else if(tw->widgets[i].type == TEXT_FIELD)  draw_text_field      (tw, *tw->widgets[i].widget.t);
		else if(tw->widgets[i].type == LABEL)       draw_label           (tw, *tw->widgets[i].widget.l);
		else if(tw->widgets[i].type == PROGRESS_BAR)draw_progress_bar    (tw, *tw->widgets[i].widget.p);
		else if(tw->widgets[i].type == ICON        )draw_icon            (tw, *tw->widgets[i].widget.i);
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
	TWindow* tw = _i_am_too_lazy_to_do_arguments_right;
	pthread_mutex_lock(&tw->mut);
	XEvent e;
	int i, j;
	char s[64];
	KeySym k;
	int l;
	char old;
	int grasp = 0;
	TextField* grasped_window;
	ComboBox* open_combo_box = NULL;
	struct pollfd fds;

	tw->c = draw_init(tw->default_width, tw->default_height, tw->default_name,
		ExposureMask |
		PointerMotionMask |
		ButtonPressMask |
		ButtonReleaseMask |
		KeyPressMask |
		Button1MotionMask |
		StructureNotifyMask |
		KeyReleaseMask);
	fds = (struct pollfd){.fd=ConnectionNumber(tw->c.disp), .events = POLLIN};
	Atom stop_atom = XInternAtom(tw->c.disp, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(tw->c.disp, tw->c.win, &stop_atom, 1);
	tw->width  = tw->default_width;
	tw->height = tw->default_height;

	tw->bg                  = create_color(tw->c,0xc0,0xc0,0xc0);
	XSetWindowBackground(tw->c.disp, tw->c.win, tw->bg.xftcolor.pixel);
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
	pthread_mutex_unlock(&tw->mut);

	for(;;) {
		if(!poll(&fds,1,5)) {
			if(tw->has_to_redraw) {
				pthread_mutex_lock(&tw->mut);
				full_redraw(tw);
				XFlush(tw->c.disp);
				tw->has_to_redraw =0;
				pthread_mutex_unlock(&tw->mut);
			}
			continue;
		}
		while(XPending(tw->c.disp)) {
			XNextEvent(tw->c.disp, &e);
			pthread_mutex_lock(&tw->mut);
			if(e.type == ConfigureNotify) {
				if(!e.xconfigure.send_event) {
					draw_flush_all(tw->c);
					draw_resize(&tw->c,draw_width(tw->c), draw_height(tw->c));
					recalculate_size(tw);
				}
			}else if(e.type == Expose){
				if(!e.xexpose.count) full_redraw(tw);
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
							continue;
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
								else {
									text_field_find_click(tw, tw->widgets[i].widget.t, tw->pointer_x, tw->pointer_y);
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
							/* for one-byte characters */
							if(!s[1] && *s) {
								l = strlen(tw->widgets[i].widget.t->typed_text);
								if(k == XK_BackSpace ) {
									if(tw->widgets[i].widget.t->cursor_position) {
										memmove(tw->widgets[i].widget.t->typed_text+tw->widgets[i].widget.t->cursor_position-1, tw->widgets[i].widget.t->typed_text+tw->widgets[i].widget.t->cursor_position, l-tw->widgets[i].widget.t->cursor_position);
										tw->widgets[i].widget.t->typed_text = realloc(tw->widgets[i].widget.t->typed_text, l);
										tw->widgets[i].widget.t->typed_text[l-1] = 0;
										tw->widgets[i].widget.t->cursor_position--;
									}
								} else {
									if(*s == 0x0d) *s = '\n';
									tw->widgets[i].widget.t->typed_text = realloc(tw->widgets[i].widget.t->typed_text, l+2);
									memmove(
										&tw->widgets[i].widget.t->typed_text[tw->widgets[i].widget.t->cursor_position+1],
										&tw->widgets[i].widget.t->typed_text[tw->widgets[i].widget.t->cursor_position],
										l-tw->widgets[i].widget.t->cursor_position+1);
									tw->widgets[i].widget.t->typed_text[tw->widgets[i].widget.t->cursor_position] = *s;
									tw->widgets[i].widget.t->cursor_position++;
								}
							} else if (k == XK_Left) {
								if(tw->widgets[i].widget.t->typed_text[tw->widgets[i].widget.t->cursor_position-1])
									tw->widgets[i].widget.t->cursor_position--;
							} else if (k == XK_Right) {
								if(tw->widgets[i].widget.t->typed_text[tw->widgets[i].widget.t->cursor_position])
									tw->widgets[i].widget.t->cursor_position++;
							} else if (k == XK_Up) {
									text_field_find_click(tw, tw->widgets[i].widget.t, tw->widgets[i].widget.t->cursor_x+tw->widgets[i].widget.t->x + draw_char_width(tw->c, tw->font, tw->widgets[i].widget.t->typed_text[tw->widgets[i].widget.t->cursor_position])/2, tw->widgets[i].widget.t->cursor_y+tw->widgets[i].widget.t->y-tw->font.xftfont->height-tw->widgets[i].widget.t->scroll);
							} else if (k == XK_Down) {
									text_field_find_click(tw, tw->widgets[i].widget.t, tw->widgets[i].widget.t->cursor_x+tw->widgets[i].widget.t->x + draw_char_width(tw->c, tw->font, tw->widgets[i].widget.t->typed_text[tw->widgets[i].widget.t->cursor_position])/2, tw->widgets[i].widget.t->cursor_y+tw->widgets[i].widget.t->y+tw->font.xftfont->height-tw->widgets[i].widget.t->scroll);
							}
							refresh_text_field(tw, tw->widgets[i].widget.t);
							if(tw->widgets[i].widget.t->scrollable) {
								if(tw->widgets[i].widget.t->cursor_y - tw->widgets[i].widget.t->scroll > tw->widgets[i].widget.t->h) {
									tw->widgets[i].widget.t->scroll = tw->widgets[i].widget.t->cursor_y - tw->widgets[i].widget.t->h+tw->font.xftfont->descent;
								} else if(tw->widgets[i].widget.t->cursor_y < tw->widgets[i].widget.t->scroll + tw->font.xftfont->ascent) {
									tw->widgets[i].widget.t->scroll = tw->widgets[i].widget.t->cursor_y - tw->font.xftfont->ascent;
								}
							}
							call_users_function(tw,tw->widgets[i].widget.t->on_content_changed);
							draw_text_field(tw,*tw->widgets[i].widget.t);
						}
					}
				}
			} else if(e.type == ClientMessage) {
				if(e.xclient.data.l[0] == stop_atom) goto goodbye;
			}
			pthread_mutex_unlock(&tw->mut);
		}
	}
	goodbye:
	pthread_mutex_unlock(&tw->mut);
	XDestroyWindow(tw->c.disp, tw->c.win);
	XCloseDisplay(tw->c.disp);
	toolkit_running--;
}

/* starts a separate thread for all the widget logic */
TWindow*
toolkit_init(int w, int h, char* name)
{
	TWindow* tw = malloc(sizeof(TWindow));
	tw->mut = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	_i_am_too_lazy_to_do_arguments_right = tw;
	tw->default_width = w;
	tw->default_height = h;
	tw->default_name = name;
	pthread_create(&tw->thread, NULL, _toolkit_thread_func, NULL);
	/* wait until the thread locks the mutex (i'm sorry) */
	usleep(1000);
	return tw;
}

/* these funtions add elements to the widget list and initialize some fields */
void
toolkit_show_button(TWindow* tw, Button* b)
{
	pthread_mutex_lock(&tw->mut);
	b->pressed = 0;
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = BUTTON,
		.widget.b = b
	};
	tw->widgets[tw->widget_count-1].widget.b->_text_length = draw_string_width(tw->c,tw->font, b->text);
	toolkit_redraw(tw);
	pthread_mutex_unlock(&tw->mut);
}

void
toolkit_show_label(TWindow* tw, Label* l)
{
	pthread_mutex_lock(&tw->mut);
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = LABEL,
		.widget.l = l
	};
	toolkit_redraw(tw);
	pthread_mutex_unlock(&tw->mut);
}

void
toolkit_show_text_field(TWindow* tw, TextField* b)
{
	pthread_mutex_lock(&tw->mut);
	b->selected = 0;
	b->scroll = 0;
	b->cursor_position = 0;
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
	toolkit_redraw(tw);
	pthread_mutex_unlock(&tw->mut);
}

void
toolkit_show_combo_box(TWindow* tw, ComboBox* c)
{
	pthread_mutex_lock(&tw->mut);
	c->open = 0;
	c->selected_option = 0;
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = COMBO_BOX,
		.widget.c = c
	};
	toolkit_redraw(tw);
	pthread_mutex_unlock(&tw->mut);
}

void
toolkit_show_progress_bar(TWindow* tw, ProgressBar* p)
{
	pthread_mutex_lock(&tw->mut);
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = PROGRESS_BAR,
		.widget.p = p
	};
	toolkit_redraw(tw);
	pthread_mutex_unlock(&tw->mut);
}

void
toolkit_show_icon(TWindow* tw, Icon* i)
{
	pthread_mutex_lock(&tw->mut);
	tw->widgets = realloc(tw->widgets, (++tw->widget_count)*sizeof(_toolkit_widget));
	tw->widgets[tw->widget_count-1] = (_toolkit_widget) {
		.type = ICON,
		.widget.i = i
	};
	i->image = create_image(tw->c, tw->bg, i->source);
	i->w = i->image.width;
	i->h = i->image.height;
	toolkit_redraw(tw);
	pthread_mutex_unlock(&tw->mut);
}

int
toolkit_remove_widget(TWindow* tw, void* widget)
{
	int i;

	pthread_mutex_lock(&tw->mut);
	for(i = 0; i < tw->widget_count; i++) {
		if(tw->widgets[i].widget.a == widget) {
			if(tw->widgets[i].type == ICON) {
				free_image(tw->widgets[i].widget.i->image);
			}
			memmove(&tw->widgets[i], &tw->widgets[i+1], (--tw->widget_count-i)*sizeof(_toolkit_widget));
			toolkit_redraw(tw);
			pthread_mutex_unlock(&tw->mut);
			return 0;
		}
	}
	pthread_mutex_unlock(&tw->mut);
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

void
update_progress_bar(TWindow* tw, ProgressBar* p, float x)
{
	pthread_mutex_lock(&tw->mut);
	p->progress = x;
	toolkit_redraw(tw);
	pthread_mutex_unlock(&tw->mut);
}
