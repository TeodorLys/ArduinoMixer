#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <Windows.h>
#include "GUI_Checkbox.h"
#include "GUI_Textbox.h"
#include "GUI_Button.h"
#include "GUI_Prefered_List.h"
#include "GUI_Entry.h"
#include "GUI_Rename_Session.h"

#define DEFAULT 1
#define PREFERED_LIST 2
#define ENTRY 3
#define RENAME 4

#define ENABLE_RENAME_SYSTEM

class Settings_GUI {
private:
	sf::RenderWindow window;
	sf::Font font;
	GUI_Checkbox loggin_c;
	GUI_Textbox loggin;
	GUI_Textbox t_prefered;
	GUI_Button b_prefered;
	GUI_Button b_exclution;
	GUI_Button b_rename;
	GUI_Textbox t_exclution;
	GUI_Textbox t_rename;
	GUI_Prefered_List p_list;
	GUI_Entry entry;
	GUI_Rename_Session c_rename;
	sf::Vector2i original_Pos;
	sf::Mouse mouse;
	sf::Cursor cursor, standard;
	int _render_mode = DEFAULT;
	const int width = 400;
	const int height = 600;
	bool opentest = false;
	bool needs_update = false;
public:
	bool get_Open() const { return opentest; }
	Settings_GUI();
	void Launch_GUI();
	void _Draw_and_Check_Poll();
	bool Get_Needs_Update() const {
		return needs_update;
	}
	void _confirm_Update_Executed() {
		needs_update = false;
	}
	HWND get_Handle() {
		return window.getSystemHandle();
	}
	std::vector<std::string> Extract_Prefered_List_cont() {
		return p_list.Extract_Prefered_List();
	}

	std::vector<reserved> Extract_Reserved_List() {
		return p_list._ERL();
	}
};

