#pragma once
#include <stdio.h>
#include <vector>
#include <chrono>
#include "Global_Variables.h"
#include "format_sessions.h"

struct IAudioSessionManager2;
struct IAudioSessionEnumerator;
struct IAudioSessionControl;
struct ISimpleAudioVolume;
typedef long HRESULT;

struct Audio_Volume_Variables {
	float *v = nullptr;
	float last_Value = 0.f;
	int page_index = 0;
	int index = 0;  //0 is an invalid number and will be seen as an error, This is also the page number...
	int option_index = 0;
	ISimpleAudioVolume *control = NULL;
	std::string display_Name;
	bool mute = false;
};

class Audio_Controller {
private:
	std::chrono::high_resolution_clock::time_point tstart;
	std::chrono::high_resolution_clock::time_point tend;
	IAudioSessionManager2 *pSession = 0;
	IAudioSessionEnumerator *pSessionEnum = 0;
	IAudioSessionControl *pSessionControl = 0;
	static std::vector<Audio_Volume_Variables> sliders;
	AudioSession *save_Vars[4];
	int old_Count = 0;
protected:
	/*
	Retrieves the process from a audio session controller, allocated in the CreateSessionManager.
	*/
	std::string Get_Session_Display_Name(IAudioSessionControl **pSessionControl);
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
public:
	Audio_Controller(AudioSession* v1, AudioSession* v2, AudioSession* v3, AudioSession* v4);
	Audio_Controller() {} //For accessing this class:es static variables; Really not used... at all!
	~Audio_Controller();
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
	NOT IN USE! but you get the idea from the name...
	*/
	float Get_Volume(int index) const;
	
	/*
	Free:s all "global" session variables i.e. not all individual audio session, 
	only the enumerator, manager and control.
	SEE: Private variables(in this class)
	*/
	void Free_All();
	/*
	Program count.
	*/
	int Get_Count();

	/*
	Reenumerates the audio sessions, free:s the active ones and reallocates the, if something goes wrong or
	a session failed to allocate, this can be called.
	TODO: Make this a user accessable function, through hotkey. Like the chrome extension.
	*/
	void Rerender_Audio_Displays();

public:
	/*
	What the volume of the audio sessions were before they got assigned to an encoder.
	Which later will be sent over to the mixer.
	*/
	float get_last_value_of_sliders(int index) {
		return sliders[index].last_Value;
	}

	size_t Get_Size() const {
		return sliders.size();
	}
private:
	/*
	Helper for push, pop and rerender functions. to enumerate audio sessions by index.
	*/
	Audio_Volume_Variables Enumarate_Audio_Session(int index);
};

