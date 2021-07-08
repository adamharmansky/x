#include "toolkit.h"

Button b =    {.position = {{-60,-60},{1,1}}, .size = {{50,50},{0,0}}, .text = "ok"};
TextField t = {.position = {{10,10},{0,0}}, .size = {{-20,-80},{1,1}}, .text = "type some text", .scrollable = 1, .input = 1};

int main()
{
	TWindow* tw = toolkit_init(100, 60, "test");
	toolkit_show_button(tw, &b);
	toolkit_show_text_field(tw, &t);
	while(toolkit_running) usleep(100000);
}
