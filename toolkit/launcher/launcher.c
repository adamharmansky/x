#include "../toolkit.h"

extern void launch_program();

enum {VIM, TERMINAL, MAIL, CHROMIUM, SC};

ComboBox terminal = {.x = 3, .y = 3, .w = 126, .h = 20, .text = "terminal"};
Button ok = {.x = 3, .y = 155, .w = 126, .h = 20, .text = "Allow Cookies", .toggle = 1};

char* terminal_options[] = {"st", "xterm", "urxvt", NULL};
char* terminal_commands[]= {"st", "xterm", "urxvt -e "};

struct {
	char* name;
	char terminal;
	Button butt;
} commands[] = {
	[VIM] =      {"vim",      1, {.x = 3, .y = 26,  .w = 83, .h = 40, .on_press = launch_program, .text = "vim"}      },
	[TERMINAL] = {"bash",     1, {.x = 3, .y = 69, .w = 40, .h = 40, .on_press = launch_program, .text = "sh"}        },
	[MAIL] =     {"mail",     1, {.x = 89,.y = 26,  .w = 40, .h = 40, .on_press = launch_program, .text = "mail"}     },
	[CHROMIUM] = {"chromium", 0, {.x = 46,.y = 69, .w = 83, .h = 83, .on_press = launch_program, .text = "chromium"}  },
	[SC] =       {"sc",       1, {.x = 3, .y = 112, .w = 40, .h = 40, .on_press = launch_program, .text = "sc"}       },
	{.name = NULL}
};

void
launch_program()
{
	int i;
	char cmd[128];

	for( i = 0; commands[i].name; i++) {
		if(commands[i].butt.pressed) {
			if(commands[i].terminal) {
				sprintf(cmd, "(%s %s&)", terminal.selected_option ? terminal_commands[terminal.selected_option-1] : "st", commands[i].name);
			} else sprintf(cmd, "(%s&)", commands[i].name);
			system(cmd);
		}
	}
}

int
main()
{
	int i;
	toolkit_init(132,178, "launcher");
	terminal.options = terminal_options;
	toolkit_show_combo_box(&terminal);
	toolkit_show_button(&ok);
	for( i = 0; commands[i].name; i++)
		toolkit_show_button(&commands[i].butt);
	pause();
}
