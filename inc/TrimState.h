#pragma once

class TrimState
{
public:
	TrimState();
	TrimState(bool isVerticalEnabled, bool isLateralEnabled, bool isForeAftEnabled);
	virtual ~TrimState();

	bool IsEnabled() const;
	bool IsVerticalEnabled() const;
	bool IsLateralEnabled() const;
	bool IsForeAftEnabled() const;

	void SetIsVerticalEnabled(bool isVerticalEnabled);
	void SetIsLateralEnabled(bool isLateralEnabled);
	void SetIsForeAftEnabled(bool isForeAftEnabled);


private:
	bool m_isVerticalEnabled;
	bool m_isLateralEnabled;
	bool m_isForeAftEnabled;
};
