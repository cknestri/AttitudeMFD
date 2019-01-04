//////////////////////////////////////////////////////////////////////////////////////////////
//
//	AttitudeMFD.cpp - Primary source file for the AttitudeMFD class
//
//	Copyright Chris Knestrick, 2003 
//	 
//	For a full description of the copyright, please see the file Copyright.txt
///////////////////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define ORBITER_MODULE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Orbitersdk.h"
#include "AttitudeMFD.h"
#include "CDK.h"
#include "AttitudeModeControllerFactory.h"

using namespace std;

static struct {  
	int mode;
	AttitudeMFD *CurrentMFD1, *CurrentMFD2;	// This allows two opens copies at once

	struct {
		// This flag indicates if this struct contains valid state data.  It is set to true
		// after a SaveStatus() or an ReadStatus()
		bool ValidStatus;
		
		REF_MODE RefMode;
		ATT_HOLD_MODE AttHoldMode;


		COLOR_MODE ColorMode;
	} SavedStatus;

} g_AttitudeMFD;


DLLCLBK void InitModule(HINSTANCE hDLL)
{
	static char *name = "Attitude";
	MFDMODESPECEX spec;
	spec.name    = name;
	spec.context = NULL;
	spec.key     = OAPI_KEY_U;
	spec.msgproc = AttitudeMFD::MsgProc;

	g_AttitudeMFD.mode = oapiRegisterMFDMode (spec);
	g_AttitudeMFD.SavedStatus.ValidStatus = false;
}

DLLCLBK void ExitModule (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode (g_AttitudeMFD.mode);
}


DLLCLBK void opcPostStep(double SimT, double SimDT, double mjd)
{
	if (g_AttitudeMFD.CurrentMFD1 != NULL) {
		g_AttitudeMFD.CurrentMFD1->UpdateState(SimDT);
	} 

	if (g_AttitudeMFD.CurrentMFD2 != NULL) {
		g_AttitudeMFD.CurrentMFD2->UpdateState(SimDT);
	}
}


AttitudeMFD::AttitudeMFD (DWORD w, DWORD h, VESSEL *vessel)
	: MFD2 (w, h, vessel)
	, m_attitudeModeController(nullptr)
	, m_buttonMenu(nullptr)
	, m_buttonMenuSize(0)
	, Width(w)
	, Height(h)
{
	// First, for the callback
	if (!g_AttitudeMFD.CurrentMFD1) {
		g_AttitudeMFD.CurrentMFD1 = this;
	} else {
		g_AttitudeMFD.CurrentMFD2 = this;	// We're the second instance of the MFD
	}

	TimeElapsed = 0.0;
	
	AttHoldMode = DISENGAGED;
	
	InitializeCommandMap();

	// State that doesn't change, so we'll only get it once
	Spacecraft = vessel;

	ChangeRefMode(USER_ATT);

	// Let's prime the pump :-)  We'll just lie about the time
	UpdateState(1.0);

	ColorMode = COLOR_OFF;

	// This is a little redundant, since we took the time to write the initial values
	if (g_AttitudeMFD.SavedStatus.ValidStatus) {
		LoadSavedStatus();
	}
}

AttitudeMFD::~AttitudeMFD()
{
	// Which instance are we?
	if (g_AttitudeMFD.CurrentMFD1 == this) {
		g_AttitudeMFD.CurrentMFD1 = NULL;
	} else {
		g_AttitudeMFD.CurrentMFD2 = NULL;	
	}

	// Save our state for future use
	SaveStatus();

	delete m_buttonMenu;
}

void AttitudeMFD::SaveStatus()
{
	g_AttitudeMFD.SavedStatus.ValidStatus = true;

	g_AttitudeMFD.SavedStatus.RefMode = RefMode;
	g_AttitudeMFD.SavedStatus.AttHoldMode = AttHoldMode;
	
	g_AttitudeMFD.SavedStatus.ColorMode = ColorMode;

}

void AttitudeMFD::LoadSavedStatus()
{
	RefMode = g_AttitudeMFD.SavedStatus.RefMode;
	AttHoldMode = g_AttitudeMFD.SavedStatus.AttHoldMode;
	
	ColorMode = g_AttitudeMFD.SavedStatus.ColorMode;
	
	// Force a state update
	UpdateState(1.0);
}

int AttitudeMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		return (int)(new AttitudeMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}

void AttitudeMFD::InitializeCommandMap()
{
	m_commandMap[OAPI_KEY_1] = [this]() { this->ChangeRefMode(USER_ATT); };
	m_commandMap[OAPI_KEY_2] = [this]() { this->ChangeRefMode(VELOCITY); };
	m_commandMap[OAPI_KEY_3] = [this]() { this->ChangeRefMode(TARGET_RELATIVE); };
	m_commandMap[OAPI_KEY_4] = [this]() { this->ChangeRefMode(EI); };
	//m_commandMap[OAPI_KEY_M] = [this]() { this->ChangeRefMode(); };
	m_commandMap[OAPI_KEY_H] = [this]() { this->ToggleAttHoldMode(); };
}

