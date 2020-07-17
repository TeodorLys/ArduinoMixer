#pragma once
#include "Settings_WPF.h"
#include "initialize_device.h"
#include "update_handler.h"
#include <Windows.h>


#define AM_EXIT WM_APP + 1
#define AM_UPDATES WM_APP + 2
#define AM_WAITING WM_APP + 3
#define AM_SETTINGS WM_APP + 4
#define AM_TOGGLE WM_APP + 5
#define AM_TOGGLE_VOLUME WM_APP + 6

class settings_handler;
class network_handler;
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
	static update_handler *_update;
	static settings_handler *load;
	static NOTIFYICONDATA notify;
	static Settings_WPF* wpf;
	static bool new_g;
public:
	System_Tray(Settings_WPF* g);
	/*
	Initilize the system tray window.
	*/
	void Start();
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

