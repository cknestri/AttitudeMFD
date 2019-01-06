//bool cbSelectTarget(void *id, char *str, void *data);
	//bool cbSetMode(void *id, char *str, void *data);
	//bool cbSetRelAttPitch(void *id, char *str, void *data);
	//bool cbSetRelAttYaw(void *id, char *str, void *data);
	//bool cbSetRelAttRoll(void *id, char *str, void *data);

	//switch(key) {
	//// Mode keys - the M key only needs to be used as a backup, when the SHIFT + number
	//// is in use by a vessel, such as the shuttle's RMS
	//case OAPI_KEY_M:
	//	oapiOpenInputBox("Select Mode", cbSetMode, 0, 20, (void *)this);
	//case OAPI_KEY_1:
	//	ChangeRefMode(USER_ATT);
	//	return true;
	//case OAPI_KEY_2:
	//	ChangeRefMode(VELOCITY);
	//	return true;
	//case OAPI_KEY_3:
	//	ChangeRefMode(TARGET_RELATIVE);
	//	return true;
	//case OAPI_KEY_4:
	//	ChangeRefMode(EI);
	//	return true;

	//case OAPI_KEY_P:
	//	if (RefMode != TARGET_RELATIVE) {
	//		oapiOpenInputBox("Select Pitch", cbSetRelAttPitch, 0, 20, (void *)this);
	//	} else {
	//		SelectPrevTarget();
	//	}
	//	return true;
	//case OAPI_KEY_Y:
	//	if (RefMode != TARGET_RELATIVE) {
	//		oapiOpenInputBox("Select Yaw", cbSetRelAttYaw, 0, 20, (void *)this);
	//	}
	//	return true;
	//case OAPI_KEY_R:
	//	if (RefMode != TARGET_RELATIVE) {
	//		oapiOpenInputBox("Select Roll", cbSetRelAttRoll, 0, 20, (void *)this);
	//	}
	//	return true;


	//case OAPI_KEY_H:
	//	ToggleAttHoldMode();
	//	return true;
	//case OAPI_KEY_C:
	//	ToggleColorMode();
	//	return true;
	//case OAPI_KEY_PERIOD:
	//	if (RefMode == USER_ATT) {
	//		SetRefAttitude();
	//	}
	//	return true;

	//default:
	//	return false;
	//}







	//const int NUM_CMNDS = 22;

	//static const MFDBUTTONMENU mnu[NUM_CMNDS] = {
	//	{"User Att", "Mode", '1'},
	//	{"Velocity", "Mode", '2'},
	//	{"Target Rel", "Mode", '3'},
	//	{"Entry Interface", "Mode", '4'},
	//	{"Select Mode", 0, 'M'},
	//	{"Set Pitch", 0, 'P'},
	//	{"Set Yaw", 0, 'Y'},
	//	{"Set Roll", 0, 'R'},
	//	{"Attitude Hold", 0, 'H'},
	//	{"Set Reference", "Attitude", '.'},
	//	{"Toggle Color Mode", 0, 'C'},


	//};
	//if (menu) *menu = mnu;
	//return NUM_CMNDS;


//// Note: This is not part of class AttitudeMFD
//void PrintRect(HDC hDC, int StartX, int StartY, int Width, int Height)
//{
//	MoveToEx(hDC, StartX, StartY, NULL);
//
//	LineTo(hDC, StartX + Width, StartY);
//	LineTo(hDC, StartX + Width, StartY + Height);
//	LineTo(hDC, StartX, StartY + Height);
//	LineTo(hDC, StartX, StartY);
//}
//
//// Note: This is not part of class AttitudeMFD
//void PrintArc(HDC hDC, int X, int Y, int Radius, int InnerRadius, double InitAngle, double SweepAngle)
//{
//
//	MoveToEx(hDC, X + (Radius * cos(Radians(InitAngle))), Y - (Radius * sin(Radians(InitAngle))), NULL);
//	AngleArc(hDC, X, Y, Radius, InitAngle, SweepAngle);
//	LineTo(hDC, X + (InnerRadius * cos(Radians(InitAngle + SweepAngle))), Y - (InnerRadius * sin(Radians(InitAngle + SweepAngle)))); 
//	AngleArc(hDC, X, Y, InnerRadius, (InitAngle + SweepAngle), -SweepAngle);
//	LineTo(hDC, X + (Radius * cos(Radians(InitAngle))), Y - (Radius * sin(Radians(InitAngle))));
//}

//void AttitudeMFD::PrintPitchThrustLevel(double Level)
//{
//	HBRUSH OldBrush; 
//
//	const int DISPLAY_START_X = 25, DISPLAY_START_Y = CurrentLine, 
//				DISPLAY_WIDTH = LINE, DISPLAY_HEIGHT = 90, 
//				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
//				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);
//
//
//	// Print the overlaying display box
//	PrintRect(hDC, DISPLAY_START_X, DISPLAY_START_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
//
//	// The color of the brush depends on if the thrust level is positive or negative
//	if (Level >= 0) {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
//	} else {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
//	}
//	
//	BeginPath(hDC);
//	PrintRect(hDC, DISPLAY_START_X, DISPLAY_MID_Y, DISPLAY_WIDTH, -((DISPLAY_HEIGHT/2) * Level)); 
//	EndPath(hDC);
//	StrokeAndFillPath(hDC);
//
//
//	// Return to the original brush
//	SelectObject(hDC, OldBrush);
//
//
//}
//

