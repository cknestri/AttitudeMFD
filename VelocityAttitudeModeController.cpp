#include "VelocityAttitudeModeController.h"
#include "CDK.h"

using namespace std;

VelocityAttitudeModeController::VelocityAttitudeModeController(
	VESSEL* spacecraft,
	const shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: BaseAttitudeModeControl(spacecraft, autopilot, createDisplay)
	, m_referenceAttitude(NULL_VECTOR)
	, m_relativeAttitude(NULL_VECTOR)
	, m_baseRotationRate(NULL_VECTOR)
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
	
	PrintMFDName(display);
	PrintReferenceMode(display, "Velocity", m_isAutopilotEngaged);
	PrintRelativeAttitude(display, m_relativeAttitude);

	display->PrintAngleAndRate("Pitch:", m_pitchYawRollAngles.data[PITCH], m_status.vrot.x);
	display->PrintAngleAndRate("Yaw:", m_pitchYawRollAngles.data[YAW], m_status.vrot.y);
	display->PrintAngleAndRate("Roll:", m_pitchYawRollAngles.data[ROLL], m_status.vrot.z);

	return true;
}

void VelocityAttitudeModeController::UpdateState()
{
	m_spacecraft->GetStatus(m_status);

	VECTOR3 angularMomention = CrossProduct(m_status.rpos, m_status.rvel);

	m_referenceAttitude = GetReferenceAttitude(m_status.rvel, angularMomention);

	m_pitchYawRollAngles = CalcPitchYawRollAngles(m_referenceAttitude);

	m_baseRotationRate = CalculateBaseRotationRate();
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
		m_autopilot->SetAttitude(NULL_VECTOR, m_pitchYawRollAngles, m_baseRotationRate, DB_FINE, deltaTime);
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

VECTOR3 VelocityAttitudeModeController::CalculateBaseRotationRate() const
{
	// Calculate the base rate at which the velocity vectory is changing
	ELEMENTS_VEC Elements;
	GetElements(Elements);

	VECTOR3 baseRotationRate = NULL_VECTOR;

	baseRotationRate.x = -(sqrt(GravBodyData[Elements.Body].Mu / Elements.p) * (1 + Elements.e * Elements.TrueAnomaly)) / Elements.r;
	
	//RotateVector(AngularRate, RelAttitude, AngularRate);

	return baseRotationRate;
}