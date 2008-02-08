/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KarbonOutlinePaintingStrategy.h"
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoLineBorder.h>

KarbonOutlinePaintingStrategy::KarbonOutlinePaintingStrategy( KoShapeManager * shapeManager )
    : KoShapeManagerPaintingStrategy( shapeManager )
{
    Q_ASSERT( shapeManager );
    shapeManager->setPaintingStrategy( this );
}

void KarbonOutlinePaintingStrategy::paint( KoShape * shape, QPainter &painter, const KoViewConverter &converter, bool forPrint )
{
    painter.save();
    painter.setMatrix( shape->absoluteTransformation(&converter) * painter.matrix() );

    KoLineBorder border( 0.0 );
    border.paintBorder( shape, painter, converter );

    painter.restore();
}
