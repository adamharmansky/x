#include "toolkit.h"

void on_press_start();
void on_press_cancel();

TWindow* tw;

Label partition_label  = {.position = {{20,20} ,{0,0}}, .size = {{-40,20},{1,0}}, .text = "Partition:"};
ComboBox partition     = {.position = {{20,50} ,{0,0}}, .size = {{-40,20},{1,0}}, .text = "please select"};
Label filesystem_label = {.position = {{20,80} ,{0,0}}, .size = {{-40,20},{1,0}}, .text = "Filesystem:"};
ComboBox filesystem    = {.position = {{20,110},{0,0}}, .size = {{-40,20},{1,0}}, .text = "please select"};
Label label_label      = {.position = {{20,140},{0,0}}, .size = {{-40,20},{1,0}}, .text = "Volume label:"};
TextField label        = {.position = {{20,170},{0,0}}, .size = {{-40,20},{1,0}}, .text = "(Not required)", .input = 1};
Button wipe            = {.position = {{20,210},{0,0}}, .size = {{-40,20},{1,0}}, .text = "Perform a full wipe", .toggle = 1};
Button start           = {.position = {{5,-40} ,{0.5,1}}, .size = {{-15,30},{0.5,0}}, .text = "Start", .on_press = on_press_start};
Button cancel          = {.position = {{10,-40} ,{0,1}}, .size = {{-15,30},{0.5,0}}, .text = "Cancel",.on_press = on_press_cancel};

char* fsoptions[] = {"ext4","FAT","VFAT", NULL};
char* fscommands[] = {"mkfs.ext4 -L", "mkfs.fat -n", "mkfs.vfat -n"};

void
on_press_start()
{
	char cmd[256];
	char wipe_cmd[256];
	char umount_cmd[256];
	if(partition.selected_option && filesystem.selected_option) {
		sprintf(umount_cmd, "umount -f %s", partition.options[partition.selected_option-1]);
		system(umount_cmd);
		if(wipe.pressed) {
			sprintf(wipe_cmd, "dd if=/dev/zero of=%s", partition.options[partition.selected_option-1]);
			system(wipe_cmd);
		}
		sprintf(cmd, "%s \"%s\" %s", fscommands[filesystem.selected_option-1], label.typed_text, partition.options[partition.selected_option-1]);
		system(cmd);
	}
}

void
on_press_cancel()
{
}

int main()
{
	char* options[64];
	FILE* pipe;
	int i, j;
	char c;

	pipe = popen("df | cut -f 1 -d \" \" | sed 1d | grep ^/", "r");

	*options = malloc(256);
	for(i = j = 0; (c=getc(pipe))!=EOF; j++){
		if(c == '\n') {
			options[i][j] = 0;
			j = -1;
			options[++i] = malloc(256);
		} else {
			options[i][j] = c;
		}
	}
	options[i] = NULL;

	partition.options = options;
	filesystem.options = fsoptions;

	tw = toolkit_init(100, 60, "test");
	toolkit_show_label(tw, &partition_label);
	toolkit_show_combo_box(tw, &partition);
	toolkit_show_label(tw, &filesystem_label);
	toolkit_show_combo_box(tw, &filesystem);
	toolkit_show_label(tw, &label_label);
	toolkit_show_text_field(tw, &label);
	toolkit_show_button(tw, &wipe);
	toolkit_show_button(tw, &start);
	toolkit_show_button(tw, &cancel);
	while(toolkit_running) usleep(100000);
}
