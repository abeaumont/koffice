/* This file is part of the KDE project

   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 1999-2001,2003 David Faure <faure@kde.org>
   Copyright 2002-2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2000-2001 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KSPREAD_CANVAS
#define KSPREAD_CANVAS

#include <qlineedit.h>
#include <qtooltip.h>
#include <qpen.h>

#include <koQueryTrader.h>
#include <koRect.h>

#include "kspread_util.h"

class KSpreadEditWidget;
class KSpreadCanvas;
class KSpreadHBorder;
class KSpreadVBorder;
class KSpreadSheet;
class KSpreadDoc;
class KSpreadPoint;
class KSpreadRange;
class KSpreadView;
class KSpreadSelection;
class KSpreadCellEditor;
class KSpreadCell;
class QWidget;
class QTimer;
class QButton;
class KSpreadLocationEditWidget;
class KSpreadComboboxLocationEditWidget;
class QPainter;
class QLabel;
class QScrollBar;

#define YBORDER_WIDTH 50
#define XBORDER_HEIGHT 20


/**
 * The canvas builds a part of the GUI of KSpread.
 * It contains the borders, scrollbars,
 * editwidget and of course it displays the table.
 * Especially most of the user interface logic is implemented here.
 * That means that this class knows what to do when a key is pressed
 * or if the mouse button was clicked.
 */
class KSpreadCanvas : public QWidget
{
    friend class KSpreadHBorder;
    friend class KSpreadVBorder;
    friend class KSpreadView;

    Q_OBJECT
public:
    /**
     * The current action associated with the mouse.
     * Default is 'NoAction'.
     */
    enum MouseActions { NoAction = 0, Mark = 1, ResizeCell = 2, AutoFill = 3 };
    enum EditorType { CellEditor, FormulaEditor, EditWidget };

    KSpreadCanvas( QWidget *_parent, KSpreadView *_view, KSpreadDoc* _doc );
    ~KSpreadCanvas( );

    /**
     * Called from @ref KSpreadView to complete the construction. Has to
     * be called before any other method on this object may be invoced.
     */
    void init();

    KSpreadCellEditor* editor() const { return m_pEditor ; }


  /**
     * If the user chooses some cells during editing a formula, then
     * this function returns the length of the textual representation.
     * For example the user selects "Sheet1!A1:B2" then this function
     * returns 12.
     */
    int chooseTextLen() const { return length_namecell; }

    KSpreadSelection* selectionInfo() const;
    QRect selection() const;
    QPoint marker() const;
    int markerColumn() const;
    int markerRow() const;

    void updateCellRect( const QRect &_rect );
    void updateSelection( const QRect& oldSelection, const QPoint& oldMarker );

    const QPen& defaultGridPen() const { return m_defaultGridPen; }

    double zoom() const;

    /**
     * Returns the width of the columns before the current screen
     */
    double xOffset() const { return m_dXOffset; }
    /**
     * Returns the height of the rows before the current screen
     */
    double yOffset() const { return m_dYOffset; }

    /**
     * Return a rect indicating which cell range is currently visible onscreen
     */
    QRect visibleCells();

    KSpreadSheet* activeTable() const;
    KSpreadSheet* findTable( const QString& _name ) const;

    /**
     * A convenience function.
     */
    bool gotoLocation( const KSpreadRange & _range );
    /**
     * A convenience function.
     */
    bool gotoLocation( const KSpreadPoint& _cell );

    /**
     * Move the cursor to the specified cell. This may include switching
     * the table. In addition @ref #KSpreadView::updateEditWidget is called.
     *
     * @param location the cell to move to
     *
     * @param table the table to move to.  If NULL, the active table is used

     * @param extendSelection determines wether this move of the marker is part
     *                        of a selection, that means: The user holds the
     *                        shift key and moves the cursor keys. In this case
     *                        the selection is updated accordingly.
     *                        If this is false, the cell will be the single cell
     *                        selected, and the selection anchor will be reset
     *                        to this cell.
     */
    void gotoLocation( QPoint const & location, KSpreadSheet* table = NULL,
                       bool extendSelection = false);

