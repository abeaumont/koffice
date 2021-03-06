/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#ifndef KODFTEXTSHAPE_H
#define KODFTEXTSHAPE_H

#include <KShapeContainer.h>
#include <KFrameShape.h>
#include <KTextShapeData.h>

#include <QTextDocument>
#include <QPainter>
#include <QMutex>
#include <QWaitCondition>

#define TextShape_SHAPEID "TextShapeID"

class KInlineTextObjectManager;
class KPageProvider;
class KImageCollection;

/**
 * A text shape.
 * The Text shape is capable of drawing structured text.
 * @see KTextShapeData
 */
class TextShape : public KShapeContainer, public KFrameShape
{
public:
    TextShape();
    virtual ~TextShape();

    /// reimplemented
    void paintComponent(QPainter &painter, const KViewConverter &converter);
    /// reimplemented
    void paintDecorations(QPainter &painter, const KViewConverter &converter, const KCanvasBase *canvas);
    /// reimplemented
    virtual void waitUntilReady(const KViewConverter &converter, bool asynchronous) const;

    /// helper method.
    QPointF convertScreenPos(const QPointF &point);

    /// set the image collection which is needed to draw bullet from images
    void setImageCollection(KImageCollection *collection) { m_imageCollection = collection; }

    /**
     * Set the shape's text to be demo text or not.
     * If true, replace the content with an lorem ipsum demo text and don't complain
     *   when there is not enough space at the end
     * If false; remove the demo text again.
     */
    void setDemoText(bool on);
    /// return if the content of this shape is demo text.
    bool demoText() const {
        return m_demoText;
    }

    /**
     * From KShape reimplemented method to load the TextShape from ODF.
     *
     * This method redirects the call to the KTextShapeData::loadOdf() method which
     * in turn will call the KTextLoader::loadBody() method that reads the element
     * into a QTextCursor.
     *
     * @param context the KShapeLoadingContext used for loading.
     * @param element element which represents the shape in odf.
     * @return false if loading failed.
     */
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);

    /**
     * From KShape reimplemented method to store the TextShape data as ODF.
     *
     * @param context the KShapeSavingContext used for saving.
     */
    virtual void saveOdf(KShapeSavingContext &context) const;

    KTextShapeData *textShapeData() {
        return m_textShapeData;
    }

    bool hasFootnoteDocument() {
        return m_footnotes != 0 && !m_footnotes->isEmpty();
    }
    QTextDocument *footnoteDocument();

    void markLayoutDone();

    /// reimplemented
    virtual bool loadOdfFrame(const KXmlElement &element, KShapeLoadingContext &context);

protected:
    virtual bool loadOdfFrameElement(const KXmlElement &element, KShapeLoadingContext &context);

    /// reimplemented
    virtual void loadStyle(const KXmlElement &element, KShapeLoadingContext &context);

    /// reimplemented
    virtual QString saveStyle(KOdfGenericStyle &style, KShapeSavingContext &context) const;

private:
    void shapeChanged(ChangeType type);

    KTextShapeData *m_textShapeData;
    QTextDocument *m_footnotes;

    bool m_demoText;
    mutable QMutex m_mutex;
    mutable QWaitCondition m_waiter;
    KImageCollection *m_imageCollection;

    QRegion m_paintRegion;
};

#endif
