/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef frame_h
#define frame_h

#include <qdom.h>

#include "defs.h"
#include <koPicture.h>
#include <koRect.h>
#include <qbrush.h>
#include <qptrlist.h>
#include "kwstyle.h"
#include "koborder.h"

namespace KFormula {
    class Container;
    class View;
}
class FormulaView;

class KCommand;
class KWAnchor;
class KWCanvas;
class KWChild;
class KWDocument;
class KWFrame;
class KWFrameSet;
class KWTextFrameSet;
class KWResizeHandle;
class KWTableFrameSet;
class KWTextDocument;
class KWTextParag;
class KWView;
class KWViewMode;
class QCursor;
class QPainter;
class QPoint;
class QRegion;
class QSize;
class QProgressDialog;
class KWTextFrameSet;
class KWFramePartMoveCommand;
class KWordFrameSetIface;
class DCOPObject;
/**
 * This class represents a single frame.
 * A frame belongs to a frameset which states its contents.
 * A frame does NOT have contents, the frameset stores that.
 * A frame is really just a square that is used to place the content
 * of a frameset.
 */
class KWFrame : public KoRect
{
public:
    /** Runaround types
     * RA_NO = No run around, all text is just printed.
     * RA_BOUNDINGRECT = run around the square of this frame.
     * RA_SKIP = stop running text on the whole horizontal space this frame occupies.
     */
    enum RunAround { RA_NO = 0, RA_BOUNDINGRECT = 1, RA_SKIP = 2 };

    /**
     * Constructor
     * @param fs parent frameset
     * @param left, top, width, height coordinates of the frame
     * The page number will be automatically determined from the position of the frame.
     * @param ra the "runaround" setting, i.e. whether text should flow below the frame,
     * around the frame, or avoiding the frame on the whole horizontal band.
     * @param gap ...
     */
    KWFrame(KWFrameSet *fs, double left, double top, double width, double height,
            RunAround ra = RA_BOUNDINGRECT, double gap = MM_TO_POINT( 1.0 ));
    KWFrame(KWFrame * frame);
    /* Destructor */
    virtual ~KWFrame();

    /** a frame can be selected by the user clicking on it. The frame
     * remembers if it is selected
     * - which is actually pretty bad in terms of doc/view design (DF)
     */
    void setSelected( bool _selected );
    bool isSelected() { return selected; }

    QCursor getMouseCursor( const KoPoint& docPoint, bool table, QCursor defaultCursor );

    double runAroundGap() { return m_runAroundGap; }
    void setRunAroundGap( double gap ) { m_runAroundGap = gap; }

    RunAround runAround() { return m_runAround; }
    void setRunAround( RunAround _ra ) { m_runAround = _ra; }


    /** what should happen when the frame is full */
    enum FrameBehavior { AutoExtendFrame=0 , AutoCreateNewFrame=1, Ignore=2 };

    FrameBehavior frameBehavior() { return m_frameBehavior; }
    void setFrameBehavior( FrameBehavior fb ) { m_frameBehavior = fb; }

    /* Frame duplication properties */

    /** This frame will only be copied to:
     *   AnySide, OddSide or EvenSide
     */
    enum SheetSide { AnySide=0, OddSide=1, EvenSide=2};
    SheetSide sheetSide()const { return m_sheetSide; }
    void setSheetSide( SheetSide ss ) { m_sheetSide = ss; }

    /** What happens on new page (create a new frame and reconnect, no followup, make copy) */
    enum NewFrameBehavior { Reconnect=0, NoFollowup=1, Copy=2 };
    NewFrameBehavior newFrameBehavior()const { return m_newFrameBehavior; }
    void setNewFrameBehavior( NewFrameBehavior nfb ) { m_newFrameBehavior = nfb; }

    /** Drawing property: if isCopy, this frame is a copy of the previous frame in the frameset */
    bool isCopy()const { return m_bCopy; }
    void setCopy( bool copy ) { m_bCopy = copy; }

    /** Data stucture methods */
    KWFrameSet *frameSet() const { return m_frameSet; }
    void setFrameSet( KWFrameSet *fs ) { m_frameSet = fs; }

    /** The page on which this frame is (0 based)*/
    int pageNum() const;
    /** Same thing, but works if the frame hasn't been added to a frameset yet */
    int pageNum( KWDocument* doc ) const;

    /** The z-order of the frame, relative to the other frames on the same page */
    void setZOrder( int z ) { m_zOrder = z; }
    int zOrder() const { return m_zOrder; }