void AttitudeMFD::BuildButtonMenu()
{
	static const MFDBUTTONMENU s_buttonMenuCommon[] = {
		{"User Att", "Mode", '1'},
		{"Velocity", "Mode", '2'},
		{"Target Rel", "Mode", '3'},
		{"Entry Interface", "Mode", '4'},
		{"Select Mode", 0, 'M'},
	};
	static const size_t s_buttonMenuCommonSize = DimensionOf(s_buttonMenuCommon);

	const MFDBUTTONMENU* attitudeModeControllerButtonMenu = nullptr;
	int attitudeModeControllerButtonMenuSize = m_attitudeModeController->GetButtonMenu(&attitudeModeControllerButtonMenu);

	m_buttonMenuSize = s_buttonMenuCommonSize + attitudeModeControllerButtonMenuSize;

	delete m_buttonMenu;
	m_buttonMenu = new MFDBUTTONMENU[m_buttonMenuSize];

	unsigned int buttonMenuIndex = 0;

	for (int index = 0; index < s_buttonMenuCommonSize; index++)
	{
		m_buttonMenu[buttonMenuIndex] = s_buttonMenuCommon[index];
		buttonMenuIndex++;
	}

	for (int index = 0; index < attitudeModeControllerButtonMenuSize; index++)
	{
		m_buttonMenu[buttonMenuIndex] = attitudeModeControllerButtonMenu[index];
		buttonMenuIndex++;
	}
}

void AttitudeMFD::ToggleColorMode()
{
	ColorMode = (COLOR_MODE)((ColorMode + 1) % 3);

}

void AttitudeMFD::ToggleAttHoldMode()
{
	AttHoldMode = (ATT_HOLD_MODE)((AttHoldMode + 1) % 2);

	if (AttHoldMode == ENGAGED)
	{
		m_attitudeModeController->EnableAutopilot();
	}
	else
	{
		m_attitudeModeController->DisableAutopilot();
	}
}


// Update spacecraft state data to be used
void inline AttitudeMFD::UpdateState(double TimeStep)
{
	if (TimeStep < 1) {
		InvalidateDisplay();
	}

	// If, we should decide if we want to do an update.  This function is called from ovcTimestep
	// every 0.25 seconds.  We probably don't want to do an update that often, so we count
	// the time since the last update.
	TimeElapsed += TimeStep;

	if (TimeElapsed < UPDATE_INTERVAL) {
		return;
	}

	m_attitudeModeController->UpdateState();

	Control();

	// Reset time
	TimeElapsed = 0.0;
}


void AttitudeMFD::Control()
{
	m_attitudeModeController->Control();
}


bool AttitudeMFD::Update(oapi::Sketchpad* sketchpad)
{
	return m_attitudeModeController->Update(sketchpad);
}

void AttitudeMFD::ChangeRefMode(REF_MODE Mode)
{	
	AttHoldMode = DISENGAGED;

	if (m_attitudeModeController != nullptr)
	{
		m_attitudeModeController->DisableAutopilot();
	}

	RefMode = Mode;

	delete m_attitudeModeController;
	m_attitudeModeController = GetAttitudeModeController(RefMode, Spacecraft, Width, Height);
	m_attitudeModeController->Start();

	BuildButtonMenu();
}

bool AttitudeMFD::ConsumeKeyBuffered(DWORD key)
{
	if (ProcessKey(key))
	{
		return true;
	}
	else
	{
		return m_attitudeModeController->ProcessKey(key);
	}
	//bool cbSelectTarget(void *id, char *str, void *data);
	//bool cbSetMode(void *id, char *str, void *data);
	//bool cbSetRelAttPitch(void *id, char *str, void *data);
	//bool cbSetRelAttYaw(void *id, char *str, void *data);
	//bool cbSetRelAttRoll(void *id, char *str, void *data);

	//switch(key) {
	//// Mode keys - the M key only needs to be used as a backup, when the SHIFT + number
	//// is in use by a vessel, such as the shuttle's RMS
	//case OAPI_KEY_M:
	//	oapiOpenInputBox("Select Mode", cbSetMode, 0, 20, (void *)this);
	//case OAPI_KEY_1:
	//	ChangeRefMode(USER_ATT);
	//	return true;
	//case OAPI_KEY_2:
	//	ChangeRefMode(VELOCITY);
	//	return true;
	//case OAPI_KEY_3:
	//	ChangeRefMode(TARGET_RELATIVE);
	//	return true;
	//case OAPI_KEY_4:
	//	ChangeRefMode(EI);
	//	return true;

	//case OAPI_KEY_P:
	//	if (RefMode != TARGET_RELATIVE) {
	//		oapiOpenInputBox("Select Pitch", cbSetRelAttPitch, 0, 20, (void *)this);
	//	} else {
	//		SelectPrevTarget();
	//	}
	//	return true;
	//case OAPI_KEY_Y:
	//	if (RefMode != TARGET_RELATIVE) {
	//		oapiOpenInputBox("Select Yaw", cbSetRelAttYaw, 0, 20, (void *)this);
	//	}
	//	return true;
	//case OAPI_KEY_R:
	//	if (RefMode != TARGET_RELATIVE) {
	//		oapiOpenInputBox("Select Roll", cbSetRelAttRoll, 0, 20, (void *)this);
	//	}
	//	return true;


	//case OAPI_KEY_H:
	//	ToggleAttHoldMode();
	//	return true;
	//case OAPI_KEY_C:
	//	ToggleColorMode();
	//	return true;
	//case OAPI_KEY_PERIOD:
	//	if (RefMode == USER_ATT) {
	//		SetRefAttitude();
	//	}
	//	return true;

	//default:
	//	return false;
	//}

}

