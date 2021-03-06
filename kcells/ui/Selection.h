/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2005-2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KCELLS_SELECTION
#define KCELLS_SELECTION

#include <QColor>
#include <QList>

#include <KToolSelection.h>

#include <kdebug.h>

#include <KCRegion.h>

class KCanvasBase;

class CellEditor;

/**
 * \class Selection
 * \brief Manages the selection of cells.
 * Represents cell selections for general operations and for cell references
 * used in formulaes.
 * \author Torben Weis <weis@kde.org>
 * \author Stefan Nikolaus <stefan.nikolaus@kdemail.net>
 */
class KCELLS_EXPORT Selection : public KToolSelection, public KCRegion
{
    Q_OBJECT

public:
    /**
     * The selection mode.
     */
    // TODO Stefan: merge with RegionSelector::SelectionMode
    enum Mode {
        SingleCell = 0,     ///< single cell selection mode
        MultipleCells = 1   ///< multiple cell selection mode
    };

    /**
     * Constructor.
     * Creates a new selection with (1,1) as initial location.
     * @param canvasBase the canvas interface
     */
    explicit Selection(KCanvasBase* canvasBase);

    /**
     * Copy Constructor.
     * Creates a copy of @p selection
     * @param selection the Selection to copy
     */
    Selection(const Selection& selection);

    /**
     * Destructor.
     */
    virtual ~Selection();

    /**
     * \return the canvas this selection works for.
     */
    KCanvasBase* canvas() const;

    /**
     * Sets the selection to @p point
     * @param point the point's location
     * @param sheet the sheet the point belongs to
     */
    void initialize(const QPoint& point, KCSheet* sheet = 0);

    /**
     * Sets the selection to @p range
     * @param range the range's location
     * @param sheet the sheet the range belongs to
     */
    void initialize(const QRect& range, KCSheet* sheet = 0);

    /**
     * Sets the selection to @p region
     * @param region the region's locations
     * @param sheet the sheet the region belongs to
     */
    void initialize(const KCRegion& region, KCSheet* sheet = 0);

    /**
     * Emits signal changed(const KCRegion&)
     */
    void update();

    /**
     * Update the marker of the selection to @p point .
     * Uses the anchor as starting point
     * @p point the new marker location
     */
    void update(const QPoint& point);

    /**
     * Extends the current selection with the Point @p point
     * @param point the point's location
     * @param sheet the sheet the point belongs to
     */
    void extend(const QPoint& point, KCSheet* sheet = 0);

    /**
     * Extends the current selection with the Range @p range
     * @param range the range's location
     * @param sheet the sheet the range belongs to
     */
    void extend(const QRect& range, KCSheet* sheet = 0);

    /**
     * Extends the current selection with the KCRegion @p region
     * @param region the region's locations
     */
    void extend(const KCRegion& region);

    /**
     * @param point the point's location
     * @param sheet the sheet the point belongs to
     */
    virtual Element* eor(const QPoint& point, KCSheet* sheet = 0);

    /**
     * The anchor is the starting point of a range. For points marker and anchor are the same
     */
    const QPoint& anchor() const;

    /**
     * The cursor represents the cursor position. This is needed for merged cells
     */
    const QPoint& cursor() const;

    /**
     * The marker is the end point of a range. For points marker and anchor are the same
     */
    const QPoint& marker() const;

    /**
     * Checks whether the region consists only of one point
     */
    bool isSingular() const;

    /**
     * @return the name of the region (e.g. "A1:A2")
     */
    QString name(KCSheet* originSheet = 0) const;

    /**
     * Sets the selection's active sheet.
     * For usual selections this is always the origin sheet,
     * but for cell choices used for formulaes it may differ.
     * @param sheet the sheet which is currently active
     */
    void setActiveSheet(KCSheet* sheet);

    /**
     * @return the selection's active sheet
     */
    KCSheet* activeSheet() const;

    /**
     * Sets the selection's origin sheet.
     * @param sheet the sheet from which the selection starts
     */
    void setOriginSheet(KCSheet* sheet);

    /**
     * @return the selection's origin sheet
     */
    KCSheet* originSheet() const;

    /**
     * Activates the cell location/range, that has \p cell as bottom left or
     * right cell. If more than one occurence would fit, the first one is taken.
     * \return the index of the activated range or \c -1, if nothing fits
     */
    int setActiveElement(const KCCell &cell);

    /**
     * Sets the element, which has @p point as anchor, as active
     */
    void setActiveElement(const QPoint& point, CellEditor* cellEditor);

    /**
     * @return the active element
     */
    Element* activeElement() const;

