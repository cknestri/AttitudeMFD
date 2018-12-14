#include "Display.h"
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

Display::Display(oapi::Sketchpad* sketchPad, DWORD width, DWORD height)
	: m_sketchPad(sketchPad)
	, m_width(width)
	, m_height(height)
	, m_currentLine(0)
{
	m_lineHeight = height / 28;
}

Display::~Display()
{
}

void Display::Reset()
{
	m_currentLine = 0;
}

void Display::DisplayText(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	vsnprintf(m_displayBuffer, sizeof(m_displayBuffer), format, args);
	m_sketchPad->Text(5, m_currentLine, m_displayBuffer, strlen(m_displayBuffer));
	
	va_end(args);
	
	IncrementCurrentLine();
}

void Display::SetTextColor(DWORD textColor)
{
	m_sketchPad->SetTextColor(textColor);
}

void Display::PrintAngle(const char* name, double angle)
{
	const int TAB = 100;
	
	m_sketchPad->Text(5, m_currentLine,  name, strlen(name));

	if (angle < 0)
	{
		sprintf(m_displayBuffer, "%.2f", DEG * angle);
	}
	else
	{
		// Explicitly add the "+"
		sprintf(m_displayBuffer, "+%.2f", DEG * angle);
	}

	m_sketchPad->Text(TAB, m_currentLine,  m_displayBuffer, strlen(m_displayBuffer));
	
	IncrementCurrentLine();

	// Adjust for negative number
	/*if (Angle < 0) {
		if (ColorMode == COLOR_WHITE) {
			sprintf(Buffer, "%.2f", DEG * Angle);
			SetTextColor(hDC, RGB(255, 255, 255));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else if (ColorMode == COLOR_RED) {
			sprintf(Buffer, "%.2f", DEG * Angle);
			SetTextColor(hDC, RGB(255, 0, 0));
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
			SetTextColor(hDC, RGB(0, 255, 0));
		} else {
			sprintf(Buffer, "%.2f", DEG * Angle);
			TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
		} 
	} else {
		sprintf(Buffer, "+%.2f", DEG * Angle);
		TextOut(hDC, TAB, CurrentLine, Buffer, strlen(Buffer));
	}

	CurrentLine += LINE;*/
}

void Display::PrintAngleAndRate(const char* name, double angle, double rate)
{
	const int TAB = 100;
	
	m_sketchPad->Text(5, m_currentLine,  name, strlen(name));

	if (angle < 0)
	{
		sprintf(m_displayBuffer, "%.2f", DEG * angle);
	}
	else
	{
		// Explicitly add the "+"
		sprintf(m_displayBuffer, "+%.2f", DEG * angle);
	}

	m_sketchPad->Text(TAB, m_currentLine,  m_displayBuffer, strlen(m_displayBuffer));
	
	if (rate < 0)
	{
		sprintf(m_displayBuffer, "%.2f", DEG * rate);
	}
	else
	{
		// Explicitly add the "+"
		sprintf(m_displayBuffer, "+%.2f", DEG * rate);
	}

	m_sketchPad->Text(TAB * 2, m_currentLine,  m_displayBuffer, strlen(m_displayBuffer));

	IncrementCurrentLine();
}
void Display::IncrementCurrentLine()
{
	m_currentLine += m_lineHeight;
}