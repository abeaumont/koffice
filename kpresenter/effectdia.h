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

#ifndef EFFECTDIA_H
#define EFFECTDIA_H

#include <qdialog.h>
#include <qlist.h>

class KPresenterView;
class QWidget;
class QComboBox;
class QLabel;
class QPushButton;
class QCheckBox;
class QVBox;
class QResizeEvent;
class KPObject;

/******************************************************************/
/* class EffectDia                                                */
/******************************************************************/

class EffectDia : public QDialog
{
    Q_OBJECT

public:

    // constructor - destructor
    EffectDia( QWidget* parent, const char*, const QList<KPObject> &_objs,
	       KPresenterView* );

protected:
    void resizeEvent( QResizeEvent *e );

    QComboBox *cEffect, *cEffect2, *cDisappear;
    QLabel *lEffect, *lEffect2, *lNum, *lDisappear, *lDEffect;
    QSpinBox *eNum,*eDisappear;
    QPushButton *cancelBut, *okBut;
    QCheckBox *disappear;
    QVBox *back;

    KPresenterView *view;
    QList<KPObject> objs;

public slots:
    void slotEffectDiaOk();

signals:
    void effectDiaOk();

protected slots:
    void disappearChanged();
    void num1Changed( int num );
    void num2Changed( int num );

};

#endif //EFFECTDIA_H


