#pragma once
#include <stdio.h>
#include <vector>
#include <chrono>
#include "Global_Variables.h"
#include "format_sessions.h"

typedef long HRESULT;

class Audio_Controller {
private:
	std::chrono::high_resolution_clock::time_point tstart;
	std::chrono::high_resolution_clock::time_point tend;
	AudioSession *save_Vars[4];
	int old_Count = 0;
protected:
	/*
	Instead of reorganizing all sessions everytime a new program has started, we look at
	the existing sessions and just push the new one at the end of that list. Butt(!) also
	keeping in mind any reservations and/or reserved spots.
	*/
	void Push_Back_Audio_Controller();
	/*
	Like the push_back function but instead we just free the spot of the newly closed program. 
	and we DO NOT reorganize at all.
	TODO: since we are now using encoders we can actually reorganize but keep the same volume. but should we?
	*/
	void Pop_Back_Audio_Controller();

	/*
	Looks through the users exclude list, to check the the session is unwanted!
	*/
	bool Check_for_Restrictions(std::string s);
	/*
	same as above but for reserved spots.
	*/
	int Check_for_Reservations(std::string s);
	std::string Check_For_New_Name(std::string s);
	std::string check_for_default_name(std::string s);
	bool is_renamed(std::string s);
public:
	Audio_Controller(AudioSession* v1, AudioSession* v2, AudioSession* v3, AudioSession* v4);
	Audio_Controller() {}
	/*
	Creates a session manager that you can in turn we can extract individual audio sessions
	*/
	HRESULT CreateSessionManager(IAudioSessionManager2** ppSessionManager);
	/*
	Both sets the volume of all active sessions, but also checks if a new program was connected
	TODO: Seperate into two function.
	*/
	bool Set_Volume();
	/*
	Program count.
	*/
	int Get_Count();

	/*
	Reenumerates the audio sessions, free:s the active ones and reallocates them, if something goes wrong or
	a session failed to allocate, this can be called.
	TODO: Make this a user accessable function, through hotkey. Like the chrome extension.
	*/
	void Rerender_Audio_Displays();
};

