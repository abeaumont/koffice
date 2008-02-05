/* This file is part of the KDE project

   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#ifndef KOSHAPECONTROLLERBASE_H
#define KOSHAPECONTROLLERBASE_H

#include <QtGlobal>

class KoShape;

/**
 * The shape controller is an abstract interface that the applications class
 * that controls the shapes should implement.  This tends to be the document.
 * @see KoShapeDeleteCommand, KoShapeCreateCommand
 */
class KoShapeControllerBase {
public:
    KoShapeControllerBase() { }
    virtual ~KoShapeControllerBase() { }

    /**
     * Add a shape to the shape controller, allowing it to be seen and saved.
     * The controller should add the shape to the ShapeManager instance(s) manually
     * if the shape is one that should be currently shown on screen.
     * @param shape the new shape
     */
    virtual void addShape( KoShape* shape ) = 0;

    /**
     * Remove a shape from the shape controllers control, allowing it to be deleted shortly after
     * The controller should remove the shape from all the ShapeManager instance(s) manually
     * @param shape the shape to remove
     */
    virtual void removeShape( KoShape* shape ) = 0;

    /**
     * This method gets called after a shape is created. It can be used to set data in the shape that
     * is needed during loading e.g. KoStyleManager for the TextShape or to set application/shape specific 
     * data. The default implementation is empty so that only applications that want to use this have 
     * to reimplement the method.
     * @param shape the newly created shape
     */
    virtual void shapeCreated( KoShape* shape ) { Q_UNUSED(shape); }
};

#endif
