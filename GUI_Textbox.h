#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class GUI_Textbox {
private:
	sf::Text t;
public:
	GUI_Textbox() {
	}

	void give_Font(sf::Font *f) { t.setFont(*f); }

	void gui_easy_setup(std::string s, int char_size, sf::Color c) {
		t.setString(s);
		t.setCharacterSize(char_size);
		t.setFillColor(c);
	}

	void setPos(float x, float y) {
		t.setPosition(x, y);
	}

	sf::Vector2f getPos() { return t.getPosition(); }

	sf::Text getObject() const { return t; }

	void setString(std::string s) { t.setString(s); }

	void draw(sf::RenderWindow *w) {
		w->draw(t);
	}
};