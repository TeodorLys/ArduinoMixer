#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include "Global_Variables.h"
#include <OGUI_Window/window_handle.h>
#include <OGUI_Graphics/render_mode.h>
#include <OGUI_Graphics/textbox.h>
#include <OGUI_Graphics/button.h>
#include <vector>

class OGUI_Exclusion {
private:
	ogui::window_handle *window;
	ogui::render_mode *mode;
	ogui::render_mode *added;
	ogui::textbox *tb;
	ogui::font font;
	std::vector<ogui::text*> added_text;
	std::vector<ogui::button*> added_button;
	int ident = 0;
	sf::Clock clock;
public:
	OGUI_Exclusion(ogui::window_handle *w, ogui::render_mode *m) : window(w), mode(m) {

		tb = new ogui::textbox(w);

		font.load_from_file("arial");

		added = new ogui::render_mode(window);

		tb->enable_background(ogui::c_color(50, 50, 50));

		mode->add_render_mode(3);

		mode->add_component(3, tb, ogui::point(27, 50), ogui::cord_point(87, 360), "UNUSED");
		mode->add_component(3, ogui::render_mode::BUTTON, ogui::point(50, 20), ogui::cord_point(10, 5), "BACK");
		mode->add_component(3, ogui::render_mode::TEXT, ogui::point(30, 50), ogui::cord_point(95, 0), "ADDED", -1, &font);
		window->_register_remote_event_handler(tb, ogui::event_handler::KEY_DOWN);

		for (int a = 0; a < (int)global::to_Exclude.size(); a++) {
			added_text.push_back(new ogui::text(window, &font));
			added_text[added_text.size() - 1]->size(ogui::point(23, 0));
			added_text[added_text.size() - 1]->position(ogui::cord_point(70, (30 * a) + 30));
			added_text[added_text.size() - 1]->string(global::to_Exclude[a]);
			added_text[added_text.size() - 1]->color(ogui::c_color(255, 255, 255));
			added_button.push_back(new ogui::button());
			added_button[added_button.size() - 1]->size(ogui::point(20, 20));
			added_button[added_button.size() - 1]->position(ogui::cord_point(20, added_text[added_text.size() - 1]->position().y));
			added_button[added_button.size() - 1]->string("X");
		}
		//added->add_render_mode(0);

		//for (int a = 0; a < global::rename_list.size(); a++) {
		//	added->add_component(0, ogui::render_mode::TEXT, ogui::point(23, 0), ogui::cord_point(70, 30 + added->get_last_compenent_added(0)->position().y), global::rename_list[a], ident, &font);
		//	added->add_component(0, ogui::render_mode::BUTTON, ogui::point(20, 20), ogui::cord_point(20, (added->get_last_compenent_added(0)->position().y)), "X", ident++);
		//}

		//added->activate_render_mode(0);
	}

	void button_events() {
		//if (added->remove_compenent_by_ident(0, added->get_ident_by_pressed(0))) {
		//	printf("%i, %i\n", added->get_last_deleted_component()->position().x, added->get_last_deleted_component()->position().y);
		//	for (int a = 0; a < added->_render[0].to_draw.size(); a++) {
		//		if (a > 1) {
		//			if (added->_render[0].to_draw[a].d->string() == "X")
		//				added->_render[0].to_draw[a].d->position(20, added->_render[0].to_draw[a - 1].d->position().y);
		//			else
		//				added->_render[0].to_draw[a].d->position(70, 30 + added->_render[0].to_draw[a - 1].d->position().y);
		//		}
		//		else {
		//			if (added->_render[0].to_draw[a].d->string() == "X")
		//				added->_render[0].to_draw[a].d->position(20, 30);
		//			else
		//				added->_render[0].to_draw[a].d->position(70, 30);
		//		}
		//	}
		//}

		for (int a = 0; a < (int)added_button.size(); a++) {
			if (added_button[a]->pressed()) {
				for (int b = 0; b < (int)global::to_Exclude.size(); b++) {
					if (global::to_Exclude[b] == added_text[a]->string()) {
						global::to_Exclude.erase(global::to_Exclude.begin() + b);
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
		case ogui::keyboard_handler::ENTER:
			if (tb->string() != "" && tb->string() != "UNUSED") {
				push_back_new_item();
				global::to_Exclude.push_back(tb->string());
			}
			tb->set_background_color(ogui::c_color(50, 50, 50));
			tb->string("");
			tb->enable_edit();
			break;
		}
	}

	void pressed() {
		if (tb->pressed()) {
			tb->enable_edit();
			tb->string("");
		}
		else {
			tb->disable_edit();
		}
	}

	void _remote_draw() {
		static bool flip = 1;
		if (clock.getElapsedTime().asMilliseconds() >= 500 && tb->edit_state()) {
			if (flip) {
				tb->set_background_color(ogui::c_color(70, 70, 70));
				flip = false;
			}
			else {
				tb->set_background_color(ogui::c_color(50, 50, 50));
				flip = true;
			}
			clock.restart();
		}
		for (int a = 0; a < (int)added_text.size(); a++) {
			window->draw(added_text[a]);
			window->draw(added_button[a]);
		}
	}

	void push_back_new_item() {
		added_text.push_back(new ogui::text(window, &font));
		added_text[added_text.size() - 1]->size(ogui::point(23, 0));
		added_text[added_text.size() - 1]->position(ogui::cord_point(70, added_text.size() > 1 ? 30 + added_text[added_text.size() - 2]->position().y : 30));
		added_text[added_text.size() - 1]->string(tb->string());
		added_text[added_text.size() - 1]->color(ogui::c_color(255, 255, 255));
		added_button.push_back(new ogui::button());
		added_button[added_button.size() - 1]->size(ogui::point(20, 20));
		added_button[added_button.size() - 1]->position(ogui::cord_point(20, added_text[added_text.size() - 1]->position().y));
		added_button[added_button.size() - 1]->string("X");
	}

};