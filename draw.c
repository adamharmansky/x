#include "draw.h"
#include <stdio.h>

draw_context
draw_init(int w, int h, char* name, long event_mask) {
	draw_context c;
	// XSizeHints hint = {
	// 	.flags = PMinSize|PMaxSize,
	// 	.min_width = w,
	// 	.min_height = h,
	// 	.max_width = w,
	// 	.max_height = h
	// };

	c.w = w;
	c.h = h;

	/* open a display */
	if ((c.disp = XOpenDisplay(NULL)) == NULL) { printf("bruh no diplay\n"); exit(1); }
	/* screen */
	c.scr = DefaultScreen(c.disp);
	/* window */
	c.win = XCreateSimpleWindow(c.disp, RootWindow(c.disp, c.scr), 10, 10, w, h, 0, WhitePixel(c.disp, c.scr), BlackPixel(c.disp, c.scr));
	/* create a drawable to draw into ( We won't be drawing directly into our window) */
	c.drawable = XCreatePixmap(c.disp, c.win, w, h, DefaultDepth(c.disp, c.scr));

	/* select input from the specified event mask */
	XSelectInput(c.disp, c.win, event_mask);

	/* more drawing intialization */
	c.gc = XCreateGC(c.disp, c.win, 0, NULL);
	c.draw  =  XftDrawCreate(c.disp, c.drawable, DefaultVisual(c.disp, c.scr), DefaultColormap(c.disp, c.scr));

	/* window properties */
	XStoreName(c.disp, c.win, name);
	// XSetWMNormalHints(c.disp, c.win, &hint);

	/* finally, map the window */
	XMapWindow(c.disp, c.win);
	return c;
}

int
draw_rectangle(draw_context c, int x, int y, int w, int h, color_t col)
{
	XftDrawRect(c.draw, &col.xftcolor, x, y, w, h);
}

int draw_text(draw_context c, int x, int y, font_t f, char* s, color_t col)
{
	XftDrawStringUtf8(c.draw, &col.xftcolor, f.xftfont, x, y, s, strlen(s));
}

void
draw_char(draw_context c, int x, int y, font_t f, char ch, color_t col)
{
	if(ch != ' ' && ch != '\t')XftDrawString8(c.draw, &col.xftcolor, f.xftfont, x, y, &ch, 1);
}

int
draw_char_width(draw_context c, font_t f, char ch)
{
	XGlyphInfo ext;
	XftTextExtents8(c.disp, f.xftfont, &ch, 1, &ext);
	return ext.xOff;
}

unsigned int
draw_width(draw_context c)
{
	union {
		Window w;
		unsigned int i;
	} junk;
	unsigned int w;

	XGetGeometry(c.disp, c.win, &junk.w, &junk.i, &junk.i, &w, &junk.i, &junk.i, &junk.i);
	return w;
}

unsigned int
draw_height(draw_context c)
{
	union {
		Window w;
		unsigned int i;
	} junk;
	unsigned int h;

	XGetGeometry(c.disp, c.win, &junk.w, &junk.i, &junk.i, &junk.i, &h, &junk.i, &junk.i);
	return h;
}

int
draw_string_width(draw_context c, font_t f, char* s)
{
	XGlyphInfo ext;
	XftTextExtentsUtf8(c.disp, f.xftfont, s, strlen(s), &ext);
	return ext.xOff;
}

font_t
load_font(draw_context c, char* name)
{
	font_t f;
	f.xftfont = XftFontOpenName(c.disp, c.scr, name);
}


int
draw_expose(draw_context c)
{
	static XExposeEvent ev = {.type=Expose, .count=0};
	XSendEvent(c.disp, c.win, 0, ExposureMask, (XEvent*)&ev);
}

color_t
create_color(draw_context c, char r, char g, char b)
{
	XRenderColor d = {.red = r<<8, .green = g<<8, .blue = b<<8, .alpha = 255<<8};
	color_t col;
	XftColorAllocValue(c.disp, DefaultVisual(c.disp, c.scr), DefaultColormap(c.disp, c.scr), &d, &col.xftcolor);
	return col;
}

void
draw_flush_all(draw_context c)
{
	XCopyArea(c.disp, c.drawable, c.win, c.gc, 0,0,c.w, c.h, 0,0);
}

void
draw_flush(draw_context c, int x, int y, int w, int h)
{
	XCopyArea(c.disp, c.drawable, c.win, c.gc, x,y,w,h,x,y);
}

void
draw_resize(draw_context* c, int w, int h)
{
	Pixmap new = XCreatePixmap(c->disp, c->win, w, h, DefaultDepth(c->disp, c->scr));
	XCopyArea(c->disp, c->drawable, new, c->gc, 0, 0, c->w, c->h, 0,0);
	XFreePixmap(c->disp, c->drawable);
	c->drawable = new;
	c->w = w;
	c->h = h;
	XftDrawChange(c->draw, c->drawable);
}
