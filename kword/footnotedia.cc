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

#include "footnotedia.h"
#include "footnotedia.moc"

#include <qbuttongroup.h>
#include <qvbox.h>
#include <qradiobutton.h>

#include <klocale.h>
#include <qlayout.h>
#include <qlineedit.h>


/******************************************************************/
/* Class: KWFootNoteDia                                           */
/******************************************************************/

KWFootNoteDia::KWFootNoteDia( NoteType _noteType, KWFootNoteVariable::Numbering _numberingType, QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, QString::null, Ok|Cancel, Ok, true )
{

    setButtonOKText(i18n("&Insert"));

    setCaption( i18n("Insert Footnote / Endnote") );

    QVBox *page = makeVBoxMainWidget();

    QButtonGroup *grp = new QButtonGroup(  page );
    QGridLayout *grid = new QGridLayout( grp, 2, 2, KDialog::marginHint(), KDialog::spacingHint());
    m_rbAuto = new QRadioButton( i18n("Automatic"), grp );
    m_rbManual= new QRadioButton( i18n("Manual"), grp );

    grp->setExclusive( true );
    grid->addWidget( m_rbAuto, 0, 0);
    grid->addWidget( m_rbManual, 1, 0);
    if ( _numberingType == KWFootNoteVariable::Auto )
        m_rbAuto->setChecked( true );
    else
        m_rbManual->setChecked( true );
    m_footLine = new QLineEdit( grp);
    connect( m_footLine, SIGNAL( textChanged ( const QString & )), this, SLOT(footLineChanged( const QString & )));
    grid->addWidget( m_footLine, 1, 1);


    grp = new QButtonGroup( 2, Qt::Vertical, page );
    m_rbFootNote = new QRadioButton( i18n("Footnote"), grp );
    m_rbEndNote = new QRadioButton( i18n("Endnote"), grp );
    grp->setExclusive( true );
    grp->insert( m_rbFootNote );
    grp->insert( m_rbEndNote );
    if (_noteType == FootNote )
        m_rbFootNote->setChecked( true );
    else
        m_rbEndNote->setChecked( true );

}

void KWFootNoteDia::footLineChanged( const QString & )
{
    m_rbManual->setChecked( true );
}

NoteType KWFootNoteDia::noteType() const
{
    return m_rbFootNote->isChecked() ? FootNote : EndNote;
}

KWFootNoteVariable::Numbering KWFootNoteDia::numberingType()const
{
    return m_rbAuto->isChecked() ? KWFootNoteVariable::Auto : KWFootNoteVariable::Manual;
}

QString KWFootNoteDia::manualString()const
{
    return m_footLine->text();
}
