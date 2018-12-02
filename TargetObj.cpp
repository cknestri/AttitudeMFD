//////////////////////////////////////////////////////////////////////////////////////////////
//
//	TargetObj.cpp - Source file for wrapper class used to stored OBJHANDLEs in a list
//
//	Copyright Chris Knestrick, 2002 
//	 
//	For a full description of the copyright, please see the file Copyright.txt
///////////////////////////////////////////////////////////////////////////////////////////////
#include "TargetObj.h"

// Constructors
TargetObj::TargetObj()
{
}

TargetObj::TargetObj(OBJHANDLE Target)
{
	Obj = Target;
}

// Destructor
TargetObj::~TargetObj()
{
	Obj = NULL;
}

// Get and Set methods
OBJHANDLE TargetObj::GetObj()
{
	return Obj;
}

void TargetObj::SetObj(OBJHANDLE Object)
{
	Obj = Object;
}

// Operators - the eqaulity operator is not defined because the key value is a real
bool TargetObj::operator>(TargetObj Source)
{	
	VECTOR3 Pos1, Pos2;
	VESSEL *Spacecraft = oapiGetFocusInterface();
	double Distance1, Distance2;

	Spacecraft->GetRelativePos(Obj, Pos1);
	Distance1 = Mag(Pos1);
	Spacecraft->GetRelativePos(Source.GetObj(), Pos2);
	Distance2 = Mag(Pos2);

	return Distance1 > Distance2;
}

bool TargetObj::operator<(TargetObj Source)
{
	VECTOR3 Pos1, Pos2;
	VESSEL *Spacecraft = oapiGetFocusInterface();
	double Distance1, Distance2;

	Spacecraft->GetRelativePos(Obj, Pos1);
	Distance1 = Mag(Pos1);
	Spacecraft->GetRelativePos(Source.GetObj(), Pos2);
	Distance2 = Mag(Pos2);

	return Distance1 < Distance2;
}

bool TargetObj::operator<=(TargetObj Source)
{
	VECTOR3 Pos1, Pos2;
	VESSEL *Spacecraft = oapiGetFocusInterface();
	double Distance1, Distance2;

	Spacecraft->GetRelativePos(Obj, Pos1);
	Distance1 = Mag(Pos1);
	Spacecraft->GetRelativePos(Source.GetObj(), Pos2);
	Distance2 = Mag(Pos2);

	return Distance1 <= Distance2;
}

bool TargetObj::operator>=(TargetObj Source)
{
	VECTOR3 Pos1, Pos2;
	VESSEL *Spacecraft = oapiGetFocusInterface();
	double Distance1, Distance2;

	Spacecraft->GetRelativePos(Obj, Pos1);
	Distance1 = Mag(Pos1);
	Spacecraft->GetRelativePos(Source.GetObj(), Pos2);
	Distance2 = Mag(Pos2);

	return Distance1 >= Distance2;
}