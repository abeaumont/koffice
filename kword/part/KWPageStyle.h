/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Sebastian Sauer <mail@dipe.org>
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

#ifndef KWPAGESTYLE_H
#define KWPAGESTYLE_H

#include "KWord.h"
#include "kword_export.h"

#include <KoPageLayout.h>
#include "frames/KWTextFrameSet.h"

/**
 * A container for all informations for the page wide style.
 *
 * For documents that have a main text auto generated we have a lot of little options
 * to do that. This class wraps all these options.
 *
 * \note that the margins are stored in a \a KoPageLayout instance.
 *
 * \note once you created an instance of \a KWPageStyle you may like to use the
 * KWPageManager::addPageStyle() method to let KWord handle the ownership else
 * you are responsible for deleting the instance and taking care that no \a KWPage
 * instance or something else still keeps a (then dangling) pointer to it.
 */
class KWORD_TEST_EXPORT KWPageStyle : public QObject
{
    Q_OBJECT
public:
    /**
     * constructor, initializing the data to some default values.
     *
     * \p masterPageName The name of this page style.
     */
    explicit KWPageStyle(const QString& mastername);

    /**
     * Return the current columns settings.
     */
    const KoColumns &columns() const { return m_columns; }
    /**
     * Set the new columns settings
     */
    void setColumns(const KoColumns &columns) { m_columns = columns; }

    /// Return the type of header the pages will get.
    KWord::HeaderFooterType headers() const { return m_headers; }
    /// set the type of header the pages will get.
    void setHeaderPolicy(KWord::HeaderFooterType p);

    /// Return the type of footers the pages will get.
    KWord::HeaderFooterType footers() const { return m_footers; }
    /// Set the type of footers the pages will get.
    void setFooterPolicy(KWord::HeaderFooterType p);

    /**
     * This is the main toggle for all automatically generated frames.
     * The generation and placing of the main text frame, as well as headers, footers,
     * end notes and footnotes for the main text flow is enabled as soon as this is on.
     * Turn it off and all the other settings on this class will be ignored.
     * @param on the big switch for auto-generated frames.
     */
    void setMainTextFrame(bool on) { m_mainFrame = on; }
    /**
     * Return if the main text frame, but also the headers/footers etc should be autogenerated.
     */
    bool hasMainTextFrame() const { return m_mainFrame; }

    /// return the distance between the main text and the header
    Q_SCRIPTABLE qreal headerDistance() const { return m_headerDistance; }
    /**
     * Set the distance between the main text and the header
     * @param distance the distance
     */
    Q_SCRIPTABLE void setHeaderDistance(qreal distance) { m_headerDistance = distance; }

    /// return the distance between the footer and the frame directly above that (footnote or main)
    Q_SCRIPTABLE qreal footerDistance() const { return m_footerDistance; }
    /**
     * Set the distance between the footer and the frame directly above that (footnote or main)
     * @param distance the distance
     */
    Q_SCRIPTABLE void setFooterDistance(qreal distance) { m_footerDistance = distance; }

    /// return the distance between the footnote and the main frame.
    Q_SCRIPTABLE qreal footnoteDistance() const { return m_footNoteDistance; }
    /**
     * Set the distance between the footnote and the main frame.
     * @param distance the distance
     */
    Q_SCRIPTABLE void setFootnoteDistance(qreal distance) { m_footNoteDistance = distance; }
    /// return the distance between the main text frame and the end notes frame.
    Q_SCRIPTABLE qreal endNoteDistance() const { return m_endNoteDistance; }
    /**
     * Set the distance between the main text frame and the end notes frame.
     * @param distance the distance
     */
    Q_SCRIPTABLE void setEndNoteDistance(qreal distance) { m_endNoteDistance = distance; }

    /// return the line length of the foot note separator line, in percent of the pagewidth
    Q_SCRIPTABLE int footNoteSeparatorLineLength() const { return m_footNoteSeparatorLineLength;}
    /// set the line length of the foot note separator line, in percent of the pagewidth
    Q_SCRIPTABLE void setFootNoteSeparatorLineLength( int length){  m_footNoteSeparatorLineLength = length;}

    /// return the thickness of the line (in pt) drawn above the foot notes
    Q_SCRIPTABLE qreal footNoteSeparatorLineWidth() const { return m_footNoteSeparatorLineWidth;}
    /// set the thickness of the line (in pt) drawn above the foot notes
    Q_SCRIPTABLE void setFootNoteSeparatorLineWidth( qreal width){  m_footNoteSeparatorLineWidth=width;}

    /// return the pen style used to draw the foot note separator line
    Qt::PenStyle footNoteSeparatorLineType() const { return m_footNoteSeparatorLineType;}
    /// set the pen style used to draw the foot note separator line
    void setFootNoteSeparatorLineType( Qt::PenStyle type) {m_footNoteSeparatorLineType = type;}

    /// return the position on the page for the foot note separator line
    KWord::FootNoteSeparatorLinePos footNoteSeparatorLinePosition() const {
        return m_footNoteSeparatorLinePos;
    }
    /// set the position on the page for the foot note separator line
    void setFootNoteSeparatorLinePosition(KWord::FootNoteSeparatorLinePos position) {
        m_footNoteSeparatorLinePos = position;
    }

    /// initialize to default settings
    Q_SCRIPTABLE void clear();

    /// return the pageLayout applied for these pages
    const KoPageLayout pageLayout() const;

    /// set the pageLayout applied for these pages
    void setPageLayout(const KoPageLayout &layout);

    /**
     * Get a frameset that is stored in this page style.
     * Example of framesets stored : the headers, footers...
     * @param hfType the type of the frameset that must be returned
     * @returns the required frameSet, 0 if none found.
     */
    KWTextFrameSet *getFrameSet (KWord::TextFrameSetType hfType) { return m_hfFrameSets[hfType]; }
    /**
     * Add a frameset in this page style.
     * Example of framesets stored : the headers, footers...
     * @param hfType the type of the frameset
     * @param fSet the frameset
     * This frameset will be destroyed when the page style is destroyed.
     */
    void addFrameSet (KWord::TextFrameSetType hfType, KWTextFrameSet *fSet) { m_hfFrameSets[hfType] = fSet; }

    /// get the master page name for this page style.
    Q_SCRIPTABLE QString masterName () const { return m_masterName; }

Q_SIGNALS:

    /**
     * This signal is emitted if a relayout is requested cause for
     * example the state of the header/footer changed.
     *
     * The KWPageManager does redirect the signal to the
     * KWDocument::relayout() function which will update all
     * pages.
     */
    void relayout();

private:
    KoColumns m_columns;
    KoPageLayout m_pageLayout;
    QString m_masterName;
    bool m_mainFrame;
    qreal m_headerDistance, m_footerDistance, m_footNoteDistance, m_endNoteDistance;
    KWord::HeaderFooterType m_headers, m_footers;
    // These framesets are deleted by KWDocument.
    QMap<KWord::TextFrameSetType, KWTextFrameSet *> m_hfFrameSets;

    qreal m_footNoteSeparatorLineWidth; ///< width of line; so more like 'thickness'
    int m_footNoteSeparatorLineLength; ///< It's a percentage of page.
    Qt::PenStyle m_footNoteSeparatorLineType; ///< foot note separate type
    KWord::FootNoteSeparatorLinePos m_footNoteSeparatorLinePos; ///< alignment in page

    /// disable copy constructor and assignment operator
    Q_DISABLE_COPY(KWPageStyle)
};

#endif
