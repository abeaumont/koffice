// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2005 Thorsten Zachmann <zachmann@kde.org>

   The code is mostly a copy from kword/framedia.cc

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

#ifndef TEXTPROPERTY_H
#define TEXTPROPERTY_H

#include <qwidget.h>

#include <koUnit.h>

#include "kprcommand.h"

class KPMarginWidget;
class QCheckBox;

class TextProperty : public QWidget
{
    Q_OBJECT
public:
    enum TextValueChange
    {
        ProtectContent = 1,
        Margins = 2
    };

    TextProperty( QWidget *parent, const char *name, const MarginsStruct &marginsStruct,
                  const KoUnit::Unit unit, PropValue protectContent );
    ~TextProperty();

    int getTextPropertyChange() const;
    MarginsStruct getMarginsStruct() const;
    bool getProtectContent() const;

    void apply();

private:
    KPMarginWidget *m_margins;
    QCheckBox *m_protectContentCheck;
    KoUnit::Unit m_unit;

    PropValue m_protectContent;

protected slots:
    void slotReset();
    void slotProtectContentChanged( bool b );
};

#endif /* TEXTPROPERTY_H */
