#include "EntryInterfaceAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "Display.h"

using namespace std;

EntryInterfaceAttitudeModeController::EntryInterfaceAttitudeModeController(
	VESSEL* spacecraft,
	const shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: BaseAttitudeModeControl(spacecraft, autopilot, createDisplay)
	, m_referenceAttitude(NULL_VECTOR)
	, m_relativeAttitude(NULL_VECTOR)
{
	InitializeCommandMap();
}

EntryInterfaceAttitudeModeController::~EntryInterfaceAttitudeModeController()
{

}

void EntryInterfaceAttitudeModeController::Start()
{
	UpdateState();
}

bool EntryInterfaceAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	auto display = m_createDisplay(sketchpad);

	display->Reset();
	
	PrintMFDName(display);
	PrintReferenceMode(display, "Entry Interface", m_isAutopilotEngaged);

	if (!m_entryInterface.InterfaceDefined)
	{
		display->DisplayText("Entry Interface Undefinited");

		return true;
	}

	display->DisplayText("Entry Angle:   %.2f", Degree(m_entryInterface.Angle));
	display->DisplayText("Time To Go:    %.f", m_entryInterface.TimeToGo);
	display->PrintNewline();

	PrintRelativeAttitude(display, m_relativeAttitude);
	
	display->PrintAngleAndRate("Pitch:", m_pitchYawRollAngles.data[PITCH], m_status.vrot.x);
	display->PrintAngleAndRate("Yaw:", m_pitchYawRollAngles.data[YAW], m_status.vrot.y);
	display->PrintAngleAndRate("Roll:", m_pitchYawRollAngles.data[ROLL], m_status.vrot.z);

	return true;
}


void EntryInterfaceAttitudeModeController::UpdateState()
{
	m_spacecraft->GetStatus(m_status);

	CalcEntryInterface(m_entryInterface);

	if (!m_entryInterface.InterfaceDefined)
	{
		return;
	}

	VECTOR3 angularMomentum = CrossProduct(m_entryInterface.Pos, m_entryInterface.Vel);

	m_referenceAttitude = GetReferenceAttitude(m_entryInterface.Vel, angularMomentum);

	m_pitchYawRollAngles = CalcPitchYawRollAngles(m_referenceAttitude);
}

void EntryInterfaceAttitudeModeController::EnableAutopilot()
{
	m_isAutopilotEngaged = true;
}

void EntryInterfaceAttitudeModeController::DisableAutopilot()
{
	m_isAutopilotEngaged = false;
	m_autopilot->ShutdownAllEngines();
}

void EntryInterfaceAttitudeModeController::Control(double deltaTime)
{
	if (m_isAutopilotEngaged)
	{
		m_autopilot->SetAttitude(NULL_VECTOR, m_pitchYawRollAngles, DB_FINE, deltaTime);
	}
}


int EntryInterfaceAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const
{
	return 0;
}

bool EntryInterfaceAttitudeModeController::ProcessKey(DWORD key)
{
	return false;
}

void EntryInterfaceAttitudeModeController::InitializeCommandMap()
{

}