    /** For KWFrameSet::updateFrames only. Clear list of frames on top of this one. */
    void clearFramesOnTop() { m_framesOnTop.clear(); }
    /** For KWFrameSet::updateFrames only. Add a frame on top of this one.
     * Note that order doesn't matter in that list, it's for clipping only. */
    void addFrameOnTop( KWFrame* fot ) { m_framesOnTop.append( fot ); }
    const QPtrList<KWFrame>& framesOnTop() const { return m_framesOnTop; }

    /** All borders can be custom drawn with their own colors etc. */
    const KoBorder &leftBorder() const { return brd_left; }
    const KoBorder &rightBorder() const { return brd_right; }
    const KoBorder &topBorder() const { return brd_top; }
    const KoBorder &bottomBorder() const { return brd_bottom; }

    void setLeftBorder( KoBorder _brd ) { brd_left = _brd; }
    void setRightBorder( KoBorder _brd ) { brd_right = _brd; }
    void setTopBorder( KoBorder _brd ) { brd_top = _brd; }
    void setBottomBorder( KoBorder _brd ) { brd_bottom = _brd; }

    /** Return the _zoomed_ rectangle for this frame, including the border - for drawing */
    QRect outerRect() const;
    /** Return the unzoomed rectangle, including the border, for the frames-on-top list.
        The default border of size 1-pixel that is drawn on screen is _not_ included here
        [since it depends on the zoom] */
    KoRect outerKoRect() const;

    /* Resize handles (in kwcanvas.h) are the dots that are drawn on selected
       frames, this creates and deletes them */
    void createResizeHandles();
    void createResizeHandlesForPage(KWCanvas *canvas);
    void removeResizeHandlesForPage(KWCanvas *canvas);
    void removeResizeHandles();
    void updateResizeHandles();
    void updateRulerHandles();

    QBrush backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor( QBrush _color ) { m_backgroundColor = _color; }

    KoRect innerRect() const;


    // The "internal Y" is the offset (in pt) of the real frame showed in this one
    // ("real" means "the last that isn't a copy")
    // This offset in pt is the sum of the height of the frames before that one.
    // For text frames, this is equivalent to the layout units (after multiplication).
    void setInternalY( double y ) { m_internalY = y; }
    double internalY() const { return m_internalY; }

    /** set left margin size */
    void setBLeft( double b ) { bleft = b; }
    /** set right margin size */
    void setBRight( double b ) { bright = b; }
    /** set top margin size */
    void setBTop( double b ) { btop = b; }
    /** set bottom margin size */
    void setBBottom( double b ) { bbottom = b; }

    /** get left margin size */
    double bLeft()const { return bleft; }
    /** get right margin size */
    double bRight()const { return bright; }
    /** get top margin size */
    double bTop()const { return btop; }
    /** get bottom margin size */
    double bBottom()const { return bbottom; }

    /** returns a copy of self */
    KWFrame *getCopy();

    void copySettings(KWFrame *frm);

    /** create XML to describe yourself */
    void save( QDomElement &frameElem );
    /** read attributes from XML. @p headerOrFooter if true some defaults are different */
    void load( QDomElement &frameElem, bool headerOrFooter, int syntaxVersion );

    void setMinFrameHeight(double h) {m_minFrameHeight=h;}
    double minFrameHeight(void)const {return m_minFrameHeight;}

    /** Return if the point is on the frame.
        @param point the point in normal coordinates.
        @param borderOfFrameOnly when true an additional check is done if the point
          is on the border.  */
    bool frameAtPos( const QPoint& nPoint, bool borderOfFrameOnly=false );

private:
    SheetSide m_sheetSide;
    RunAround m_runAround;
    FrameBehavior m_frameBehavior;
    NewFrameBehavior m_newFrameBehavior;
    double m_runAroundGap;
    double bleft, bright, btop, bbottom; // margins
    double m_minFrameHeight;

    double m_internalY;
    int m_zOrder;
    bool m_bCopy;
    bool selected;

    QBrush m_backgroundColor;
    KoBorder brd_left, brd_right, brd_top, brd_bottom;

    QPtrList<KWResizeHandle> handles;
    QPtrList<KWFrame> m_framesOnTop; // List of frames on top of us, those we shouldn't overwrite
    KWFrameSet *m_frameSet;

    // Prevent operator= and copy constructor
    KWFrame &operator=( const KWFrame &_frame );
    KWFrame ( const KWFrame &_frame );
};

/**
 * This object is created to edit a particular frameset in a particular view
 * The view's canvas creates it, and destroys it.
 */
