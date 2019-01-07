#include "UserAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "CDK.h"

using namespace std;

UserAttitudeModeController::UserAttitudeModeController(
	VESSEL* spacecraft,
	const shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: BaseAttitudeModeControl(spacecraft, autopilot, createDisplay)
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
	
	PrintMFDName(display);
	PrintReferenceMode(display, "Attitude", m_isAutopilotEngaged);

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

	PrintRelativeAttitude(display, m_relativeAttitude);

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

	m_pitchYawRollAngles = CalcPitchYawRollAngles(m_referenceAttitude);
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

void UserAttitudeModeController::Control(double deltaTime)
{
	if (m_isAutopilotEngaged)
	{
		m_autopilot->SetAttitude(NULL_VECTOR, m_pitchYawRollAngles, DB_FINE, deltaTime);
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
	m_pitchYawRollAngles = CalcPitchYawRollAngles(m_referenceAttitude);
}