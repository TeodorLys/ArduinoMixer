#pragma once
#include "Settings_GUI.h"
#include <Windows.h>

#define AM_EXIT WM_APP + 1
#define AM_UPDATES WM_APP + 2
#define AM_WAITING WM_APP + 3
#define AM_SETTINGS WM_APP + 4
#define AM_TOGGLE WM_APP + 5

class Load_Externals;
class Network_Functionality;
class Arduino_Parse;

class System_Tray {
private:
	HWND window;
	HINSTANCE hinst;
	std::string tip;
	static Arduino_Parse *ap;
	static Network_Functionality *nf;
	static Load_Externals *load;
	static NOTIFYICONDATA notify;
	static std::unique_ptr<Settings_GUI> *gui;
public:
	System_Tray(Network_Functionality *net, Arduino_Parse *ard, Load_Externals *ext, std::unique_ptr<Settings_GUI> *g);
	void Start();
	void Set_Version(std::string ver) { tip = "ARDUINO MIXER PROTOTYPE v." + ver; 	}
	~System_Tray();
private:
	static LRESULT CALLBACK OnEvent(HWND Handle, UINT message, WPARAM wParam, LPARAM lParam);
	static void ShowContextWindow(HWND hwnd);
	void Register_System_Tray_Icon(HWND hwnd);

public:
	NOTIFYICONDATA *Get_Notify() const { return &notify; }
};