class KWFrameSetEdit
{
public:
    KWFrameSetEdit( KWFrameSet * fs, KWCanvas * canvas );
    virtual ~KWFrameSetEdit() {}

    KWFrameSet * frameSet() const { return m_fs; }
    KWCanvas * canvas() const { return m_canvas; }
    KWFrame * currentFrame() const { return m_currentFrame; }

    /**
     * Return the current most-low-level text edit object
     */
    virtual KWFrameSetEdit* currentTextEdit() { return 0L; }

    /**
     * Called before destruction, when terminating edition - use to e.g. hide cursor
     */
    virtual void terminate(bool /*removeselection*/ = true) {}

    /**
     * Paint this frameset in "has focus" mode (e.g. with a cursor)
     * See KWFrameSet for explanation about the arguments.
     * Most framesets don't need to reimplement that (the KWFrameSetEdit gets passed to drawFrame)
     */
    virtual void drawContents( QPainter *, const QRect &,
                               QColorGroup &, bool onlyChanged, bool resetChanged,
                               KWViewMode *viewMode, KWCanvas *canvas );

    // Events forwarded by the canvas (when being in "edit" mode)
    virtual void keyPressEvent( QKeyEvent * ) {}
    virtual void keyReleaseEvent( QKeyEvent * ) {}
    virtual void mousePressEvent( QMouseEvent *, const QPoint &, const KoPoint & ) {}
    virtual void mouseMoveEvent( QMouseEvent *, const QPoint &, const KoPoint & ) {} // only called if button is pressed
    virtual void mouseReleaseEvent( QMouseEvent *, const QPoint &, const KoPoint & ) {}
    virtual void mouseDoubleClickEvent( QMouseEvent *, const QPoint &, const KoPoint & ) {}
    virtual void dragEnterEvent( QDragEnterEvent * ) {}
    virtual void dragMoveEvent( QDragMoveEvent *, const QPoint &, const KoPoint & ) {}
    virtual void dragLeaveEvent( QDragLeaveEvent * ) {}
    virtual void dropEvent( QDropEvent *, const QPoint &, const KoPoint & ) {}
    virtual void focusInEvent() {}
    virtual void focusOutEvent() {}
    virtual void copy() {}
    virtual void cut() {}
    virtual void paste() {}
    virtual void selectAll() {}

protected:
    KWFrameSet * m_fs;
    KWCanvas * m_canvas;
    /**
     * The Frameset-Edit implementation is responsible for updating that one
     * (to the frame where the current "cursor" is)
     */
    KWFrame * m_currentFrame;
};

/**
 * Class: KWFrameSet
 * Base type, a frameset holds content as well as frames to show that
 * content.
 * The different types of content are implemented in the different
 * types of frameSet implementations (see below)
 * @see KWTextFrameSet, KWPartFrameSet, KWPictureFrameSet,
 *      KWPartFrameSet, KWFormulaFrameSet
 */
class KWFrameSet : public QObject
{
    Q_OBJECT
public:
    // constructor
    KWFrameSet( KWDocument *doc );
    // destructor
    virtual ~KWFrameSet();

    virtual KWordFrameSetIface* dcopObject();

    /** The type of frameset. Use this to differentiate between different instantiations of
     *  the framesets. Each implementation will return a different frameType.
     */
    virtual FrameSetType type() { return FT_BASE; }

    virtual void addTextFrameSets( QPtrList<KWTextFrameSet> & /*lst*/ ) {};

    /** The different types of textFramesets (that TEXT is important here!)
     * FI_BODY = normal text frames.<br>
     * FI_FIRST_HEADER = Header on page 1<br>
     * FI_ODD_HEADER = header on any odd page (can be including page 1)<br>
     * FI_EVEN_HEADER = header on any even page<br>
     * FI_FIRST_FOOTER = footer on page 1<br>
     * FI_ODD_FOOTER = footer on any odd page (can be including page 1)<br>
     * FI_EVEN_FOOTER = footer on any even page<br>
     * FI_FOOTNOTE = a footnote frame.
     */
    enum Info { FI_BODY = 0, FI_FIRST_HEADER = 1, FI_ODD_HEADER = 2, FI_EVEN_HEADER = 3,
                FI_FIRST_FOOTER = 4, FI_ODD_FOOTER = 5, FI_EVEN_FOOTER = 6,
                FI_FOOTNOTE = 7 };
    /** Returns the type of TextFrameSet this is */
    Info frameSetInfo() { return m_info; }
    /** Set the type of TextFrameSet this is */
    void setFrameSetInfo( Info fi ) { m_info = fi; }

