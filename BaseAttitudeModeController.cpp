#include "BaseAttitudeModeController.h"

typedef struct {
	VECTOR3 Pitch;
	VECTOR3 Yaw;
	VECTOR3 Roll;
} RefPoints;

BaseAttitudeModeControl::BaseAttitudeModeControl(
	VESSEL* spacecraft,
	const std::shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: m_spacecraft(spacecraft)
	, m_autopilot(autopilot)
	, m_createDisplay(createDisplay)
{

}

BaseAttitudeModeControl::~BaseAttitudeModeControl()
{

}

void BaseAttitudeModeControl::PrintMFDName(const std::shared_ptr<IDisplay>& display) const
{
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->PrintNewline();
}

void BaseAttitudeModeControl::PrintReferenceMode(const std::shared_ptr<IDisplay>& display, const char * referenceMode, bool isAutopilotEngaged) const
{
	char* holdString = "";

	if (isAutopilotEngaged)
	{
		holdString = " (Hold)";
	}
	
	display->DisplayText("Ref Mode: %s%s", referenceMode, holdString);
	display->PrintNewline();
}

void BaseAttitudeModeControl::PrintRelativeAttitude(const std::shared_ptr<IDisplay>& display, const Attitude& relativeAttitude) const
{
	display->PrintAngle("Set Pitch", relativeAttitude.data[PITCH]);
	display->PrintAngle("Set Yaw ", relativeAttitude.data[YAW]);
	display->PrintAngle("Set Roll", relativeAttitude.data[ROLL]);
	display->PrintNewline();
}

VECTOR3 BaseAttitudeModeControl::CalcPitchYawRollAngles(const Attitude& referenceAttitude)
{
	RefPoints globalPts, localPts;
	VECTOR3 pitchUnit = { 0, 0, 1.0 }, yawRollUnit = { 1.0, 0, 0 };

	RotateVector(pitchUnit, NULL_VECTOR, pitchUnit);
	RotateVector(yawRollUnit, NULL_VECTOR, yawRollUnit);

	RotateVector(pitchUnit, referenceAttitude, globalPts.Pitch);
	RotateVector(yawRollUnit, referenceAttitude, globalPts.Yaw);

	VECTOR3 globalSpacecraftPosition;
	oapiGetGlobalPos(m_spacecraft->GetHandle(), &globalSpacecraftPosition);

	globalPts.Pitch = globalSpacecraftPosition + globalPts.Pitch;
	globalPts.Yaw = globalSpacecraftPosition + globalPts.Yaw;

	m_spacecraft->Global2Local(globalPts.Pitch, localPts.Pitch);
	m_spacecraft->Global2Local(globalPts.Yaw, localPts.Yaw);

	return GetPYR(localPts.Pitch, localPts.Yaw);
}

VECTOR3 BaseAttitudeModeControl::GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll)
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

VECTOR3 BaseAttitudeModeControl::GetReferenceAttitude(VECTOR3 Pitch, VECTOR3 YawRoll)
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