    /**
     * convenience function
     */
    void gotoLocation( int col, int row, KSpreadSheet* table = NULL,
                       bool extendSelection = false)
    {gotoLocation(QPoint(col, row), table, extendSelection);}


    /**
     * Paint all visible cells that have a paint dirty flag set
     */
    void paintUpdates();


    /**
     * Makes sure a cell is visible onscreen by scrolling up/down and left/right
     *
     * @param location the cell coordinates to scroll to
     */
    void scrollToCell(QPoint location);
    /**
     * Chooses the correct @ref #EditorType by looking at
     * the current cells value. By default CellEditor is chosen.
     */
    void createEditor();
    bool createEditor( EditorType type, bool addFocus = true );
    /**
     * Deletes the current cell editor.
     *
     * @see #createEditor
     * @see #editor
     * @param saveChanges if true, the edited text is stored in the cell.
     *                    if false, the changes are discarded.
     */
    void deleteEditor( bool saveChanges );

    /**
     * Called from @ref KSpreadEditWidget and KSpreadCellEditor
     * if they loose the focus because the user started a "choose selection".
     * This is done because the editor wants to get its focus back afterwards.
     * But somehow KSpreadCanvas must know whether the EditWidget or the CellEditor
     * lost the focus when the user clicked on the canvas.
     */
    void setLastEditorWithFocus( EditorType type ) { m_focusEditorType = type; }

    /**
     * Switches to choose mode and sets the initial selection to the
     * position returned by @ref #marker.
     */
    void startChoose();
    /**
     * Switches to choose mode and sets the initial @p selection.
     */
    void startChoose( const QRect& selection );
    void endChoose();

    bool chooseMode(){ return m_bChoose; }

    /**
    * Adjust a area in height and width
    */
    void adjustArea(bool makeUndo=true);

    void equalizeRow();
    void equalizeColumn();

    void updatePosWidget();

    void closeEditor();

    // Created by the view since it's layout is managed there,
    // but is in fact a sibling of the canvas, which needs to know about it.
    void setEditWidget( KSpreadEditWidget * ew ) { m_pEditWidget = ew; }

    KSpreadView* view()const { return m_pView; }
    KSpreadDoc* doc()const { return m_pDoc; }

    virtual bool focusNextPrevChild( bool );

    bool chooseFormulaArea() const { return m_bChoose;}

    /**
     * Depending on the offset in "zoomed" screen pixels
     * for the horizontal direction,
     * the function returns the steps in unzoomed points
     * for the autoscroll acceleration
     */
    double autoScrollAccelerationX( int offset );
    /**
     * Depending on the offset in "zoomed" screen pixels
     * for the vertical direction,
     * the function returns the steps in unzoomed points
     * for the autoscroll acceleration
     */
    double autoScrollAccelerationY( int offset );

public slots:
    void slotScrollVert( int _value );
    void slotScrollHorz( int _value );

    void slotMaxColumn( int _max_column );
    void slotMaxRow( int _max_row );

protected:
    virtual void keyPressEvent ( QKeyEvent* _ev );
    virtual void paintEvent ( QPaintEvent* _ev );
    virtual void mousePressEvent( QMouseEvent* _ev );
    virtual void mouseReleaseEvent( QMouseEvent* _ev );
    virtual void mouseMoveEvent( QMouseEvent* _ev );
    virtual void mouseDoubleClickEvent( QMouseEvent* );
    virtual void wheelEvent( QWheelEvent* );
    virtual void focusInEvent( QFocusEvent* );
    virtual void focusOutEvent( QFocusEvent* );
    virtual void resizeEvent( QResizeEvent * _ev );
    virtual void dragMoveEvent(QDragMoveEvent * _ev);
    virtual void dropEvent(QDropEvent * _ev);
    virtual void dragLeaveEvent(QDragLeaveEvent * _ev);

private slots:
    void doAutoScroll();

private:
    virtual void chooseMousePressEvent( QMouseEvent* _ev );
    virtual void chooseMouseReleaseEvent( QMouseEvent* _ev );
    virtual void chooseMouseMoveEvent( QMouseEvent* _ev );

