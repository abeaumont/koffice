/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#ifndef ARTISTICTEXTSHAPECONFIGWIDGET_H
#define ARTISTICTEXTSHAPECONFIGWIDGET_H

#include <ui_ArtisticTextShapeConfigWidget.h>

#include "ArtisticTextShape.h"

class ArtisticTextShape;

class ArtisticTextShapeConfigWidget : public QWidget
{
    Q_OBJECT
public:
    ArtisticTextShapeConfigWidget();

public slots:
    /// initializes widget from given shape
    void initializeFromShape(ArtisticTextShape *shape, KCanvasBase *canvas);

    /// updates the widget form the current one
    void updateWidget();

private slots:
    void propertyChanged();

private:
    void blockChildSignals(bool block);
    Ui::ArtisticTextShapeConfigWidget widget;
    ArtisticTextShape * m_shape;
    KCanvasBase * m_canvas;
    QButtonGroup * m_anchorGroup;
};

#endif // ARTISTICTEXTSHAPECONFIGWIDGET_H
