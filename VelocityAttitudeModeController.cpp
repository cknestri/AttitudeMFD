#include "VelocityAttitudeModeController.h"
#include "CDK.h"

using namespace std;

typedef struct {
	VECTOR3 Pitch;
	VECTOR3 Yaw;
	VECTOR3 Roll;
} RefPoints;

static VECTOR3 GetReferenceAttitude(VECTOR3 Pitch, VECTOR3 YawRoll)
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

VelocityAttitudeModeController::VelocityAttitudeModeController(
	VESSEL* spacecraft,
	const shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: BaseAttitudeModeControl(spacecraft, autopilot, createDisplay)
	, m_globalSpacecraftPosition(NULL_VECTOR)
	, m_referenceAttitude(NULL_VECTOR)
	, m_relativeAttitude(NULL_VECTOR)
{
}

VelocityAttitudeModeController::~VelocityAttitudeModeController()
{

}

void VelocityAttitudeModeController::Start()
{
	UpdateState();
}

bool VelocityAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	auto display = m_createDisplay(sketchpad);

	display->Reset();
	
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->PrintNewline();

	display->DisplayText("Ref Mode: Velocity");
	display->PrintNewline();

	display->PrintAngle("Set Pitch", m_relativeAttitude.data[PITCH]);
	display->PrintAngle("Set Yaw ", m_relativeAttitude.data[YAW]);
	display->PrintAngle("Set Roll", m_relativeAttitude.data[ROLL]);
	display->PrintNewline();

	display->PrintAngleAndRate("Pitch:", m_pitchYawRollAngles.data[PITCH], m_status.vrot.x);
	display->PrintAngleAndRate("Yaw:", m_pitchYawRollAngles.data[YAW], m_status.vrot.y);
	display->PrintAngleAndRate("Roll:", m_pitchYawRollAngles.data[ROLL], m_status.vrot.z);

	return true;
}

void VelocityAttitudeModeController::UpdateState()
{
	m_spacecraft->GetStatus(m_status);
	oapiGetGlobalPos(m_spacecraft->GetHandle(), &m_globalSpacecraftPosition);

	VECTOR3 angularMomention = CrossProduct(m_status.rpos, m_status.rvel);

	m_referenceAttitude = GetReferenceAttitude(m_status.rvel, angularMomention);

	m_pitchYawRollAngles = CalcPitchYawRollAngles();
}

VECTOR3 VelocityAttitudeModeController::CalcPitchYawRollAngles()
{
	RefPoints globalPts, localPts;
	VECTOR3 pitchUnit = { 0, 0, 1.0 }, yawRollUnit = { 1.0, 0, 0 };

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

VECTOR3 VelocityAttitudeModeController::GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll)
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

void VelocityAttitudeModeController::EnableAutopilot()
{
	m_isAutopilotEngaged = true;
}

void VelocityAttitudeModeController::DisableAutopilot()
{
	m_isAutopilotEngaged = false;
	m_autopilot->ShutdownAllEngines();
}

void VelocityAttitudeModeController::Control(double deltaTime)
{
	if (m_isAutopilotEngaged)
	{
		m_autopilot->SetAttitude(NULL_VECTOR, m_pitchYawRollAngles, DB_FINE, deltaTime);
	}
}

int VelocityAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const
{
	return 0;
}

bool VelocityAttitudeModeController::ProcessKey(DWORD key)
{
	return false;
}

void VelocityAttitudeModeController::InitializeCommandMap()
{

}