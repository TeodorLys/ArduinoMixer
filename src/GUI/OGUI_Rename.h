#pragma once
#include "Global_Variables.h"
#include <OGUI_Window/window_handle.h>
#include <OGUI_Graphics/render_mode.h>
#include <OGUI_Graphics/textbox.h>
#include <OGUI_Graphics/button.h>
#include <SFML/System/Clock.hpp>
#include <vector>

class OGUI_Rename {
private:
	ogui::window_handle *window;
	ogui::render_mode *mode;
	ogui::textbox *new_tb;
	ogui::textbox *old_tb;
	ogui::font font;
	std::vector<ogui::text*> added_text;
	std::vector<ogui::button*> added_button;
	sf::Clock clock;
	int ident = 0;
public:
	OGUI_Rename(ogui::window_handle *w, ogui::render_mode *m) : window(w), mode(m) {

		new_tb = new ogui::textbox(w);
		old_tb = new ogui::textbox(w);

		font.load_from_file("arial");

		new_tb->enable_background(ogui::c_color(50, 50, 50));
		old_tb->enable_background(ogui::c_color(50, 50, 50));

		mode->add_render_mode(2);

		mode->add_component(2, new_tb, ogui::point(27, 50), ogui::cord_point(160, 360), "UNUSED");
		mode->add_component(2, old_tb, ogui::point(27, 50), ogui::cord_point(20, 360), "UNUSED");
		mode->add_component(2, ogui::render_mode::BUTTON, ogui::point(50, 20), ogui::cord_point(10, 5), "BACK");
		mode->add_component(2, ogui::render_mode::TEXT, ogui::point(20, 0), ogui::cord_point(21, 330), "OLD NAME", -1, &font);
		mode->add_component(2, ogui::render_mode::TEXT, ogui::point(20, 0), ogui::cord_point(161, 330), "NEW NAME", -1, &font);
		mode->add_component(2, ogui::render_mode::TEXT, ogui::point(30, 50), ogui::cord_point(95, 0), "ADDED", -1, &font);
		window->_register_remote_event_handler(new_tb, ogui::event_handler::KEY_DOWN);
		window->_register_remote_event_handler(old_tb, ogui::event_handler::KEY_DOWN);

		for (int a = 0; a < (int)global::rename_list.size(); a++) {
			added_text.push_back(new ogui::text(window, &font));
			added_text[added_text.size() - 1]->size(ogui::point(23, 0));
			added_text[added_text.size() - 1]->position(ogui::cord_point(70, (30 * a) + 30));
			added_text[added_text.size() - 1]->string(global::rename_list[a]);
			added_text[added_text.size() - 1]->color(ogui::c_color(255, 255, 255));
			added_button.push_back(new ogui::button());
			added_button[added_button.size() - 1]->size(ogui::point(20, 20));
			added_button[added_button.size() - 1]->position(ogui::cord_point(20, added_text[added_text.size() - 1]->position().y));
			added_button[added_button.size() - 1]->string("X");
		}
	}

	~OGUI_Rename() {
		delete new_tb;
		delete old_tb;
	}

	void button_events() {
		for (int a = 0; a < (int)added_button.size(); a++) {
			if (added_button[a]->pressed()) {
				for (int b = 0; b < (int)global::rename_list.size(); b++) {
					printf("%s, %s\n", global::rename_list[b].c_str(), added_text[a]->string().c_str());
					if (global::rename_list[b] == added_text[a]->string()) {
						printf("did it\n");
						global::rename_list.erase(global::rename_list.begin() + b);
						break;
					}
				}

				added_button[a]->destroy();
				added_text[a]->destroy();
				delete added_button[a];
				delete added_text[a];
				added_text.erase(added_text.begin() + a);
				added_button.erase(added_button.begin() + a);

				/*
				Reformat the objects, so there is no gaps(visually...)
				*/

				for (int b = 0; b < (int)added_text.size(); b++) {
					added_button[b]->position(20, (30 * b) + 30);
					added_text[b]->position(70, (30 * b) + 30);
				}
				return;
			}
		}

	}

	void _remote_deactivate() {
		for (int a = 0; a < (int)added_text.size(); a++) {
			added_text[a]->hide();
			added_button[a]->hide(true);
		}
	}

