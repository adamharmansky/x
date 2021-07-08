#include "toolkit.h"

Button b = {.position = {{-60,-60},{1,1}}, .size = {{50,50},{0,0}}, .text = " "};

int main()
{
	toolkit_init(100, 60, "test");
	toolkit_show_button(&b);
	pause();
}
