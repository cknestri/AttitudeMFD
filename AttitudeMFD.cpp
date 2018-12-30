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

		// For reference modes OTHER than TARGET_RELATIVE
		Attitude RefAttitude;
		Attitude RelAttitude;

		// For TARGET_RELATIVE reference mode
		OBJHANDLE TargetRef;		
		int Index;
		char TargetName[MAX_TARGET_NAME];	
		TRIM_STATUS TrimStatus;
		TRIM_MODE TrimMode;

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
{
	// First, for the callback
	if (!g_AttitudeMFD.CurrentMFD1) {
		g_AttitudeMFD.CurrentMFD1 = this;
	} else {
		g_AttitudeMFD.CurrentMFD2 = this;	// We're the second instance of the MFD
	}

	TimeElapsed = 0.0;
	
	RelAttitude = NULL_VECTOR;
	AttHoldMode = DISENGAGED;
	
	InitializeCommandMap();

	// State that doesn't change, so we'll only get it once
	Spacecraft = vessel;

	ChangeRefMode(USER_ATT);

	// Let's prime the pump :-)  We'll just lie about the time
	UpdateState(1.0);

	ColorMode = COLOR_OFF;
	LINE = h / 28;

	// Store the width and height
	Width = w;
	Height = h;

	// Set the brush for the thruster display
	ThrusterBrush = CreateSolidBrush(RGB(0, 200, 0));
	ThrusterBrushNeg = CreateSolidBrush(RGB(100, 100, 100));

	// For sanity
	CurrentLine = 0;

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

	if (RefMode == TARGET_RELATIVE) {
		g_AttitudeMFD.SavedStatus.TargetRef = TargetRef;		
		g_AttitudeMFD.SavedStatus.Index = Index;
		strcpy(g_AttitudeMFD.SavedStatus.TargetName, TargetName);
		g_AttitudeMFD.SavedStatus.TrimStatus = TrimStatus;
		g_AttitudeMFD.SavedStatus.TrimMode = TrimMode;
	} else {
		g_AttitudeMFD.SavedStatus.RefAttitude = RefAttitude;		
		g_AttitudeMFD.SavedStatus.RelAttitude = RelAttitude;		
	}
	
	g_AttitudeMFD.SavedStatus.ColorMode = ColorMode;

}

void AttitudeMFD::LoadSavedStatus()
{
	RefMode = g_AttitudeMFD.SavedStatus.RefMode;
	AttHoldMode = g_AttitudeMFD.SavedStatus.AttHoldMode;

	if (RefMode == TARGET_RELATIVE) {
		TargetRef = g_AttitudeMFD.SavedStatus.TargetRef;		
		Index = g_AttitudeMFD.SavedStatus.Index;
		strcpy(TargetName, g_AttitudeMFD.SavedStatus.TargetName);
		TrimStatus = g_AttitudeMFD.SavedStatus.TrimStatus;
		TrimMode = g_AttitudeMFD.SavedStatus.TrimMode;
	} else {
		RefAttitude = g_AttitudeMFD.SavedStatus.RefAttitude;		
		RelAttitude = g_AttitudeMFD.SavedStatus.RelAttitude;		
	}
	
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

void AttitudeMFD::StartModeVelocity()
{
	CalcVelocity();
}

void AttitudeMFD::StartModeEI()
{
	CalcEI();
}

void AttitudeMFD::SetRefAttitude()
{	
	// Set the current attitude as the reference attitude
	RefAttitude = Status.arot;
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



void inline AttitudeMFD::SetTrimMode(TRIM_MODE Mode)
{
	if (RefMode != TARGET_RELATIVE) {
		return;
	}

	// If we hit the same key a second time, we'll disable the trim mode
	if ((TrimStatus == T_ENGAGED) && (TrimMode == Mode)) {
		ToggleTrimStatus();
		return;
	} 

	ToggleTrimStatus();
	TrimMode = Mode;
}

void AttitudeMFD::ToggleTrimStatus()
{
	TrimStatus = (TRIM_STATUS)((TrimStatus + 1) % 2);

	// Don't leave the thrusters firing!
	if (TrimStatus == T_DISENGAGED) {
		Spacecraft->SetAttitudeLinLevel(NULL_VECTOR);
	}

}


// Update spacecraft state data to be used
void inline AttitudeMFD::UpdateState(double TimeStep)
{
	// This allows the thruster information to be updated at a higher rate than the other
	// information
	RotLevel.data[PITCH] = Spacecraft->GetThrusterGroupLevel(THGROUP_ATT_PITCHUP) - 
							Spacecraft->GetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN);
	RotLevel.data[YAW] = Spacecraft->GetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT) - 
							Spacecraft->GetThrusterGroupLevel(THGROUP_ATT_YAWLEFT);

	RotLevel.data[ROLL] = Spacecraft->GetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT) - 
							Spacecraft->GetThrusterGroupLevel(THGROUP_ATT_BANKLEFT);

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

	Spacecraft->GetStatus(Status);
	oapiGetGlobalPos(Spacecraft->GetHandle(), &GSpacecraftPos);
	oapiGetGlobalVel(Spacecraft->GetHandle(), &GSpacecraftVel);
	Mass = Spacecraft->GetMass();

	m_attitudeModeController->UpdateState();

	Control();

	// Reset time
	TimeElapsed = 0.0;

	VECTOR3 thrustGroupLevel;
	thrustGroupLevel.data[PITCH] = Spacecraft->GetThrusterGroupLevel(THGROUP_ATT_PITCHUP);
	thrustGroupLevel.data[YAW] = Spacecraft->GetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN);
	thrustGroupLevel.data[ROLL] = 0;

	PrintAngleVector(thrustGroupLevel);
}

