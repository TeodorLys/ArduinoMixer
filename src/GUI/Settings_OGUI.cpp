#include "Settings_OGUI.h"
#include "OGUI_Main_Screen.h"
#include "OGUI_Prefered_List.h"
#include "OGUI_Rename.h"
#include "OGUI_Exclusion.h"
#include <OGUI_Graphics/Image.h>
#include <OGUI_Window/event_handler.h>

Settings_OGUI::Settings_OGUI() {
	thread = new sf::Thread(&Settings_OGUI::_Draw_and_Check_Poll, this);
}

void Settings_OGUI::Launch_GUI() {
	opentest = true;
	thread->launch();
}

void Settings_OGUI::terminate_thread() {
	window->close();
	opentest = false;
	thread->terminate();
}

Settings_OGUI::~Settings_OGUI() {
	delete thread;
	delete main;
	delete list;
	delete rename;
	delete exclude;
	delete mode;
	delete window;
}

void Settings_OGUI::_Draw_and_Check_Poll() {
	ogui::image img;
	img.load_from_file("icon.ico", ogui::image::LOAD_ICON);
	window = new ogui::window_handle(ogui::point(290, 440), "SETTINGS", ogui::style::window_decore::Close, img);

	mode = new ogui::render_mode(window);
	ci_keycombo = new ogui::textbox(window);
	ci_keycombo->enable_background(ogui::c_color(50, 50, 50));
	c = new ogui::check_box;
	auto_update = new ogui::check_box;
	c->state(global::enable_logging);
	auto_update->state(global::auto_update);
	last_logging_state = global::enable_logging;

	main = new OGUI_Main_Screen(window, mode);
	mode->add_component(0, c, ogui::point(0, 0), ogui::cord_point(167 + 50, 40), "logging");
	mode->add_component(0, auto_update, ogui::point(0, 0), ogui::cord_point(167 + 50, 65), "auto_update");
	list = new OGUI_Prefered_List(window, mode);
	rename = new OGUI_Rename(window, mode);
	exclude = new OGUI_Exclusion(window, mode);
	mode->add_component(0, ci_keycombo, ogui::point(20, 50), ogui::cord_point(100, 270), "KEYPRESS");
	window->_register_remote_event_handler(ci_keycombo, ogui::event_handler::KEY_DOWN);

	if(global::ci_key != "")
		ci_keycombo->string(global::ci_key);

	window->size(290, mode->get_last_compenent_added(0)->position().y + 100);

	mode->activate_render_mode(0);

	while (window->window_is_open()) {
		ogui::event_handler event;
		_ci_keycombo_cursor();
		while (window->poll_event(&event)) {
			switch (event.type) {
			case ogui::event_handler::CLOSE:
				if (mode->activated_render_mode() != 0) {
					needs_update = true;
				}
				opentest = false;
				window->close();
				break;

			case ogui::event_handler::BUTTON_PRESSED:
				rename->button_events();
				exclude->button_events();
				list->button_events();
				if (mode->get_pressed_component_by_name() == "Reserved") {
					window->size(290, 440);
					window->size(600, 175);
					list->_remote_activate();
					mode->activate_render_mode(1);
				}
				else if (mode->get_pressed_component_by_name() == "Rename") {
					window->size(290, 440);
					rename->_remote_activate();
					mode->activate_render_mode(2);
				}
				else if (mode->get_pressed_component_by_name() == "Exclusion") {
					window->size(290, 440);
					exclude->_remote_activate();
					mode->activate_render_mode(3);
				}
				else if (mode->get_pressed_component_by_name() == "CONNECT") {
					Chrome_Integration ci;
					ci.Manifest_Creation();
				}
				else if (mode->get_pressed_component_by_name() == "BACK" && mode->activated_render_mode() == 1) {
					needs_update = true;
					window->size(290, mode->get_last_compenent_added(0)->position().y + 100);
					mode->activate_render_mode(0);
					list->_remote_deactivate();
				}
				else if (mode->get_pressed_component_by_name() == "BACK" && mode->activated_render_mode() == 2) {
					window->size(290, mode->get_last_compenent_added(0)->position().y + 100);
					needs_update = true;
					mode->activate_render_mode(0);
					rename->_remote_deactivate();
				}
				else if (mode->get_pressed_component_by_name() == "BACK" && mode->activated_render_mode() == 3) {
					window->size(290, mode->get_last_compenent_added(0)->position().y + 100);
					needs_update = true;
					mode->activate_render_mode(0);
					exclude->_remote_deactivate();
				}
				else if (mode->get_pressed_component_by_name() == "logging") {
					global::enable_logging = c->state();
					needs_update = true;
				}
				else if (mode->get_pressed_component_by_name() == "auto_update") {
					global::auto_update = auto_update->state();
					needs_update = true;
				}
				break;

			case ogui::event_handler::LMOUSE_RELEASED:
				list->pressed();
				rename->pressed();
				exclude->pressed();
				if (ci_keycombo->pressed()) {
					ci_keycombo->string("");
					ci_combo_editing = true;
					addition_key = 0;
				}
				else {
					ci_combo_editing = false;
				}
				break;

			case ogui::event_handler::RMOUSE_RELEASED:
				list->r_pressed();
				break;

			case ogui::event_handler::KEY_DOWN:
				if (ci_combo_editing) {
					std::string t_combo = ogui::keyboard_handler::enum_to_string(event.key);
					
					if (t_combo == "CTRL" || t_combo == "SHIFT") {
						if (addition_key != event.key) {
							addition_key = event.key;
							ci_keycombo->string(t_combo);
						}
					}
					else {
							if (ci_keycombo->string() != "") {
								global::ci_key = ci_keycombo->string() + "+" + t_combo;
								ci_keycombo->string(ci_keycombo->string() + "+" + t_combo);
							}
							else {
								keycombo = event.key;
								ci_keycombo->string(t_combo);
								global::ci_key = t_combo;
							}
							global::ci_key_hex = event.key;
							global::ci_add_hex = addition_key;
							ci_combo_editing = false;
							needs_update = true;
					}
				}
				break;

			case ogui::event_handler::KEY_UP:
				rename->key_events(&event);
				exclude->key_events(&event);
				list->key_events(&event);
				break;

			}
		}
		window->clear(ogui::c_color(100, 100, 100));
		mode->draw();
		if (mode->activated_render_mode() == 1)
			list->_remote_draw();
		else if (mode->activated_render_mode() == 2)
			rename->_remote_draw();
		else if (mode->activated_render_mode() == 3)
			exclude->_remote_draw();
		window->display();
	}
}