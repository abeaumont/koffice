/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VCTOOLSINUS_H__
#define __VCTOOLSINUS_H__

#include "vshapetool.h"

class KarbonPart;
class VSinusDlg;

// A singleton state to create a sinus

class VCToolSinus : public VShapeTool
{
public:
	virtual ~VCToolSinus();
	static VCToolSinus* instance( KarbonPart* part );

	virtual VCommand* createCmd( double x, double y, double d1, double d2 );

	virtual void drawTemporaryObject(
		KarbonView* view, const KoPoint& p, double d1, double d2 );

	virtual void showDialog() const;

protected:
	VCToolSinus( KarbonPart* part );

private:
	static VCToolSinus* s_instance;

	VSinusDlg* m_dialog;
};

#endif

