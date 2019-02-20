#pragma once
#include <SFML/Graphics.hpp>
#include "GUI_Prefered_List.h"
#include "GUI_Text_Field.h"
#include "GUI_Textbox.h"
#include "GUI_Button.h"
#include "Global_Variables.h"

struct entry_object{
	GUI_Textbox obj;
	sf::RectangleShape r_x[2];

	entry_object() {
		r_x[0].setSize(sf::Vector2f(5, 25));
		r_x[0].rotate(45);
		r_x[0].setFillColor(sf::Color(200, 100, 100));

		r_x[1].setSize(sf::Vector2f(5, 25));
		r_x[1].rotate(315);
		r_x[1].setFillColor(sf::Color(200, 100, 100));
	}

	void gui_easy_setup(std::string title, int char_size, sf::Color c) {
		obj.gui_easy_setup(title, char_size, c);
	}

	void give_Font(sf::Font *f) { obj.give_Font(f); }

	void setPos(float x, float y) {
		obj.setPos(x, y);
		int Xoffset = 20;
		int Yoffset = 12;
		r_x[0].setPosition(obj.getPos().x - Xoffset + (0 + r_x[0].getGlobalBounds().width / 2), obj.getPos().y + Yoffset);
		r_x[1].setPosition(obj.getPos().x - Xoffset + (3 - r_x[1].getGlobalBounds().width / 2), obj.getPos().y + (r_x[1].getSize().x / 2) + Yoffset);
	}


	void draw(sf::RenderWindow *w) {
		obj.draw(w);
		w->draw(r_x[1]);
		w->draw(r_x[0]);
	}
	bool _pressed(sf::RenderWindow *w, sf::Mouse *m) {
		for (int a = 0; a < 2; a++) {
			if (r_x[a].getGlobalBounds().contains(sf::Vector2f(m->getPosition(*w)))) {
				return true;
			}
		}
		return false;
	}

};

class GUI_Entry{
private:
	GUI_Text_Field tf;
	sf::RenderWindow *window;
	sf::Mouse *mouse;
	entry_object buff;
	GUI_Textbox title;
	GUI_Button back;
	std::vector<entry_object> exclutions;
public:
	GUI_Entry(){
		title.gui_easy_setup("ADDED", 24, sf::Color(255, 255, 255));
		buff.gui_easy_setup("UNUSED", 36, sf::Color(255, 255, 255));
	}

	int _Look_for_Mouse_Press() {
		tf._Look_for_Mouse_Press(mouse, window);

		if (tf.get_Enter_Hit()) {
			tf._Manual_Disable();
			tf._Manual_Enable();
			tf._Registered_Enter_Hit();
		}

		for (int a = 0; a < (int)exclutions.size(); a++) {
			if (exclutions[a]._pressed(window, mouse)) {
				exclutions.erase(exclutions.begin() + a);
				for (int b = 0; b < (int)exclutions.size(); b++) {
					if (b == 0)
						exclutions[b].setPos(120, 40);
					else
						exclutions[b].setPos(120, exclutions[b - 1].obj.getPos().y + 36);
				}
				global::to_Exclude.clear();
				for (int a = 0; a < (int)exclutions.size(); a++) {
					if (exclutions[a].obj.getObject().getString() != "UNUSED" && exclutions[a].obj.getObject().getString() != "RESERV") {
						global::to_Exclude.push_back(exclutions[a].obj.getObject().getString());
					}
				}
				break;
			}
		}

		if (back._Look_for_Mouse_Press(mouse, window)) {
			return 1;
		}
		return 0;
	}

	bool Check_Hover(sf::Mouse *m, sf::RenderWindow *w) {
		back.Check_Hover(m, w);
		return tf.Check_Hover(m, w);
	}

	void give_Remote_Variables(sf::RenderWindow *w, sf::Mouse *m) {
		window = w; 
		mouse = m;
		title.setPos(150, 0);
		tf.set_Pos(120, 500);
		tf.Enable_Frame();
	}

	void give_font(sf::Font *f) { 
		tf.give_Font(f); 
		buff.give_Font(f); 
		title.give_Font(f);
		back.give_Font(f);
		back.gui_easy_setup("BACK", 18, sf::Color(80, 80, 80), 5);
		back.setPos(10, 10);

		for (int a = 0; a < (int)global::to_Exclude.size(); a++) {
			buff.obj.setString(global::to_Exclude[a]);
			if (exclutions.size() == 0) {
				buff.setPos(120, 40);
			}
			else {
				buff.setPos(120, exclutions[exclutions.size() - 1].obj.getPos().y + 36);
			}
			exclutions.push_back(buff);
		}
	}

	void Enable_Writing() {
		tf._Manual_Enable();
	}

	void Disable_Writing() {
		tf._Manual_Disable();
	}

	void draw(sf::RenderWindow *w) {
		tf.draw(w);
		title.draw(w);
		for (int a = 0; a < (int)exclutions.size(); a++) {
			exclutions[a].draw(w);
		}
		back.draw(w);
	}

	void _Text_Enter_Check(sf::Event *e) {
		tf.Value_Field(e);
		if (tf.get_Enter_Hit()) {
			if (exclutions.size() == 0) {
				buff.setPos(120, 40);
			}
			else {
				buff.setPos(120, exclutions[exclutions.size() - 1].obj.getPos().y + 36);
			}
			buff.obj.setString(tf.Extract_Written_String());
			exclutions.push_back(buff);
			global::to_Exclude.clear();
			for (int a = 0; a < (int)exclutions.size(); a++) {
				if (exclutions[a].obj.getObject().getString() != "UNUSED" && exclutions[a].obj.getObject().getString() != "RESERV") {
					global::to_Exclude.push_back(exclutions[a].obj.getObject().getString());
				}
			}
			tf._Manual_Enable();
			tf._Registered_Enter_Hit();
		}
	}

};
