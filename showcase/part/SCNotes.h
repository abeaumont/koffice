/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2008-2009 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCNOTES_H
#define SCNOTES_H

#include <KOdfPageLayoutData.h>
#include <KoPAPage.h>

class KShape;
class KImageCollection;
class SCDocument;
class SCPage;

class SCNotes : public KoPAPage
{
public:
    SCNotes(SCPage * page, SCDocument * document);
    ~SCNotes();

    /// Get the main text note shape for this presentation notes
    KShape *textShape();

    /// reimplemented
    virtual void saveOdf(KShapeSavingContext &context) const;

    /// reimplemented
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);

    /// reimplemented
    virtual void paintComponent(QPainter&painter, const KViewConverter &converter);

    /// Get the page layout for this presentation notes
    virtual KOdfPageLayoutData pageLayout() const;

    /// update the page thumbnail to reflect current page
    void updatePageThumbnail();

    /// reimplemented
    virtual bool displayMasterShapes();
    /// reimplemented
    virtual void setDisplayMasterShapes(bool);
    /// reimplemented
    virtual bool displayMasterBackground();
    /// reimplemented
    virtual void setDisplayMasterBackground(bool);
    /// reimplemented
    virtual bool displayShape(KShape *shape) const;
    /// reimplemented
    virtual QPixmap generateThumbnail(const QSize&);
    /// reimplemented
    virtual void paintPage(QPainter &painter, KoZoomHandler &zoomHandler);

private:
    QImage createPageThumbnail() const;

private:
    KShape *m_textShape;
    KShape *m_thumbnailShape;
    KOdfPageLayoutData m_pageLayout;
    SCPage *m_page;
    SCDocument *m_doc;
    KImageCollection * m_imageCollection;
};

#endif // SCNOTES_H

