#pragma once
#include <string>
#include <SFML/Graphics.hpp>
class GUI_Text_Field {
private:
	sf::Text write;
	sf::RectangleShape to_Write__in;
	sf::RectangleShape _write_cursor;
	sf::RenderWindow *window;
	std::string written = "UNUSED";
	std::string s_default = "UNUSED";
	sf::Clock clock;
	bool is_Writing = false;
	bool tab_Hit = false;
	bool enter_hit;
	bool _frame = false;
	const std::string _limit_List = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ1234567890+,.-<>'";
public:
	GUI_Text_Field();
	bool Value_Field(sf::Event *e);
	void draw(sf::RenderWindow *w);
	void _Look_for_Mouse_Press(sf::Mouse *m, sf::RenderWindow *w);

public:
	void give_Font(sf::Font *f) { write.setFont(*f); }

	void Enable_Frame() {
		_frame = true;
	}

	void Set_Frame_Size(int x, int y) {
		to_Write__in.setSize(sf::Vector2f((float)x, (float)y));
	}

	bool Check_Hover(sf::Mouse *m, sf::RenderWindow *w) {
		return (to_Write__in.getGlobalBounds().contains(sf::Vector2f(m->getPosition(*w))));
	}

	bool get_Tab_Hit() const { return tab_Hit; }
	bool get_Is_Writing() const { return is_Writing; }
	bool get_Enter_Hit() const { return enter_hit; }
	std::string Extract_Written_String() const { return written; }
	void _Registered_Tab_Hit() { tab_Hit = false; }
	void _Registered_Enter_Hit() { enter_hit = false; }


	void _Manual_Enable() {
		is_Writing = true;
		written = "";
		write.setString(written);
		_line_cursor_pos();
	}

	void _Manual_Disable(bool reset = false) {
		if (written.size() == 0) {
			written = s_default;
			write.setString(written);
		}
		if (reset) {
			written = "UNUSED";
			write.setString(written);
		}
		is_Writing = false;
	}

	void Set_Default_String(std::string s) {
		written = s;
		s_default = s;
		write.setString(written);
		_line_cursor_pos();
	}

	void set_Pos(float x, float y) {
		write.setPosition(x, y);
		to_Write__in.setPosition(write.getPosition().x - 11, write.getPosition().y + 2);
	}

private:
	void _line_cursor_pos() {
		_write_cursor.setPosition(sf::Vector2f(write.getPosition().x + write.getLocalBounds().width, write.getPosition().y + 5));
	}
};
