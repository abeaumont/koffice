/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "kpresenter_doc.h"
#include "inspagedia.h"
#include "inspagedia.moc"

#include <qlabel.h>
#include <qpushbt.h>
#include <qradiobutton.h>

#include <kapp.h>
#include <qspinbox.h>
#include <klocale.h>

/******************************************************************/
/* class InsPageDia						  */
/******************************************************************/

/*==================== constructor ===============================*/
InsPageDia::InsPageDia( QWidget* parent, const char* name, KPresenterDoc *_doc, int currPageNum )
    : QDialog( parent, name, true )
{
    doc = _doc;

    QGridLayout* grid = new QGridLayout( this, 6, 5, 15, 7 );

    before = new QRadioButton( i18n( "&Before page:" ), this );
    before->resize( before->sizeHint() );
    connect( before, SIGNAL( clicked() ), this, SLOT( beforeClicked() ) );
    grid->addWidget( before, 0, 0 );

    after = new QRadioButton( i18n( "&After page:" ), this );
    after->resize( after->sizeHint() );
    connect( after, SIGNAL( clicked() ), this, SLOT( afterClicked() ) );
    grid->addWidget( after, 1, 0 );

    spinBox = new QSpinBox( 1, doc->getPageNums(), 1, this );
    spinBox->setValue( currPageNum );
    spinBox->resize( spinBox->sizeHint() );
    grid->addMultiCellWidget( spinBox, 0, 1, 1, 1 );

    leave = new QRadioButton( i18n( "&Leave all objects untouched." ), this );
    leave->resize( leave->sizeHint() );
    connect( leave, SIGNAL( clicked() ), this, SLOT( leaveClicked() ) );
    grid->addMultiCellWidget( leave, 2, 2, 0, 4 );

    _move = new QRadioButton( i18n( "&Move the objects which are behind the inserted page \n"
				    "one page forward, so that they stay on their current page." ), this );
    _move->resize( _move->sizeHint() );
    connect( _move, SIGNAL( clicked() ), this, SLOT( moveClicked() ) );
    grid->addMultiCellWidget( _move, 3, 3, 0, 4 );

    cancel = new QPushButton( this, "BCancel" );
    cancel->setText( i18n( "Cancel" ) );
    cancel->resize( cancel->sizeHint() );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    grid->addWidget( cancel, 5, 4 );

    ok = new QPushButton( this, "BOK" );
    ok->setText( i18n( "OK" ) );
    ok->setAutoRepeat( false );
    ok->setAutoResize( false );
    ok->setAutoDefault( true );
    ok->setDefault( true );
    ok->resize( cancel->size() );
    connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( ok, SIGNAL( clicked() ), this, SLOT( okClicked() ) );
    grid->addWidget( ok, 5, 3 );

    //resize( cancel->x() + cancel->width() + 20, cancel->y() + cancel->height() + 20 );
    uncheckAllPos();
    uncheckAllMode();
    after->setChecked( true );
    _move->setChecked( true );

    //   grid->addRowSpacing( 0, before->height() / 2 );
    //   grid->addRowSpacing( 1, after->height() / 2 );
    //   grid->addRowSpacing( 2, leave->height() );
    //   grid->addRowSpacing( 3, _move->height() );
    //   grid->addRowSpacing( 4, 0 );
    //   grid->addRowSpacing( 5, ok->height() );
    //   grid->setRowStretch( 4, 1 );

    //   grid->addColSpacing(0,max(before->width(),after->width()));
    //   grid->addColSpacing( 1, spinBox->width() );
    //   grid->addColSpacing(2,max(leave->width(),_move->width()) -
    //	      (max(before->width(),after->width()) + spinBox->width() +
    //	       ok->width() + cancel->width() + 30 ) );
    //   grid->addColSpacing( 3, ok->width() );
    //   grid->addColSpacing( 4, cancel->width() );
    //   grid->setColStretch( 2, 1 );

    //   grid->activate();
}

/*================================================================*/
void InsPageDia::uncheckAllMode()
{
    leave->setChecked( false );
    _move->setChecked( false );
}

/*================================================================*/
void InsPageDia::uncheckAllPos()
{
    before->setChecked( false );
    after->setChecked( false );
}

/*================================================================*/
void InsPageDia::leaveClicked()
{
    uncheckAllMode();
    leave->setChecked( true );
}

/*================================================================*/
void InsPageDia::moveClicked()
{
    uncheckAllMode();
    _move->setChecked( true );
}

/*================================================================*/
void InsPageDia::beforeClicked()
{
    uncheckAllPos();
    before->setChecked( true );
}

/*================================================================*/
void InsPageDia::afterClicked()
{
    uncheckAllPos();
    after->setChecked( true );
}

/*================================================================*/
void InsPageDia::okClicked()
{
    InsPageMode ipm = IPM_LET_OBJS;

    if ( leave->isChecked() )
	ipm = IPM_LET_OBJS;
    else if ( _move->isChecked() )
	ipm = IPM_MOVE_OBJS;

    InsertPos ip = IP_AFTER;

    if ( before->isChecked() )
	ip = IP_BEFORE;
    else if ( after->isChecked() )
	ip = IP_AFTER;

    pageNum = spinBox->value() - 1;
    insPageMode = ipm;
    insertPos = ip;

    emit insertPage( spinBox->value() - 1, ipm, ip );
}