    bool isAHeader() const;
    bool isAFooter() const;
    bool isHeaderOrFooter() const { return isAHeader() || isAFooter(); }

    bool isAWrongHeader( KoHFType t ) const;
    bool isAWrongFooter( KoHFType t ) const;

    bool isMainFrameset() const;
    bool isMoveable() const;

    // frame management
    virtual void addFrame( KWFrame *_frame, bool recalc = true );
    virtual void delFrame( unsigned int _num );
    virtual void delFrame( KWFrame *frm, bool remove = true );
    void deleteAllFrames();
    void deleteAllCopies(); // for headers/footers only

    /** retrieve frame from x and y coords (unzoomed coords) */
    KWFrame *frameAtPos( double _x, double _y );

    /** Return if the point is on the frame.
        @param point the point in normal coordinates.
        @param borderOfFrameOnly when true an additional check is done if the point
          is on the border.  */
    virtual bool isFrameAtPos( KWFrame* frame, const QPoint& nPoint, bool borderOfFrameOnly=false );

    /** return a frame if nPoint in on one of its borders */
    KWFrame *frameByBorder( const QPoint & nPoint );

    /** get a frame by number */
    KWFrame *frame( unsigned int _num );

    /** get the frame whose settings apply for @p frame
        (usually @p frame, but can also be the real frame if frame is a copy) */
    static KWFrame * settingsFrame(KWFrame* frame);

    /** Iterator over the child frames */
    const QPtrList<KWFrame> &frameIterator() const { return frames; }
    /** Get frame number */
    int frameFromPtr( KWFrame *frame );
    /** Get number of child frames */
    unsigned int getNumFrames() const { return frames.count(); }

    /** Called when the user resizes a frame. Calls resizeFrame. */
    void resizeFrameSetCoords( KWFrame* frame, double newLeft, double newTop, double newRight, double newBottom, bool finalSize );
    /** Called when the user resizes a frame. Reimplemented by KWPictureFrameSet. */
    virtual void resizeFrame( KWFrame* frame, double newWidth, double newHeight, bool finalSize );

    /** True if the frameset was deleted (but not destroyed, since it's in the undo/redo) */
    bool isDeleted() const { return frames.isEmpty(); }

    /** Create a framesetedit object to edit this frameset in @p canvas */
    virtual KWFrameSetEdit * createFrameSetEdit( KWCanvas * ) { return 0L; }

    /**
     * @param emptyRegion The region is modified to subtract the areas painted, thus
     *                    allowing the caller to determine which areas remain to be painted.
     * Framesets that can be transparent should reimplement this and make it a no-op,
     * so that the background is painted below the transparent frame.
     */
    virtual void createEmptyRegion( const QRect & crect, QRegion & emptyRegion, KWViewMode *viewMode );

    /**
     * Paint the borders for one frame of this frameset.
     * @param painter The painter in which to draw the contents of the frameset
     * @param frame The frame to be drawn
     * @param settingsFrame The frame from which we take the settings (usually @p frame, but not with Copy behaviour)
     * @param crect The rectangle (in "contents coordinates") to be painted
     * @param canvas The canvas in which we are drawing (for settings)
     */
    void drawFrameBorder( QPainter *painter, KWFrame *frame, KWFrame *settingsFrame,
                          const QRect &crect, KWViewMode *viewMode, KWCanvas *canvas );

    /**
     * Paint this frameset
     * @param painter The painter in which to draw the contents of the frameset
     * @param crect The rectangle (in "contents coordinates") to be painted
     * @param cg The colorgroup from which to get the colors
     * @param onlyChanged If true, only redraw what has changed (see KWCanvas::repaintChanged)
     * @param resetChanged If true, set the changed flag to false after drawing.
     * @param edit If set, this frameset is being edited, so a cursor is needed.
     * @param viewMode For coordinate conversion, always set.
     * @param canvas For view settings. WARNING: canvas can be 0 (e.g. in embedded documents).
     *
     * The way this "onlyChanged/resetChanged" works is: when something changes,
     * all views are asked to redraw themselves with onlyChanged=true.
     * But all views except the last one shouldn't reset the changed flag to false,
     * otherwise the other views wouldn't repaint anything.
     * So resetChanged is called with "false" for all views except the last one,
     * and with "true" for the last one, so that it resets the flag.
     *
     * Framesets shouldn't reimplement this one in theory [but KWTableFrameSet has to].
     */
    virtual void drawContents( QPainter *painter, const QRect &crect,
                               QColorGroup &cg, bool onlyChanged, bool resetChanged,
                               KWFrameSetEdit *edit, KWViewMode *viewMode, KWCanvas *canvas );