VECTOR3 GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll)
{	
	VECTOR3 Res = { 0, 0, 0 };

	// Normalize the vectors
	Pitch = Normalize(Pitch);
	YawRoll = Normalize(YawRoll);
	VECTOR3 H = Normalize(CrossProduct(Pitch, YawRoll));

	Res.data[YAW] = -asin(YawRoll.z);

	Res.data[ROLL] = atan2(YawRoll.y, YawRoll.x);

	Res.data[PITCH] = atan2(H.z, Pitch.z);

	return Res;

}


// This is the same as the previous function, except that we use the transpose matrix
VECTOR3 GetPYR2(VECTOR3 Pitch, VECTOR3 YawRoll)
{	
	VECTOR3 Res = { 0, 0, 0 };

	// Normalize the vectors
	Pitch = Normalize(Pitch);
	YawRoll = Normalize(YawRoll);
	VECTOR3 H = Normalize(CrossProduct(Pitch, YawRoll));

	Res.data[YAW] = -asin(Pitch.x);

	Res.data[ROLL] = atan2(H.x, YawRoll.x);

	Res.data[PITCH] = atan2(Pitch.y, Pitch.z);


	return Res;

}


VECTOR3 GetPY(VECTOR3 PitchYaw)
{	
	VECTOR3 Res = { 0, 0, 0 };

	Res = GetPitchYawRoll(PitchYaw, NULL_VECTOR);

	// Change the signs to make it consistent
	Res.data[PITCH] = -Res.data[PITCH];

	return Res;

}



// Returns a vector containing the pitch, yaw, and roll angles based upon the desired attitude
// relative to the current reference attitude
VECTOR3 inline AttitudeMFD::CalcPitchYawRollAngles()
{
	RefPoints GlobalPts, LocalPts;
	VECTOR3 PitchUnit = {0, 0, 1.0}, YawRollUnit = {1.0, 0, 0};

	RotateVector(PitchUnit, RelAttitude, PitchUnit);
	RotateVector(YawRollUnit, RelAttitude, YawRollUnit);

	RotateVector(PitchUnit, RefAttitude, GlobalPts.Pitch);
	RotateVector(YawRollUnit, RefAttitude, GlobalPts.Yaw);

	GlobalPts.Pitch = GSpacecraftPos + GlobalPts.Pitch;
	GlobalPts.Yaw = GSpacecraftPos + GlobalPts.Yaw;	

	Spacecraft->Global2Local(GlobalPts.Pitch, LocalPts.Pitch);
	Spacecraft->Global2Local(GlobalPts.Yaw, LocalPts.Yaw);

	return GetPYR(LocalPts.Pitch, LocalPts.Yaw);
}

