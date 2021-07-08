#ifndef DRAW_C
#define DRAW_C
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xft/Xft.h>
#include <pthread.h>
#include <string.h>
typedef struct {
	XftColor xftcolor;
} color_t;
typedef struct {
	XftFont* xftfont;
} font_t;

extern Display* _draw_disp;
extern int _draw_scr;
extern Window _draw_win;
extern XftDraw* _draw_draw;

extern int draw_init(int, int, char*, long);
extern int draw_rectangle(int, int, int, int, color_t);
extern int draw_text(int, int, font_t, char*, color_t);
extern void draw_char(int, int, font_t, char, color_t);
extern color_t create_color(char, char, char);
extern font_t  load_font(char* name);
extern int draw_height();
extern int draw_width();
extern int draw_expose();
extern int draw_char_width(font_t f, char c);
extern int draw_string_width(font_t f, char* c);
extern void draw_flush_all();
extern void draw_flush(int x, int y, int w, int h);
#endif
