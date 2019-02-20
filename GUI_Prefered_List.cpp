#include "GUI_Prefered_List.h"

GUI_Prefered_List::GUI_Prefered_List() {
	printf("give\n");
	prefered_List = global::pref;
	reserv = global::reserved_List;
	tf[0].set_Pos(60, 100);
	tf[1].set_Pos(235, 100);
	tf[2].set_Pos(405, 100);
	tf[3].set_Pos(580, 100);


	for (int a = 0; a < 4; a++)
		tf[a].Set_Default_String(prefered_List[a]);

	screen.setFillColor(sf::Color(60, 83, 102));
	screen.setSize(sf::Vector2f(700, 100));
	screen.setPosition(50, 50);

	page.setString("PAGE 1/9");
	page.setCharacterSize(24);
	page.setStyle(sf::Text::Style::Bold);
	page.setPosition((800 / 2) - (page.getGlobalBounds().width / 2), screen.getPosition().y + 10);

	page_Buttons[0].setFillColor(sf::Color(102, 20, 20));
	page_Buttons[1].setFillColor(sf::Color(102, 20, 20));
	page_Buttons[0].setRadius(20);
	page_Buttons[1].setRadius(20);
	page_Buttons[0].setPosition(5, 80);
	page_Buttons[1].setPosition(755, 80);
}

void GUI_Prefered_List::Page_Text_Formatting(int mode) {
	//Save the current display text
	for (int a = 0; a < 4; a++) {
		if (tf[a].Extract_Written_String() != "UNUSED" && tf[a].Extract_Written_String() != "RESERV" && !_Already_Included(tf[a].Extract_Written_String())) {
			bool found = false;
			for (reserved r : reserv)
				if (r.index == ((current_Page - 1) * 4) + a)
					found = true;
			if (!found)
				reserv.push_back(reserved(((current_Page - 1) * 4) + a, tf[a].Extract_Written_String()));
		}
		/*
		Checks if any of the reservations was removed,
		and removes it from the reserv vector
		*/
		else if (tf[a].Extract_Written_String() == "UNUSED") {
			for (int b = 0; b < (int)reserv.size(); b++) {
				
				if (reserv[b].index == ((current_Page - 1) * 4) + a) {
					printf("removed, %i... %i\n", reserv[b].index, ((current_Page - 1) * 4) + a);
					reserv.erase(reserv.begin() + b);
					break;
				}
			}
		}
		if (!_Already_Included(tf[a].Extract_Written_String()) || tf[a].Extract_Written_String() == "UNUSED") {
			prefered_List[((current_Page - 1) * 4) + a] = tf[a].Extract_Written_String();
		}
	}
	if (mode == 1) {
		if (current_Page < 9)
			current_Page++;
		else if (current_Page == 9)
			current_Page = 1;
	}
	else if (!mode) {
		if (current_Page > 1)
			current_Page--;
		else if (current_Page == 1)
			current_Page = 9;
	}

	for (int a = 0; a < 4; a++) {
		tf[a].Set_Default_String(prefered_List[((current_Page - 1) * 4) + a]);
	}

	std::string temp = "PAGE ";
	temp += std::to_string(current_Page) + "/9";
	page.setString(temp);
}

void GUI_Prefered_List::_Text_Enter_Check(sf::Event *e) {
	int active = -1;
	for (int a = 0; a < 4; a++) {
		tf[a].Value_Field(e);
		if (tf[a].get_Tab_Hit()) {
			if (a + 1 != 4) {
				tf[a + 1]._Manual_Enable();
				tf[a]._Registered_Tab_Hit();
				active = a + 1;
				break;
			}
			else if (a + 1 == 4) {
				Page_Text_Formatting(1);
				tf[0]._Manual_Enable();
				tf[a]._Registered_Tab_Hit();
				active = 0;
				break;
			}
		}
	}
}

int GUI_Prefered_List::_Look_for_Mouse_Press(sf::Mouse *m, sf::RenderWindow *w) {
	for (int a = 0; a < 2; a++) {
		if (page_Buttons[a].getGlobalBounds().contains(sf::Vector2f(m->getPosition(*w)))) {
			Page_Text_Formatting(a);
		}
	}

	if (back._Look_for_Mouse_Press(m, w)) {
		Page_Text_Formatting(3);
		return 1;
	}

	for (int a = 0; a < 4; a++)
		tf[a]._Look_for_Mouse_Press(m, w);
	return 0;
}