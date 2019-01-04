//////////////////////////////////////////////////////////////////////////////////////////////
//
//	AttitudeMFD.h - Header  file for the AttitudeMFD class
//
//	Copyright Chris Knestrick, 2003 
//	 
//	For a full description of the copyright, please see the file Copyright.txt
///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AUTOPILOT_MFD_H
#define AUTOPILOT_MFD_H
#include "CDK.h"
#include "IAttitudeModeController.h"
#include <functional>
#include <map>

// The angular rotations from the a specific reference frame for a new reference frame
typedef VECTOR3 Attitude;

enum REF_MODE {USER_ATT = 1, VELOCITY, TARGET_RELATIVE,  EI };						
enum ATT_HOLD_MODE {DISENGAGED, ENGAGED}; 
enum COLOR_MODE {COLOR_OFF, COLOR_WHITE, COLOR_RED};

const double UPDATE_INTERVAL = 1.0;		// How much time should pass between updates


class AttitudeMFD: public MFD2 {
public:

	AttitudeMFD(DWORD w, DWORD h, VESSEL *vessel);
	~AttitudeMFD();

	bool Update(oapi::Sketchpad* sketchpad);
	void UpdateState(double TimeStep);
	
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
	bool ConsumeKeyBuffered(DWORD key);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
	void ChangeRefMode(REF_MODE Mode);

private:
	IAttitudeModeController* m_attitudeModeController;

	std::map<DWORD, std::function<void()>> m_commandMap;

	MFDBUTTONMENU* m_buttonMenu;
	int m_buttonMenuSize;
	DWORD Width, Height;

	REF_MODE RefMode;
	ATT_HOLD_MODE AttHoldMode;
	VESSEL *Spacecraft;
	COLOR_MODE ColorMode;
	double TimeElapsed;

	void InitializeCommandMap();
	void BuildButtonMenu();
	bool ProcessKey(DWORD key);
	void Control();
	
	void ToggleColorMode();
	void ToggleAttHoldMode();

	void SaveStatus();
	void LoadSavedStatus();
};

#endif