void inline AttitudeMFD::CalcTargetRelative()
{
	VECTOR3 SpacecraftPos, TargetPos, GTargetPos, GVel;

	oapiGetGlobalPos(TargetRef, &GTargetPos);
	Spacecraft->Global2Local(GSpacecraftPos, SpacecraftPos);
	Spacecraft->Global2Local(GTargetPos, TargetPos);
	RelPos = TargetPos - SpacecraftPos; 

	PitchYawRoll = GetPY(RelPos);

	// Calculate relative velocity
	Spacecraft->GetRelativeVel(TargetRef, GVel);
	Spacecraft->Global2Local((GVel + GSpacecraftPos), RelVel);

	// Compute the radial component
	RadialVel = (RelPos * RelVel) / Mag(RelPos);


}
void inline AttitudeMFD::CalcAttitude()
{

	PitchYawRoll = CalcPitchYawRollAngles();

}

void inline AttitudeMFD::CalcVelocity()
{
	VECTOR3 H;

	H = CrossProduct(Status.rpos, Status.rvel);

	RefAttitude = GetPYR2(Status.rvel, H);

	PitchYawRoll = CalcPitchYawRollAngles();
	
}


void inline AttitudeMFD::CalcEI()
{
	VECTOR3 H;
	CalcEntryInterface(Interface);

	if (!Interface.InterfaceDefined) {
		return;
	}

	H = CrossProduct(Interface.Pos, Interface.Vel);

	RefAttitude = GetPYR2(Interface.Vel, H);

	PitchYawRoll = CalcPitchYawRollAngles();
	
}


void AttitudeMFD::Control()
{
	m_attitudeModeController->Control();
	
	/*if (AttHoldMode == ENGAGED) {
		
		if (RefMode == EI && !Interface.InterfaceDefined) {
			AttHoldMode = DISENGAGED;
			return;
		}

		SetAttitude(0, PitchYawRoll.data[PITCH], PITCH, DB_FINE);
		SetAttitude(0, PitchYawRoll.data[YAW], YAW, DB_FINE);
		SetAttitude(0, PitchYawRoll.data[ROLL], ROLL, DB_FINE);			
	}

	if (TrimStatus == T_ENGAGED) {
		Trim();
	}*/
}

void inline AttitudeMFD::SetTrimLevelVert()
{
	double Level = 0.0;

	// First, we'll calculate the thrust level assuming that we'll
	// be using the linear thruster
	Level = -(Mass * RelVel.data[TA_VERT]) / MaxAttThrust;
	
	// We need to decided if we really want to use linear thrusters, or if
	// it's better to use one of the engines
	if (HaveHoverEngine() && Level > 3.0) {
			Level = -(Mass * RelVel.data[TA_VERT]) / MaxHoverThrust;
			// NormalizeThrustLevel(Level);
			Spacecraft->SetEngineLevel(ENGINE_HOVER, Level);
	} else {
		// NormalizeThrustLevel(Level);
		Spacecraft->SetAttitudeLinLevel((int)TA_VERT, Level);
	}

}

void inline AttitudeMFD::SetTrimLevelLat()
{
	double Level = 0.0;

	// First, we'll calculate the thrust level assuming that we'll
	// be using the linear thruster
	Level = -(Mass * RelVel.data[TA_LAT]) / MaxAttThrust;
	
	// NormalizeThrustLevel(Level);
	Spacecraft->SetAttitudeLinLevel((int)TA_LAT, Level);
	
}


void inline AttitudeMFD::SetTrimLevelFA()
{
	double Level = 0.0;

	// First, we'll calculate the thrust level assuming that we'll
	// be using the linear thruster
	Level = -(Mass * RelVel.data[TA_FA]) / MaxAttThrust;

	if (HaveMainEngine() && Level > 3.0) {
		Level = -(Mass * RelVel.data[TA_FA]) / MaxMainThrust;
		// NormalizeThrustLevel(Level);
		Spacecraft->SetEngineLevel(ENGINE_MAIN, Level);
	} else if (HaveRetroEngine() &&Level < -3.0) {
		Level = (Mass * RelVel.data[TA_FA]) / MaxRetroThrust;
		// NormalizeThrustLevel(Level);
		Spacecraft->SetEngineLevel(ENGINE_RETRO, Level);
	} else {
		// NormalizeThrustLevel(Level);
		Spacecraft->SetAttitudeLinLevel((int)TA_FA, Level);
	}
}

