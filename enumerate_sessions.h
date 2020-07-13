#pragma once
#include "Global_Variables.h"
#include <string>
#include <vector>
#include <audiopolicy.h>

#define SAFE_RELEASE(punk)  \
              if ((punk) != nullptr)  \
                { (punk)->Release(); (punk) = nullptr; }

struct session {
	std::string name = "UNUSED";
	std::string reserved_name;
	bool reserved = false;
	bool mute;
	ISimpleAudioVolume* control = NULL;
	float initial_volume;
	float* volume = nullptr;
};
class enumerate_sessions {
private:
	IAudioSessionManager2* pSession = 0;
	IAudioSessionEnumerator* pSessionEnum = 0;
	IAudioSessionControl* pSessionControl = 0;

public:
	session* Enumarate_Audio_Session(int index);
	std::string Get_Session_Display_Name(IAudioSessionControl** pSessionControl);
	HRESULT CreateSessionManager(IAudioSessionManager2** ppSessionManager);

	void reinitialize() {
		CreateSessionManager(&pSession);
		pSession->GetSessionEnumerator(&pSessionEnum);
	}

	int Get_Count() {
		int b = 0;
		IAudioSessionEnumerator* pEnum = 0;
		IAudioSessionManager2* pSez = 0;
		CreateSessionManager(&pSez);
		pSez->GetSessionEnumerator(&pEnum);

		pEnum->GetCount(&b);

		SAFE_RELEASE(pEnum);
		SAFE_RELEASE(pSez);
		CoUninitialize();
		return b;
	}

	void free_controller() {
		SAFE_RELEASE(pSessionControl);
	}

	void Free_All() {
		CoUninitialize();
		SAFE_RELEASE(pSessionEnum);
		SAFE_RELEASE(pSessionControl);
		SAFE_RELEASE(pSession);
	}

};

