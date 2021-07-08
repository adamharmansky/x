#include "draw.h"

Display* _draw_disp;
int _draw_scr;
Window _draw_win;
XftDraw* _draw_draw;
Drawable _draw_drawable;
GC _draw_gc;
int _draw_w, _draw_h;

int
draw_init(int w, int h, char* name, long event_mask) {
	XSizeHints hint = {
		.flags = PMinSize|PMaxSize,
		.min_width = w,
		.min_height = h,
		.max_width = w,
		.max_height = h
	};

	_draw_w = w;
	_draw_h = h;

	/* open a display */
	if ((_draw_disp = XOpenDisplay(NULL)) == NULL) { printf("bruh no diplay\n"); return 1; }
	/* screen */
	_draw_scr = DefaultScreen(_draw_disp);
	/* window */
	_draw_win = XCreateSimpleWindow(_draw_disp, RootWindow(_draw_disp, _draw_scr), 10, 10, w, h, 1, WhitePixel(_draw_disp, _draw_scr), BlackPixel(_draw_disp, _draw_scr));
	/* create a drawable to draw into ( We won't be drawing directly into our window) */
	_draw_drawable = XCreatePixmap(_draw_disp, _draw_win, w, h, DefaultDepth(_draw_disp, _draw_scr));

	/* select input from the specified event mask */
	XSelectInput(_draw_disp, _draw_win, event_mask);
	/* finally, map the window */
	XMapWindow(_draw_disp, _draw_win);

	/* more drawing intialization */
	_draw_draw  = XftDrawCreate  (_draw_disp, _draw_drawable, DefaultVisual(_draw_disp, _draw_scr), DefaultColormap(_draw_disp, _draw_scr));
	_draw_gc = XCreateGC(_draw_disp, _draw_win, 0, NULL);

	/* window properties */
	XStoreName(_draw_disp, _draw_win, name);
	XSetWMNormalHints(_draw_disp, _draw_win, &hint);
}

int
draw_rectangle(int x, int y, int w, int h, color_t col)
{
	XftDrawRect(_draw_draw, &col.xftcolor, x, y, w, h);
}

int draw_text(int x, int y, font_t f, char* s, color_t c)
{
	XftDrawStringUtf8(_draw_draw, &c.xftcolor, f.xftfont, x, y, s, strlen(s));
}

void
draw_char(int x, int y, font_t f, char c, color_t col)
{
	if(c != ' ' && c != '\t')XftDrawString8(_draw_draw, &col.xftcolor, f.xftfont, x, y, &c, 1);
}

int
draw_char_width(font_t f, char c)
{
	XGlyphInfo ext;
	XftTextExtents8(_draw_disp, f.xftfont, &c, 1, &ext);
	return ext.xOff;
}

int
draw_string_width(font_t f, char* c)
{
	XGlyphInfo ext;
	XftTextExtentsUtf8(_draw_disp, f.xftfont, c, strlen(c), &ext);
	return ext.xOff;
}

font_t
load_font(char* name)
{
	font_t f;
	f.xftfont = XftFontOpenName(_draw_disp, _draw_scr, name);
}

int
draw_width()
{
	int ww; int junk[20];
	XGetGeometry(_draw_disp, _draw_win,
			(Window*)junk, junk, junk, &ww, junk, junk, junk);
	return ww;
}

int
draw_height()
{
	int wh; int junk[20];
	XGetGeometry(_draw_disp, _draw_win,
			(Window*)junk, junk, junk, junk, &wh, junk, junk);
	return wh;
}

int
draw_expose()
{
	static XExposeEvent ev = {.type=Expose, .count=0};
	XSendEvent(_draw_disp, _draw_win, 0, ExposureMask, (XEvent*)&ev);
}

color_t
create_color(char r, char g, char b)
{
	XRenderColor d = {.red = r<<8, .green = g<<8, .blue = b<<8, .alpha = 255<<8};
	color_t c;
	XftColorAllocValue(_draw_disp, DefaultVisual(_draw_disp, _draw_scr), DefaultColormap(_draw_disp, _draw_scr), &d, &c.xftcolor);
	return c;
}

void
draw_flush_all()
{
	XCopyArea(_draw_disp, _draw_drawable, _draw_win, _draw_gc, 0,0,_draw_w, _draw_h, 0,0);
}

void
draw_flush(int x, int y, int w, int h)
{
	XCopyArea(_draw_disp, _draw_drawable, _draw_win, _draw_gc, x,y,w,h,x,y);
}