//void AttitudeMFD::PrintYawThrustLevel(double Level)
//{
//	HBRUSH OldBrush; 
//	const int DISPLAY_START_X = 70, DISPLAY_START_Y = CurrentLine + (45 - (LINE / 2)), 
//				DISPLAY_WIDTH = 90, DISPLAY_HEIGHT = LINE, 
//				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
//				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);
//
//
//	// Print the overlaying display box
//	PrintRect(hDC, DISPLAY_START_X, DISPLAY_START_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
//
//	// The color of the brush depends on if the thrust level is positive or negative
//	if (Level >= 0) {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
//	} else {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
//	}
//	
//	BeginPath(hDC);
//	PrintRect(hDC, DISPLAY_MID_X, DISPLAY_START_Y, ((DISPLAY_WIDTH/2) * Level), DISPLAY_HEIGHT); 
//	EndPath(hDC);
//	StrokeAndFillPath(hDC);
//
//
//	// Return to the original brush
//	SelectObject(hDC, OldBrush);
//
//
//}

//void AttitudeMFD::PrintRollThrustLevel(double Level)
//{
//	HBRUSH OldBrush; 
//
//	const int DISPLAY_START_X = 225, DISPLAY_START_Y = CurrentLine + 70, 
//				DISPLAY_WIDTH = LINE, DISPLAY_HEIGHT = 90, 
//				DISPLAY_MID_X = DISPLAY_START_X + (DISPLAY_WIDTH/2),
//				DISPLAY_MID_Y = DISPLAY_START_Y + (DISPLAY_HEIGHT/2);
//
//
//	PrintArc(hDC, DISPLAY_START_X, DISPLAY_START_Y, 45, 45 - LINE, 179.0, -180.0);
//
//	// The color of the brush depends on if the thrust level is positive or negative
//	if (Level >= 0) {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrush);
//	} else {
//		OldBrush = (HBRUSH)SelectObject(hDC, ThrusterBrushNeg);		
//	}
//	
//	BeginPath(hDC);
//	PrintArc(hDC, DISPLAY_START_X, DISPLAY_START_Y, 45, 45 - LINE, 90.0, -(90.0 * Level)); 
//	EndPath(hDC);
//	StrokeAndFillPath(hDC);
//
//
//	// Return to the original brush
//	SelectObject(hDC, OldBrush);
//
//
//}
//
//void AttitudeMFD::PrintRotThrust()
//{
//	char Buffer[100];
//	int START_LINE = LINE * 17;
//	
//	if (CurrentLine < START_LINE) {
//		CurrentLine = START_LINE;
//	}
//
//	MoveToEx(hDC, 0, CurrentLine, NULL);
//	LineTo(hDC, Width, CurrentLine);
//	MoveToEx(hDC, 60, CurrentLine, NULL);
//	LineTo(hDC, 60, Height);
//	MoveToEx(hDC, 170, CurrentLine, NULL);
//	LineTo(hDC, 170, Height);
//
//	CurrentLine += LINE / 4;
//
//	SetTextColor(hDC, RGB(255, 255, 255));
//
//	sprintf(Buffer, "%s", "Pitch");
//	TextOut(hDC, 10, CurrentLine, Buffer, strlen(Buffer));
//	sprintf(Buffer, "%s", "Yaw");
//	TextOut(hDC, 105, CurrentLine, Buffer, strlen(Buffer));
//	sprintf(Buffer, "%s", "Roll");
//	TextOut(hDC, 205, CurrentLine, Buffer, strlen(Buffer));
//	SetTextColor(hDC, RGB(0, 255, 0));
//
//	CurrentLine += 1.5 * LINE;
//	MoveToEx(hDC, 0, CurrentLine, NULL);
//	LineTo(hDC, Width, CurrentLine);
//	CurrentLine += LINE;
//
//	/*PrintPitchThrustLevel(RotLevel.data[PITCH]);
//	PrintYawThrustLevel(RotLevel.data[YAW]);
//	PrintRollThrustLevel(RotLevel.data[ROLL]);*/
//
//}

//bool cbSetRelAttPitch(void *id, char *str, void *data)
//{
//	return (((AttitudeMFD *)data)->SetRelAttitude(str, PITCH));
//}
//
//bool cbSetRelAttYaw(void *id, char *str, void *data)
//{
//	return (((AttitudeMFD *)data)->SetRelAttitude(str, YAW));
//}
//
//bool cbSetRelAttRoll(void *id, char *str, void *data)
//{
//	return (((AttitudeMFD *)data)->SetRelAttitude(str, ROLL));
//}
//
//bool cbSetMode(void *id, char *str, void *data)
//{
//	REF_MODE Mode = (REF_MODE)atoi(str);
//
//	if (Mode >= USER_ATT && Mode <= EI) {
//		((AttitudeMFD *)data)->ChangeRefMode(Mode);
//		return true;
//	}
//
//	return false;
//}
//
//bool AttitudeMFD::SetRelAttitude(char *str, AXIS Axis)
//{
//	// This allows a space to represent a decimal point
//	for (unsigned int i = 0; i < strlen(str); i++) {
//		if (str[i] == ' ') {
//			str[i] = '.';
//			break;
//		}
//	}
//
//	RelAttitude.data[Axis] = Radians(atof(str));
//	return true;
//}