    /**
     * Sets the starting position and the length of a sub-region.
     * On inserting/updating the selection the sub-region gets replaced
     * by the new cell location/range.
     * A \p length of \c 0 results in no replacement, but just in inserting the
     * new cell location/range before the range index \p start.
     * \param start The index of a range in this selection. It has to be a valid
     * index; otherwise the sub-region will be set to the whole region.
     * \param length The amount of ranges in the sub-region. If it exceeds the
     * amount of ranges, beginning from \p start to the end of range list, it
     * will be adjusted.
     * \param active The active element within the sub-region.
     * \verbatim start <= active <= start + length \endverbatim
     */
    void setActiveSubRegion(int start, int length, int active = -1);

    /**
     *
     */
    QString activeSubRegionName() const;

    /**
     * Clears the elements of the subregion
     */
    void clearSubRegion();

    /**
     * fix subregion dimensions
     */
    void fixSubRegionDimension();

    /**
     * Deletes all elements of the region. The result is an empty region.
     */
    virtual void clear();


    /**
     * \param mode single cell or multiple cell selection
     */
    void setSelectionMode(Mode mode);

    /**
     * Extends \p area to include the merged cells, that are not fully covered,
     * completely.
     * \return the extended area
     */
    QRect extendToMergedAreas(const QRect& area) const;

    const QList<QColor>& colors() const;

    void selectAll();

    /** Start using a reference selection instead of normal one. */
    void startReferenceSelection();
    /** End using reference selection. */
    void endReferenceSelection(bool saveChanges = true);
    /** Enable/disable reference choosing mode. */
    void setReferenceSelectionMode(bool enable);
    /** Are we in reference choosing mode ? */
    bool referenceSelectionMode() const;
    /** Are we currently using a reference selection ? */
    bool referenceSelection() const;

    void emitAboutToModify();
    void emitModified();
    void emitRefreshSheetViews();
    void emitVisibleSheetRequested(KCSheet* sheet);
    void emitCloseEditor(bool saveChanges, bool expandMatrix = false);
    void emitRequestFocusEditor();

signals:
    /**
     * Emitted when the Selection was changed.
     * @param region the changed part of the Selection
     */
    void changed(const KCRegion& region);

    /**
     * An operation on the selection is about to happen.
     */
    void aboutToModify(const KCRegion& region);

    /**
     * Emitted when the content was modified.
     */
    void modified(const KCRegion& region);

    void refreshSheetViews();
    void visibleSheetRequested(KCSheet* sheet);
    void closeEditor(bool saveChanges, bool expandMatrix);
    void activeSheetChanged(KCSheet* sheet);
    void requestFocusEditor();

    void documentReadWriteToggled(bool readWrite);
    void sheetProtectionToggled(bool protect);

protected:
    class Point;
    class Range;

    /**
     * @internal used to create derived Points
     */
    virtual KCRegion::Point* createPoint(const QPoint&) const;

    /**
     * @internal used to create derived Points
     */
    virtual KCRegion::Point* createPoint(const QString&) const;

    /**
     * @internal used to create derived Points
     */
    virtual KCRegion::Point* createPoint(const KCRegion::Point&) const;

    /**
     * @internal used to create derived Ranges
     */
    virtual KCRegion::Range* createRange(const QRect&) const;

    /**
     * @internal used to create derived Ranges
     */
    virtual KCRegion::Range* createRange(const KCRegion::Point&, const KCRegion::Point&) const;

    /**
     * @internal used to create derived Ranges
     */
    virtual KCRegion::Range* createRange(const QString&) const;

    /**
     * @internal used to create derived Ranges
     */
    virtual KCRegion::Range* createRange(const KCRegion::Range&) const;

    /**
     * Dilates the region and emits the changed() signal.
     * \internal
     */
    void emitChanged(const KCRegion& changedRegion);

    /**
     * @internal
     */
    void dump() const;

private:
    // do not allow assignment
    Selection& operator=(const Selection&);

    class Private;
    Private * const d;
};

/***************************************************************************
  class Selection::Point
****************************************************************************/

/**
 * This Point is extended by an color attribute.
 */
class Selection::Point : public KCRegion::Point
{
public:
    Point(const QPoint& point);
    Point(const QString& string);
    Point(const KCRegion::Point& point);

    void setColor(const QColor& color) {
        m_color = color;
    }
    virtual const QColor& color() const {
        return m_color;
    }

private:
    QColor m_color;
};

/***************************************************************************
  class Selection::Range
****************************************************************************/

/**
 * This Range is extended by an color attribute.
 */
class Selection::Range : public KCRegion::Range
{
public:
    Range(const QRect& rect);
    Range(const KCRegion::Point& tl, const KCRegion::Point& br);
    Range(const QString& string);
    Range(const KCRegion::Range& range);

    void setColor(const QColor& color) {
        m_color = color;
    }
    const QColor& color() const {
        return m_color;
    }

private:
    QColor m_color;
};

#endif // KCELLS_SELECTION
