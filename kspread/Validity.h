/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2004 Tomas Mecir <mecirt@gmail.com>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002,2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2003 Stefan Hetzl <shetzl@chello.at>
   Copyright 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1999-2001 David Faure <faure@kde.org>
   Copyright 2000-2001 Werner Trobin <trobin@kde.org>
   Copyright 2000 Simon Hausmann <hausmann@kde.org
   Copyright 1998-1999 Torben Weis <weis@kde.org>
   Copyright 1999 Michael Reiher <michael.reiher@gmx.de>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef KSPREAD_VALIDITY
#define KSPREAD_VALIDITY

// Qt
#include <QDate>
#include <QSharedData>
#include <QStringList>
#include <QTime>

// KOffice
#include <KoXmlReader.h>

// KSpread
#include "Condition.h"

namespace KSpread
{

/**
 * This namespace collects enumerations related to
 * cell content restrictions.
 */
namespace Restriction
{
  /**
   * The type of the restriction
   */
  enum Type
  {
    None /** No restriction */,
    Number /** Restrict to numbers */,
    Text /** Restrict to texts */,
    Time /** Restrict to times */,
    Date /** Restrict to dates */,
    Integer /** Restrict to integers  */,
    TextLength /** Restrict text length */,
    List /** Restrict to lists */
  };
} // namespace Restriction

namespace Action
{
  enum Type
  {
    Stop,
    Warning,
    Information
  };
}

class Validity : public QSharedData
{
public:
    Validity();

    bool testValidity( const Cell* cell ) const;

    bool loadXML( Cell* const cell, const KoXmlElement& validityElement );
    QDomElement saveXML( QDomDocument& doc ) const;

    Action::Type action() const;
    bool allowEmptyCell() const;
    Conditional::Type condition() const;
    bool displayMessage() const;
    bool displayValidationInformation() const;
    const QString& messageInfo() const;
    const QDate& maximumDate() const;
    const QTime& maximumTime() const;
    double maximumValue() const;
    const QString& message() const;
    const QDate& minimumDate() const;
    const QTime& minimumTime() const;
    double minimumValue() const;
    Restriction::Type restriction() const;
    const QString& title() const;
    const QString& titleInfo() const;
    const QStringList& validityList() const;

    void setAction( Action::Type action );
    void setAllowEmptyCell( bool allow );
    void setCondition( Conditional::Type condition );
    void setDisplayMessage( bool display );
    void setDisplayValidationInformation( bool display );
    void setMaximumDate( const QDate& date );
    void setMaximumTime( const QTime& time );
    void setMaximumValue( double value );
    void setMessage( const QString& message );
    void setMessageInfo( const QString& info );
    void setMinimumDate( const QDate& date );
    void setMinimumTime( const QTime& time );
    void setMinimumValue( double value );
    void setRestriction( Restriction::Type restriction );
    void setTitle( const QString& title );
    void setTitleInfo( const QString& info );
    void setValidityList( const QStringList& list );

    void operator=( const Validity& other ) const;
    bool operator==( const Validity& other ) const;
    inline bool operator!=( const Validity& other ) const { return !operator==( other ); }

private:
    QString m_message;
    QString m_title;
    QString m_titleInfo;
    QString m_messageInfo;
    double valMin;
    double valMax;
    Conditional::Type m_cond;
    Action::Type m_action;
    Restriction::Type m_restriction;
    QTime  timeMin;
    QTime  timeMax;
    QDate  dateMin;
    QDate  dateMax;
    bool m_displayMessage;
    bool m_allowEmptyCell;
    bool m_displayValidationInformation;
    QStringList m_listValidity;
};

} // namespace KSpread

#endif // KSPREAD_VALIDITY
