/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPAGELAYOUT_H
#define KOPAGELAYOUT_H

#include "KOdfColumnData.h"
#include "KOdfGenericStyles.h"
#include "KOdfPageFormat.h"
#include "KXmlReader.h"
#include "KOdfBorders.h"

#include "kodf_export.h"

/**
 * This structure defines the page layout, including
 * its size in points, its format (e.g. A4), orientation, unit, margins etc.
 */
struct KOdfPageLayoutData {
    /** Page format */
    KOdfPageFormat::Format format;
    /** Page orientation */
    KOdfPageFormat::Orientation orientation;

    /** Page width in points */
    qreal width;
    /** Page height in points */
    qreal height;

    /**
     * margin on left edge
     * Using a margin >= 0 here indicates this is a single page system and the
     *  bindingSide + pageEdge variables should be -1 at the same time.
     */
    qreal leftMargin;
    /**
     * margin on right edge
     * Using a margin >= 0 here indicates this is a single page system and the
     *  bindingSide + pageEdge variables should be -1 at the same time.
     */
    qreal rightMargin;
    /** Top margin in points */
    qreal topMargin;
    /** Bottom margin in points */
    qreal bottomMargin;

    /**
     * margin on page edge
     * Using a margin >= 0 here indicates this is a pageSpread (facing pages) and the
     *  left + right variables should be -1 at the same time.
     */
    qreal pageEdge;
    /**
     * margin on page-binding edge
     * Using a margin >= 0 here indicates this is a pageSpread (facing pages) and the
     *  left + right variables should be -1 at the same time.
     */
    qreal bindingSide;

    /** Left padding in points */
    qreal leftPadding;
    /** Right padding in points */
    qreal rightPadding;
    /** Top padding in points */
    qreal topPadding;
    /** Bottom padding in points */
    qreal bottomPadding;

    /// page border definition
    KOdfBorders  border;

    KODF_EXPORT bool operator==(const KOdfPageLayoutData &l) const;
    KODF_EXPORT bool operator!=(const KOdfPageLayoutData& l) const;

    /**
     * Save this page layout to ODF.
     */
    KODF_EXPORT KOdfGenericStyle saveOdf() const;

    /**
     * Load this page layout from ODF
     */
    KODF_EXPORT void loadOdf(const KXmlElement &style);

    /**
     * Construct a page layout with the default page size depending on the locale settings,
     * default margins (2 cm), and portrait orientation.
     */
    KODF_EXPORT KOdfPageLayoutData();
};

#endif /* KOPAGELAYOUT_H */
