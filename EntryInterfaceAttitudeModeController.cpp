#include "EntryInterfaceAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "Display.h"

using namespace std;

EntryInterfaceAttitudeModeController::EntryInterfaceAttitudeModeController(
	VESSEL* spacecraft,
	const shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: BaseAttitudeModeControl(spacecraft, autopilot, createDisplay)
	, m_globalSpacecraftPosition(NULL_VECTOR)
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


	//char Buffer[100];

	//PrintRefMode();

	//if (Interface.InterfaceDefined) {
	//	sprintf(Buffer, "Entry Angle:   %.2f", Degree(Interface.Angle));
	//	TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
	//	CurrentLine += LINE;

	//	sprintf(Buffer, "Time To Go:    %.f", Interface.TimeToGo);
	//	TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
	//	CurrentLine += 2 * LINE;
	//}
	//else {
	//	CurrentLine += LINE;
	//	sprintf(Buffer, "Entry Interface Undefined");
	//	TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
	//	CurrentLine += 2 * LINE;

	//	return;
	//}


	//PrintAngle("Set Pitch", RelAttitude.data[PITCH]);
	//PrintAngle("Set Yaw ", RelAttitude.data[YAW]);
	//PrintAngle("Set Roll", RelAttitude.data[ROLL]);
	//CurrentLine += 2 * LINE;


	//PrintAngleRate("Pitch:", PitchYawRoll.data[PITCH], Status.vrot.x);
	//PrintAngleRate("Yaw:", PitchYawRoll.data[YAW], Status.vrot.y);
	//PrintAngleRate("Roll:", PitchYawRoll.data[ROLL], Status.vrot.z);

	//CurrentLine += 2 * LINE;

	//PrintRotThrust();

bool EntryInterfaceAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	auto display = m_createDisplay(sketchpad);

	display->Reset();
	
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->PrintNewline();

	display->DisplayText("Ref Mode: Entry Interface");
	
	return true;
}

//VECTOR3 H;
//CalcEntryInterface(Interface);
//
//if (!Interface.InterfaceDefined) {
//	return;
//}
//
//H = CrossProduct(Interface.Pos, Interface.Vel);
//
//RefAttitude = GetPYR2(Interface.Vel, H);
//
//PitchYawRoll = CalcPitchYawRollAngles();
void EntryInterfaceAttitudeModeController::UpdateState()
{
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

void EntryInterfaceAttitudeModeController::Control()
{
	if (m_isAutopilotEngaged)
	{
		m_autopilot->SetAttitude(NULL_VECTOR, m_pitchYawRollAngles, DB_FINE, 1.0);
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
