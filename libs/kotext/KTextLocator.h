/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#ifndef KTEXTLOCATOR_H
#define KTEXTLOCATOR_H

#include "KInlineObject.h"
#include "kodftext_export.h"

#include <QString>

class KTextBlockData;
class KTextReference;
class KTextLocatorPrivate;

/**
 * This inline object can be inserted in text to mark it and to later get location information from.
 * After inserting this locator you can request things like pageNumber() and chapter() for the
 * place where the locator has been positioned in the document.
 */
class KODFTEXT_EXPORT KTextLocator : public KInlineObject
{
public:
    /// constructor
    KTextLocator();
    virtual ~KTextLocator();

    /// reimplemented from super
    virtual void updatePosition(QTextInlineObject object, const QTextCharFormat &format);
    /// reimplemented from super
    virtual void resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented from super
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QRectF &rect,
            QTextInlineObject object, const QTextCharFormat &format);

    /// returns the text of the paragraph that is the first chapter before the index.
    QString chapter() const;
    /// Return the block data of the chapter, useful for numbering info etc.  Returns 0 if nothing was found.
    KTextBlockData *chapterBlockData() const;
    /// return the page number on which the locator is placed.
    int pageNumber() const;
    /// return the word in which the locator is inserted.
    QString word() const;

    /// Add a text reference that is interrested in knowing when this locator is laid-out in a differen position.
    void addListener(KTextReference *reference);
    /// Remove a reference from the listeners.
    void removeListener(KTextReference *reference);

    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    virtual void saveOdf(KShapeSavingContext &context);

private:
    Q_DECLARE_PRIVATE(KTextLocator)
};

#endif