void inline AttitudeMFD::SetTrimThrustLevel(T_AXIS Axis)
{
	const double DEADBAND = 0.001;

	if (fabs(RelVel.data[Axis]) > DEADBAND) {	
		switch (Axis) {
		case TA_VERT:
			SetTrimLevelVert();
			TrimStatus = T_ENGAGED;
			break;
		case TA_LAT:
			SetTrimLevelLat();
			TrimStatus = T_ENGAGED;
			break;
		case TA_FA:
			SetTrimLevelFA();
			TrimStatus = T_ENGAGED;
			break;
		default:
			break;
		}	
	}
}


// This function assumes that trim is engaged
void inline AttitudeMFD::Trim()
{
	
	// "Clear the board"
	Spacecraft->SetAttitudeLinLevel(NULL_VECTOR);

	// This will be reset by SetTrimThrustLevel() if trim is used
	TrimStatus = T_DISENGAGED;			

	if (TrimMode == T_VERT || 
		TrimMode == T_ALL || 
		TrimMode == T_VERT_LAT ||
		TrimMode == T_VERT_FA) {
	
		// Clear hover engine thrust
		Spacecraft->SetEngineLevel(ENGINE_HOVER, 0.0);

		SetTrimThrustLevel(TA_VERT);
	}

	 if (TrimMode == T_LAT ||
		 TrimMode == T_ALL ||
		 TrimMode == T_VERT_LAT ||
		 TrimMode == T_LAT_FA) {
		SetTrimThrustLevel(TA_LAT);
	 }

	 if (TrimMode == T_FA || 
		 TrimMode == T_ALL ||
		 TrimMode == T_LAT_FA ||
		 TrimMode == T_VERT_FA) {

		 // Clear main/retro thrust
		Spacecraft->SetEngineLevel(ENGINE_MAIN, 0.0);
		Spacecraft->SetEngineLevel(ENGINE_RETRO, 0.0);

		SetTrimThrustLevel(TA_FA);
	}

}

bool AttitudeMFD::Update(oapi::Sketchpad* sketchpad)
{
	return m_attitudeModeController->Update(sketchpad);
}


void AttitudeMFD::DisplayVelocity()
{
	char Buffer[100];

	PrintRefMode();
	
	// Print velocity
	TextOut(hDC, 5, CurrentLine, "Velocity:", 9);	
	ScaleOutput(Buffer, Mag(Status.rvel));

	TextOut(hDC, 100, CurrentLine, Buffer, strlen(Buffer));
	CurrentLine += 2 * LINE;

	PrintAngle("Set Pitch", RelAttitude.data[PITCH]);
	PrintAngle("Set Yaw ", RelAttitude.data[YAW]);
	PrintAngle("Set Roll", RelAttitude.data[ROLL]);
	CurrentLine += 2 * LINE;


	PrintAngleRate("Pitch:", PitchYawRoll.data[PITCH], Status.vrot.x);
	PrintAngleRate("Yaw:", PitchYawRoll.data[YAW], Status.vrot.y);
	PrintAngleRate("Roll:", PitchYawRoll.data[ROLL], Status.vrot.z);

	CurrentLine += 2 * LINE;

	PrintRotThrust();

}

