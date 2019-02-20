#include "Settings_GUI.h"
#include "Global_Variables.h"

Settings_GUI::Settings_GUI() {
	cursor.loadFromSystem(sf::Cursor::Text);
	standard.loadFromSystem(sf::Cursor::Arrow);

	if (!font.loadFromFile("c:\\windows\\fonts\\arial.ttf")) {
		printf("could not loat font...\n");
	}
	/*
	Passes the font object to all of the UI elements,
	so you dont need to allocate and load in a font object
	to every element
	*/
	loggin.give_Font(&font);
	t_prefered.give_Font(&font);
	b_prefered.give_Font(&font);
	t_exclution.give_Font(&font);
	b_exclution.give_Font(&font);
	t_rename.give_Font(&font);
	b_rename.give_Font(&font);
	p_list.give_Font(&font);
	entry.give_Remote_Variables(&window, &mouse);
	entry.give_font(&font);
	c_rename.give_font(&font);
	c_rename.give_Remote_Variables(&window, &mouse);
	/**/
	loggin.gui_easy_setup("Enable Logging", 24, sf::Color(255,255,255));
	loggin.setPos(width / 2 - (loggin.getObject().getGlobalBounds().width), 100);
	loggin_c.setPos(loggin.getObject().getPosition().x + (loggin.getObject().getGlobalBounds().width + 100), loggin.getObject().getPosition().y + 17);
	t_prefered.gui_easy_setup("Prefered list", 24, sf::Color(255, 255, 255));
	t_prefered.setPos(loggin.getObject().getPosition().x, loggin.getObject().getPosition().y + 50);
	b_prefered.gui_easy_setup("...", 24, sf::Color(90, 90, 90), -5);
	b_prefered.setPos(t_prefered.getObject().getPosition().x + (t_prefered.getObject().getGlobalBounds().width + 120) - 2, t_prefered.getObject().getPosition().y + 10);
	t_exclution.gui_easy_setup("Exclusion", 24, sf::Color(255, 255, 255));
	t_exclution.setPos(t_prefered.getObject().getPosition().x, t_prefered.getObject().getPosition().y + 50);
	t_rename.gui_easy_setup("Rename", 24, sf::Color(255, 255, 255));
	t_rename.setPos(t_exclution.getObject().getPosition().x, t_exclution.getObject().getPosition().y + 50);
	b_exclution.gui_easy_setup("...", 24, sf::Color(90, 90, 90), -5);
	b_exclution.setPos(t_exclution.getObject().getPosition().x + (t_exclution.getObject().getGlobalBounds().width + 150) + 3, t_exclution.getObject().getPosition().y + 10);
	b_rename.gui_easy_setup("...", 24, sf::Color(90, 90, 90), -5);
	b_rename.setPos(t_rename.getObject().getPosition().x + (t_rename.getObject().getGlobalBounds().width + 150) + 10, t_rename.getObject().getPosition().y + 10);
	p_list._ERL(true);
}

void Settings_GUI::Launch_GUI() {
	loggin_c.set_Default(global::enable_logging);
	opentest = true;
	window.create(sf::VideoMode(width, height), "SETTINGS", sf::Style::Close);
	window.setFramerateLimit(30);
	window.setActive(false);
	original_Pos = window.getPosition();
}

void Settings_GUI::_Draw_and_Check_Poll() {
	//HOVER
	if (window.isOpen()) {
		b_prefered.Check_Hover(&mouse, &window);
		b_exclution.Check_Hover(&mouse, &window);
		p_list.Check_Hover(&mouse, &window);
#ifdef ENABLE_RENAME_SYSTEM
		b_rename.Check_Hover(&mouse, &window);
		c_rename.Check_Hover(&mouse, &window);
#endif
		sf::Event event;
		while (window.pollEvent(event)) {
			p_list._Text_Enter_Check(&event);
			entry._Text_Enter_Check(&event);
#ifdef ENABLE_RENAME_SYSTEM
			c_rename._Text_Enter_Check(&event);
#endif
			if (entry.Check_Hover(&mouse, &window)) {
				window.setMouseCursor(cursor);
			}
			else {
				window.setMouseCursor(standard);
			}
			switch (event.type) {
			case sf::Event::Closed:
				if (_render_mode != DEFAULT)
					needs_update = true;
				opentest = false;
				window.close();
				break;
				
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left) {
					if (_render_mode == ENTRY && entry._Look_for_Mouse_Press()) {
						entry.Disable_Writing();
						needs_update = true;
						window.setSize(sf::Vector2u(width, height));
						window.setPosition(original_Pos);
						window.setView(sf::View(sf::FloatRect(0.f, 0.f, (float)width, (float)height)));
						_render_mode = DEFAULT;
					}

					if (_render_mode == RENAME && c_rename._Look_for_Mouse_Press()) {
						c_rename.Disable_Writing();
						needs_update = true;
						_render_mode = DEFAULT;
					}
						
					if (_render_mode == PREFERED_LIST && p_list._Look_for_Mouse_Press(&mouse, &window)) {
						needs_update = true;
						window.setSize(sf::Vector2u(width, height));
						window.setPosition(original_Pos);
						window.setView(sf::View(sf::FloatRect(0.f, 0.f, (float)width, (float)height)));
						_render_mode = DEFAULT;
					}

					if (_render_mode == DEFAULT) {
						bool old = global::enable_logging;
						global::enable_logging = loggin_c._Look_for_Mouse_Press(&mouse, &window);
						if (old != global::enable_logging)
							needs_update = true;
						if (b_prefered._Look_for_Mouse_Press(&mouse, &window)) {
							window.setSize(sf::Vector2u(800, 200));
							original_Pos = window.getPosition();
							window.setPosition(sf::Vector2i(window.getPosition().x - ((800 - width) / 2), window.getPosition().y - ((200 - height) / 2)));
							window.setView(sf::View(sf::FloatRect(0.f, 0.f, 800, 200)));
							_render_mode = PREFERED_LIST;
						}
						if (b_exclution._Look_for_Mouse_Press(&mouse, &window)) {
							_render_mode = ENTRY;
							entry.Enable_Writing();
						}
#ifdef ENABLE_RENAME_SYSTEM
						if (b_rename._Look_for_Mouse_Press(&mouse, &window)) {
							_render_mode = RENAME;
							c_rename.Enable_Writing();
						}
#endif
					}
				}
				break;
			}
		}
		window.clear(sf::Color(100,100,100));
		if (_render_mode == DEFAULT) {
			loggin_c.draw(&window);
			loggin.draw(&window);
			t_prefered.draw(&window);
			b_prefered.draw(&window);
			t_exclution.draw(&window);
			b_exclution.draw(&window);
#ifdef ENABLE_RENAME_SYSTEM
			t_rename.draw(&window);
			b_rename.draw(&window);
#endif
		}
		else if (_render_mode == PREFERED_LIST) {
			p_list.draw(&window);
		}
		else if (_render_mode == ENTRY) {
			entry.draw(&window);
		}
		else if (_render_mode == RENAME) {
			c_rename.draw(&window);
		}
		window.display();
	}
}
