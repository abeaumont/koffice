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

#ifndef rotatecmd_h
#define rotatecmd_h

#include <qlist.h>

#include "command.h"
#include "kpobject.h"

class KPresenterDoc;

/******************************************************************/
/* Class: RotateCmd                                               */
/******************************************************************/

class RotateCmd : public Command
{
public:
    struct RotateValues
    {
        float angle;
    };

    RotateCmd( QString _name, QList<RotateValues> &_oldRotate, float _newAngle,
               QList<KPObject> &_objects, KPresenterDoc *_doc );
    ~RotateCmd();

    virtual void execute();
    virtual void unexecute();

protected:
    RotateCmd()
    {; }

    KPresenterDoc *doc;
    QList<RotateValues> oldRotate;
    QList<KPObject> objects;
    float newAngle;

};

#endif