bool AttitudeMFD::ProcessKey(DWORD key)
{
	auto iter = m_commandMap.find(key);

	if (iter != m_commandMap.end())
	{
		iter->second();
		return true;
	}

	return false;
}

int AttitudeMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
{	
	if (menu != NULL)
	{
		*menu = m_buttonMenu;
	}

	return m_buttonMenuSize;

	//const int NUM_CMNDS = 22;

	//static const MFDBUTTONMENU mnu[NUM_CMNDS] = {
	//	{"User Att", "Mode", '1'},
	//	{"Velocity", "Mode", '2'},
	//	{"Target Rel", "Mode", '3'},
	//	{"Entry Interface", "Mode", '4'},
	//	{"Select Mode", 0, 'M'},
	//	{"Set Pitch", 0, 'P'},
	//	{"Set Yaw", 0, 'Y'},
	//	{"Set Roll", 0, 'R'},
	//	{"Attitude Hold", 0, 'H'},
	//	{"Set Reference", "Attitude", '.'},
	//	{"Toggle Color Mode", 0, 'C'},


	//};
	//if (menu) *menu = mnu;
	//return NUM_CMNDS;
}



// Note: This is not part of class AttitudeMFD
void PrintRect(HDC hDC, int StartX, int StartY, int Width, int Height)
{
	MoveToEx(hDC, StartX, StartY, NULL);

	LineTo(hDC, StartX + Width, StartY);
	LineTo(hDC, StartX + Width, StartY + Height);
	LineTo(hDC, StartX, StartY + Height);
	LineTo(hDC, StartX, StartY);
}

// Note: This is not part of class AttitudeMFD
void PrintArc(HDC hDC, int X, int Y, int Radius, int InnerRadius, double InitAngle, double SweepAngle)
{

	MoveToEx(hDC, X + (Radius * cos(Radians(InitAngle))), Y - (Radius * sin(Radians(InitAngle))), NULL);
	AngleArc(hDC, X, Y, Radius, InitAngle, SweepAngle);
	LineTo(hDC, X + (InnerRadius * cos(Radians(InitAngle + SweepAngle))), Y - (InnerRadius * sin(Radians(InitAngle + SweepAngle)))); 
	AngleArc(hDC, X, Y, InnerRadius, (InitAngle + SweepAngle), -SweepAngle);
	LineTo(hDC, X + (Radius * cos(Radians(InitAngle))), Y - (Radius * sin(Radians(InitAngle))));
}

//void AttitudeMFD::PrintPitchThrustLevel(double Level)
//{
//	HBRUSH OldBrush; 
//
//	const int DISPLAY_START_X = 25, DISPLAY_START_Y = CurrentLine, 
//				DISPLAY_WIDTH = LINE, DISPLAY_HEIGHT = 90, 
//				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
//				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);
//
//
//	// Print the overlaying display box
//	PrintRect(hDC, DISPLAY_START_X, DISPLAY_START_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
//
//	// The color of the brush depends on if the thrust level is positive or negative
//	if (Level >= 0) {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
//	} else {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
//	}
//	
//	BeginPath(hDC);
//	PrintRect(hDC, DISPLAY_START_X, DISPLAY_MID_Y, DISPLAY_WIDTH, -((DISPLAY_HEIGHT/2) * Level)); 
//	EndPath(hDC);
//	StrokeAndFillPath(hDC);
//
//
//	// Return to the original brush
//	SelectObject(hDC, OldBrush);
//
//
//}
//

