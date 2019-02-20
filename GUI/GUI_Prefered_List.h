#pragma once
#include <SFML/Graphics.hpp>
#include "GUI_Button.h"
#include "GUI_Text_Field.h"
#include "Global_Variables.h"

class GUI_Prefered_List {
private:
	sf::Text page;
	sf::Font font;
	sf::RectangleShape screen;
	sf::CircleShape page_Buttons[2];
	int current_Page = 1;
	GUI_Text_Field tf[4];
	GUI_Button back;
	std::vector<std::string> prefered_List = std::vector<std::string>(36);
	std::vector<reserved> reserv;
public:
	GUI_Prefered_List();
	void Page_Text_Formatting(int mode);
	void _Text_Enter_Check(sf::Event *e);
	int _Look_for_Mouse_Press(sf::Mouse *m, sf::RenderWindow *w);

public:
	void give_Font(sf::Font *f) { 
		page.setFont(*f); 
		for (int a = 0; a < 4; a++)
			tf[a].give_Font(f);
		back.give_Font(f);
		back.gui_easy_setup("BACK", 18, sf::Color(80, 80, 80), 5);
		page.setPosition((800 / 2) - (page.getGlobalBounds().width / 2), screen.getPosition().y + 10);
		back.setPos(10, 10);

	}

	void Check_Hover(sf::Mouse *m, sf::RenderWindow *w) {
		back.Check_Hover(m, w);
	}

	void draw(sf::RenderWindow *w) {
		w->draw(screen);
		w->draw(page_Buttons[0]);
		w->draw(page_Buttons[1]);
		w->draw(page);
		back.draw(w);
		for (int a = 0; a < 4; a++)
			tf[a].draw(w);
	}

	std::vector<std::string> Extract_Prefered_List() {
		return prefered_List;
	}

	std::vector<reserved> _ERL(bool mode = false) {
		if(mode)
			Page_Text_Formatting(3);
		return reserv;
	}

private:
	bool _Already_Included(std::string name) {
		for (std::string s : prefered_List) {
			if (name == s)
				return true;
		}
		return false;
	}
};
