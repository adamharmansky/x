#include <stdio.h>
#include <unistd.h>
#include "toolkit.h"

extern void button_pressed(void);
extern void remove_error(void);
extern void option_selected();
extern void button_1_release();
extern void button_2_press();
extern void button_2_release();
extern void user_typed();

char was_error = 0;

Button    open_button   = {.x = 3,  .y = 3,  .w = 80,  .h = 34, .text = "Open",          .on_press = button_pressed , .on_release = button_1_release};
TextField file_name     = {.x = 84, .y = 3,  .w = 320, .h = 34, .text = "filename..",    .input = 1, .on_content_changed = user_typed};
TextField file_contents = {.x = 3,  .y = 38, .w = 201, .h = 320,.scrollable = 1};
TextField lmao_desc     = {.x = 205,.y = 38, .w = 179, .h = 20,.text = "lorem ipsum:"};
Button button_that_does_nothing = {.x = 384,.y = 38, .w = 20, .h = 20,.text = ":)", .toggle = 1, .on_press = button_2_press, .on_release = button_2_release};
TextField lmao          = {.x = 205,.y = 59, .w = 199, .h = 140,.scrollable = 1, .text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur et mattis orci. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Nunc eu ultricies dui. Nulla nunc eros, placerat a est et, scelerisque semper nunc. Aliquam maximus est vel velit tincidunt pulvinar. Quisque tortor orci, laoreet."};
TextField error_field   = {.x = 205,.y = 324,.w = 165, .h = 34, .text = "ERROR OPENING FILE!!!"};
Button    error_button  = {.x = 370,.y = 324,.w = 34, .h = 34, .text = "×",             .on_press = remove_error};
ComboBox  combo         = {.x = 210,.y = 205, .w = 100, .h = 20, .text = "Select", .on_select_option = option_selected};
Label     label         = {.x = 210,.y = 239, .w = 100, .h = 50};

void
remove_error()
{
	toolkit_remove_widget(&error_field);
	toolkit_remove_widget(&error_button);
	was_error = 0;
}

void
button_pressed()
{
	FILE* f;
	char c;
	int size = 0;

	if(was_error) remove_error();
	printf("loading %s\n", file_name.typed_text);

	f = fopen(file_name.typed_text, "r");

	if(f == NULL) {
		toolkit_show_text_field(&error_field);
		toolkit_show_button(&error_button);
		was_error = 1;
		return;
	}

	for(;(c=getc(f))!=EOF;) {
		file_contents.text = realloc(file_contents.text, ++size);
		file_contents.text[size-1] = c;
	}
	file_contents.text = realloc(file_contents.text, ++size);
	file_contents.text[size-1] = 0;
}

void option_selected() { sprintf(label.text, "%d", combo.selected_option); }
void button_1_release() { sprintf(label.text, "released!"); }
void button_2_press() { sprintf(label.text, "yo"); }
void button_2_release() { sprintf(label.text, "why u mad bro"); }
void user_typed() {strcpy(label.text, file_name.typed_text);}

int
main()
{
	char* options[] = {"lmao", "test", "haha", NULL};

	toolkit_init(407,361,"toolkit test");
	file_contents.text = malloc(1);
	*file_contents.text = 0;
	combo.options = options;
	label.text = malloc(32);
	*label.text = 0;

	toolkit_show_button(&open_button);
	toolkit_show_text_field(&file_name);
	toolkit_show_text_field(&lmao);
	toolkit_show_text_field(&lmao_desc);
	toolkit_show_text_field(&file_contents);
	toolkit_show_button(&button_that_does_nothing);
	toolkit_show_combo_box(&combo);
	toolkit_show_label(&label);
	pause();
}
