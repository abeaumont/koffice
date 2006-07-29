/* This file is part of the KDE project
   Copyright (C) 2002 Till Busch <till@bux.at>
   Copyright (C) 2003 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2006 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and,or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.

   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#ifndef KEXITABLEVIEW_H
#define KEXITABLEVIEW_H

//we want to have kde-support:
#define USE_KDE

#include <qscrollview.h>
#include <qtimer.h>
#include <qvariant.h>
#include <qptrlist.h>
#include <qheader.h>

#include <kdebug.h>

#include "kexitableviewdata.h"
#include "kexitableedit.h"
#include <kexiutils/tristate.h>
#include <widget/utils/kexirecordnavigator.h>
#include <widget/utils/kexisharedactionclient.h>
#include "kexidataawareobjectiface.h"

class KPopupMenu;
class KPrinter;
class KAction;

class KexiTableHeader;
class KexiTableItem;
class KexiTableEdit;
class KexiTableViewPrivate;
class KActionCollection;

namespace KexiDB {
	class RowEditBuffer;
}

//! minimum column width in pixels
#define KEXITV_MINIMUM_COLUMN_WIDTH 10

//! @short KexiTableView class provides a widget for displaying data in a tabular view.
/*! @see KexiFormScrollView
*/
class KEXIDATATABLE_EXPORT KexiTableView :
	public QScrollView,
	public KexiRecordNavigatorHandler,
	public KexiSharedActionClient,
	public KexiDataAwareObjectInterface
{
Q_OBJECT
KEXI_DATAAWAREOBJECTINTERFACE
public:

	/*! Defines table view's detailed appearance settings. */
	class KEXIDATATABLE_EXPORT Appearance {
	public:
		Appearance(QWidget *widget = 0);

		/*! base color for cells, default is "Base" color for application's 
		 current active palette */
		QColor baseColor;

		/*! text color for cells, default is "Text" color for application's 
		 current active palette */
		QColor textColor;

		/*! border color for cells, default is QColor(200,200,200) */
		QColor borderColor;

		/*! empty area color, default is "Base" color for application's 
		 current active palette */
		QColor emptyAreaColor;

		/*! alternate background color, default is KGlobalSettings::alternateBackgroundColor() */
		QColor alternateBackgroundColor;

		/*! true if background altering should be enabled, true by default */
		bool backgroundAltering : 1;

		/*! true if full-row-selection mode is set,
		 what means that all cells of the current row are always selected, instead of single cell.
		 This mode is usable for read-only table views, when we're interested only in navigating
		 by rows. False by default, even for read-only table views.
		*/
		bool fullRowSelection : 1;

		/*! true if fullgrid is enabled. True by default. 
		 It is set to false for comboboxpopup table, to mimic original 
		 combobox look and feel. */
		bool gridEnabled : 1;

		/*! \if the navigation panel is enabled (visible) for the view.
		 True by default. */
		bool navigatorEnabled : 1;

		/*! true if "row highlight" behaviour is enabled. False by default. */
		bool rowHighlightingEnabled : 1;

		/*! true if "row highlight over " behaviour is enabled. False by default. */
		bool rowMouseOverHighlightingEnabled : 1;

		/*! true if selection of a row should be kept when a user moved mouse 
		 pointer over other rows. Makes only sense when rowMouseOverHighlightingEnabled is true. 
		 True by default. It is set to false for comboboxpopup table, to mimic original 
		 combobox look and feel. */
		bool persistentSelections : 1;

		/*! color for row highlight, default is intermediate (33%/60%) between
		 active highlight and base color. */
		QColor rowHighlightingColor;

		/*! color for text under row highlight, default is the same as textColor. 
		 Used when rowHighlightingEnabled is true; */
		QColor rowHighlightingTextColor;

		/*! color for row highlight for mouseover, default is intermediate (20%/80%) between
		 active highlight and base color. Used when rowMouseOverHighlightingEnabled is true. */
		QColor rowMouseOverHighlightingColor;

		/*! color for text under row highlight for mouseover, default is the same as textColor. 
		 Used when rowMouseOverHighlightingEnabled is true; */
		QColor rowMouseOverHighlightingTextColor;

		/*! Like rowMouseOverHighlightingColor but for areas painted with alternate color. 
		 This is computed using active highlight color and alternateBackgroundColor. */
		QColor rowMouseOverAlternateHighlightingColor;
	};
	
	KexiTableView(KexiTableViewData* data=0, QWidget* parent=0, const char* name=0);
	virtual ~KexiTableView();

	/*! \return current appearance settings */
	Appearance appearance() const;

	/*! Sets appearance settings. Table view is updated automatically. */
	void setAppearance(const Appearance& a);

	/*! \return string displayed for column's header \a colNum */
	QString columnCaption(int colNum) const;

	/*! Convenience function. 
	 \return field object that define column \a colNum or NULL if there is no such column */
	KexiDB::Field* field(int colNum) const;

	/*! Reimplementation for KexiDataAwareObjectInterface */
	virtual void setSpreadSheetMode();

	/*! \return true if vertical scrollbar's tooltips are enabled (true by default). */
	bool scrollbarToolTipsEnabled() const;

	/*! Enables or disables vertical scrollbar's. */
	void setScrollbarToolTipsEnabled(bool set);

	/*! \return maximum number of rows that can be displayed per one "page" 
	 for current table view's size. */
	virtual int rowsPerPage() const;

	QRect cellGeometry(int row, int col) const;
	int columnWidth(int col) const;
	int rowHeight() const;
	int columnPos(int col) const;
	int rowPos(int row) const;
	int columnAt(int pos) const;
	int rowAt(int pos, bool ignoreEnd=false) const;

	/*! Redraws specified cell. */
	virtual void updateCell(int row, int col);

	/*! Redraws the current cell. Implemented after KexiDataAwareObjectInterface. */
	virtual void updateCurrentCell();

	/*! Redraws all cells of specified row. */
	virtual void updateRow(int row);

	bool editableOnDoubleClick() const;
	void setEditableOnDoubleClick(bool set);

	//! \return true if the vertical header is visible
	bool verticalHeaderVisible() const;
	//! Sets vertical header's visibility
	void setVerticalHeaderVisible(bool set);

	//! \return true if the horizontal header is visible
	bool horizontalHeaderVisible() const;

	//! Sets horizontal header's visibility
	void setHorizontalHeaderVisible(bool set);

#ifndef KEXI_NO_PRINT
	// printing
//	void		setupPrinter(KPrinter &printer);
	void		print(KPrinter &printer);
#endif

	// reimplemented for internal reasons
	virtual QSizePolicy sizePolicy() const;
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;

	/*! Reimplemented to update cached fonts and row sizes for the painter. */
	void setFont(const QFont &f);

	virtual QSize tableSize() const;

	void emitSelected();

	//! single shot after 1ms for contents updatinh
	void triggerUpdate();

	typedef enum ScrollDirection
	{
		ScrollUp,
		ScrollDown,
		ScrollLeft,
		ScrollRight
	};

	virtual bool eventFilter( QObject *o, QEvent *e );

	//! Initializes standard editor cell editor factories. This is called internally, once.
	static void initCellEditorFactories();

	/*! \return highlighted row number or -1 if no row is highlighted. 
	 Makes sense if row highlighting is enabled.
	 @see Appearance::rowHighlightingEnabled setHighlightedRow() */
	int highlightedRow() const;

	KexiTableItem *highlightedItem() const;


public slots:
	virtual void setData( KexiTableViewData *data, bool owner = true )
		{ KexiDataAwareObjectInterface::setData( data, owner ); }

	virtual void clearColumnsInternal(bool repaint);

	/*! Adjusts \a colNum column's width to its (current) contents. 
	 If \a colNum == -1, all columns' width is adjusted. */
	void adjustColumnWidthToContents(int colNum = -1);

	//! Sets width of column width to \a width.
	void setColumnWidth(int col, int width);

	/*! If \a set is true, \a colNum column is resized to take full possible width.
	 If \a set is false, no automatic resize will be performed. 
	 If \a colNum is -1, all columns are equally resized, when needed, to take full possible width.
	 This method behaves like QHeader::setStretchEnabled ( bool b, int section ). */
	void setColumnStretchEnabled( bool set, int colNum );

	/*! Maximizes widths of columns selected by \a columnList, so the horizontal 
	 header has maximum overall width. Each selected column's width will be increased 
	 by the same value. Does nothing if \a columnList is empty or there is no free space 
	 to resize columns. If this table view is not visible, resizing will be performed on showing. */
	void maximizeColumnsWidth( const QValueList<int> &columnList );

	/*! Adjusts the size of the sections to fit the size of the horizontal header 
	 as completely as possible. Only sections for which column stretch is enabled will be resized.
	 \sa setColumnStretchEnabled() QHeader::adjustHeaderSize() */
	void adjustHorizontalHeaderSize();

	/*! Sets highlighted row number or -1 if no row has to be highlighted. 
	 Makes sense if row highlighting is enabled.
	 @see Appearance::rowHighlightingEnabled */
	void setHighlightedRow(int row);

	/*! Sets no row that will be highlighted. Equivalent to setHighlightedRow(-1). */
	inline void clearHighlightedRow() { setHighlightedRow(-1); }

	/*! Ensures that cell at \a row and \a col is visible. 
	 If \a col is -1, current column number is used. \a row and \a col (if not -1) must 
	 be between 0 and rows() (or cols() accordingly). */
	virtual void ensureCellVisible(int row, int col/*=-1*/);

//	void			gotoNext();
//js	int			findString(const QString &string);
	
	/*! Deletes currently selected record; does nothing if no record 
	 is currently selected. If record is in edit mode, editing 
	 is cancelled before deleting.  */
	virtual void deleteCurrentRow() { KexiDataAwareObjectInterface::deleteCurrentRow(); }

	/*! Inserts one empty row above row \a row. If \a row is -1 (the default),
	 new row is inserted above the current row (or above 1st row if there is no current).
	 A new item becomes current if row is -1 or if row is equal currentRow().
	 This method does nothing if:
	 -inserting flag is disabled (see isInsertingEnabled())
	 -read-only flag is set (see isReadOnly())
	 \ return inserted row's data
	*/
	virtual KexiTableItem *insertEmptyRow(int row = -1) 
		{ return KexiDataAwareObjectInterface::insertEmptyRow(row); }

	/*! Used when Return key is pressed on cell or "+" nav. button is clicked.
	 Also used when we want to continue editing a cell after "invalid value" message
	 was displayed (in this case, \a setText is usually not empty, what means
	 that text will be set in the cell replacing previous value).
	*/
	virtual void startEditCurrentCell(const QString& setText = QString::null)
		{ KexiDataAwareObjectInterface::startEditCurrentCell(setText); }

	/*! Deletes currently selected cell's contents, if allowed. 
	 In most cases delete is not accepted immediately but "row editing" mode is just started. */
	virtual void deleteAndStartEditCurrentCell() 
		{ KexiDataAwareObjectInterface::deleteAndStartEditCurrentCell(); }

	/*! Cancels row editing All changes made to the editing 
	 row during this current session will be undone. */
	virtual void cancelRowEdit() { KexiDataAwareObjectInterface::cancelRowEdit(); }

	/*! Accepts row editing. All changes made to the editing 
	 row during this current session will be accepted (saved). 
	 \return true if accepting was successful, false otherwise 
	 (e.g. when current row contain data that does not meet given constraints). */
	virtual bool acceptRowEdit() { return KexiDataAwareObjectInterface::acceptRowEdit(); }

	/*! Specifies, if this table view automatically accepts 
	 row editing (using acceptRowEdit()) on accepting any cell's edit 
	 (i.e. after acceptEditor()). \sa acceptsRowEditAfterCellAccepting() */
	virtual void setAcceptsRowEditAfterCellAccepting(bool set) 
		{ KexiDataAwareObjectInterface::setAcceptsRowEditAfterCellAccepting(set); }

	/*! Specifies, if this table accepts dropping data on the rows. 
	 If enabled:
	 - dragging over row is indicated by drawing a line at bottom side of this row
	 - dragOverRow() signal will be emitted on dragging,
	  -droppedAtRow() will be emitted on dropping
	 By default this flag is set to false. */
	virtual void setDropsAtRowEnabled(bool set) { KexiDataAwareObjectInterface::setDropsAtRowEnabled(set); }

	virtual void cancelEditor() { KexiDataAwareObjectInterface::cancelEditor(); }
	virtual bool acceptEditor() { return KexiDataAwareObjectInterface::acceptEditor(); }

signals:
	virtual void dataSet( KexiTableViewData *data );

	virtual void itemSelected(KexiTableItem *);
	virtual void cellSelected(int col, int row);

	void itemReturnPressed(KexiTableItem *, int row, int col);
	void itemDblClicked(KexiTableItem *, int row, int col);
	void itemMouseReleased(KexiTableItem *, int row, int col);

	void dragOverRow(KexiTableItem *item, int row, QDragMoveEvent* e);
	void droppedAtRow(KexiTableItem *item, int row, QDropEvent *e, KexiTableItem*& newItem);

	/*! Data has been refreshed on-screen - emitted from initDataContents(). */
	virtual void dataRefreshed();

	virtual void itemChanged(KexiTableItem *, int row, int col);
	virtual void itemChanged(KexiTableItem *, int row, int col, QVariant oldValue);
	virtual void itemDeleteRequest(KexiTableItem *, int row, int col);
	virtual void currentItemDeleteRequest();
	void addRecordRequest();
//	void contextMenuRequested(KexiTableItem *,  int row, int col, const QPoint &);
	void sortedColumnChanged(int col);

	//! emmited when row editing is started (for updating or inserting)
	void rowEditStarted(int row);

	//! emmited when row editing is terminated (for updating or inserting)
	//! no matter if accepted or not
	void rowEditTerminated(int row);

	//! Emitted in initActions() to force reload actions
	//! You should remove existing actions and add them again.
	void reloadActions();

protected slots:
	void slotSettingsChanged(int category);

	virtual void slotDataDestroying() { KexiDataAwareObjectInterface::slotDataDestroying(); }

	virtual void slotRowsDeleted( const QValueList<int> & ); 

	//! updates display after many rows deletion
	void slotColumnWidthChanged( int col, int os, int ns );

	void slotSectionHandleDoubleClicked( int section );

	void slotUpdate();
	//! implemented because we needed this as slot
	virtual void sortColumnInternal(int col, int order = 0)
		{ KexiDataAwareObjectInterface::sortColumnInternal(col, order); }

	void slotAutoScroll();

	//! internal, used after vscrollbar's value has been changed
	void vScrollBarValueChanged(int v);
	void vScrollBarSliderReleased();
	void scrollBarTipTimeout();
	//! internal, used when top header's size changed
	void slotTopHeaderSizeChange( int section, int oldSize, int newSize );

	//! receives a signal from cell editors
	void slotEditRequested();

	/*! Reloads data for this widget.
	 Handles KexiTableViewData::reloadRequested() signal. */
	virtual void reloadData() { KexiDataAwareObjectInterface::reloadData(); }

	//! Handles KexiTableViewData::rowRepaintRequested() signal
	virtual void slotRowRepaintRequested(KexiTableItem& item);

	//! Handles KexiTableViewData::aboutToDeleteRow() signal. Prepares info for slotRowDeleted().
	virtual void slotAboutToDeleteRow(KexiTableItem& item, KexiDB::ResultInfo* result, bool repaint)
	{ KexiDataAwareObjectInterface::slotAboutToDeleteRow(item, result, repaint); }

	//! Handles KexiTableViewData::rowDeleted() signal to repaint when needed.
	virtual void slotRowDeleted() { KexiDataAwareObjectInterface::slotRowDeleted(); }

	//! Handles KexiTableViewData::rowInserted() signal to repaint when needed.
	virtual void slotRowInserted(KexiTableItem *item, bool repaint)
	{ KexiDataAwareObjectInterface::slotRowInserted(item, repaint); }

	//! Like above, not db-aware version
	virtual void slotRowInserted(KexiTableItem *item, uint row, bool repaint)
	{ KexiDataAwareObjectInterface::slotRowInserted(item, row, repaint); }

protected:
	/*! Reimplementation for KexiDataAwareObjectInterface 
	 Initializes data contents (resizes it, sets cursor at 1st row).
	 Called on setData(). Also called once on show event after
	 reloadRequested() signal was received from KexiTableViewData object. */
	virtual void initDataContents();

	/*! Implementation for KexiDataAwareObjectInterface.
	 Updates widget's contents size using QScrollView::resizeContents() 
	 depending on tableSize(). */
	virtual void updateWidgetContentsSize();

	/*! Reimplementation for KexiDataAwareObjectInterface */
	virtual void clearVariables();

	/*! Implementation for KexiDataAwareObjectInterface */
	virtual int currentLocalSortingOrder() const;

	/*! Implementation for KexiDataAwareObjectInterface */
	virtual int currentLocalSortColumn() const;

	/*! Implementation for KexiDataAwareObjectInterface */
	virtual void setLocalSortingOrder(int col, int order);

	/*! Implementation for KexiDataAwareObjectInterface */
	virtual void updateGUIAfterSorting();

	/*! Implementation for KexiDataAwareObjectInterface */
	virtual void updateWidgetScrollBars() { updateScrollBars(); }

//	/*! Implementation for KexiDataAwareObjectInterface */
//	virtual void emitSortedColumnChanged(int col) { emit sortedColumnChanged(col); }

//	/*! Implementation for KexiDataAwareObjectInterface */
//	virtual void emitRowEditTerminated(int row) { emit rowEditTerminated(row); }

	/*! Implementation for KexiDataAwareObjectInterface.
	 Adds another sections within horizontal header. */
	virtual void addHeaderColumn(const QString& caption, const QString& description, int size);

	//! painting and layout
	void drawContents(QPainter *p, int cx, int cy, int cw, int ch);
	void createBuffer(int width, int height);
	void paintCell(QPainter* p, KexiTableItem *item, int col, int row, const QRect &cr, bool print=false);
	void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);
	void updateGeometries();

	QPoint contentsToViewport2( const QPoint &p );
	void contentsToViewport2( int x, int y, int& vx, int& vy );
	QPoint viewportToContents2( const QPoint& vp );

	// event handling
	virtual void contentsMousePressEvent(QMouseEvent*);
	virtual void contentsMouseReleaseEvent(QMouseEvent*);
	bool handleContentsMousePressOrRelease(QMouseEvent* e, bool release);
	virtual void contentsMouseMoveEvent(QMouseEvent*);
	virtual void contentsMouseDoubleClickEvent(QMouseEvent*);
	virtual void keyPressEvent(QKeyEvent*);
	virtual void focusInEvent(QFocusEvent*);
	virtual void focusOutEvent(QFocusEvent*);
	virtual void resizeEvent(QResizeEvent *);
	virtual void viewportResizeEvent( QResizeEvent *e );//js
	virtual void showEvent(QShowEvent *e);
	virtual void contentsDragMoveEvent(QDragMoveEvent *e);
	virtual void contentsDropEvent(QDropEvent *ev);
	virtual void viewportDragLeaveEvent( QDragLeaveEvent * );
	virtual void paletteChange( const QPalette & );
	
	/*! Implementation for KexiDataAwareObjectInterface */
	virtual KexiDataItemInterface *editor( int col, bool ignoreMissingEditor = false );

	/*! Implementation for KexiDataAwareObjectInterface */
	virtual void editorShowFocus( int row, int col );

	//! Creates editors and shows it, what usually means the beginning of a cell editing
	virtual void createEditor(int row, int col, const QString& addText = QString::null, 
		bool removeOld = false);

	bool focusNextPrevChild(bool next);

	/*! Used in key event: \return true if event \a e should execute action \a action_name.
	 Action shortcuts defined by shortCutPressed() are reused, if present, and if \a e matches 
	 given action's shortcut - false is returned (beause action is already performed at main 
	 window's level).
	*/
	bool shortCutPressed( QKeyEvent *e, const QCString &action_name );