//void AttitudeMFD::PrintYawThrustLevel(double Level)
//{
//	HBRUSH OldBrush; 
//	const int DISPLAY_START_X = 70, DISPLAY_START_Y = CurrentLine + (45 - (LINE / 2)), 
//				DISPLAY_WIDTH = 90, DISPLAY_HEIGHT = LINE, 
//				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
//				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);
//
//
//	// Print the overlaying display box
//	PrintRect(hDC, DISPLAY_START_X, DISPLAY_START_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
//
//	// The color of the brush depends on if the thrust level is positive or negative
//	if (Level >= 0) {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
//	} else {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
//	}
//	
//	BeginPath(hDC);
//	PrintRect(hDC, DISPLAY_MID_X, DISPLAY_START_Y, ((DISPLAY_WIDTH/2) * Level), DISPLAY_HEIGHT); 
//	EndPath(hDC);
//	StrokeAndFillPath(hDC);
//
//
//	// Return to the original brush
//	SelectObject(hDC, OldBrush);
//
//
//}

//void AttitudeMFD::PrintRollThrustLevel(double Level)
//{
//	HBRUSH OldBrush; 
//
//	const int DISPLAY_START_X = 225, DISPLAY_START_Y = CurrentLine + 70, 
//				DISPLAY_WIDTH = LINE, DISPLAY_HEIGHT = 90, 
//				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
//				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);
//
//
//	PrintArc(hDC, DISPLAY_START_X, DISPLAY_START_Y, 45, 45 - LINE, 179.0, -180.0);
//
//	// The color of the brush depends on if the thrust level is positive or negative
//	if (Level >= 0) {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
//	} else {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
//	}
//	
//	BeginPath(hDC);
//	PrintArc(hDC, DISPLAY_START_X, DISPLAY_START_Y, 45, 45 - LINE, 90.0, -(90.0 * Level)); 
//	EndPath(hDC);
//	StrokeAndFillPath(hDC);
//
//
//	// Return to the original brush
//	SelectObject(hDC, OldBrush);
//
//
//}
//
//void AttitudeMFD::PrintRotThrust()
//{
//	char Buffer[100];
//	int START_LINE = LINE * 17;
//	
//	if (CurrentLine < START_LINE) {
//		CurrentLine = START_LINE;
//	}
//
//	MoveToEx(hDC, 0, CurrentLine, NULL);
//	LineTo(hDC, Width, CurrentLine);
//	MoveToEx(hDC, 60, CurrentLine, NULL);
//	LineTo(hDC, 60, Height);
//	MoveToEx(hDC, 170, CurrentLine, NULL);
//	LineTo(hDC, 170, Height);
//
//	CurrentLine += LINE / 4;
//
//	SetTextColor(hDC, RGB(255, 255, 255));
//
//	sprintf(Buffer, "%s", "Pitch");
//	TextOut(hDC, 10, CurrentLine, Buffer, strlen(Buffer));
//	sprintf(Buffer, "%s", "Yaw");
//	TextOut(hDC, 105, CurrentLine, Buffer, strlen(Buffer));
//	sprintf(Buffer, "%s", "Roll");
//	TextOut(hDC, 205, CurrentLine, Buffer, strlen(Buffer));
//	SetTextColor(hDC, RGB(0, 255, 0));
//
//	CurrentLine += 1.5 * LINE;
//	MoveToEx(hDC, 0, CurrentLine, NULL);
//	LineTo(hDC, Width, CurrentLine);
//	CurrentLine += LINE;
//
//	/*PrintPitchThrustLevel(RotLevel.data[PITCH]);
//	PrintYawThrustLevel(RotLevel.data[YAW]);
//	PrintRollThrustLevel(RotLevel.data[ROLL]);*/
//
//}

//bool cbSetRelAttPitch(void *id, char *str, void *data)
//{
//	return (((AttitudeMFD *)data)->SetRelAttitude(str, PITCH));
//}
//
//bool cbSetRelAttYaw(void *id, char *str, void *data)
//{
//	return (((AttitudeMFD *)data)->SetRelAttitude(str, YAW));
//}
//
//bool cbSetRelAttRoll(void *id, char *str, void *data)
//{
//	return (((AttitudeMFD *)data)->SetRelAttitude(str, ROLL));
//}
//
//bool cbSetMode(void *id, char *str, void *data)
//{
//	REF_MODE Mode = (REF_MODE)atoi(str);
//
//	if (Mode >= USER_ATT && Mode <= EI) {
//		((AttitudeMFD *)data)->ChangeRefMode(Mode);
//		return true;
//	}
//
//	return false;
//}
//
//bool AttitudeMFD::SetRelAttitude(char *str, AXIS Axis)
//{
//	// This allows a space to represent a decimal point
//	for (unsigned int i = 0; i < strlen(str); i++) {
//		if (str[i] == ' ') {
//			str[i] = '.';
//			break;
//		}
//	}
//
//	RelAttitude.data[Axis] = Radians(atof(str));
//	return true;
//}

