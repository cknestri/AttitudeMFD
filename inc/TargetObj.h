//////////////////////////////////////////////////////////////////////////////////////////////
//
//	TargetObj.h - Header file for wrapper class used to stored OBJHANDLEs in a list
//
//	Copyright Chris Knestrick, 2002 
//	 
//	For a full description of the copyright, please see the file Copyright.txt
///////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TARGET_OBJ_H
#define TARGET_OBJ_H
#include "Orbitersdk.h"
#include "OrbiterMath.h"

class TargetObj{
public:
	TargetObj();
	TargetObj(OBJHANDLE Target);
	~TargetObj();

	OBJHANDLE GetObj();
	void SetObj(OBJHANDLE Object);

	bool operator>(TargetObj Source);
	bool operator<(TargetObj Source);
	bool operator>=(TargetObj Source);
	bool operator<=(TargetObj Source);

private:
	OBJHANDLE Obj;
};

#endif