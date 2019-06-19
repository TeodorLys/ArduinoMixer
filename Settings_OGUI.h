#pragma once
#include <SFML/System/Thread.hpp>
#include <OGUI_Window/window_handle.h>
#include <OGUI_Graphics/render_mode.h>
#include <OGUI_Graphics/check_box.h>
#include <mutex>
#include <atomic>


class OGUI_Main_Screen;
class OGUI_Prefered_List;
class OGUI_Rename;
class OGUI_Exclusion;

class Settings_OGUI {
private:
	ogui::render_mode *mode;
	ogui::window_handle *window;
	std::atomic<bool> opentest = false;
	sf::Thread *thread;
	OGUI_Main_Screen *main;
	OGUI_Prefered_List *list;
	OGUI_Rename *rename;
	OGUI_Exclusion *exclude;
	ogui::check_box c;
	mutable std::mutex mut;
	bool needs_update = false;
	bool last_logging_state = false;
public:

	Settings_OGUI();

	~Settings_OGUI();

	bool get_Open() const { std::lock_guard<std::mutex> l(mut); return opentest; }
	void Launch_GUI();
	void _Draw_and_Check_Poll();

public:
	void terminate_thread();

	bool Get_Needs_Update() const {
		std::lock_guard<std::mutex> l(mut);
		return needs_update;
	}

	void _confirm_Update_Executed() {
		std::lock_guard<std::mutex> l(mut);
		needs_update = false;
	}

	HWND get_Handle() {
		std::lock_guard<std::mutex> l(mut);
		return *window->get_system_handle();
	}

};

