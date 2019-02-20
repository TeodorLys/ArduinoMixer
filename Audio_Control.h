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
	float *v;
	float last_Value = 0.f;
	int index = 0;  //NULL is an invalid number and will be seen as an error
	ISimpleAudioVolume *control = NULL;
	std::string display_Name;
};

class Audio_Control {
private:
	std::chrono::high_resolution_clock::time_point tstart;
	std::chrono::high_resolution_clock::time_point tend;
	IAudioSessionManager2 *pSession = 0;
	IAudioSessionEnumerator *pSessionEnum = 0;
	IAudioSessionControl *pSessionControl = 0;
	std::vector<Audio_Volume_Variables> sliders;
	float *save_Vars[4];
	int old_Count = 0;
protected:
	std::string Get_Session_Display_Name(IAudioSessionControl **pSessionControl);
	void Pop_Back_Audio_Control();
	void Push_Back_Audio_Control();
	bool Check_for_Restrictions(std::string s);
	int Check_for_Reservations(std::string s);
	std::string Check_For_New_Name(std::string s);
public:
	Audio_Control(float* v1, float* v2, float* v3, float* v4);
	~Audio_Control();
	HRESULT CreateSessionManager(IAudioSessionManager2** ppSessionManager);
	bool Set_Volume();
	float Get_Volume(int index) const;
	
	void Free_All();
	int Get_Count();
	void Rerender_Audio_Displays();
public:
	size_t Get_Size() const {
		return sliders.size();
	}
};

