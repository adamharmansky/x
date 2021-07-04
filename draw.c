#include "draw.h"

Display* _draw_disp;
int _draw_scr;
Window _draw_win;
XftDraw* _draw_draw;

int
draw_init(int w, int h, char* name) {
	if ((_draw_disp = XOpenDisplay(NULL)) == NULL) { printf("bruh no diplay\n"); return 1; }
	_draw_scr = DefaultScreen(_draw_disp);
	_draw_win = XCreateSimpleWindow(_draw_disp, RootWindow(_draw_disp, _draw_scr), 10, 10, w, h, 1, WhitePixel(_draw_disp, _draw_scr), BlackPixel(_draw_disp, _draw_scr));
	XSelectInput(_draw_disp, _draw_win,
	/* FEEL FREE TO CHANGE THESE FLAGS IF YOU NEED TO */
		ExposureMask |
		KeyPressMask |
		ButtonPressMask |
		ButtonReleaseMask );
	XMapWindow(_draw_disp, _draw_win);
	_draw_draw  = XftDrawCreate  (_draw_disp, _draw_win, DefaultVisual(_draw_disp, _draw_scr), DefaultColormap(_draw_disp, _draw_scr));
	XStoreName(_draw_disp, _draw_win, name);
}

int
draw_rectangle(int x, int y, int w, int h, color_t col)
{
	XftDrawRect(_draw_draw, &col.xftcolor, x, y, w, h);
}

int draw_text(int x, int y, font_t f, char* s, color_t c)
{
	XftDrawString8(_draw_draw, &c.xftcolor, f.xftfont, x, y, s, strlen(s));
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
	XRenderColor d = {.red = r<<8, .green = g<<8, .blue = b<<8};
	color_t c;
	XftColorAllocValue(_draw_disp, DefaultVisual(_draw_disp, _draw_scr), DefaultColormap(_draw_disp, _draw_scr), &d, &c.xftcolor);
	return c;
}
