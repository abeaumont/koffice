/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KCREATESHAPESTOOL_H
#define KCREATESHAPESTOOL_H

#include "KInteractionTool.h"

#include "flake_export.h"

#include <QString>

class KCanvasBase;
class KProperties;
class KCreateShapesToolPrivate;

#define KoCreateShapesTool_ID "CreateShapesTool"

/**
 * A tool to create shapes with.
 */
class FLAKE_EXPORT KCreateShapesTool : public KInteractionTool
{
public:
    /**
     * Create a new tool; typically not called by applications, only by the KToolManager
     * @param canvas the canvas this tool works for.
     */
    explicit KCreateShapesTool(KCanvasBase *canvas);
    /// destructor
    virtual ~KCreateShapesTool();
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);

    void paint(QPainter &painter, const KViewConverter &converter);

    /**
     * Each shape-type has an Id; as found in KShapeFactoryBase::id().id(), to choose which
     * shape this controller should actually create; set the id before the user starts to
     * create the new shape.
     * @param id the SHAPEID of the to be generated shape
     */
    void setShapeId(const QString &id);
    /**
     * return the shape Id that is to be created.
     * @return the shape Id that is to be created.
     */
    QString shapeId() const;

    /**
     * Set the shape properties that the create controller will use for the next shape it will
     * create.
     * @param properties the properties or 0 if the default shape should be created.
     */
    void setShapeProperties(KProperties *properties);
    /**
     * return the properties to be used for creating the next shape
     * @return the properties to be used for creating the next shape
     */
    KProperties const *shapeProperties();

protected:
    virtual KInteractionStrategy *createStrategy(KPointerEvent *event);
    virtual void mouseReleaseEvent(KPointerEvent *event);

private:
    Q_DECLARE_PRIVATE(KCreateShapesTool)
};

#endif