    virtual bool eventFilter( QObject *o, QEvent *e );

    KSpreadHBorder* hBorderWidget() const;
    KSpreadVBorder* vBorderWidget() const;
    QScrollBar* horzScrollBar() const;
    QScrollBar* vertScrollBar() const;
    KSpreadEditWidget* editWidget() const { return m_pEditWidget; }

    void drawChooseMarker( );
    void drawChooseMarker( const QRect& );

    /**
     * Clips out the children region from the painter
     */
    void clipoutChildren( QPainter& painter, QWMatrix& matrix );

    /**
     * Paints the children
     */
    void paintChildren( QPainter& painter, QWMatrix& matrix );

    /**
     * @see #setLastEditorWithFocus
     */
    EditorType lastEditorWithFocus() const { return m_focusEditorType; }

    /**
     * Hides the marker. Hiding it multiple times means that it has to be shown ( using @ref #showMarker ) multiple times
     * to become visible again. This function is optimized since it does not create a new painter.
     */
    // void hideMarker( QPainter& );
    // void showMarker( QPainter& );

    // void drawMarker( QPainter * _painter = 0L );

    bool choose_visible;
    int  length_namecell;
    int  length_text;

    KSpreadView *m_pView;
    KSpreadDoc* m_pDoc;
    QTimer * m_scrollTimer;

    /**
     * If the user is dragging around with the mouse then this tells us what he is doing.
     * The user may want to mark cells or he started in the lower right corner
     * of the marker which is something special. The values for the 2 above
     * methods are called 'Mark' and 'ResizeCell' or 'AutoFill' depending
     * on the mouse button used. By default this variable holds
     * the value 'NoAction'.
     */
    MouseActions m_eMouseAction;

    /**
     * Used to indicate whether the user started drawing a rubber band rectangle.
     */
    bool m_bGeometryStarted;
    QPoint m_ptGeometryStart;
    QPoint m_ptGeometryEnd;


    /**
     * True when the mouse button is pressed
     */
    bool m_bMousePressed;

    /**
     * If we use the lower right corner of the marker to start autofilling, then this
     * rectangle conatins all cells that were already marker when the user started
     * to mark the rectangle which he wants to become autofilled.
     *
     * @see #mousePressEvent
     * @see #mouseReleeaseEvent
     */
    QRect m_rctAutoFillSrc;

    /**
     * If the mouse is over some anchor ( in the sense of HTML anchors )
     * then this one is stored here.
     */
    QString m_strAnchor;

    /**
     * Non visible range left from current screen
     * Example:
     * If the first visible column is 'E', then m_dXOffset stores
     * the width of the invisible columns 'A' to 'D'.
     */
    double m_dXOffset;

    /**
     * Non visible range on top of the current screen
     * Example:
     * If the first visible row is '5', then m_dYOffset stores
     * the height of the invisible rows '1' to '4'.
     */
    double m_dYOffset;

    /**
     * Start coordinates for drag and drop
     */
    QPoint m_dragStart;
    bool   m_dragging;

    KSpreadComboboxLocationEditWidget *m_pPosWidget;
    KSpreadEditWidget *m_pEditWidget;
    KSpreadCellEditor *m_pEditor;

    /**
     * Used to draw the grey grid that is usually only visible on the
     * screen, but not by printing on paper.
     */
    QPen m_defaultGridPen;

    // int m_iMarkerColumn;
    // int m_iMarkerRow;
    /**
     * A value of 1 means that it is visible, every lower value means it is
     * made invisible multiple times.
     *
     * @see #hideMarker
     * @see #showMarker
     */
    // int m_iMarkerVisible;


