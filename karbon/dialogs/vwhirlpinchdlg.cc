/* This file is part of the KDE project
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

#include <qevent.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qwidget.h>

#include <klocale.h>
#include <knuminput.h>
#include "vwhirlpinchdlg.h"

VWhirlPinchDlg::VWhirlPinchDlg( QWidget* parent, const char* name )
	: KDialog( parent, name, true, Qt::WStyle_Customize |
	  WType_Dialog | Qt::WStyle_NormalBorder | Qt::WStyle_Title )
{
	setCaption( i18n( "Whirl Pinch" ) );

	QBoxLayout* outerbox = new QHBoxLayout( this );

	// add input fields on the left:
	QGroupBox* group = new QGroupBox(
		2, Qt::Horizontal,
		i18n( "Properties" ), this );

 	outerbox->addWidget( group );

	new QLabel( i18n( "Angle:" ), group );
	m_angle = new KDoubleNumInput( 0, group );
	new QLabel( i18n( "Pinch:" ), group );
	m_pinch = new KDoubleNumInput( 0, group );
	new QLabel( i18n( "Radius:" ), group );
	m_radius = new KDoubleNumInput( 0, group );

	outerbox->addSpacing( 2 );

	// add buttons on the right side:
	QBoxLayout* innerbox = new QVBoxLayout( outerbox );

	innerbox->addStretch();

	QPushButton* okbutton = new QPushButton( i18n( "&Ok" ), this );
	QPushButton* cancelbutton = new QPushButton( i18n( "&Cancel" ), this );

	okbutton->setMaximumSize( okbutton->sizeHint() );
	cancelbutton->setMaximumSize( cancelbutton->sizeHint() );

	okbutton->setFocus();

	innerbox->addWidget( okbutton );
	innerbox->addSpacing( 2 );
	innerbox->addWidget( cancelbutton );

	// signals and slots:
	connect( okbutton, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( cancelbutton, SIGNAL( clicked() ), this, SLOT( reject() ) );
        resize( 300, 80);
}

double
VWhirlPinchDlg::angle() const
{
	return m_angle->value();
}

double
VWhirlPinchDlg::pinch() const
{
	return m_pinch->value();
}

double
VWhirlPinchDlg::radius() const
{
	return m_radius->value();
}

void
VWhirlPinchDlg::setAngle( double value )
{
    m_angle->setValue( value);
}

void
VWhirlPinchDlg::setPinch( double value )
{
    m_pinch->setValue(value);
}

void
VWhirlPinchDlg::setRadius( double value )
{
    m_radius->setValue( value);
}

#include "vwhirlpinchdlg.moc"

