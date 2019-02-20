#pragma once
#include <SFML/Graphics.hpp>
#include "GUI_Prefered_List.h"
#include "GUI_Text_Field.h"
#include "GUI_Textbox.h"
#include "GUI_Button.h"
#include "Global_Variables.h"

class GUI_Rename_Session {
private:
	GUI_Text_Field tf;
	GUI_Text_Field new_tf;
	sf::RenderWindow *window;
	sf::Mouse *mouse;
	entry_object buff;
	GUI_Textbox title;
	GUI_Textbox new_name, old_name;
	GUI_Button back;
	std::vector<entry_object> renames;
public:
	GUI_Rename_Session() {
		title.gui_easy_setup("ADDED", 24, sf::Color(255, 255, 255));
		new_name.gui_easy_setup("OLD NAME", 24, sf::Color(255, 255, 255));
		old_name.gui_easy_setup("NEW NAME", 24, sf::Color(255, 255, 255));
		buff.gui_easy_setup("UNUSED", 36, sf::Color(255, 255, 255));
	}

	int _Look_for_Mouse_Press() {
		tf._Look_for_Mouse_Press(mouse, window);
		new_tf._Look_for_Mouse_Press(mouse, window);

		if (tf.get_Enter_Hit()) {
			tf._Manual_Disable();
			tf._Manual_Enable();
			tf._Registered_Enter_Hit();
		}
		else if (new_tf.get_Enter_Hit()) {
			new_tf._Manual_Disable();
			tf._Manual_Enable();
			new_tf._Registered_Enter_Hit();
		}

		for (int a = 0; a < (int)renames.size(); a++) {
			if (renames[a]._pressed(window, mouse)) {
				for (int b = 0; b < (int)global::renamed.size(); b++) {
					if (renames[a].obj.getObject().getString().toAnsiString().find(global::renamed[b].default_name) != std::string::npos) {
						printf("%s\n", global::renamed[a].default_name.c_str());
						global::renamed.erase(global::renamed.begin() + b);
					}

					if (renames[a].obj.getObject().getString().toAnsiString().find(global::rename_list[b]) != std::string::npos) {
						printf("%s\n", global::rename_list[a].c_str());
						global::rename_list.erase(global::rename_list.begin() + b);
					}

				}

				for (rename_session rs : global::renamed) {
					printf("def: %s, newn: %s\n", rs.default_name.c_str(), rs.new_name.c_str());
				}

				for (std::string s : global::rename_list) {
					printf("saveable %s\n", s.c_str());
				}

				renames.erase(renames.begin() + a);
				for (int b = 0; b < (int)renames.size(); b++) {
					if (b == 0)
						renames[b].setPos(50, 40);
					else
						renames[b].setPos(50, renames[b - 1].obj.getPos().y + 36);
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
		new_tf.Check_Hover(m, w);
		return tf.Check_Hover(m, w);
	}

	void give_Remote_Variables(sf::RenderWindow *w, sf::Mouse *m) {
		window = w;
		mouse = m;
		title.setPos(180, 0);
		old_name.setPos(220, 460);
		new_name.setPos(20, 460);
		tf.set_Pos(20, 500);
		tf.Enable_Frame();
		tf.Set_Frame_Size(180, 42);
		new_tf.set_Pos(220, 500);
		new_tf.Enable_Frame();
		new_tf.Set_Frame_Size(180, 42);
	}

	void give_font(sf::Font *f) {
		tf.give_Font(f);
		new_tf.give_Font(f);
		buff.give_Font(f);
		title.give_Font(f);
		new_name.give_Font(f);
		old_name.give_Font(f);
		back.give_Font(f);
		back.gui_easy_setup("BACK", 18, sf::Color(80, 80, 80), 5);
		back.setPos(10, 10);

		for (int a = 0; a < (int)global::rename_list.size(); a++) {
			buff.obj.setString(global::rename_list[a]);
			if (renames.size() == 0) {
				buff.setPos(50, 40);
			}
			else {
				buff.setPos(50, renames[renames.size() - 1].obj.getPos().y + 36);
			}
			renames.push_back(buff);
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
		new_tf.draw(w);
		title.draw(w);
		new_name.draw(w);
		old_name.draw(w);
		for (int a = 0; a < (int)renames.size(); a++) {
			renames[a].draw(w);
		}
		back.draw(w);
	}

	void _Text_Enter_Check(sf::Event *e) {
		tf.Value_Field(e);
		new_tf.Value_Field(e);
		if (new_tf.get_Enter_Hit() && new_tf.Extract_Written_String() != "UNUSED" && tf.Extract_Written_String() != "UNUSED") {
			if (renames.size() == 0) {
				buff.setPos(50, 40);
			}
			else {
				buff.setPos(50, renames[renames.size() - 1].obj.getPos().y + 36);
			}
			buff.obj.setString(tf.Extract_Written_String() + "->" + new_tf.Extract_Written_String());
			renames.push_back(buff);

			if (buff.obj.getObject().getString().toAnsiString().find("UNUSED") == std::string::npos && buff.obj.getObject().getString().toAnsiString().find("RESERV") == sf::String::InvalidPos) {
				global::rename_list.push_back(buff.obj.getObject().getString());
				global::renamed.push_back(rename_session(tf.Extract_Written_String(), new_tf.Extract_Written_String()));
			}

			tf._Manual_Enable();
			new_tf._Manual_Disable(true);
			new_tf._Registered_Enter_Hit();
		}
		else if (tf.get_Tab_Hit() || tf.get_Enter_Hit()) {
			tf._Manual_Disable();
			new_tf._Manual_Enable();
			tf._Registered_Tab_Hit();
			tf._Registered_Enter_Hit();
		}
	}

};
