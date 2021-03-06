/*
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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
#ifndef TEMPLATESHAPE_H
#define TEMPLATESHAPE_H

#include "IconShape.h"

#include <KShapeFactoryBase.h>

/**
 * The shape selector uses IconShapes to represent a shape factory.
 * The shape selector is meant to allow the user to insert any type of flake shape
 * and the way that works in Flake is that there is a KShapeFactoryBase per shape-type.
 * Each factory is capable of inserting at least one type of shape into a KOffice
 * document. To visualize the shape without having to actually ask the factory to
 * create one (which may be expensive) either the TemplateShape or the GroupShape is used
 * to show the icon (KShapeFactoryBase::icon()) and things like a tooltip.
 * The TemplateShape is used for a KoShapeTemplate, when the factory has none (so only
 * KShapeFactoryBase::createDefaultShape() can be called) the GroupShape is used instead.
 */
class TemplateShape : public IconShape
{
public:
    TemplateShape(const KoShapeTemplate &shapeTemplate);

    virtual void visit(KCreateShapesTool *tool);
    virtual void save(QDomElement &root);

    virtual QString toolTip();

    const KoShapeTemplate &shapeTemplate() const { return m_shapeTemplate; }

    static TemplateShape *createShape(const QDomElement &element);

private:
    KoShapeTemplate m_shapeTemplate;
};

#endif