#if 0 //we have now KexiActionProxy
	/*! Updates visibility/accesibility of popup menu items,
	returns false if no items are visible after update. */
	bool updateContextMenu();
#endif

	/*! Shows context menu at \a pos for selected cell
	 if menu is configured,
	 else: contextMenuRequested() signal is emmited.
	 Method used in contentsMousePressEvent() (for right button)
	 and keyPressEvent() for Qt::Key_Menu key.
	 If \a pos is QPoint(-1,-1) (the default), menu is positioned below the current cell.
	*/
	void showContextMenu( const QPoint& pos = QPoint(-1,-1) );

	/*! internal */
	inline void paintRow(KexiTableItem *item,
		QPainter *pb, int r, int rowp, int cx, int cy, 
		int colfirst, int collast, int maxwc);

	virtual void setHBarGeometry( QScrollBar & hbar, int x, int y, int w, int h );

	//! Setups navigator widget
	void setupNavigator();

	//! internal, to determine valid row number when navigator text changed
	int validRowNumber(const QString& text);

	/*! Reimplementation for KexiDataAwareObjectInterface 
	 (viewport()->setFocus() is just added) */
	virtual void removeEditor();

	//! Internal: updated sched fonts for painting.
	void updateFonts(bool repaint = false);

	/*! @internal Changes bottom margin settings, in pixels. 
	 At this time, it's used by KexiComboBoxPopup to decrease margin for popup's table. */
	void setBottomMarginInternal(int pixels);

	virtual void updateWidgetContents() { update(); }

	//! for navigator
	virtual void moveToRecordRequested(uint r);
	virtual void moveToLastRecordRequested();
	virtual void moveToPreviousRecordRequested();
	virtual void moveToNextRecordRequested();
	virtual void moveToFirstRecordRequested();
	virtual void addNewRecordRequested() { KexiDataAwareObjectInterface::addNewRecordRequested(); }

	//! Copy current selection to a clipboard (e.g. cell)
	virtual void copySelection();

	//! Cut current selection to a clipboard (e.g. cell)
	virtual void cutSelection();

	//! Paste current clipboard contents (e.g. to a cell)
	virtual void paste();

	/*! Used in KexiDataAwareObjectInterface::slotRowDeleted() 
	 to repaint tow \a row and all visible below. */
	virtual void updateAllVisibleRowsBelow(int row);

	void updateAfterCancelRowEdit();
	void updateAfterAcceptRowEdit();

