#include <stdio.h>
#include <unistd.h>
#include "../draw.h"

#define AVG_WORD_WIDTH 80

font_t font;
color_t fg;

color_t bg_1;
color_t bg_2;

char* buf;
size_t buf_size;

int camera_y = 0;

int margin = 100;
int tabindent = 100;

int
redraw()
{
	int i=0;
	XGlyphInfo ext;
	int x = margin;
	int h = font.xftfont->height;
	int b = draw_width()-margin-AVG_WORD_WIDTH;
	int dh = draw_height(), dw = draw_width();
	int a = h/3 - 2 ;
	int nl = 0;
	int y = margin - camera_y + h;

	/* if there are more events, quit! */
	if(XPending(_draw_disp)) return 1;

	for(;y<0 && buf[i];i++) {
		x += draw_char_width(font, buf[i]);
		if(buf[i] == '\t') {
			x = (int)((x-margin)/tabindent + 1)*tabindent + margin;
		}
		if((x >= b && buf[i] == ' ')|| buf[i] == '\n')
		{
			nl++;
			x = margin;
			y += h;
		}
	}
	draw_rectangle(0,0,dw, y + a, (nl%2)?bg_1:bg_2);
	for(; buf[i]; i++) {
		if(buf[i] == '\n') goto hm;
		draw_char(x, y, font, buf[i], fg);
		x += draw_char_width(font, buf[i]);
		if(buf[i] == '\t') {
			x = (int)((x-margin)/tabindent + 1)*tabindent + margin;
		}
		if(x >= b && buf[i] == ' ')
		{
	hm:
			nl++;
			draw_rectangle(0,y+a,dw, h, (nl%2)?bg_1:bg_2);
			x = margin;
			y = y+h;
			if(y > dh) return 0;
		}
	}
	draw_rectangle(0,y,dw, dh-y, (nl%2)?bg_1:bg_2);
}

int
main(int argc, char** argv)
{
	XEvent e;
	int x;
	int i, c;
	int old_w, old_h;
	FILE* file;

	draw_init(800,600,"this is a test", ButtonPressMask | KeyPressMask | ExposureMask);
	bg_1  = create_color(0x30,0x32,0x36);
	bg_2  = create_color(0x36,0x38,0x3e);
	fg  = create_color(255,255,255);
	font = load_font("Serif:size=15");

	buf = malloc(16);

	if(argc > 1) file = fopen(argv[1], "r");
	else         file = stdin;

	for(i=0; ; i++) {
		if(i+1 > buf_size){
			/* don't realloc the buffer every time you add something! */
			buf_size += 32;
			buf = realloc(buf, buf_size);
		}
		if( (c = getc(file)) == EOF ) {
			buf[i] = 0;
			break;
		}
		buf[i] = c;
	}


	for(;;) {
		XNextEvent(_draw_disp, &e);
		if(e.type == Expose) {
			if(e.xexpose.count) continue;
		} else if (e.type == KeyPress) {
			KeySym k = XLookupKeysym(&e.xkey, 0);
			     if (k == XK_Down || k == XK_j) camera_y += 32;
			else if (k == XK_Up   || k == XK_k) camera_y -= 32;
			else if (k == XK_Page_Down) camera_y += draw_height();
			else if (k == XK_Page_Up  ) camera_y -= draw_height();
			else if (k == XK_d) camera_y += draw_height()/4;
			else if (k == XK_u) camera_y -= draw_height()/4;
			else if (k == XK_l) margin += 5;
			else if (k == XK_h) margin -= 5;
			else if ((k == XK_g && ~e.xkey.state & ShiftMask)|| k == XK_Home) camera_y = 0;
			else if ((k == XK_g && e.xkey.state & ShiftMask) || k == XK_End) {
				camera_y = 0;
				x = 0;
				for(i = 0;buf[i];i++) {
					x += draw_char_width(font, buf[i]);
					if(buf[i] == '\t') {
						x = (int)((x-margin)/tabindent + 1)*tabindent + margin;
					}
					if((x >= draw_width()-margin-AVG_WORD_WIDTH && buf[i] == ' ')|| buf[i] == '\n')
					{
						x = margin;
						camera_y += font.xftfont->height;
					}
				}
				camera_y -= draw_width()-margin;
			} else if( k == XK_q ) return 0;
		} else if(e.type == ButtonPress) {
			if     (e.xbutton.button == 4) camera_y -= 32;
			else if(e.xbutton.button == 5) camera_y += 32;
		}
		redraw();
	}
}
