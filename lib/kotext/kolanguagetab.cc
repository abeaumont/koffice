/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include "kolanguagetab.h"
#include "koGlobal.h"

#include <kcombobox.h>
#include <kiconloader.h>

#include <qstringlist.h>

#include "kolanguagetab.moc"


KoLanguageTab::KoLanguageTab( KSpell2::Broker::Ptr broker, QWidget* parent, const char* name, WFlags fl ) 
        : KoLanguageTabBase( parent, name, fl )
{
    const QStringList langNames = KoGlobal::listOfLanguages();
    const QStringList langTags = KoGlobal::listTagOfLanguages();
    QStringList spellCheckLanguages;

    if ( broker )
        spellCheckLanguages = broker->languages();

    QStringList::ConstIterator itName = langNames.begin();
    QStringList::ConstIterator itTag = langTags.begin();
    for ( ; itName != langNames.end() && itTag != langTags.end(); ++itName, ++itTag )
    {
        if ( spellCheckLanguages.find( *itTag ) != spellCheckLanguages.end() )
            languageKComboBox->insertItem( SmallIcon( "spellcheck" ), *itName );
        else
            languageKComboBox->insertItem( *itName );
    }
    connect( languageKComboBox, SIGNAL( activated( int ) ), this, SIGNAL( languageChanged( int ) ) );
}

KoLanguageTab::~KoLanguageTab()
{
}

QString KoLanguageTab::getLanguage()
{
    return languageKComboBox->currentText();
}

void KoLanguageTab::setLanguage( QString item )
{
    languageKComboBox->setCurrentText( item );
}
