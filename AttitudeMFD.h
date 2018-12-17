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
#include "TargetObj.h"
#include "List.h"
#include "CDK.h"
#include "IAttitudeModeController.h"

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
enum T_AXIS {TA_LAT, TA_VERT, TA_FA};


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
const int MAX_TARGET_NAME = 50;
const double UPDATE_INTERVAL = 1.0;		// How much time should pass between updates


// For ease of accessing the target list
#define CurrentTarget (TargetList.GetCurrent()).GetObj()

class AttitudeMFD: public MFD2 {
public:

	AttitudeMFD(DWORD w, DWORD h, VESSEL *vessel);
	~AttitudeMFD();

	bool Update(oapi::Sketchpad* sketchpad);
	void UpdateState(double TimeStep);
	
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
	bool ConsumeKeyBuffered(DWORD key);
	bool SelectTarget(char *name);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
	bool SetRelAttitude(char *str, AXIS Axis);
	void ChangeRefMode(REF_MODE Mode);

private:
	IAttitudeModeController* m_attitudeModeController;

	// MFD Display-related variables
	HDC hDC;	
	DWORD Width, Height;
	int LINE;				// Number of pixels to jump to write the next line
	int CurrentLine;
	
	
	// Common variables
	REF_MODE RefMode;
	Attitude RefAttitude;
	Attitude RelAttitude;
	VECTOR3 PitchYawRoll;
	VECTOR3 RotLevel;
	ATT_HOLD_MODE AttHoldMode;

	
	// For TARGET_RELATIVE reference mode
	List<TargetObj> TargetList;	// List of targets, sorted by distance (nearest to farthest)
	OBJHANDLE TargetRef;
	char TargetName[MAX_TARGET_NAME];
	int Index;
	double RadialVel;
	VECTOR3 RelPos, RelVel;
	TRIM_STATUS TrimStatus;
	TRIM_MODE TrimMode;

	// For EI reference mode
	ENTRY_INTERFACE Interface;


	// Basic state data
	VESSEL *Spacecraft;
	VESSELSTATUS Status;
	VECTOR3 GSpacecraftPos, GSpacecraftVel, Airspeed;
	double AOA, SlipAngle, Mass, 
			MaxAttThrust, MaxMainThrust, 
			MaxRetroThrust, MaxHoverThrust;

	// Misc variables
	HBRUSH ThrusterBrush, ThrusterBrushNeg;
	COLOR_MODE ColorMode;
	double TimeElapsed;


	// Functions
	bool IsModeChangeKey(DWORD key) const;
	bool IsAttitudeHoldToggleKey(DWORD key) const;
	bool ProcessModeChangeKey(DWORD key);

	VECTOR3 CalcPitchYawRollAngles();

	void CalcTargetRelative();
	void CalcVelocity();	
	void CalcAttitude();
	void CalcEI();
	void Control();

	void SetTrimLevelVert();
	void SetTrimLevelLat();
	void SetTrimLevelFA();
	void SetTrimThrustLevel(T_AXIS Axis);
	void Trim();

	void DisplayTargetRelative();
	void DisplayVelocity();	
	void DisplayAttitude();
	void DisplayEI();

	void StartModeTargetRelative();
	void StartModeVelocity();	
	void StartModeAttitude();
	void StartModeEI();
		
	void ToggleColorMode();
	void ToggleAttHoldMode();
	void SetTrimMode(TRIM_MODE Mode);
	void ToggleTrimStatus();
	void PrintMFDHeading();
	void SetRefAttitude();
	void SelectNextTarget();
	void SelectPrevTarget();
	void SelectClosestTarget();
	void SetTarget();
	void UpdateTargetList();
	void BuildTargetList();
	bool SelectBase();
	void PrintRefMode();
	void PrintAngleRate(const char *Heading, double Angle, double Rate);
	void PrintAngle(const char *Heading, double Angle);
	void PrintRate(const char *Heading, double Rate);
	void PrintRate(const char *Heading, double Rate, bool Trim);
	void PrintPitchThrustLevel(double Level);
	void PrintYawThrustLevel(double Level);
	void PrintRollThrustLevel(double Level);
	void PrintRotThrust();
	void PrintRelVel();

	bool HaveMainEngine();
	bool HaveRetroEngine();
	bool HaveHoverEngine();

	void SaveStatus();
	void LoadSavedStatus();

};

#endif