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

#include <qlabel.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qwidget.h>

#include <klocale.h>
#include <koMainWindow.h>
#include <kseparator.h>

#include "karbon_part.h"
#include "karbon_view.h"
#include "vcolor.h"
#include "vcolorslider.h"
#include "vfillcmd.h"
#include "vselection.h"
#include "vstrokecmd.h"

#include "vcolordocker.h"

VColorDocker::VColorDocker( KarbonPart* part, KarbonView* parent, const char* /*name*/ )
	: VDocker( parent->shell() ), m_part ( part ), m_view( parent )
{
	m_isStrokeDocker = false;
	setCaption( i18n( "Fill Color" ) );

	mainWidget = new QWidget( this );
	mTabWidget = new QTabWidget( mainWidget );
	
	/* ##### RGB WIDGET ##### */
	mRGBWidget = new QWidget( mTabWidget );
	QGridLayout *mainLayout = new QGridLayout( mRGBWidget, 4, 1 );
	
	//RGB
	mRedSlider = new VColorSlider( i18n( "R:" ), QColor( "red" ), QColor( "black" ), 0, 255, 0, mRGBWidget );
	mGreenSlider = new VColorSlider( i18n( "G:" ), QColor( "green" ), QColor( "black" ), 0, 255, 0, mRGBWidget );
	mBlueSlider = new VColorSlider( i18n( "B:" ), QColor( "blue" ), QColor( "black" ), 0, 255, 0, mRGBWidget );
	mainLayout->addWidget( mRedSlider, 1, 0 );
	mainLayout->addWidget( mGreenSlider, 2, 0 );
	mainLayout->addWidget( mBlueSlider, 3, 0 );

	//Connections for Sliders
	connect( mRedSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( updateRGB() ) );
	connect( mGreenSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( updateRGB() ) );
	connect( mBlueSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( updateRGB() ) );                                                                                              
	
	mainLayout->activate();
	mTabWidget->addTab( mRGBWidget, i18n( "RGB" ) );
	
	/* ##### CMYK WIDGET ##### */
	mCMYKWidget = new QWidget( mTabWidget );
	QGridLayout *mainCMYKLayout = new QGridLayout( mCMYKWidget, 4, 1);

	//Sliders
	mCyanSlider = new VColorSlider( i18n( "C:" ), QColor( "cyan" ), QColor( "white" ), 0, 100, 0, mCMYKWidget );
	mMagentaSlider = new VColorSlider( i18n( "M:" ), QColor( "magenta" ), QColor( "white" ), 0, 100, 0, mCMYKWidget );
	mYellowSlider = new VColorSlider( i18n( "Y:" ), QColor( "yellow" ), QColor( "white" ), 0, 100, 0, mCMYKWidget );
	mBlackSlider = new VColorSlider( i18n( "K:" ), QColor( "black" ), QColor( "white" ), 0, 100, 0, mCMYKWidget );
	mainCMYKLayout->addWidget( mCyanSlider, 1, 0 );
	mainCMYKLayout->addWidget( mMagentaSlider, 2, 0 );
	mainCMYKLayout->addWidget( mYellowSlider, 3, 0 );
	mainCMYKLayout->addWidget( mBlackSlider, 4, 0 );
	
	//Connections for Sliders
	connect( mCyanSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( updateCMYK() ) );
	connect( mMagentaSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( updateCMYK() ) );
	connect( mYellowSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( updateCMYK() ) );
	connect( mBlackSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( updateCMYK() ) );
	
	mainCMYKLayout->activate();
	mTabWidget->addTab( mCMYKWidget, i18n("CMYK") );
	
	//Opacity
	mOpacity = new VColorSlider( i18n( "Opacity:" ), QColor( "black" ), QColor( "white" ), 0, 100, 100, mainWidget );
	//TODO: Make "white" a transparent color
	connect( mOpacity, SIGNAL( valueChanged ( int ) ), this, SLOT( updateOpacity() ) );
	
	QVBoxLayout *mainWidgetLayout = new QVBoxLayout( mainWidget, 3 );
	mainWidgetLayout->addWidget( mTabWidget );
	mainWidgetLayout->addWidget( mOpacity );
	mainWidgetLayout->activate();
	mainWidget->setMinimumWidth( 194 );
	setWidget( mainWidget );
	
	m_Color = new VColor();
}

void VColorDocker::updateCanvas()
{
	switch( m_isStrokeDocker ) {
	case false:
		if( m_part )
			m_part->addCommand( new VFillCmd( &m_part->document(), VFill( *m_Color ) ), true );
		m_view->selectionChanged();
		break;
	case true:
		if( m_part )
			m_part->addCommand( new VStrokeColorCmd( &m_part->document(), m_Color ), true );
		m_view->selectionChanged();
		break;
	}
}

void VColorDocker::updateRGB()
{
	float r = mRedSlider->value() / 255.0;
	float g = mGreenSlider->value() / 255.0;
	float b = mBlueSlider->value() / 255.0;

	m_Color->setColorSpace( VColor::rgb );
	m_Color->set( r, g, b );
	updateCanvas();
}

void VColorDocker::updateCMYK()
{
	float c = mCyanSlider->value() / 100.0;
	float m = mMagentaSlider->value() / 100.0;
	float y = mYellowSlider->value() / 100.0;
	float k = mBlackSlider->value() / 100.0;

	m_Color->setColorSpace( VColor::cmyk );
	m_Color->set( c, m, y, k );
	updateCanvas();
}

void VColorDocker::updateOpacity()
{
	float op = mOpacity->value() / 100.0;
	m_Color->setOpacity( op );
	updateCanvas();
}

void VColorDocker::setFillDocker()
{
	m_isStrokeDocker = false;
	setCaption( i18n( "Fill Color" ) );
}

void VColorDocker::setStrokeDocker()
{
	m_isStrokeDocker = true;
	setCaption( i18n( "Stroke Color" ) );
}

#include "vcolordocker.moc"