    // This is used (e.g. by KWTextParag) to get the view settings
    KWCanvas * currentDrawnCanvas() const { return m_currentDrawnCanvas; }

    /**
     * Draw a particular frame of this frameset.
     * This is called by drawContents and is what framesets must reimplement.
     * @param crect rectangle to be repainted, in the _frame_'s coordinate system (in pixels).
     */
    virtual void drawFrame( KWFrame *frame, QPainter *painter, const QRect &crect,
                            QColorGroup &cg, bool onlyChanged, bool resetChanged,
                            KWFrameSetEdit *edit ) = 0;

    /**
     * Called when our frames change, or when another frameset's frames change.
     * Framesets can reimplement it, but should always call the parent method.
     */
    virtual void updateFrames();

    /** Return list of frames in page @p pageNum.
     * This is fast since it uses the m_framesInPage array.*/
    const QPtrList<KWFrame> & framesInPage( int pageNum ) const;

    /** relayout text in frames, so that it flows correctly around other frames */
    virtual void layout() {}
    virtual void invalidate() {}

    /** returns true if we have a frame occupying that position */
    virtual bool contains( double mx, double my );

    virtual bool getMouseCursor( const QPoint &nPoint, bool controlPressed, QCursor & cursor );

    /** Show a popup menu - called when right-clicking inside a frame of this frameset.
     * The default implementation shows "frame_popup".
     * @param frame the frame which was clicked. Always one of ours.
     * @param edit the current edit object. Either 0L or our own edit object (usually).
     * @param view the view - we use it to get the popupmenu by name
     * @param point the mouse position (at which to show the menu)
     */
    virtual void showPopup( KWFrame *frame, KWFrameSetEdit *edit, KWView *view, const QPoint &point );

    /** save to XML - when saving */
    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true ) = 0;
    /** save to XML - when copying to clipboard */
    virtual QDomElement toXML( QDomElement &parentElem, bool saveFrames = true )
    { return save( parentElem, saveFrames ); }

    /** load from XML - when loading */
    virtual void load( QDomElement &framesetElem, bool loadFrames = true );
    /** load from XML - when pasting from clipboard */
    virtual void fromXML( QDomElement &framesetElem, bool loadFrames = true, bool /*useNames*/ = true )
    { load( framesetElem, loadFrames ); }

    /** Apply the new zoom/resolution - values are to be taken from kWordDocument() */
    virtual void zoom( bool forPrint );

    //virtual void preparePrinting( QPainter *, QProgressDialog *, int & ) { }

    /** Called once the frameset has been completely loaded or constructed.
     * The default implementation calls updateFrames() and zoom(). Call the parent :) */
    virtual void finalize();

    virtual int paragraphs() { return 0; }
    virtual int paragraphsSelected() { return 0; }
    virtual bool statistics( QProgressDialog */*progress*/,  ulong & /*charsWithSpace*/, ulong & /*charsWithoutSpace*/, ulong & /*words*/,
        ulong & /*sentences*/, ulong & /*syllables*/, bool /*process only selected */ ) { return true; }

    KWDocument* kWordDocument() const { return m_doc; }

    // Return true if page @p num can be removed, as far as this frameset is concerned
    virtual bool canRemovePage( int num );

    // only used for headers and footers...
    void setCurrent( int i ) { m_current = i; }
    int getCurrent()const { return m_current; }

    //Note: none of those floating-frameset methods creates undo/redo
    //They are _called_ by the undo/redo commands.

    /** Make this frameset floating, as close to its current position as possible. */
    void setFloating();
    /** Make this frameset floating, with the anchor at @p paragId,@p index in the text frameset @p textfs */
    void setAnchored( KWTextFrameSet* textfs, int paragId, int index, bool placeHolderExists = false );
    /** Note that this frameset has been made floating already, and store anchor position */
    void setAnchored( KWTextFrameSet* textfs );
    /** Make this frameset fixed, i.e. not anchored */
    void setFixed();
    /** Return true if this frameset is floating, false if it's fixed */
    bool isFloating() const { return m_anchorTextFs; }
    /** Return the frameset in which our anchor is - assuming isFloating() */
    KWTextFrameSet * anchorFrameset() const { return m_anchorTextFs; }
    /** Return the anchor object for this frame number */
    KWAnchor * findAnchor( int frameNum );

    /** Create an anchor for the floating frame identified by frameNum */
    virtual KWAnchor * createAnchor( KoTextDocument *txt, int frameNum );

    /** Move the frame frameNum to the given position - this is called when
        the frame is anchored and the anchor moves (see KWAnchor). */
    virtual void moveFloatingFrame( int frameNum, const KoPoint &position );
    /** Get the [zoomed] size of the "floating frame" identified by frameNum.
        By default a real frame but not for tables. */
    virtual QSize floatingFrameSize( int frameNum );
    /** Get the 'baseline' to use for the "floating frame" identified by frameNum.
        -1 means same as the height (usual case) */
    virtual int floatingFrameBaseline( int /*frameNum*/ ) { return -1; }
    /** Store command for creating an anchored object */
    virtual KCommand * anchoredObjectCreateCommand( int frameNum );
    /** Store command for deleting an anchored object */
    virtual KCommand * anchoredObjectDeleteCommand( int frameNum );

    /** make this frameset part of a groupmanager
     * @see KWTableFrameSet
     */
    void setGroupManager( KWTableFrameSet *gm ) { grpMgr = gm; }
    /** returns the groupManager this frameset belongs to. A Groupmanager is better known as a table */
    KWTableFrameSet *getGroupManager()const { return grpMgr; }

    /** table headers can created by the groupmanager, we store the fact that
     this is one in here. */
    void setIsRemoveableHeader( bool h ) { m_removeableHeader = h; }
    bool isRemoveableHeader()const { return m_removeableHeader; }

    /** returns if one of our frames has been selected. */
    bool hasSelectedFrame();

    /**
     * Returns true if the frameset is visible.
     * A frameset is visible if setVisible(false) wasn't called,
     * but also, for a header frameset, if m_doc->isHeaderVisible is true, etc.
     * For an "even pages header" frameset, the corresponding headerType setting
     * must be selected (i.e. different headers for even and odd pages).
     * @param viewMode pass the current viewmode when using this method for any visual
     * stuff (drawing, handling input etc.). Frameset visibility depends on the viewmode.
     */
    bool isVisible( KWViewMode* viewMode = 0L ) const;

    /** set the visibility of the frameset. */
    virtual void setVisible( bool v );

    /** get/set frameset name. For tables in particular, this _must_ be unique */
    QString getName() const { return m_name; }
    void setName( const QString &_name ) { m_name = _name; }

    /** set frameBehavior on all frames, see KWFrame for specifics */
    void setFrameBehavior( KWFrame::FrameBehavior fb );

    /** set newFrameBehavior on all frames, see KWFrame for specifics */
    void setNewFrameBehavior( KWFrame::NewFrameBehavior nfb );