void AttitudeMFD::DisplayEI()
{
	char Buffer[100];

	PrintRefMode();

	if (Interface.InterfaceDefined) {
		sprintf(Buffer, "Entry Angle:   %.2f", Degree(Interface.Angle));
		TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
		CurrentLine += LINE;

		sprintf(Buffer, "Time To Go:    %.f", Interface.TimeToGo);
		TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
		CurrentLine += 2 * LINE;
	} else {
		CurrentLine += LINE;
		sprintf(Buffer, "Entry Interface Undefined");
		TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
		CurrentLine += 2 * LINE;
			
		return;
	}


	PrintAngle("Set Pitch", RelAttitude.data[PITCH]);
	PrintAngle("Set Yaw ", RelAttitude.data[YAW]);
	PrintAngle("Set Roll", RelAttitude.data[ROLL]);
	CurrentLine += 2 * LINE;


	PrintAngleRate("Pitch:", PitchYawRoll.data[PITCH], Status.vrot.x);
	PrintAngleRate("Yaw:", PitchYawRoll.data[YAW], Status.vrot.y);
	PrintAngleRate("Roll:", PitchYawRoll.data[ROLL], Status.vrot.z);

	CurrentLine += 2 * LINE;

	PrintRotThrust();


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

	//// Target relative keys
	//case OAPI_KEY_T:
	//	if (RefMode == TARGET_RELATIVE) {
	//		oapiOpenInputBox("Select Target", cbSelectTarget, 0, 20, (void *)this);
	//	}
	//	return true;
	//case OAPI_KEY_N:
	//	if (RefMode == TARGET_RELATIVE) {
	//		SelectNextTarget();
	//	}
	//	return true;
	//case OAPI_KEY_SPACE:
	//	if (RefMode == TARGET_RELATIVE) {
	//		SelectClosestTarget();
	//	}
	//	return true;
	//case OAPI_KEY_B:
	//	if (RefMode == TARGET_RELATIVE) {
	//		SelectBase();
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

	//// Trim functions
	//case OAPI_KEY_NUMPAD5:
	//	SetTrimMode(T_ALL);
	//	return true;
	//case OAPI_KEY_NUMPAD1:
	//	SetTrimMode(T_VERT);
	//	return true;
	//case OAPI_KEY_NUMPAD2:
	//	SetTrimMode(T_LAT);
	//	return true;
	//case OAPI_KEY_NUMPAD3:
	//	SetTrimMode(T_FA);
	//	return true;
	//case OAPI_KEY_NUMPAD7:
	//	SetTrimMode(T_VERT_LAT);
	//	return true;
	//case OAPI_KEY_NUMPAD8:
	//	SetTrimMode(T_LAT_FA);
	//	return true;
	//case OAPI_KEY_NUMPAD9:
	//	SetTrimMode(T_VERT_FA);
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


	//	{"Next Target", "(Target Mode)", 'N'},
	//	{"Prev Target", "(Target Mode)", 'P'},
	//	{"Closest Target", "(Target Mode)", ' '},
	//	{"Select Base", "(Target Mode)", 'B'},

	//
	//	{"Trim Vertical", "(Numpad)", '1'},
	//	{"Trim Lateral", "(Numpad)", '2'},
	//	{"Trim F/A", "(Numpad)", '3'},
	//	{"Trim All", "(Numpad)", '5'},
	//	{"Trim Vert, Lat", "(Numpad)", '7'},
	//	{"Trim Lat, F/A", "(Numpad)", '8'},
	//	{"Trim Vert, F/A", "(Numpad)", '9'}
	//};
	//if (menu) *menu = mnu;
	//return NUM_CMNDS;
}


bool AttitudeMFD::SelectBase()
{
	VESSELSTATUS Status;

	oapiGetFocusInterface()->GetStatus(Status);

	if (Status.base) {
		TargetRef = Status.base;
		oapiGetObjectName(TargetRef, TargetName, 20);

		Index = 0;

		return true;
	}

	return false;
}

void inline AttitudeMFD::PrintMFDHeading()
{
	char Buffer[100];

	SetTextColor(hDC, RGB(255, 255, 255));
	sprintf(Buffer, "Attitude MFD");
	TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
	SetTextColor(hDC, RGB(0, 255, 0));

	CurrentLine += LINE;
}

