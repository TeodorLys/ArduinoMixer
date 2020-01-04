#pragma once
#include "Settings_OGUI.h"
#include "Settings_GUI.h"
#include <Windows.h>


#define AM_EXIT WM_APP + 1
#define AM_UPDATES WM_APP + 2
#define AM_WAITING WM_APP + 3
#define AM_SETTINGS WM_APP + 4
#define AM_TOGGLE WM_APP + 5
#define AM_TOGGLE_VOLUME WM_APP + 6

class Load_Externals;
class Network_Functionality;
class device_IO;

class System_Tray {
private:
	HWND window;
	HINSTANCE hinst;
	std::string tip;
	static device_IO *io;
	static Network_Functionality *nf;
	static Load_Externals *load;
	static NOTIFYICONDATA notify;
	static Settings_GUI	 *gui;
	static Settings_OGUI *new_gui;
	static bool new_g;
public:
	System_Tray(Settings_GUI *g);
	System_Tray(Settings_OGUI *g);
	void Start();
	void Set_Version(std::string ver) { tip = "ARDUINO MIXER PROTOTYPE v." + ver; 	}
	~System_Tray();
private:
	static LRESULT CALLBACK OnEvent(HWND Handle, UINT message, WPARAM wParam, LPARAM lParam);
	static void ShowContextWindow(HWND hwnd);
	void Register_System_Tray_Icon(HWND hwnd);

public:
	static void exit_sequence();
	NOTIFYICONDATA *Get_Notify() const { return &notify; }
};