#ifndef NDEBUG
    virtual void printDebug();
    virtual void printDebug( KWFrame * );
#endif

signals:

    // Emitted when something has changed in this frameset,
    // so that all views repaint it. KWDocument connects to it,
    // and KWCanvas connects to KWDocument.
    void repaintChanged( KWFrameSet * frameset );

protected:

    /** save the common attributes for the frameset */
    void saveCommon( QDomElement &parentElem, bool saveFrames );

    /**Determine the clipping rectangle for drawing the contents of @p frame with @p painter
     * in the rectangle delimited by @p crect.
     * This determines where to clip the painter to draw the contents of a given frame
     * It clips to the frame if clipFrame=true, and clips out any "on top" frame if onlyChanged=true.
     */
    QRegion frameClipRegion( QPainter * painter, KWFrame *frame, const QRect & crect,
                             KWViewMode * viewMode, bool onlyChanged, bool clipFrame = true );

    void deleteAnchor( KWAnchor * anchor );
    virtual void deleteAnchors();
    virtual void createAnchors( KWTextParag * parag, int index, bool placeHolderExists = false );

    KWDocument *m_doc;            // Document
    QPtrList<KWFrame> frames;        // Our frames

    // Cached info for optimization
    /** This array provides a direct access to the frames on page N */
    QPtrVector< QPtrList<KWFrame> > m_framesInPage;
    /** always equal to m_framesInPage[0].first()->pageNum() :) */
    int m_firstPage;
    /** always empty, for convenience in @ref framesInPage */
    QPtrList<KWFrame> m_emptyList; // ## make static pointer to save memory ?

    Info m_info;
    int m_current; // used for headers and footers, not too sure what it means
    KWTableFrameSet *grpMgr;
    bool m_removeableHeader, m_visible;
    QString m_name;
    KWTextFrameSet * m_anchorTextFs;
    KWCanvas * m_currentDrawnCanvas;           // The canvas currently being drawn.
    KWordFrameSetIface *m_dcop;
};

