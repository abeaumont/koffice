/* This file is part of the KDE project
   Copyright (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 1999-2001 Laurent Montel <montel@kde.org>
             (C) 1998-1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kmessagebox.h>

#include "Doc.h"
#include "Localization.h"
#include "Sheet.h"
#include "View.h"
#include "Selection.h"

#include "AreaDialog.h"

using namespace KSpread;

AreaDialog::AreaDialog( View * parent, const char * name, const QPoint & _marker )
  : KDialog( parent )
{
  setCaption( i18n("Area Name") );
  setObjectName( name );
  setButtons( Ok | Cancel );
  setModal( true );

  m_pView  = parent;
  m_marker = _marker;

  QWidget * page = new QWidget();
  setMainWidget( page );
  QVBoxLayout * lay1 = new QVBoxLayout( page );
  lay1->setMargin(KDialog::marginHint());
  lay1->setSpacing(KDialog::spacingHint());

  QLabel * label = new QLabel( i18n("Enter the area name:"), page );
  lay1->addWidget( label );

  m_areaName = new QLineEdit(page);
  m_areaName->setMinimumWidth( m_areaName->sizeHint().width() * 3 );

  lay1->addWidget( m_areaName );
  m_areaName->setFocus();
  connect ( m_areaName, SIGNAL(textChanged ( const QString & )), this, SLOT(slotAreaNamechanged( const QString &)));
  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
  enableButtonOk(!m_areaName->text().isEmpty());

}

void AreaDialog::slotAreaNamechanged( const QString & text)
{
  enableButtonOk(!text.isEmpty());
}

void AreaDialog::slotOk()
{
  QString tmp(m_areaName->text());
  if( !tmp.isEmpty() )
  {
    tmp = tmp.toLower();

    QRect rect( m_pView->selection()->selection() );
    bool newName = true;
    QList<Reference>::Iterator it;
    QList<Reference> area = m_pView->doc()->listArea();
    for ( it = area.begin(); it != area.end(); ++it )
    {
      if(tmp == (*it).ref_name)
        newName = false;
    }
    if (newName)
    {
      m_pView->doc()->emitBeginOperation( false );
      m_pView->doc()->addAreaName(rect, tmp, m_pView->activeSheet()->sheetName());
      m_pView->slotUpdateView( m_pView->activeSheet() );
      accept();
    }
    else
      KMessageBox::error( this, i18n("This name is already used."));
  }
  else
  {
    KMessageBox::error( this, i18n("Area text is empty.") );
  }
}

#include "AreaDialog.moc"
