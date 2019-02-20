#pragma once
#include <SFML/Graphics.hpp>


class GUI_Button {
private:
	sf::RectangleShape b;
	sf::Text text;
	sf::Color save_Background;
	float yoffset = 0.f;
public:
	GUI_Button() {
	}

	void give_Font(sf::Font *f) { text.setFont(*f); }

	void gui_easy_setup(std::string s, int char_size, sf::Color c, float off = 0.f) {
		text.setString(s);
		text.setCharacterSize(char_size);
		
		b.setSize(sf::Vector2f(text.getGlobalBounds().width + 20, text.getGlobalBounds().height + 20));
		save_Background = c;
		b.setFillColor(c);
		b.setOutlineThickness(2);
		b.setOutlineColor(sf::Color(100, 120, 150));
		yoffset = off;
	}

	void setPos(float x, float y) {
		b.setPosition(x, y);
		text.setPosition(b.getPosition().x + (b.getSize().x / 2) - (text.getGlobalBounds().width / 2), b.getPosition().y + (text.getGlobalBounds().height / 2) - (5 - yoffset));
	}

	sf::RectangleShape getObject() const { return b; }

	void draw(sf::RenderWindow *w) {
		w->draw(b);
		w->draw(text);
	}

	void Check_Hover(sf::Mouse *m, sf::RenderWindow *w) {
		if (b.getGlobalBounds().contains(sf::Vector2f(m->getPosition(*w)))) {
			b.setFillColor(sf::Color(50, 50, 50));
		}
		else {
			b.setFillColor(save_Background);
		}
	}

	bool _Look_for_Mouse_Press(sf::Mouse *m, sf::RenderWindow *w) {
		if (b.getGlobalBounds().contains(sf::Vector2f(m->getPosition(*w)))) {
			return true;
		}
		else {
			return false;
		}
	}

};
