#include "UserAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "CDK.h"

using namespace std;

typedef struct {
	VECTOR3 Pitch;
	VECTOR3 Yaw;
	VECTOR3 Roll;			
} RefPoints;

UserAttitudeModeController::UserAttitudeModeController(
	VESSEL* spacecraft,
	const shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: BaseAttitudeModeControl(spacecraft, autopilot, createDisplay)
	, m_globalSpacecraftPosition(NULL_VECTOR)
	, m_pitchYawRollAngles(NULL_VECTOR)
	, m_referenceAttitude(NULL_VECTOR)
	, m_relativeAttitude(NULL_VECTOR)
	, m_isAutopilotEngaged(false)
{
	InitializeCommandMap();
}

UserAttitudeModeController::~UserAttitudeModeController()
{
}

void UserAttitudeModeController::Start()
{
	SetReferenceAttitude();
	CalculateAttitude();
}

bool UserAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	auto display = m_createDisplay(sketchpad);

	display->Reset();
	
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->PrintNewline();

	if (m_isAutopilotEngaged)
	{
		display->DisplayText("Ref Mode: Attitude (Hold)");
	}
	else
	{
		display->DisplayText("Ref Mode: Attitude");
	}


	display->DisplayText(
		"Reference Att: %.1f %.1f %.1f", 
		DEG * m_referenceAttitude.x,
		DEG * m_referenceAttitude.y,
		DEG * m_referenceAttitude.z);
	display->DisplayText(
		"Current Att:   %.1f %.1f %.1f", 
		DEG * m_status.arot.x,
		DEG * m_status.arot.y,
		DEG * m_status.arot.z );
	display->PrintNewline();

	display->PrintAngle("Set Pitch", m_relativeAttitude.data[PITCH]);
	display->PrintAngle("Set Yaw ", m_relativeAttitude.data[YAW]);
	display->PrintAngle("Set Roll", m_relativeAttitude.data[ROLL]);

	display->PrintNewline();
	display->PrintNewline();

	display->PrintAngleAndRate("Pitch:", m_pitchYawRollAngles.data[PITCH], m_status.vrot.x);
	display->PrintAngleAndRate("Yaw:", m_pitchYawRollAngles.data[YAW], m_status.vrot.y);
	display->PrintAngleAndRate("Roll:", m_pitchYawRollAngles.data[ROLL], m_status.vrot.z);

	display->PrintNewline();
	display->PrintNewline();

	return true;
}

void UserAttitudeModeController::UpdateState()
{
	m_spacecraft->GetStatus(m_status);
	oapiGetGlobalPos(m_spacecraft->GetHandle(), &m_globalSpacecraftPosition);

	m_pitchYawRollAngles = CalcPitchYawRollAngles();
}

void UserAttitudeModeController::EnableAutopilot()
{
	m_isAutopilotEngaged = true;
}

void UserAttitudeModeController::DisableAutopilot()
{
	m_isAutopilotEngaged = false;
	m_autopilot->ShutdownAllEngines();
}

void UserAttitudeModeController::Control()
{
	if (m_isAutopilotEngaged)
	{
		m_autopilot->SetAttitude(NULL_VECTOR, m_pitchYawRollAngles, DB_FINE, 1.0);
	}
}

void UserAttitudeModeController::SetReferenceAttitude()
{
	VESSELSTATUS status;
	m_spacecraft->GetStatus(status);

	m_referenceAttitude = status.arot;
}

int UserAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const
{
	static const MFDBUTTONMENU s_buttonMenu[] = {
		{"Set Reference", "Attitude", '.'},
		{"Set Pitch", 0, 'P'},
		{"Set Yaw", 0, 'Y'},
		{"Set Roll", 0, 'R'},
	};

	if (buttonMenu != NULL)
	{
		*buttonMenu = s_buttonMenu;
	}

	return (sizeof(s_buttonMenu)/sizeof(s_buttonMenu[0]));
}

bool UserAttitudeModeController::ProcessKey(DWORD key)
{
	auto iter = m_commandMap.find(key);

	if (iter != m_commandMap.end())
	{
		iter->second();
		return true;
	}

	return false;
}

void UserAttitudeModeController::InitializeCommandMap()
{
	m_commandMap[OAPI_KEY_PERIOD] = [this]() { SetReferenceAttitude(); };
}

void UserAttitudeModeController::CalculateAttitude()
{
	m_pitchYawRollAngles = CalcPitchYawRollAngles();
}

VECTOR3 UserAttitudeModeController::CalcPitchYawRollAngles()
{
	RefPoints globalPts, localPts;
	VECTOR3 pitchUnit = {0, 0, 1.0}, yawRollUnit = {1.0, 0, 0};

	RotateVector(pitchUnit, NULL_VECTOR, pitchUnit);
	RotateVector(yawRollUnit, NULL_VECTOR, yawRollUnit);

	RotateVector(pitchUnit, m_referenceAttitude, globalPts.Pitch);
	RotateVector(yawRollUnit, m_referenceAttitude, globalPts.Yaw);

	globalPts.Pitch = m_globalSpacecraftPosition + globalPts.Pitch;
	globalPts.Yaw = m_globalSpacecraftPosition + globalPts.Yaw;	

	m_spacecraft->Global2Local(globalPts.Pitch, localPts.Pitch);
	m_spacecraft->Global2Local(globalPts.Yaw, localPts.Yaw);

	return GetPYR(localPts.Pitch, localPts.Yaw);
}

VECTOR3 UserAttitudeModeController::GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll)
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