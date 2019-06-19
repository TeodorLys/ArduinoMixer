#pragma once
#include <OGUI_Window/window_handle.h>
#include <OGUI_Graphics/render_mode.h>
#include <OGUI_Graphics/font.h>
#include <OGUI_Graphics/textbox.h>
#include <OGUI_Graphics/rectangle.h>
#include "Global_Variables.h"

class OGUI_Prefered_List {
private:
	ogui::window_handle *window;
	ogui::font _f;
	ogui::rectangle r;
	ogui::render_mode *mode;
	ogui::render_mode *tb_mode;
	ogui::textbox *tb[36];
	const int _w = 180;
	const int _h = 30;
public:
	OGUI_Prefered_List(ogui::window_handle *w, ogui::render_mode *m) : window(w), mode(m) {

		printf("%i\n", sizeof(ogui::textbox));

		for (int a = 0; a < 36; a++) {
			tb[a] = new ogui::textbox(w);
		}

		_f.load_from_file("arial");

		r.color(ogui::c_color(60, 83, 102));

		mode->add_render_mode(1);
		mode->add_component(1, ogui::render_mode::BUTTON, ogui::point(50, 20), ogui::cord_point(10, 5), "BACK");
		mode->add_component(1, ogui::render_mode::BUTTON, ogui::point(50, 20), ogui::cord_point(528, 60), "NEXT");
		mode->add_component(1, ogui::render_mode::BUTTON, ogui::point(50, 20), ogui::cord_point(5, 60), "PREV");
		mode->add_component(1, &r, ogui::point(460, 75), ogui::cord_point(60, 30));
		mode->add_component(1, ogui::render_mode::TEXT, ogui::point(20, 20), ogui::cord_point(270, 35), "page 1/9", -1, &_f);


		tb_mode = new ogui::render_mode(w);

		for (int a = 0; a < 9; a++) {
			tb_mode->add_render_mode(a);

			tb_mode->add_component(a, tb[a * 4 + 0], ogui::point(27, 30), ogui::cord_point(70, 60), "UNUSED");
			tb_mode->add_component(a, tb[a * 4 + 1], ogui::point(27, 30), ogui::cord_point(185, 60), "UNUSED");
			tb_mode->add_component(a, tb[a * 4 + 2], ogui::point(27, 30), ogui::cord_point(300, 60), "UNUSED");
			tb_mode->add_component(a, tb[a * 4 + 3], ogui::point(27, 30), ogui::cord_point(415, 60), "UNUSED");

			window->_register_remote_event_handler(tb[a * 4 + 0], ogui::event_handler::KEY_DOWN);
			window->_register_remote_event_handler(tb[a * 4 + 1], ogui::event_handler::KEY_DOWN);
			window->_register_remote_event_handler(tb[a * 4 + 2], ogui::event_handler::KEY_DOWN);
			window->_register_remote_event_handler(tb[a * 4 + 3], ogui::event_handler::KEY_DOWN);
		}
		tb_mode->activate_render_mode(0);


		for (int a = 0; a < global::reserved_List.size(); a++) {
			tb[global::reserved_List[a].index]->string(global::reserved_List[a].name);
		}

	}

	void button_events() {
		if (mode->get_pressed_component_by_name() == "NEXT") {
			if (tb_mode->activated_render_mode() < 8) {
				tb_mode->activate_render_mode(tb_mode->activated_render_mode() + 1);
				mode->get_last_compenent_added(1)->string("page " + std::to_string(tb_mode->activated_render_mode() + 1) + "/9");
			}
		}
		else if (mode->get_pressed_component_by_name() == "PREV") {
			if (tb_mode->activated_render_mode() > 0) {
				tb_mode->activate_render_mode(tb_mode->activated_render_mode() - 1);
				mode->get_last_compenent_added(1)->string("page " + std::to_string(tb_mode->activated_render_mode() + 1) + "/9");
			}
		}
	}

	void key_events(ogui::event_handler *e) {
		switch (e->key) {
		case ogui::keyboard_handler::TAB: {
			int c_index = -1;
			for (int a = tb_mode->activated_render_mode() * 4; a < (tb_mode->activated_render_mode() * 4) + 4; a++) {
				if (tb[a]->edit_state()) {
					c_index = a;
				}
			}
			if (c_index != -1) {
				if (tb[c_index]->string() == "") {
					tb[c_index]->string("UNUSED");
				}
				for (int a = tb_mode->activated_render_mode() * 4; a < (tb_mode->activated_render_mode() * 4) + 4; a++) {
					tb[a]->disable_edit();
				}
				if (c_index < 35) {
					tb[c_index + 1]->enable_edit();
					tb[c_index + 1]->string("");
				}
				else {
					tb[0]->enable_edit();
					tb[0]->string("");
				}

				push_back_new_item(c_index);

				if (c_index % 4 == 3 && c_index > 1) {
					if (tb_mode->activated_render_mode() < 8) {
						tb_mode->activate_render_mode(tb_mode->activated_render_mode() + 1);
						mode->get_last_compenent_added(1)->string("page " + std::to_string(tb_mode->activated_render_mode() + 1) + "/9");
					}
					else {
						tb_mode->activate_render_mode(0);
						mode->get_last_compenent_added(1)->string("page " + std::to_string(tb_mode->activated_render_mode() + 1) + "/9");
					}
				}
			}
			break;
		}

		case ogui::keyboard_handler::ENTER:
			for (int a = 0; a < 36; a++) {
				if (tb[a]->edit_state()) {
					if (tb[a]->string() == "") {
						tb[a]->string(tb[a]->get_last_value());
					}
					else {
						push_back_new_item(a);
					}
				}
				tb[a]->disable_edit();
			}
			break;

		case ogui::keyboard_handler::ESCAPE:
			for (int a = 0; a < 36; a++) {
				if (tb[a]->edit_state()) {
					tb[a]->disable_edit();
					tb[a]->string(tb[a]->get_last_value());

				}
			}
			break;

		}
	}

	void pressed() {
		for (int a = tb_mode->activated_render_mode() * 4; a < (tb_mode->activated_render_mode() * 4) + 4; a++) {
			tb[a]->disable_edit();
			if (tb[a]->string() == "") {
				tb[a]->string("UNUSED");
			}
		}
		for (int a = tb_mode->activated_render_mode() * 4; a < (tb_mode->activated_render_mode() * 4) + 4; a++) {
			if (tb[a]->pressed()) {
				tb[a]->enable_edit();
				tb[a]->string("");
				break;
			}
		}
	}

	void r_pressed() {
		for (int a = tb_mode->activated_render_mode() * 4; a < (tb_mode->activated_render_mode() * 4) + 4; a++) {
			if (tb[a]->pressed()) {
				global::pref.erase(global::pref.begin() + a);
				for (int b = 0; b < global::reserved_List.size(); b++) {
					if (global::reserved_List[b].index == a)
						global::reserved_List.erase(global::reserved_List.begin() + b);
				}
				tb[a]->string("UNUSED");
			}
		}
	}

	void _remote_draw() {
		tb_mode->draw();
	}

	private:
		void push_back_new_item(int c_index) {
			if (tb[c_index]->string() != "" && tb[c_index]->string() != "UNUSED") {
				global::pref[c_index] = tb[c_index]->string();
				bool already_included = false;
				for (int a = 0; a < global::reserved_List.size(); a++) {
					if (global::reserved_List[a].name == tb[c_index]->string()) {
						already_included = true;
						break;
					}
				}
				if (!already_included) {
					global::reserved_List.push_back(reserved(c_index, tb[c_index]->string()));
				}
			}
		}

};