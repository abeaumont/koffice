/* This file is part of the KDE project
   Copyright (C) 2005-2006 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kexiframe.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <kexiutils/utils.h>

//! @internal
class KexiFrame::Private
{
	public:
		Private()
		{
		}
		~Private()
		{
		}
		QColor frameColor;
#if 0
//todo
		KexiFrame::Shape frameShape;
		KexiFrame::Shadow frameShadow;
#endif
};

//=========================================================

KexiFrame::KexiFrame( QWidget * parent, const char * name, WFlags f )
	: QFrame(parent, name, f)
	, d( new Private() )
{
	//defaults
	d->frameColor = palette().active().foreground();
//! @todo obtain these defaults from current template's style...
	setLineWidth(2);
	setFrameStyle(QFrame::StyledPanel|QFrame::Raised);
}

KexiFrame::~KexiFrame()
{
	delete d;
}

#define ClassName KexiFrame
#define SuperClassName QFrame
#include "kexiframeutils_p.cpp"
#include "kexiframe.moc"
