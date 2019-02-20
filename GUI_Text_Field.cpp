#include "GUI_Text_Field.h"



GUI_Text_Field::GUI_Text_Field() {
	write.setFillColor(sf::Color(255, 255, 255));
	write.setCharacterSize(36);
	write.setStyle(sf::Text::Bold);
	write.setString(written);
	to_Write__in.setSize(sf::Vector2f(180, 42));
	to_Write__in.setFillColor(sf::Color(50, 50, 50));
	to_Write__in.setOutlineThickness(5);
	to_Write__in.setOutlineColor(sf::Color(0, 0, 0));
	_write_cursor.setSize(sf::Vector2f(2, 32));
	_write_cursor.setFillColor(sf::Color(255, 255, 255, 0));
}

void GUI_Text_Field::_Look_for_Mouse_Press(sf::Mouse *m, sf::RenderWindow *w) {
	if (to_Write__in.getGlobalBounds().contains(sf::Vector2f(m->getPosition(*w)))) {
		is_Writing = true;
		written = "";
		write.setString(written);
	}
	else {
		if (written.size() == 0) {
			written = s_default;
			write.setString(written);
		}
		is_Writing = false;
	}
	_line_cursor_pos();
}

bool GUI_Text_Field::Value_Field(sf::Event *e) {
	if (e->type == sf::Event::TextEntered  && is_Writing) {
		switch (e->key.code) {
		case 8:   // BACKSPACE
			if (written.size() > 0) {
				written.pop_back();
			}
			break;
		case 13:  //RETURN / ENTER
			if (written.size() == 0) {
				written = s_default;
			}
			enter_hit = true;
			is_Writing = false;
			break;
		case 27:  // Escape
			written = s_default;
			is_Writing = false;
			break;

		case 9:   // TAB
			if (is_Writing) {
				is_Writing = false;
				tab_Hit = true;
				if (written == "")
					written = "UNUSED";
				return false;
			}
			break;

		default:
			if (_limit_List.find(e->text.unicode) != std::string::npos && written.size() < 6) {
				written.push_back(toupper(e->text.unicode));
			}
			break;
		}
		write.setString(written);
		_line_cursor_pos();
		return true;
	}
	return true;
}

void GUI_Text_Field::draw(sf::RenderWindow *w) {
	if (is_Writing && clock.getElapsedTime().asMilliseconds() >= 500) {
		if (_write_cursor.getFillColor().a == 255) {
			_write_cursor.setFillColor(sf::Color(255, 255, 255, 0));
		}
		else {
			_write_cursor.setFillColor(sf::Color(255, 255, 255, 255));
		}
		clock.restart();
	}
	else if (!is_Writing) {
		_write_cursor.setFillColor(sf::Color(255, 255, 255, 0));
	}

	if (_frame)
		w->draw(to_Write__in);
	w->draw(write);
	w->draw(_write_cursor);
}