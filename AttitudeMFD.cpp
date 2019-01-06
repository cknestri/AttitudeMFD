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

	Control(TimeElapsed);

	// Reset time
	TimeElapsed = 0.0;
}


void AttitudeMFD::Control(double deltaTime)
{
	m_attitudeModeController->Control(deltaTime);
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
}
