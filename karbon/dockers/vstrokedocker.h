/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002, The Karbon Developers

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __VSTROKEDOCKER_H__
#define __VSTROKEDOCKER_H__

#include <vdocker.h>

class QHButtonGroup;
class QWidget;

class TKUFloatSpinBox;

class KoMainWindow;
class KarbonView;
class KarbonPart;

class VStrokeDocker : public VDocker
{
	Q_OBJECT

public:
	 VStrokeDocker( KarbonPart* part, KarbonView* parent = 0L, const char* name = 0L );
	 VStroke stroke() { return *m_stroke; }
  
public slots:
	virtual void setStroke( const VStroke & );

private:
	QHButtonGroup *m_capGroup;
	QHButtonGroup *m_joinGroup;
	QWidget *mainWidget;
	KarbonPart *m_part;
	KarbonView *m_view;
	TKUFloatSpinBox *m_setLineWidth;

private slots:
	void slotCapChanged( int ID );
	void slotJoinChanged( int ID );
	void updateCanvas();
	void updateDocker();
	void widthChanged();

protected:
	VStroke *m_stroke;
};

#endif

