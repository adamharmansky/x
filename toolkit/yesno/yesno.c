#include "../toolkit.h"
#include <unistd.h>

void pressed_yes() { exit(0); }
void pressed_no()  { exit(1); }

int main(int argc, char** argv)
{
	char* question = argc > 1 ? argv[1] : "are you sure you want to proceed?";

	Button no  = {.x = 8, .y = 100, .w = 83, .h = 25, .text = "No", .on_press = pressed_no};
	Button yes = {.x = 98, .y = 100, .w = 83, .h = 25, .text = "Yes", .on_press = pressed_yes};
	Label q = {.x = 10, .y = 10, .w = 168, .h = 80, .text = question};

	toolkit_init(188, 133, question);
	toolkit_show_button(&yes);
	toolkit_show_button(&no);
	toolkit_show_label(&q);

	pause();
}