    /**
     * Is true if the user is to choose a cell.
     *
     * @see #startChoose
     * @see #endChoose
     * @see KSpreadAssistant2
     */
    bool m_bChoose;
    /**
     * If a choose selection is started (@ref #startChoose) the current
     * table is saved here.
     */
    KSpreadSheet* m_chooseStartTable;

    /**
     * @see #setLastEditorWithFocus
     * @see #lastEditorWithFocus
     */
    EditorType m_focusEditorType;

    QLabel *m_validationInfo;

private:

  void startTheDrag();
  void paintSelectionChange(QRect area1, QRect area2);

  /**
   * Small helper function to take a rect representing a selection onscreen and
   * extend it one cell in every direction (taking into account hidden columns
   * and rows)
   */
  void ExtendRectBorder(QRect& area);

  /* helpers for the paintUpdates function */
  void paintChooseRect(QPainter& painter, const KoRect &viewRect);

  void paintNormalMarker(QPainter& painter, const KoRect &viewRect);

  void retrieveMarkerInfo( const QRect &marker, const KoRect &viewRect,
                           double positions[], bool paintSides[] );



  bool formatKeyPress( QKeyEvent * _ev );
  
  /** helper method for formatKeyPress */
  bool formatCellByKey (KSpreadCell *cell, int key, const QRect &rect);
  
  double getDouble( KSpreadCell * cell );
  void convertToDouble( KSpreadCell * cell );
  void convertToPercent( KSpreadCell * cell );
  void convertToMoney( KSpreadCell * cell );
  void convertToTime( KSpreadCell * cell );
  void convertToDate( KSpreadCell * cell );

  void processClickSelectionHandle(QMouseEvent *event);
  void processLeftClickAnchor();


  /**
   * Helper function used from many places to extend the current selection such as with
   * a shift-click, mouse drag, shift-arrow key, etc.
   *
   * @param cell coordinates of the cell we want to extend towards.
   */
  void extendCurrentSelection(QPoint cell);

  /** current cursor position, be it marker of choose marker */
  QPoint cursorPos ();
  
  /**
   * returns the rect that needs to be redrawn
   */
  QRect moveDirection(KSpread::MoveTo direction, bool extendSelection);

  void processEnterKey(QKeyEvent *event);
  void processArrowKey(QKeyEvent *event);
  void processEscapeKey(QKeyEvent *event);
  bool processHomeKey(QKeyEvent *event);
  bool processEndKey(QKeyEvent *event);
  bool processPriorKey(QKeyEvent *event);
  bool processNextKey(QKeyEvent *event);
  void processDeleteKey(QKeyEvent *event);
  void processF2Key(QKeyEvent *event);
  void processF4Key(QKeyEvent *event);
  void processOtherKey(QKeyEvent *event);
  bool processControlArrowKey(QKeyEvent *event);

  void updateChooseRect(const QPoint &newMarker, const QPoint &newAnchor);

  /**
   * This function sets the paint dirty flag for a selection change.  The idea
   * here is that only the edges of the selection need to change (no matter whether
   * its the real selection or the 'choose' selection).  This calculates which
   * cells really should look different with the new selection rather than repainting
   * the entire area
   */
  void setSelectionChangePaintDirty(KSpreadSheet* sheet,
                                    QRect area1, QRect area2);
};

/**
 */
class KSpreadHBorder : public QWidget
{
    Q_OBJECT
public:
    KSpreadHBorder( QWidget *_parent, KSpreadCanvas *_canvas, KSpreadView *_view  );
    ~KSpreadHBorder();

    int markerColumn() const { return  m_iSelectionAnchor; }
    void resizeColumn( double resize, int nb = -1, bool makeUndo = true );
    void adjustColumn( int _col = -1, bool makeUndo = true );
    void equalizeColumn( double resize );

    void updateColumns( int from, int to );
    
