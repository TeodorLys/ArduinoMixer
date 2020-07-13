#include "System_Tray.h"
#include "Global_Variables.h"
#include "BalloonTip.h"
#include "resource.h"
#include "Network_Functionality.h"
#include "Load_Externals.h"
#include "device_IO.h"
#include "crash_logger.h"

#include <Dbt.h>

initialize_device System_Tray::io_init;
Network_Functionality *System_Tray::nf = nullptr;
device_IO *System_Tray::io = nullptr;
Load_Externals *System_Tray::load = nullptr;
Settings_GUI *System_Tray::gui = nullptr;
Settings_OGUI *System_Tray::new_gui = nullptr;
NOTIFYICONDATA System_Tray::notify;
bool System_Tray::new_g = false;

LRESULT CALLBACK System_Tray::OnEvent(HWND Handle, UINT message, WPARAM wParam, LPARAM lParam) {
	INT wmid, wmEvent;

	switch (message) {
	case WM_DEVICECHANGE:
		io->check_for_disconnection();
		if(!io->_Is_Connected())
			io_init.reenumeration();
		break;

	case WM_QUERYENDSESSION:
		if (lParam == 0) {
			PostQuitMessage(0);
			Shell_NotifyIcon(NIM_DELETE, &notify);
			io->Dehook_from_mixer();
		}

		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		Shell_NotifyIcon(NIM_DELETE, &notify);
		io->Dehook_from_mixer();
		break;

	case WM_APP:
		switch (lParam) {
			//If you click the system tray icon
		case WM_RBUTTONDOWN: case WM_LBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextWindow(Handle);
		break;
		case NIN_BALLOONUSERCLICK:
			if (BalloonTip::_Get_Identifier() == 1) {
				nf->Download_Updates();
				BalloonTip::_Set_Identifier(0);
			}
			break;
		}
		break;

	case WM_COMMAND:
		wmid = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Button events for when you click the system tray icon
		switch (wmid) {
		case AM_EXIT:
			if(new_gui->get_Open())
				new_gui->terminate_thread();
			PostQuitMessage(0);
			Shell_NotifyIcon(NIM_DELETE, &notify);
			io->Dehook_from_mixer();
			return 0;

		case AM_TOGGLE:
			io->Arduino_Display_Toggle();
			break;

		case AM_TOGGLE_VOLUME:
			io->Toggle_Volume_Procentage();
			break;

		case AM_UPDATES:
			if (nf->Check_For_Updates(false))
				load->Update_Save_File();
			break;

		case AM_WAITING:
			nf->Download_Updates();
			break;
		case AM_SETTINGS:
			if(!new_g)
				gui->Launch_GUI();
			else 
				new_gui->Launch_GUI();
			break;

		}
		return 1;
	}

	return DefWindowProc(Handle, message, wParam, lParam);
}

void System_Tray::exit_sequence() {
	if (new_gui->get_Open())
		new_gui->terminate_thread();
	PostQuitMessage(0);
	Shell_NotifyIcon(NIM_DELETE, &notify);
	io->Dehook_from_mixer();
}

void System_Tray::ShowContextWindow(HWND hwnd) {
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu) {
		InsertMenu(hMenu, -1, MF_BYPOSITION, AM_SETTINGS, "SETTINGS");
		if (!global::waiting_Update)
			InsertMenu(hMenu, -1, MF_BYPOSITION, AM_UPDATES, "CHECK FOR UPDATES");
		else
			InsertMenu(hMenu, -1, MF_BYPOSITION, AM_WAITING, "INSTALL UPDATES");
		InsertMenu(hMenu, -1, MF_BYPOSITION, AM_TOGGLE, "TOGGLE DISPLAY");
		InsertMenu(hMenu, -1, MF_BYPOSITION, AM_TOGGLE_VOLUME, "TOGGLE VOLUME");
		InsertMenu(hMenu, -1, MF_BYPOSITION, AM_EXIT, "EXIT");

		SetForegroundWindow(hwnd);

		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
		DestroyMenu(hMenu);
	}
}

System_Tray::System_Tray(/* std::unique_ptr<Settings_GUI> *g*/ Settings_GUI *g) {
	nf = new Network_Functionality();
	io = new device_IO();
	load = new Load_Externals();
	gui = g;
}

System_Tray::System_Tray(/* std::unique_ptr<Settings_GUI> *g*/ Settings_OGUI *g) {
	new_g = true;
	nf = new Network_Functionality();
	io = new device_IO();
	load = new Load_Externals();
	new_gui = g;
}

System_Tray::~System_Tray(){
	Shell_NotifyIcon(NIM_DELETE, &notify);
	DestroyWindow(window);
	UnregisterClass("NON", hinst);

	//delete ap;
	//delete nf;
	//delete load;
	//delete gui;
	//delete new_gui;
}

void  System_Tray::Start() {
	hinst = GetModuleHandle(NULL);
	WNDCLASS windowclass;
	windowclass.style = 0;
	windowclass.lpfnWndProc = &OnEvent;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hInstance = hinst;
	windowclass.hIcon = NULL;
	windowclass.hCursor = 0;
	windowclass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	windowclass.lpszMenuName = NULL;
	windowclass.lpszClassName = "NON";
	RegisterClass(&windowclass);

	HWND window = CreateWindow("non", "non", WS_SYSMENU | WS_MINIMIZE, 0, 0, 400, 500, NULL, NULL, hinst, NULL);
	Register_System_Tray_Icon(window);
}

void System_Tray::Register_System_Tray_Icon(HWND hwnd) {
	ZeroMemory(&notify, sizeof(notify));
	notify.cbSize = sizeof(NOTIFYICONDATA);

	notify.uID = 5;
	notify.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	notify.hIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(MYICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

	strcpy_s(notify.szTip, tip.c_str());
	notify.uCallbackMessage = WM_APP;  // What message to look for when creating the event handler.
	notify.hWnd = hwnd;

	if (Shell_NotifyIcon(NIM_ADD, &notify) == FALSE) {
		printf("Could not add...\n");

		crash_logger ci;
		ci.log_message_with_last_error(__FUNCTION__);
		exit(0);
	}
}