//	//! Called to repaint contents after a row is deleted.
//	void repaintAfterDelete();

	inline KexiRecordMarker* verticalHeader() const { return m_verticalHeader; }

	//--------------------------

	KexiTableViewPrivate *d;

	class WhatsThis;
	friend class KexiTableItem;
	friend class WhatsThis;
};

/*! A header with additional actions.
 Displays field description (Field::description()) text as tool tip, if available.
 Displays tool tips if a pointed section is not wide enough to fit its label text.

 \todo react on indexChange ( int section, int fromIndex, int toIndex ) signal
*/
class KEXIDATATABLE_EXPORT TableViewHeader : public QHeader
{
	Q_OBJECT

	public:
		TableViewHeader(QWidget * parent = 0, const char * name = 0);

		int addLabel ( const QString & s, int size = -1 );

		int addLabel ( const QIconSet & iconset, const QString & s, int size = -1 );

		void removeLabel( int section );

		/*! Sets \a toolTip for \a section. */
		void setToolTip( int section, const QString & toolTip );

		virtual bool eventFilter(QObject * watched, QEvent * e);

	protected slots:
		void slotSizeChange(int section, int oldSize, int newSize );

	protected:
		int m_lastToolTipSection;
		QRect m_toolTipRect;

		QStringList m_toolTips;
};

#endif
