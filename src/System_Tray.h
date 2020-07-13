#pragma once
#include "Settings_OGUI.h"
#include "Settings_GUI.h"
#include "initialize_device.h"
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
	/*
	These are static because the event function needs to static...
	*/
	static initialize_device io_init;
	static device_IO *io;
	static Network_Functionality *nf;
	static Load_Externals *load;
	static NOTIFYICONDATA notify;
	static Settings_GUI	 *gui;
	static Settings_OGUI *new_gui;
	static bool new_g;
public:
	System_Tray(Settings_GUI *g); /*@DEPRECATED!*/
	System_Tray(Settings_OGUI *g);
	/*
	Initilize the system tray window.
	*/
	void Start();
	/*
	MAYBE TODO: do this in the start function. ^
	*/
	void Set_Version(std::string ver) { tip = "ARDUINO MIXER PROTOTYPE v." + ver; 	}
	~System_Tray();
private:
	static LRESULT CALLBACK OnEvent(HWND Handle, UINT message, WPARAM wParam, LPARAM lParam);
	/*
	Right-click on the icon in the system tray.
	*/
	static void ShowContextWindow(HWND hwnd);
	/*
	Assign a icon to this program. Same icon as used in the OGUI functions.
	*/
	void Register_System_Tray_Icon(HWND hwnd);

public:
	/*
	removes the icon from system tray.
	*/
	static void exit_sequence();
	/*
	For the ballontip class.
	*/
	NOTIFYICONDATA *Get_Notify() const { return &notify; }
};

