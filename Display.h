#pragma once

#include "IDisplay.h"
#include "Orbitersdk.h"

class Display: public IDisplay
{
public:
	Display(oapi::Sketchpad* sketchPad, DWORD width, DWORD height);
	virtual ~Display();
	
	void Reset() override;
	void DisplayText(const char* format, ...) override;
	void SetTextColor(DWORD textColor) override;
	void PrintNewline() override;
	void PrintAngle(const char* name, double angle) override;
	void PrintAngleAndRate(const char* name, double angle, double rate) override;

private:
	oapi::Sketchpad* m_sketchPad;
	DWORD m_width;
	DWORD m_height;
	int m_currentLine;
	int m_lineHeight;

	static const int s_maxDisplayBufferSize = 100;
	char m_displayBuffer[s_maxDisplayBufferSize];
};