	void _remote_activate() {
		for (int a = 0; a < (int)added_text.size(); a++) {
			added_text[a]->show();
			added_button[a]->hide(false);
		}
	}

	void key_events(ogui::event_handler *e) {
		switch (e->key) {
		case ogui::keyboard_handler::TAB:
			if (old_tb->edit_state()) {
				old_tb->disable_edit();
				new_tb->enable_edit();
				new_tb->string("");
				new_tb->set_background_color(ogui::c_color(50, 50, 50));
				old_tb->set_background_color(ogui::c_color(50, 50, 50));
			}
			else if (new_tb->edit_state()) {
				if (new_tb->string() != "" && old_tb->string() != "") {
					old_tb->disable_edit();
					new_tb->disable_edit();
				}
				else if (new_tb->string() == "" || old_tb->string() == "") {
					new_tb->string("UNUSED");
					new_tb->disable_edit();
					old_tb->string("");
					old_tb->enable_edit();
				}
				new_tb->set_background_color(ogui::c_color(50, 50, 50));
				old_tb->set_background_color(ogui::c_color(50, 50, 50));
			}
			break;
		case ogui::keyboard_handler::ENTER:
			if (new_tb->edit_state() || new_tb->string() != "UNUSED") {
				if (old_tb->string() != "" && old_tb->string() != "UNUSED" && new_tb->string() != "UNUSED" && new_tb->string() != "UNUSED") {
					push_back_new_item();
					global::rename_list.push_back(old_tb->string() + "->" + new_tb->string());
				}
				old_tb->enable_edit();
				old_tb->string("");
				new_tb->string("UNUSED");
				new_tb->disable_edit();
				new_tb->set_background_color(ogui::c_color(50, 50, 50));
				old_tb->set_background_color(ogui::c_color(50, 50, 50));
			}
			else {
				old_tb->disable_edit();
				new_tb->enable_edit();
				new_tb->string("");
				new_tb->set_background_color(ogui::c_color(50, 50, 50));
				old_tb->set_background_color(ogui::c_color(50, 50, 50));
			}
			break;
		}
	}

	void pressed() {
		if (new_tb->pressed()) {
			old_tb->disable_edit();
			new_tb->enable_edit();
		}else if (old_tb->pressed()) {
			old_tb->string("");
			new_tb->disable_edit();
			old_tb->enable_edit();
		}
	}

	void _remote_draw() {
		static bool flip = 1;
		if (clock.getElapsedTime().asMilliseconds() >= 500 && old_tb->edit_state()) {
			if (flip) {
				new_tb->set_background_color(ogui::c_color(50, 50, 50));
				old_tb->set_background_color(ogui::c_color(70, 70, 70));
				flip = false;
			}
			else {
				new_tb->set_background_color(ogui::c_color(50, 50, 50));
				old_tb->set_background_color(ogui::c_color(50, 50, 50));
				flip = true;
			}
			clock.restart();
		}
		if (clock.getElapsedTime().asMilliseconds() >= 500 && new_tb->edit_state()) {
			if (flip) {
				old_tb->set_background_color(ogui::c_color(50, 50, 50));
				new_tb->set_background_color(ogui::c_color(70, 70, 70));
				flip = false;
			}
			else {
				old_tb->set_background_color(ogui::c_color(50, 50, 50));
				new_tb->set_background_color(ogui::c_color(50, 50, 50));
				flip = true;
			}
			clock.restart();
		}
		for (int a = 0; a < (int)added_text.size(); a++) {
			window->draw(added_text[a]);
			window->draw(added_button[a]);
		}
	}

	private:
		void push_back_new_item() {
			added_text.push_back(new ogui::text(window, &font));
			added_text[added_text.size() - 1]->size(ogui::point(23, 0));
			added_text[added_text.size() - 1]->position(ogui::cord_point(70, added_text.size() > 1 ? 30 + added_text[added_text.size() - 2]->position().y : 30));
			added_text[added_text.size() - 1]->string(old_tb->string() + "->" + new_tb->string());
			added_text[added_text.size() - 1]->color(ogui::c_color(255, 255, 255));
			added_button.push_back(new ogui::button());
			added_button[added_button.size() - 1]->size(ogui::point(20, 20));
			added_button[added_button.size() - 1]->position(ogui::cord_point(20, added_text[added_text.size() - 1]->position().y));
			added_button[added_button.size() - 1]->string("X");
		}

};