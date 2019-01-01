#include "TrimState.h"

TrimState::TrimState() : TrimState(false, false, false)
{
}

TrimState::TrimState(bool isVerticalEnabled, bool isLateralEnabled, bool isForeAftEnabled)
	: m_isVerticalEnabled(isVerticalEnabled)
	, m_isLateralEnabled(isLateralEnabled)
	, m_isForeAftEnabled(isForeAftEnabled)
{
}

TrimState::~TrimState()
{
}

bool TrimState::IsEnabled() const
{
	return IsVerticalEnabled() || IsLateralEnabled() || IsForeAftEnabled();
}

bool TrimState::IsVerticalEnabled() const
{
	return m_isVerticalEnabled;
}

bool TrimState::IsLateralEnabled() const
{
	return m_isLateralEnabled;
}

bool TrimState::IsForeAftEnabled() const
{
	return m_isForeAftEnabled;
}

void TrimState::SetIsVerticalEnabled(bool isVerticalEnabled)
{
	m_isVerticalEnabled = isVerticalEnabled;
}

void TrimState::SetIsLateralEnabled(bool isLateralEnabled)
{
	m_isLateralEnabled = isLateralEnabled;
}

void TrimState::SetIsForeAftEnabled(bool isForeAftEnabled)
{
	m_isForeAftEnabled = isForeAftEnabled;
}
