#pragma once
#include <OGUI_Window/window_handle.h>
#include <OGUI_Graphics/render_mode.h>
#include <OGUI_Graphics/font.h>
class OGUI_Main_Screen {
private:
	ogui::window_handle *window;
	ogui::font _f;
	ogui::render_mode *mode;
	const int _w = 180;
	const int _h = 30;
public:
	OGUI_Main_Screen(ogui::window_handle *w, ogui::render_mode *m) : window(w), mode(m){
		
		_f.load_from_file("arial");

		mode->add_render_mode(0);
		mode->add_component(0, ogui::render_mode::TEXT, ogui::point(20, _h), ogui::cord_point(50, 37), "Enable Logging", -1, &_f);
		mode->add_component(0, ogui::render_mode::TEXT, ogui::point(20, _h), ogui::cord_point(50, 62), "Auto Update Check", -1, &_f);
		mode->add_component(0, ogui::render_mode::BUTTON, ogui::point(_w, _h), ogui::cord_point(50, 100), "Reserved");
		mode->add_component(0, ogui::render_mode::BUTTON, ogui::point(_w, _h), ogui::cord_point(50, 140), "Rename");
		mode->add_component(0, ogui::render_mode::BUTTON, ogui::point(_w, _h), ogui::cord_point(50, 180), "Exclusion");
		mode->add_component(0, ogui::render_mode::BUTTON, ogui::point(_w, _h), ogui::cord_point(50, 220), "CONNECT");
	}


};