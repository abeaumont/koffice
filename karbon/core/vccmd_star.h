/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VCCMDSTAR_H__
#define __VCCMDSTAR_H__

#include "vcommand.h"

// create a star-shape.

class VPath;

class VCCmdStar : public VCommand
{
public:
	VCCmdStar( KarbonPart* part, const double center_x, const double center_y,
		const double outer_r, const double inner_r, const uint edges );
	virtual ~VCCmdStar() {}

	virtual void execute();
	virtual void unexecute();

private:
	VPath* m_object;
	double m_centerX;
	double m_centerY;
	double m_outerR;
	double m_innerR;
	uint m_edges;
};

#endif
