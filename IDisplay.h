#pragma once
#include <windows.h>

class IDisplay
{
	virtual void Reset() = 0;
	virtual void DisplayText(const char* format, ...) = 0;
	virtual void SetTextColor(DWORD textColor) = 0;
	virtual void IncrementCurrentLine() = 0;
	virtual void PrintAngle(const char* name, double angle) = 0;
	virtual void PrintAngleAndRate(const char* name, double angle, double rate) = 0;
};