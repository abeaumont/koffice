/* This file is part of the KDE project
   Copyright (C) 2002-2004 Ariya Hidayat <ariya@kde.org>
             (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 1999-2002 Laurent Montel <montel@kde.org>
             (C) 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2000-2001 Werner Trobin <trobin@kde.org>
             (C) 1998-2000 Torben Weis <weis@kde.org>

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

#ifndef __kspread_dlg_series__
#define __kspread_dlg_series__

#include <kdialogbase.h>

class KSpreadView;
class KSpreadSheet;
class KSpreadCell;
class QRadioButton;
class QCheckBox;
class KDoubleNumInput;

class KSpreadSeriesDlg : public KDialogBase
{
    Q_OBJECT
public:

    KSpreadSeriesDlg(KSpreadView* parent, const char* name,const QPoint &_marker);

    KSpreadSheet* table;

public slots:
    void slotOk();
protected:
    KSpreadView* m_pView;
    KDoubleNumInput *start;
    KDoubleNumInput *end;
    KDoubleNumInput *step;

    QRadioButton* column;
    QRadioButton* row;
    QRadioButton* linear;
    QRadioButton* geometric;
    QPoint  marker;
};

#endif
