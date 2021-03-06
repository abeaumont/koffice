/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#ifndef KTEXTWRITER_H
#define KTEXTWRITER_H

#include "kodftext_export.h"

class KShapeSavingContext;
class KXmlWriter;
class KTextDocumentLayout;
class KStyleManager;
class QTextDocument;
class QTextTable;

class KChangeTracker;
class KDocumentRdfBase;

#include <QTextBlock>

/**
 * KTextWriter saves the text ODF of a shape
 */
class KODFTEXT_EXPORT KTextWriter
{
public:
    /**
    * Constructor.
    *
    * @param context The context the KTextWriter is called in
    */
    explicit KTextWriter(KShapeSavingContext &context, KDocumentRdfBase *rdfData = 0);

    /**
    * Destructor.
    */
    ~KTextWriter();

    /**
     * Save a paragraph style used in a text block
     *
     * This checks if the style is a document style or a automatic style
     * and saves it accordingly.
     *
     * @param block The block form which the style information are taken
     * @param styleManager The used style manager
     * @param context The saving context
     */
    static QString saveParagraphStyle(const QTextBlock &block, KStyleManager *styleManager, KShapeSavingContext &context);

    static QString saveParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &charFormat, KStyleManager *styleManager, KShapeSavingContext &context);

    /**
     * Writes the portion of document contained within 'from' and 'to'
     */
    void write(QTextDocument *document, int from, int to = -1);

private:
    class Private;
    Private* const d;
    class TagInformation;
};

#endif
