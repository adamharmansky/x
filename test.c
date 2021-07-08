#include "toolkit.h"

Button b =    {.position = {{-60,-60},{1,1}}, .size = {{50,50},{0,0}}, .text = " "};
Button c =    {.position = {{10,10},{0,0}}, .size = {{50,50},{0,0}}, .text = " "};
TextField t = {.position = {{10,10},{0,0}}, .size = {{-20,-80},{1,1}}, .text = "type some text", .scrollable = 1, .input = 1};

int main()
{
	TWindow* tw = toolkit_init(100, 60, "test");
	TWindow* tx = toolkit_init(70, 70, "lmao");
	toolkit_show_button(tw, &b);
	toolkit_show_text_field(tw, &t);
	toolkit_show_button(tx, &c);
	while(toolkit_running) sleep(1);
}
