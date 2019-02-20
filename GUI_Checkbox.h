#pragma once
#include <SFML/Graphics.hpp>

class GUI_Checkbox {
private:
	bool draw_Cross = false;
public:
	sf::RectangleShape box;
	sf::RectangleShape cross[2];

	GUI_Checkbox(bool b_default = false) {
		draw_Cross = b_default;
		cross[0].setSize(sf::Vector2f(5, 20));
		cross[0].rotate(45);
		cross[0].setOrigin(cross[0].getSize().x / 2, cross[0].getSize().y / 2);
		cross[1].setSize(sf::Vector2f(5, 20));
		cross[1].rotate(315);
		cross[1].setOrigin(cross[1].getSize().x / 2, cross[1].getSize().y / 2);

		box.setSize(sf::Vector2f(20, 20));
		box.setFillColor(sf::Color(100, 100, 100));
		box.setOutlineThickness(2);
		box.setOutlineColor(sf::Color(220, 220, 220));
		box.setOrigin(box.getSize().x / 2, box.getSize().y / 2);

	}

	void setPos(float x, float y) {
		box.setPosition(x, y);
		cross[0].setPosition(box.getPosition());
		cross[1].setPosition(box.getPosition());
	}

	bool _Look_for_Mouse_Press(sf::Mouse *m, sf::RenderWindow *window) {
		if (box.getGlobalBounds().contains(sf::Vector2f(m->getPosition(*window)))) {
			draw_Cross = !draw_Cross;
		}
		return draw_Cross;
	}

	void draw(sf::RenderWindow* w) {
		w->draw(box);
		if (draw_Cross) {
			w->draw(cross[0]);
			w->draw(cross[1]);
		}
	}

	void set_Default(bool b_default) { draw_Cross = b_default; }

};