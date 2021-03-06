/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef SCPRESENTERVIEWINTERFACE
#define SCPRESENTERVIEWINTERFACE

#include "SCPresenterViewBaseInterface.h"

class QLabel;
class KTextEdit;

class KoPACanvas;
class KoPAPage;

/**
 * SCPresenterViewInterface
 * This widget is the main interface, this widget shows current slide, next slide
 * and the presenter's notes
 */
class SCPresenterViewInterface : public SCPresenterViewBaseInterface
{
    Q_OBJECT
public:
    SCPresenterViewInterface(const QList<KoPAPage *> &pages, KoPACanvas *canvas, QWidget *parent = 0);

    void setPreviewSize(const QSize &size);

public slots:
    /// reimplemented
    virtual void setActivePage(int pageIndex);

private:
    KoPACanvas *m_canvas;
    QLabel *m_currentSlideLabel;
    QLabel *m_nextSlideLabel;
    QLabel *m_nextSlidePreview;
    KTextEdit *m_notesTextEdit;
    QSize m_previewSize;
};

#endif