void AttitudeMFD::PrintRefMode()
{
	char Buffer[100], Buffer2[10];
		
	sprintf(Buffer, "Ref Mode:");
	TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
	
	switch (RefMode) {
	case USER_ATT:
		sprintf(Buffer, "Attitude");	
		break;
	case VELOCITY:
		sprintf(Buffer, "%s", "Velocity");
		break;
	case TARGET_RELATIVE:
		sprintf(Buffer, "%s", "Target Relative");
		break;
	case EI:
		sprintf(Buffer, "%s", "Entry Interface");
		break;
	};
	

	if (AttHoldMode == ENGAGED) {
		sprintf(Buffer2, "%s", " (Hold)");
		strcat(Buffer, Buffer2);
	}

	SetTextColor(hDC, RGB(255, 255, 255));
	TextOut(hDC, 90, CurrentLine, Buffer, strlen(Buffer));
	SetTextColor(hDC, RGB(0, 255, 0));

	CurrentLine += LINE;
}

void AttitudeMFD::PrintAngleRate(const char *Heading, double Angle, double Rate)
{
	const int TAB = 100;
	char Buffer[10];	

	TextOut(hDC, 5, CurrentLine, Heading, strlen(Heading));
	
	sprintf(Buffer, "%.2f", Angle);

	// Adjust for negative number
	if (Angle < 0) {
		if (ColorMode == COLOR_WHITE) {
			sprintf(Buffer, "%.2f", DEG * Angle);
			SetTextColor(hDC, RGB(255, 255, 255));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else if (ColorMode == COLOR_RED) {
			sprintf(Buffer, "%.2f", DEG * Angle);
			SetTextColor(hDC, RGB(255, 0, 0));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else {
			sprintf(Buffer, "%.2f", DEG * Angle);
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
		} 
	} else {
		sprintf(Buffer, "+%.2f", DEG * Angle);
		TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
	}

	// Adjust for negative number
	if (Rate < 0) {
		if (ColorMode == COLOR_WHITE) {
			sprintf(Buffer, "%.2f", DEG * Rate);
			SetTextColor(hDC, RGB(255, 255, 255));
			TextOut(hDC, 2 * TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else if (ColorMode == COLOR_RED) {
			sprintf(Buffer, "%.2f", DEG * Rate);
			SetTextColor(hDC, RGB(255, 0, 0));
			TextOut(hDC, 2 * TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else {
			sprintf(Buffer, "%.2f", DEG * Rate);
			TextOut(hDC, 2 * TAB, CurrentLine, Buffer, strlen(Buffer));
		} 
	} else {
		sprintf(Buffer, "+%.2f", DEG * Rate);
		TextOut(hDC, 2 * TAB, CurrentLine, Buffer, strlen(Buffer));
	}

	CurrentLine += LINE;
}

void AttitudeMFD::PrintAngle(const char *Heading, double Angle)
{
	const int TAB = 100;
	char Buffer[10];	

	TextOut(hDC, 5, CurrentLine, Heading, strlen(Heading));
	
	sprintf(Buffer, "%.2f", Angle);

	// Adjust for negative number
	if (Angle < 0) {
		if (ColorMode == COLOR_WHITE) {
			sprintf(Buffer, "%.2f", DEG * Angle);
			SetTextColor(hDC, RGB(255, 255, 255));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else if (ColorMode == COLOR_RED) {
			sprintf(Buffer, "%.2f", DEG * Angle);
			SetTextColor(hDC, RGB(255, 0, 0));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else {
			sprintf(Buffer, "%.2f", DEG * Angle);
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
		} 
	} else {
		sprintf(Buffer, "+%.2f", DEG * Angle);
		TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
	}

	CurrentLine += LINE;
}

void AttitudeMFD::PrintRate(const char *Heading, double Rate)
{
	const int TAB = 100;
	char Buffer[25];

	TextOut(hDC, 5, CurrentLine, Heading, strlen(Heading));
	
	ScaleOutput(Buffer, Rate);

	// Adjust for negative number
	if (Rate < 0) {
		ScaleOutput(Buffer, Rate);

		if (ColorMode == COLOR_WHITE) {
			//sprintf(Buffer, "%.2f", DEG * Rate);
			SetTextColor(hDC, RGB(255, 255, 255));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else if (ColorMode == COLOR_RED) {
			//sprintf(Buffer, "%.2f", DEG * Rate);
			SetTextColor(hDC, RGB(255, 0, 0));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else {
			//sprintf(Buffer, "%.2f", DEG * Rate);
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
		} 
	} else {
		// Add + sign
		Buffer[0] = '+';
		ScaleOutput(&Buffer[1], Rate);
		TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
	}

	CurrentLine += LINE;
}



void AttitudeMFD::PrintRate(const char *Heading, double Rate, bool Trim)
{
	const int TAB = 100;
	char Buffer[25];

	// If this axis is being trimmed, the heading is in white
	if (Trim) {
		SetTextColor(hDC, RGB(255, 255, 255));
	}
	
	TextOut(hDC, 5, CurrentLine, Heading, strlen(Heading));
	
	// Reset the text color
	SetTextColor(hDC, RGB(0, 255, 0));
	
	ScaleOutput(Buffer, Rate);

	// Adjust for negative number
	if (Rate < 0) {
		ScaleOutput(Buffer, Rate);

		if (ColorMode == COLOR_WHITE) {
			//sprintf(Buffer, "%.2f", DEG * Rate);
			SetTextColor(hDC, RGB(255, 255, 255));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else if (ColorMode == COLOR_RED) {
			//sprintf(Buffer, "%.2f", DEG * Rate);
			SetTextColor(hDC, RGB(255, 0, 0));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else {
			//sprintf(Buffer, "%.2f", DEG * Rate);
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
		} 
	} else {
		// Add + sign
		Buffer[0] = '+';
		ScaleOutput(&Buffer[1], Rate);
		TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
	}

	CurrentLine += LINE;
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

void AttitudeMFD::PrintPitchThrustLevel(double Level)
{
	HBRUSH OldBrush; 

	const int DISPLAY_START_X = 25, DISPLAY_START_Y = CurrentLine, 
				DISPLAY_WIDTH = LINE, DISPLAY_HEIGHT = 90, 
				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);


	// Print the overlaying display box
	PrintRect(hDC, DISPLAY_START_X, DISPLAY_START_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);

	// The color of the brush depends on if the thrust level is positive or negative
	if (Level >= 0) {
		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
	} else {
		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
	}
	
	BeginPath(hDC);
	PrintRect(hDC, DISPLAY_START_X, DISPLAY_MID_Y, DISPLAY_WIDTH, -((DISPLAY_HEIGHT/2) * Level)); 
	EndPath(hDC);
	StrokeAndFillPath(hDC);


	// Return to the original brush
	SelectObject(hDC, OldBrush);


}


void AttitudeMFD::PrintYawThrustLevel(double Level)
{
	HBRUSH OldBrush; 
	const int DISPLAY_START_X = 70, DISPLAY_START_Y = CurrentLine + (45 - (LINE / 2)), 
				DISPLAY_WIDTH = 90, DISPLAY_HEIGHT = LINE, 
				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);


	// Print the overlaying display box
	PrintRect(hDC, DISPLAY_START_X, DISPLAY_START_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);

	// The color of the brush depends on if the thrust level is positive or negative
	if (Level >= 0) {
		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
	} else {
		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
	}
	
	BeginPath(hDC);
	PrintRect(hDC, DISPLAY_MID_X, DISPLAY_START_Y, ((DISPLAY_WIDTH/2) * Level), DISPLAY_HEIGHT); 
	EndPath(hDC);
	StrokeAndFillPath(hDC);


	// Return to the original brush
	SelectObject(hDC, OldBrush);


}

void AttitudeMFD::PrintRollThrustLevel(double Level)
{
	HBRUSH OldBrush; 

	const int DISPLAY_START_X = 225, DISPLAY_START_Y = CurrentLine + 70, 
				DISPLAY_WIDTH = LINE, DISPLAY_HEIGHT = 90, 
				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);


	PrintArc(hDC, DISPLAY_START_X, DISPLAY_START_Y, 45, 45 - LINE, 179.0, -180.0);

	// The color of the brush depends on if the thrust level is positive or negative
	if (Level >= 0) {
		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
	} else {
		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
	}
	
	BeginPath(hDC);
	PrintArc(hDC, DISPLAY_START_X, DISPLAY_START_Y, 45, 45 - LINE, 90.0, -(90.0 * Level)); 
	EndPath(hDC);
	StrokeAndFillPath(hDC);


	// Return to the original brush
	SelectObject(hDC, OldBrush);


}

void AttitudeMFD::PrintRotThrust()
{
	char Buffer[100];
	int START_LINE = LINE * 17;
	
	if (CurrentLine < START_LINE) {
		CurrentLine = START_LINE;
	}

	MoveToEx(hDC, 0, CurrentLine, NULL);
	LineTo(hDC, Width, CurrentLine);
	MoveToEx(hDC, 60, CurrentLine, NULL);
	LineTo(hDC, 60, Height);
	MoveToEx(hDC, 170, CurrentLine, NULL);
	LineTo(hDC, 170, Height);

	CurrentLine += LINE / 4;

	SetTextColor(hDC, RGB(255, 255, 255));

	sprintf(Buffer, "%s", "Pitch");
	TextOut(hDC, 10, CurrentLine, Buffer, strlen(Buffer));
	sprintf(Buffer, "%s", "Yaw");
	TextOut(hDC, 105, CurrentLine, Buffer, strlen(Buffer));
	sprintf(Buffer, "%s", "Roll");
	TextOut(hDC, 205, CurrentLine, Buffer, strlen(Buffer));
	SetTextColor(hDC, RGB(0, 255, 0));

	CurrentLine += 1.5 * LINE;
	MoveToEx(hDC, 0, CurrentLine, NULL);
	LineTo(hDC, Width, CurrentLine);
	CurrentLine += LINE;

	PrintPitchThrustLevel(RotLevel.data[PITCH]);
	PrintYawThrustLevel(RotLevel.data[YAW]);
	PrintRollThrustLevel(RotLevel.data[ROLL]);

}

void AttitudeMFD::PrintRelVel()
{
	bool TrimVert = false,
			TrimLat = false,
			TrimFA = false;

	if (TrimStatus == T_ENGAGED) {
		if (TrimMode == T_VERT || 
			TrimMode == T_ALL || 
			TrimMode == T_VERT_LAT ||
			TrimMode == T_VERT_FA) {
			TrimVert = true;
		}

		 if (TrimMode == T_LAT ||
			 TrimMode == T_ALL ||
			 TrimMode == T_VERT_LAT ||
			 TrimMode == T_LAT_FA) {
			TrimLat = true;
		 }

		if (TrimMode == T_FA || 
			 TrimMode == T_ALL ||
			 TrimMode == T_LAT_FA ||
			TrimMode == T_VERT_FA) {
			TrimFA = true;
		}
	}

	PrintRate("Vertical", RelVel.y, TrimVert);
	PrintRate("Lateral", RelVel.x, TrimLat);
	PrintRate("Fore/Aft", RelVel.z, TrimFA);
}

bool inline AttitudeMFD::HaveMainEngine()
{
	return MaxMainThrust > 0;
}

bool inline AttitudeMFD::HaveRetroEngine()
{
	return MaxRetroThrust > 0;
}

bool inline AttitudeMFD::HaveHoverEngine()
{
	return MaxHoverThrust > 0;
}

bool cbSetRelAttPitch(void *id, char *str, void *data)
{
	return (((AttitudeMFD *)data)->SetRelAttitude(str, PITCH));
}

bool cbSetRelAttYaw(void *id, char *str, void *data)
{
	return (((AttitudeMFD *)data)->SetRelAttitude(str, YAW));
}

bool cbSetRelAttRoll(void *id, char *str, void *data)
{
	return (((AttitudeMFD *)data)->SetRelAttitude(str, ROLL));
}

bool cbSetMode(void *id, char *str, void *data)
{
	REF_MODE Mode = (REF_MODE)atoi(str);

	if (Mode >= USER_ATT && Mode <= EI) {
		((AttitudeMFD *)data)->ChangeRefMode(Mode);
		return true;
	}

	return false;
}

bool AttitudeMFD::SetRelAttitude(char *str, AXIS Axis)
{
	// This allows a space to represent a decimal point
	for (unsigned int i = 0; i < strlen(str); i++) {
		if (str[i] == ' ') {
			str[i] = '.';
			break;
		}
	}

	RelAttitude.data[Axis] = Radians(atof(str));
	return true;
}

