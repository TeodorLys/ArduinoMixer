#pragma once
#include <stdio.h>
#include <vector>
#include <chrono>
#include "Global_Variables.h"

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
	std::string Get_Session_Display_Name(IAudioSessionControl **pSessionControl);
	void Pop_Back_Audio_Controller();
	void Push_Back_Audio_Controller();
	bool Check_for_Restrictions(std::string s);
	int Check_for_Reservations(std::string s);
	std::string Check_For_New_Name(std::string s);
public:
	Audio_Controller(AudioSession* v1, AudioSession* v2, AudioSession* v3, AudioSession* v4);
	Audio_Controller() {} //For accessing this classes static variables;
	~Audio_Controller();
	HRESULT CreateSessionManager(IAudioSessionManager2** ppSessionManager);
	bool Set_Volume();
	float Get_Volume(int index) const;
	
	void Free_All();
	int Get_Count();
	void Rerender_Audio_Displays();

public:

	float get_last_value_of_sliders(int index) {
		return sliders[index].last_Value;
	}

	size_t Get_Size() const {
		return sliders.size();
	}
private:
	Audio_Volume_Variables Enumarate_Audio_Session(int index);
};

