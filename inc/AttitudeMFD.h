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

typedef struct {
	VECTOR3 Pitch;
	VECTOR3 Yaw;
	VECTOR3 Roll;			
} RefPoints;

// The angular rotations from the a specific reference frame for a new reference frame
typedef VECTOR3 Attitude;

enum REF_MODE {USER_ATT = 1, VELOCITY, TARGET_RELATIVE,  EI };						
enum ATT_HOLD_MODE {DISENGAGED, ENGAGED}; 
enum COLOR_MODE {COLOR_OFF, COLOR_WHITE, COLOR_RED};

enum TRIM_STATUS {T_ENGAGED, T_DISENGAGED};


// Trim modes:
// T_VERT == Vertical (Numpad 1)
// T_LAT == Lateral (Numpad 2)
// T_FA == Fore/Aft (Numpad 3)
// T_ALL == All Axes (Numpad 5)
// T_VERT_LAT == Vertical AND Lateral (Numpad 7)
// T_LAT_FA == Lateral AND Fore/Aft (Numpad 8)
// T_VERT_FA == Vertical AND Fore/Aft (Numpad 9)
enum TRIM_MODE {T_VERT, T_LAT, T_FA, T_ALL, T_VERT_LAT, T_LAT_FA, T_VERT_FA};

//const int LINE = 15;					// Number of pixels to jump to write the next line
const int NUM_CONTROL_MODES = 3;
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
	bool SetRelAttitude(char *str, AXIS Axis);
	void ChangeRefMode(REF_MODE Mode);

private:
	IAttitudeModeController* m_attitudeModeController;

	std::map<DWORD, std::function<void()>> m_commandMap;

	MFDBUTTONMENU* m_buttonMenu;
	int m_buttonMenuSize;
	DWORD Width, Height;
	
	
	// Common variables
	REF_MODE RefMode;
	ATT_HOLD_MODE AttHoldMode;


	// Basic state data
	VESSEL *Spacecraft;

	COLOR_MODE ColorMode;
	double TimeElapsed;


	// Functions
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