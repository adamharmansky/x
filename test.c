#include "toolkit.h"

Label       l = {.position={{10,10},{0,0}},.size={{-20,20},{1,0}}, .text = "Downloading virus..."};
ProgressBar p = {.position={{70,50},{0,0}},.size={{-80,20},{1,0}}};
Button      b = {.position={{-80,-50},{1,1}},.size={{70,40},{0,0}}, .text = "OK"};
Icon        ic= {.position={{10,50},{0,0}},.size={{70,40},{0,0}}, .source = "icons/48/windows.ff"};

int main()
{
	float f;
	TWindow* tw = toolkit_init(200, 150, "?test");
	toolkit_show_progress_bar(tw,&p);
	toolkit_show_label(tw,&l);
	toolkit_show_button(tw,&b);
	toolkit_show_icon(tw,&ic);

	for(f = 0; f < 1 && toolkit_running; f += 0.001){
		usleep(10000);
		update_progress_bar(tw,&p,f);
	}
}