/******************************************************************/
/* Class: KWPictureFrameSet                                       */
/******************************************************************/
class KWPictureFrameSet : public KWFrameSet
{
public:
    KWPictureFrameSet( KWDocument *_doc, const QString & name );
    virtual ~KWPictureFrameSet();

    virtual KWordFrameSetIface* dcopObject();

    /** The type of frameset. Use this to differentiate between different instantiations of
     *  the framesets. Each implementation will return a different frameType.
     */
    virtual FrameSetType type() { return FT_PICTURE; }

    void setImage( const KoPicture &image ) { m_image = image; }
    KoPicture image() const { return m_image; }

    KoPictureKey key() const { return m_image.getKey(); }

    void loadImage( const QString &fileName, const QSize &_imgSize );
    void setSize( const QSize & _imgSize );

    /** Called when the user resizes a frame. */
    virtual void resizeFrame( KWFrame* frame, double newWidth, double newHeight, bool finalSize );

    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true );
    virtual void load( QDomElement &attributes, bool loadFrames = true );

    virtual void drawFrame( KWFrame *frame, QPainter *painter, const QRect & crect,
                            QColorGroup &, bool onlyChanged, bool resetChanged,
                            KWFrameSetEdit *edit = 0L );

    // Pixmaps can be transparent
    virtual void createEmptyRegion( const QRect &, QRegion &, KWViewMode * ) { }

    virtual bool isFrameAtPos( KWFrame* frame, const QPoint& nPoint, bool borderOfFrameOnly=false );
#ifndef NDEBUG
    virtual void printDebug( KWFrame* frame );
#endif

    bool keepAspectRatio() const { return m_keepAspectRatio; }
    void setKeepAspectRatio( bool b ) { m_keepAspectRatio = b; }
protected:
    KoPicture m_image;
    bool m_keepAspectRatio;
};

/******************************************************************/
/* Class: KWClipartFrameSet                                       */
/******************************************************************/
class KWClipartFrameSet : public KWFrameSet
{
public:
    KWClipartFrameSet( KWDocument *_doc, const QString & name );
    virtual ~KWClipartFrameSet() {}

    /** The type of frameset. Use this to differentiate between different instantiations of
     *  the framesets. Each implementation will return a different frameType.
     */
    virtual FrameSetType type() { return FT_CLIPART; }

    void setClipart( const KoPicture &clipart ) { m_clipart = clipart; }
    KoPicture clipart() const { return m_clipart; }

    KoPictureKey key() const { return m_clipart.getKey(); }

    void loadClipart( const QString &fileName );
    //void setSize( const QSize & _imgSize );

    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true );
    virtual void load( QDomElement &attributes, bool loadFrames = true );

    virtual void drawFrame( KWFrame *frame, QPainter *painter, const QRect & crect,
                            QColorGroup &, bool onlyChanged, bool resetChanged,
                            KWFrameSetEdit *edit = 0L );

    // Cliparts can be transparent
    virtual void createEmptyRegion( const QRect &, QRegion &, KWViewMode * ) { }

    virtual bool isFrameAtPos( KWFrame* frame, const QPoint& nPoint, bool borderOfFrameOnly=false );
protected:
    KoPicture m_clipart;
};

/******************************************************************/
/* Class: KWPartFrameSet                                          */
/******************************************************************/

class KWPartFrameSet : public KWFrameSet
{
    Q_OBJECT
public:
    KWPartFrameSet( KWDocument *_doc, KWChild *_child, const QString & name );
    virtual ~KWPartFrameSet();

    virtual KWordFrameSetIface* dcopObject();


    /** The type of frameset. Use this to differentiate between different instantiations of
     *  the framesets. Each implementation will return a different frameType.
     */
    virtual FrameSetType type() { return FT_PART; }

    virtual KWFrameSetEdit * createFrameSetEdit( KWCanvas * );

    KWChild *getChild() { return m_child; }

    void updateChildGeometry();
    virtual void updateFrames();

    virtual void drawFrame( KWFrame * frame, QPainter * p, const QRect & crect,
                            QColorGroup &, bool onlyChanged, bool resetChanged,
                            KWFrameSetEdit *edit = 0L );

    // Embedded parts can be transparent
    virtual void createEmptyRegion( const QRect &, QRegion &, KWViewMode * ) { }

    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true );
    virtual void load( QDomElement &attributes, bool loadFrames = true );

protected:
    KWChild *m_child;
};

class KWPartFrameSetEdit :  public QObject, public KWFrameSetEdit
{
    Q_OBJECT
public:
    KWPartFrameSetEdit( KWPartFrameSet * fs, KWCanvas * canvas );
    virtual ~KWPartFrameSetEdit();

