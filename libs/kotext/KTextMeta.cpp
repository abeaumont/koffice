/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "KTextMeta.h"

#include <KShapeLoadingContext.h>
#include <KShapeSavingContext.h>
#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KTextInlineRdf.h>

#include <QTextDocument>
#include <QTextInlineObject>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>

#include <KDebug>

class KTextMeta::Private
{
public:
    KTextMeta *endBookmark;
    BookmarkType type;
};

KTextMeta::KTextMeta()
        : KInlineObject(false),
        d(new Private())
{
    d->endBookmark = 0;
}

KTextMeta::~KTextMeta()
{
    delete d;
}

void KTextMeta::saveOdf(KShapeSavingContext &context)
{
    KXmlWriter &writer = context.xmlWriter();

    kDebug(30015) << "kom.save() this:" << (void*)this << " d->type:" << d->type;
    if (inlineRdf()) {
        kDebug(30015) << "kom.save() have inline Rdf";
    }

    if (d->type == StartBookmark) {
        writer.startElement("text:meta", false);
        writer.addAttribute("text:name", "foo");

        if (inlineRdf()) {
            inlineRdf()->saveOdf(context, &writer);
        }
    } else {
        kDebug(30015) << "adding endelement.";
        writer.endElement();
    }
    kDebug(30015) << "kom.save() done this:" << (void*)this << " d->type:" << d->type;
}

bool KTextMeta::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    kDebug(30015) << "kom.load()";
    return true;
}

void KTextMeta::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KTextMeta::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
}

void KTextMeta::paint(QPainter &, QPaintDevice *, const QRectF &, QTextInlineObject, const QTextCharFormat &)
{
    // nothing to paint.
}

void KTextMeta::setType(BookmarkType type)
{
    d->type = type;
}

KTextMeta::BookmarkType KTextMeta::type() const
{
    return d->type;
}

void KTextMeta::setEndBookmark(KTextMeta *bookmark)
{
    d->type = StartBookmark;
    bookmark->d->type = EndBookmark;
    d->endBookmark = bookmark;
}

KTextMeta *KTextMeta::endBookmark() const
{
    return d->endBookmark;
}

KShape *KTextMeta::shape() const
{
    return shapeForPosition(document(), textPosition());
}