    QSize sizeHint() const;

private slots:
    void doAutoScroll();

protected:
    virtual void paintEvent ( QPaintEvent* _ev );
    virtual void mousePressEvent( QMouseEvent* _ev );
    virtual void mouseReleaseEvent( QMouseEvent* _ev );
    virtual void mouseDoubleClickEvent( QMouseEvent* _ev );
    virtual void mouseMoveEvent( QMouseEvent* _ev );
    virtual void wheelEvent( QWheelEvent* );
    virtual void focusOutEvent( QFocusEvent* ev );
    void paintSizeIndicator( int mouseX, bool firstTime );

private:
    KSpreadCanvas *m_pCanvas;
    KSpreadView *m_pView;
    QTimer * m_scrollTimer;

    /**
     * Flag that inidicates whether the user wants to mark columns.
     * The user may mark columns by dragging the mouse around in th XBorder widget.
     * If he is doing that right now, this flag is TRUE. Mention that the user may
     * also resize columns by dragging the mouse. This case is not covered by this flag.
     */
    bool m_bSelection;

    /**
     * The column over which the user pressed the mouse button.
     * If the user marks columns in the XBorder widget, then this is the initial
     * column on which he pressed the mouse button.
     */
    int m_iSelectionAnchor;

    /**
     * Flag that indicates whether the user resizes a column
     * The user may resize columns by dragging the mouse around in the HBorder widget.
     * If he is doing that right now, this flag is TRUE.
     */
    bool m_bResize;

    /**
     * The column over which the user pressed the mouse button.
     * The user may resize columns by dragging the mouse around the XBorder widget.
     * This is the column over which he pressed the mouse button. This column is going
     * to be resized.
      */
    int m_iResizedColumn;

    /**
     * Last position of the mouse, when resizing.
     */
    int m_iResizePos;

    /**
     * The label used for showing the current size, when resizing
     */
    QLabel *m_lSize;

    /**
     * True when the mouse button is pressed
     */
    bool m_bMousePressed;

private:
};

/**
 */
class KSpreadVBorder : public QWidget
{
    Q_OBJECT
public:
    KSpreadVBorder( QWidget *_parent, KSpreadCanvas *_canvas, KSpreadView *_view );
    ~KSpreadVBorder();

    int markerRow() const { return  m_iSelectionAnchor; }
    void resizeRow( double resize, int nb = -1, bool makeUndo = true );
    void adjustRow( int _row = -1, bool makeUndo = true );
    void equalizeRow( double resize );
    void updateRows( int from, int to );

    QSize sizeHint() const;

private slots:
    void doAutoScroll();

protected:
    virtual void paintEvent ( QPaintEvent* _ev );
    virtual void mousePressEvent( QMouseEvent* _ev );
    virtual void mouseReleaseEvent( QMouseEvent* _ev );
    virtual void mouseMoveEvent( QMouseEvent* _ev );
    virtual void mouseDoubleClickEvent( QMouseEvent* _ev );
    virtual void wheelEvent( QWheelEvent* );
    virtual void focusOutEvent( QFocusEvent* ev );
    void paintSizeIndicator( int mouseY, bool firstTime );

private:
    KSpreadCanvas *m_pCanvas;
    KSpreadView *m_pView;
    QTimer * m_scrollTimer;

    bool m_bSelection;
    int m_iSelectionAnchor;
    bool m_bResize;
    int m_iResizedRow;
    int m_iResizePos;
    /**
     * The label used for showing the current size, when resizing
     */
    QLabel *m_lSize;

    /**
     * True when the mouse button is pressed
     */
    bool m_bMousePressed;
};

/*
 * Tooltip, which displays the comment and cell content, when it's too short
 */
class KSpreadToolTip : public QToolTip
{
public:
    KSpreadToolTip( KSpreadCanvas* canvas );

protected:
    /**
     * @reimp
     */
    void maybeTip( const QPoint& p );

private:
    KSpreadCanvas* m_canvas;
};

#endif // KSPREAD_CANVAS