    virtual DCOPObject* dcopObject();


    KWPartFrameSet * partFrameSet() const
    {
        return static_cast<KWPartFrameSet*>(frameSet());
    }

    // Events forwarded by the canvas (when being in "edit" mode)
    virtual void mousePressEvent( QMouseEvent *, const QPoint &, const KoPoint & );
    virtual void mouseDoubleClickEvent( QMouseEvent *, const QPoint &, const KoPoint & );

private:
    KWFramePartMoveCommand *m_cmdMoveChild;
    DCOPObject *m_dcop;

protected slots:
    void slotChildChanged();
    void slotChildActivated(bool);
};

/******************************************************************/
/* Class: KWFormulaFrameSet                                       */
/******************************************************************/

class KWFormulaFrameSet : public KWFrameSet
{
    Q_OBJECT
public:
    KWFormulaFrameSet( KWDocument *_doc, const QString & name );
    virtual ~KWFormulaFrameSet();

    virtual KWordFrameSetIface* dcopObject();

    /** The type of frameset. Use this to differentiate between different instantiations of
     *  the framesets. Each implementation will return a different frameType.
     */
    virtual FrameSetType type() { return FT_FORMULA; }
    virtual void addFrame( KWFrame *_frame, bool recalc = true );

    virtual KWFrameSetEdit* createFrameSetEdit(KWCanvas*);

    /**
     * Paint this frameset
     */
    virtual void drawFrame(KWFrame *, QPainter*, const QRect&,
                           QColorGroup&, bool onlyChanged, bool resetChanged,
                           KWFrameSetEdit *edit = 0L);

    virtual void updateFrames();

    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true );
    virtual void load( QDomElement &attributes, bool loadFrames = true );
    void paste( QDomNode& formulaElem );

    /** Apply the new zoom/resolution - values are to be taken from kWordDocument() */
    virtual void zoom( bool forPrint );

    KFormula::Container* getFormula() const { return formula; }

    void setChanged() { m_changed = true; }

    virtual int floatingFrameBaseline( int /*frameNum*/ );

    void showPopup( KWFrame *, KWFrameSetEdit *, KWView *view, const QPoint &point );

protected slots:

    void slotFormulaChanged( double width, double height );

private:
    KFormula::Container* formula;
    bool m_changed;
};


class KWFormulaFrameSetEdit : public QObject, public KWFrameSetEdit
{
    Q_OBJECT
public:
    KWFormulaFrameSetEdit(KWFormulaFrameSet* fs, KWCanvas* canvas);
    virtual ~KWFormulaFrameSetEdit();

    KWFormulaFrameSet* formulaFrameSet() const
    {
        return static_cast<KWFormulaFrameSet*>(frameSet());
    }

    const KFormula::View* getFormulaView() const;
    KFormula::View* getFormulaView();

    virtual DCOPObject* dcopObject();

    // Events forwarded by the canvas (when being in "edit" mode)
    virtual void keyPressEvent(QKeyEvent*);
    virtual void mousePressEvent(QMouseEvent*, const QPoint & n, const KoPoint & d );
    virtual void mouseMoveEvent(QMouseEvent*, const QPoint & n, const KoPoint & d); // only called if button is pressed
    virtual void mouseReleaseEvent(QMouseEvent*, const QPoint & n, const KoPoint & d);
    //virtual void mouseDoubleClickEvent( QMouseEvent *, const QPoint & n, const KoPoint & d ) {}
    //virtual void dragEnterEvent( QDragEnterEvent * ) {}
    //virtual void dragMoveEvent( QDragMoveEvent *, const QPoint &, const KoPoint & ) {}
    //virtual void dragLeaveEvent( QDragLeaveEvent * ) {}
    //virtual void dropEvent( QDropEvent *, const QPoint &, const KoPoint & ) {}
    virtual void focusInEvent();
    virtual void focusOutEvent();
    virtual void copy();
    virtual void cut();
    virtual void paste();
    virtual void selectAll();

    /** Moves the cursor to the first position */
    void moveHome();
    /** Moves the cursor to the last position */
    void moveEnd();

    /** Gets called if the cursor ties to leave the formula at its begin. */
    void exitLeft();

    /** Gets called if the cursor ties to leave the formula at its end. */
    void exitRight();

    void removeFormula();

protected slots:

    /**
     * Make sure the cursor can be seen at its new position.
     */
    void cursorChanged( bool visible, bool selecting );

private:
    FormulaView* formulaView;
    DCOPObject *dcop;
};

#endif
