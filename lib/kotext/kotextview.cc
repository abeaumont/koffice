/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include <qtimer.h>
#include <qclipboard.h>
#include "kotextview.h"
#include "koparagcounter.h"
#include "kotextobject.h"
#include <klocale.h>
#include <kstdaccel.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kdatatool.h>
#include <krun.h>
#include <kmessagebox.h>

struct KoTextView::KoTextViewPrivate
{
    KoTextViewPrivate()
    {
        m_currentUnicodeNumber = 0;
    }
    int m_currentUnicodeNumber; // For the alt+0123 feature
};

KoTextView::KoTextView( KoTextObject *textobj )
{
    d = new KoTextViewPrivate;
    m_bReadWrite = true;
    m_textobj = textobj;

    connect( m_textobj, SIGNAL( hideCursor() ), this, SLOT( hideCursor() ) );
    connect( m_textobj, SIGNAL( showCursor() ), this, SLOT( showCursor() ) );
    connect( m_textobj, SIGNAL( setCursor( QTextCursor * ) ), this, SLOT( setCursor( QTextCursor * ) ) );
    connect( m_textobj, SIGNAL( updateUI(bool, bool) ), this, SLOT( updateUI(bool, bool) ) );
    connect( m_textobj, SIGNAL( showCurrentFormat() ), this, SLOT( showCurrentFormat() ) );
    connect( m_textobj, SIGNAL( ensureCursorVisible() ), this, SLOT( ensureCursorVisible() ) );

    m_cursor = new QTextCursor( m_textobj->textDocument() );

    m_cursorVisible = false;

    showCursor();
    blinkTimer = new QTimer( this );
    connect( blinkTimer, SIGNAL( timeout() ),
             this, SLOT( blinkCursor() ) );
    blinkTimer->start( QApplication::cursorFlashTime() / 2 );

    dragStartTimer = new QTimer( this );
    connect( dragStartTimer, SIGNAL( timeout() ),
             this, SLOT( startDrag() ) );

    m_textobj->formatMore();

    blinkCursorVisible = FALSE;
    inDoubleClick = FALSE;
    mightStartDrag = FALSE;
    possibleTripleClick = FALSE;
    afterTripleClick = FALSE;
    m_currentFormat = 0;
    //updateUI( true, true );
}

KoTextView::~KoTextView()
{
    delete m_cursor;
    delete d;
}

void KoTextView::terminate(bool removeselection)
{
    textObject()->clearUndoRedoInfo();
    if ( removeselection && textDocument()->removeSelection( QTextDocument::Standard ) )
        textObject()->selectionChangedNotify();
    hideCursor();
}

void KoTextView::deleteWordForward()
{
    if ( textDocument()->hasSelection( QTextDocument::Standard ) ) {
        textObject()->removeSelectedText( m_cursor );
        return;
    }
    textDocument()->setSelectionStart( QTextDocument::Standard, m_cursor );

    do {
        m_cursor->gotoRight();
    } while ( !m_cursor->atParagEnd()
              && !m_cursor->parag()->at( m_cursor->index() )->c.isSpace() );
    textDocument()->setSelectionEnd( QTextDocument::Standard, m_cursor );
    textObject()->removeSelectedText( m_cursor, QTextDocument::Standard, i18n("Remove word") );
}

void KoTextView::deleteWordBack()
{
    if ( textDocument()->hasSelection( QTextDocument::Standard ) ) {
        textObject()->removeSelectedText( m_cursor );
        return;
    }
    textDocument()->setSelectionStart( QTextDocument::Standard, m_cursor );

    do {
        m_cursor->gotoLeft();
    } while ( !m_cursor->atParagStart()
              && !m_cursor->parag()->at( m_cursor->index()-1 )->c.isSpace() );
    textDocument()->setSelectionEnd( QTextDocument::Standard, m_cursor );
    textObject()->removeSelectedText( m_cursor, QTextDocument::Standard, i18n("Remove word") );
}

void KoTextView::handleKeyPressEvent( QKeyEvent * e )
{
    textObject()->typingStarted();

    /* bool selChanged = FALSE;
    for ( int i = 1; i < textDocument()->numSelections(); ++i )
        selChanged = textDocument()->removeSelection( i ) || selChanged;

    if ( selChanged ) {
        // m_cursor->parag()->document()->nextDoubleBuffered = TRUE; ######## we need that only if we have nested items/documents
        textFrameSet()->selectionChangedNotify();
    }*/

    bool clearUndoRedoInfo = TRUE;

    if ( KStdAccel::isEqual( e, KStdAccel::deleteWordBack() ) )
    {
        deleteWordBack();
        clearUndoRedoInfo = TRUE;
    } else if ( KStdAccel::isEqual( e, KStdAccel::deleteWordForward() ) )
    {
        deleteWordForward();
        clearUndoRedoInfo = TRUE;
    }
    else
    switch ( e->key() ) {
    case Key_Left:
        moveCursor( e->state() & ControlButton ? MoveWordBackward : MoveBackward, e->state() & ShiftButton );
        break;
    case Key_Right:
        moveCursor( e->state() & ControlButton ? MoveWordForward : MoveForward, e->state() & ShiftButton );
        break;
    case Key_Up:
        moveCursor( e->state() & ControlButton ? MoveParagUp : MoveUp, e->state() & ShiftButton );
        break;
    case Key_Down:
        moveCursor( e->state() & ControlButton ? MoveParagDown : MoveDown, e->state() & ShiftButton );
        break;
    case Key_Home:
        moveCursor( e->state() & ControlButton ? MoveHome : MoveLineStart, e->state() & ShiftButton );
        break;
    case Key_End:
        moveCursor( e->state() & ControlButton ? MoveEnd : MoveLineEnd, e->state() & ShiftButton );
        break;
    case Key_Prior:
        moveCursor( e->state() & ControlButton ? MovePgUp : MoveViewportUp, e->state() & ShiftButton );
        break;
    case Key_Next:
        moveCursor( e->state() & ControlButton ? MovePgDown : MoveViewportDown, e->state() & ShiftButton );
        break;
    case Key_Return: case Key_Enter:
        if ( textDocument()->hasSelection( QTextDocument::Standard ) )
            textObject()->removeSelectedText( m_cursor );
        clearUndoRedoInfo = FALSE;
        textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionReturn );
        Q_ASSERT( m_cursor->parag()->prev() );
        if ( m_cursor->parag()->prev() )
            doAutoFormat( m_cursor, static_cast<KoTextParag*>(m_cursor->parag()->prev()),
                          m_cursor->parag()->prev()->length() - 1, '\n' );
        break;
    case Key_Delete:
        if ( textDocument()->hasSelection( QTextDocument::Standard ) ) {
            textObject()->removeSelectedText( m_cursor );
            break;
        }

        textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionDelete );

        clearUndoRedoInfo = FALSE;
        break;
    case Key_Backspace:
        if ( textDocument()->hasSelection( QTextDocument::Standard ) ) {
            textObject()->removeSelectedText( m_cursor );
            break;
        }
        if ( !m_cursor->parag()->prev() &&
             m_cursor->atParagStart() )
        {
            KoTextParag * parag = static_cast<KoTextParag *>(m_cursor->parag());
            if ( parag->counter() && parag->counter()->style() != KoParagCounter::STYLE_NONE)
                textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionBackspace );
            break;
        }
        textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionBackspace );

        clearUndoRedoInfo = FALSE;
        break;
    case Key_F16: // Copy key on Sun keyboards
        emit copy();
        break;
    case Key_F18:  // Paste key on Sun keyboards
        emit paste();
        break;
    case Key_F20:  // Cut key on Sun keyboards
        emit cut();
        break;
    default: {
            //kdDebug() << "KoTextView::keyPressEvent ascii=" << e->ascii() << " text=" << e->text()[0].unicode() << " state=" << e->state() << endl;
            if ( e->text().length() &&
//               !( e->state() & AltButton ) &&
                 ( !e->ascii() || e->ascii() >= 32 ) ||
                 ( e->text() == "\t" && !( e->state() & ControlButton ) ) ) {
                clearUndoRedoInfo = FALSE;
                if ( e->key() == Key_Tab ) {
                    // We don't have support for nested counters at the moment.
                    /*if ( m_cursor->index() == 0 && m_cursor->parag()->style() &&
                         m_cursor->parag()->style()->displayMode() == QStyleSheetItem::DisplayListItem ) {
                        static_cast<KWTextParag * >(m_cursor->parag())->incDepth();
                        emit hideCursor();
                        emit repaintChanged();
                        emit showCursor();
                        break;
                    }*/
                }
                // Port to setCounter if we really want that - and make configurable
                /*if ( m_cursor->parag()->style() &&
                     m_cursor->parag()->style()->displayMode() == QStyleSheetItem::DisplayBlock &&
                     m_cursor->index() == 0 && ( e->text() == "-" || e->text() == "*" ) ) {
                    setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListDisc );
                    break;
                }*/
                QString text = e->text();

                insertText( text );

                doAutoFormat( m_cursor, static_cast<KoTextParag*>(m_cursor->parag()),
                              m_cursor->index() - 1, text[ text.length() - 1 ] );
                break;
            }
            // We should use KAccel instead, to make this configurable !
            // Well, those are all alternate keys, for keys already configurable (KDE-wide)
            // and a kaccel makes it hard to
            if ( e->state() & ControlButton ) {
                switch ( e->key() ) {
                case Key_F16: // Copy key on Sun keyboards
                    copy();
                    break;
                case Key_A:
                    moveCursor( MoveLineStart, e->state() & ShiftButton );
                    break;
                case Key_E:
                    moveCursor( MoveLineEnd, e->state() & ShiftButton );
                    break;
                case Key_K:
                    textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionKill );
                    break;
                case Key_Insert:
                    copy();
                    break;
                }
                break;
            }
        }
    }

    if ( clearUndoRedoInfo ) {
        textObject()->clearUndoRedoInfo();
    }
    textObject()->typingDone();
}

void KoTextView::insertText( const QString &text )
{
    textObject()->insert( m_cursor, m_currentFormat, text, false, true, i18n("Insert Text") );
}

void KoTextView::moveCursor( CursorAction action, bool select )
{
    hideCursor();
    if ( select ) {
        if ( !textDocument()->hasSelection( QTextDocument::Standard ) )
            textDocument()->setSelectionStart( QTextDocument::Standard, m_cursor );
        moveCursor( action );
        if ( textDocument()->setSelectionEnd( QTextDocument::Standard, m_cursor ) ) {
            //      m_cursor->parag()->document()->nextDoubleBuffered = TRUE; ##### we need that only if we have nested items/documents
            textObject()->selectionChangedNotify();
        } else {
            showCursor();
        }
    } else {
        bool redraw = textDocument()->removeSelection( QTextDocument::Standard );
        moveCursor( action );
        if ( redraw ) {
            //m_cursor->parag()->document()->nextDoubleBuffered = TRUE; // we need that only if we have nested items/documents
            textObject()->selectionChangedNotify();
        }
    }

    ensureCursorVisible();
    showCursor();
    updateUI( true );
}

void KoTextView::moveCursor( CursorAction action )
{
    switch ( action ) {
        case MoveBackward:
            m_cursor->gotoLeft();
            break;
        case MoveWordBackward:
            m_cursor->gotoWordLeft();
            break;
        case MoveForward:
            m_cursor->gotoRight();
            break;
        case MoveWordForward:
            m_cursor->gotoWordRight();
            break;
        case MoveUp:
            m_cursor->gotoUp();
            break;
        case MoveDown:
            m_cursor->gotoDown();
            break;
        case MoveViewportUp:
            pgUpKeyPressed();
            break;
        case MoveViewportDown:
            pgDownKeyPressed();
            break;
        case MovePgUp:
            ctrlPgUpKeyPressed();
            break;
        case MovePgDown:
            ctrlPgDownKeyPressed();
            break;
        case MoveLineStart:
            m_cursor->gotoLineStart();
            break;
        case MoveHome:
            m_cursor->gotoHome();
            break;
        case MoveLineEnd:
            m_cursor->gotoLineEnd();
            break;
        case MoveEnd:
            textObject()->ensureFormatted( textDocument()->lastParag() );
            m_cursor->gotoEnd();
            break;
        case MoveParagUp: {
            Qt3::QTextParag * parag = m_cursor->parag()->prev();
            if ( parag )
            {
                m_cursor->setParag( parag );
                m_cursor->setIndex( 0 );
            }
        } break;
        case MoveParagDown: {
            Qt3::QTextParag * parag = m_cursor->parag()->next();
            if ( parag )
            {
                m_cursor->setParag( parag );
                m_cursor->setIndex( 0 );
            }
        } break;
    }

    updateUI( true );
}

QTextCursor KoTextView::selectWordUnderCursor()
{
    QTextCursor c1 = *m_cursor;
    QTextCursor c2 = *m_cursor;
    if ( m_cursor->index() > 0 && !m_cursor->parag()->at( m_cursor->index()-1 )->c.isSpace() && !m_cursor->parag()->at( m_cursor->index()-1 )->isCustom())
        c1.gotoWordLeft();
    if ( !m_cursor->parag()->at( m_cursor->index() )->c.isSpace() && !m_cursor->atParagEnd() && !m_cursor->parag()->at( m_cursor->index() )->isCustom())
        c2.gotoWordRight();

    textDocument()->setSelectionStart( QTextDocument::Standard, &c1 );
    textDocument()->setSelectionEnd( QTextDocument::Standard, &c2 );
    return c2;
}

QTextCursor KoTextView::selectParagUnderCursor()
{
    QTextCursor c1 = *m_cursor;
    QTextCursor c2 = *m_cursor;
    c1.setIndex(0);
    c2.setIndex(c1.parag()->string()->length() - 1);
    textDocument()->setSelectionStart( QTextDocument::Standard, &c1 );
    textDocument()->setSelectionEnd( QTextDocument::Standard, &c2 );
    return c2;
}

void KoTextView::handleMousePressEvent( QMouseEvent *e, const QPoint &iPoint )
{
    mightStartDrag = FALSE;
    hideCursor();

    if (possibleTripleClick)
    {
        handleMouseTripleClickEvent( e, iPoint );
        return;
    }

    QTextCursor oldCursor = *m_cursor;
    placeCursor( iPoint );
    ensureCursorVisible();

    if ( e->button() != LeftButton )
    {
        showCursor();
        return;
    }

    QTextDocument * textdoc = textDocument();
    if ( textdoc->inSelection( QTextDocument::Standard, iPoint ) ) {
        mightStartDrag = TRUE;
        m_textobj->emitShowCursor();
        dragStartTimer->start( QApplication::startDragTime(), TRUE );
        dragStartPos = e->pos();
        return;
    }

    bool redraw = FALSE;
    if ( textdoc->hasSelection( QTextDocument::Standard ) ) {
        if ( !( e->state() & ShiftButton ) ) {
            redraw = textdoc->removeSelection( QTextDocument::Standard );
            textdoc->setSelectionStart( QTextDocument::Standard, m_cursor );
        } else {
            redraw = textdoc->setSelectionEnd( QTextDocument::Standard, m_cursor ) || redraw;
        }
    } else {
        if ( !( e->state() & ShiftButton ) ) {
            textdoc->setSelectionStart( QTextDocument::Standard, m_cursor );
        } else {
            textdoc->setSelectionStart( QTextDocument::Standard, &oldCursor );
            redraw = textdoc->setSelectionEnd( QTextDocument::Standard, m_cursor ) || redraw;
        }
    }

    //for ( int i = 1; i < textdoc->numSelections(); ++i )
    //    redraw = textdoc->removeSelection( i ) || redraw;

    //kdDebug() << "KoTextView::mousePressEvent redraw=" << redraw << endl;
    if ( !redraw ) {
        showCursor();
    } else {
        textObject()->selectionChangedNotify();
    }
}

void KoTextView::handleMouseMoveEvent( QMouseEvent*, const QPoint& iPoint )
{
    hideCursor();
    QTextCursor oldCursor = *m_cursor;
    placeCursor( iPoint );

    // Double click + mouse still down + moving the mouse selects full words.
    if ( inDoubleClick ) {
        QTextCursor cl = *m_cursor;
        cl.gotoWordLeft();
        QTextCursor cr = *m_cursor;
        cr.gotoWordRight();

        int diff = QABS( oldCursor.parag()->at( oldCursor.index() )->x - iPoint.x() );
        int ldiff = QABS( cl.parag()->at( cl.index() )->x - iPoint.x() );
        int rdiff = QABS( cr.parag()->at( cr.index() )->x - iPoint.x() );

        if ( m_cursor->parag()->lineStartOfChar( m_cursor->index() ) !=
             oldCursor.parag()->lineStartOfChar( oldCursor.index() ) )
            diff = 0xFFFFFF;

        if ( rdiff < diff && rdiff < ldiff )
            *m_cursor = cr;
        else if ( ldiff < diff && ldiff < rdiff )
            *m_cursor = cl;
        else
            *m_cursor = oldCursor;

    }

    bool redraw = FALSE;
    if ( textDocument()->hasSelection( QTextDocument::Standard ) )
        redraw = textDocument()->setSelectionEnd( QTextDocument::Standard, m_cursor ) || redraw;
    else // it may be that the initial click was out of the frame
        textDocument()->setSelectionStart( QTextDocument::Standard, m_cursor );

    if ( redraw )
        textObject()->selectionChangedNotify( false );

    showCursor();
}

void KoTextView::handleMouseReleaseEvent()
{
    if ( dragStartTimer->isActive() )
        dragStartTimer->stop();
    if ( mightStartDrag ) {
        textObject()->selectAll( FALSE );
        mightStartDrag = false;
    }
    else
    {
        if ( textDocument()->selectionStartCursor( QTextDocument::Standard ) == textDocument()->selectionEndCursor( QTextDocument::Standard ) )
            textDocument()->removeSelection( QTextDocument::Standard );

        textObject()->selectionChangedNotify();

        // Copy the selection.
        QApplication::clipboard()->setSelectionMode( true );
        emit copy();
        QApplication::clipboard()->setSelectionMode( false );
    }

    inDoubleClick = FALSE;
    m_textobj->emitShowCursor();
}

void KoTextView::handleMouseDoubleClickEvent( QMouseEvent*ev, const QPoint& i/* Currently unused */ )
{
  //after a triple click it's not a double click but a simple click
  //but as triple click didn't exist it's necessary to do it.
    if(afterTripleClick)
      {
	handleMousePressEvent( ev, i );
	return;
      }

    inDoubleClick = TRUE;
    *m_cursor = selectWordUnderCursor();
    textObject()->selectionChangedNotify();
    // No auto-copy, will readd with Qt 3 using setSelectionMode(true/false)
    // But auto-copy in readonly mode, since there is no action available in that case.
    if ( !m_bReadWrite )
        emit copy();

    possibleTripleClick=true;

    QTimer::singleShot(QApplication::doubleClickInterval(),this,SLOT(tripleClickTimeout()));

}

void KoTextView::tripleClickTimeout()
{
   possibleTripleClick=false;
}

void KoTextView::handleMouseTripleClickEvent( QMouseEvent*ev, const QPoint& /* Currently unused */ )
{
    if ( ev->button() != LeftButton)
    {
        showCursor();
        return;
    }
    afterTripleClick= true;
    inDoubleClick = FALSE;
    *m_cursor = selectParagUnderCursor();
    textObject()->selectionChangedNotify();
    // No auto-copy, will readd with Qt 3 using setSelectionMode(true/false)
    // But auto-copy in readonly mode, since there is no action available in that case.
    if ( !m_bReadWrite )
        emit copy();
    QTimer::singleShot(QApplication::doubleClickInterval(),this,SLOT(afterTripleClickTimeout()));

}
void KoTextView::afterTripleClickTimeout()
{
  afterTripleClick=false;
}


bool KoTextView::maybeStartDrag( QMouseEvent* e )
{
    if ( mightStartDrag ) {
        dragStartTimer->stop();
        if ( ( e->pos() - dragStartPos ).manhattanLength() > QApplication::startDragDistance() )
            startDrag();
        return true;
    }
    return false;
}

void KoTextView::placeCursor( const QPoint &pos )
{
    m_cursor->restoreState();
    Qt3::QTextParag *s = textDocument()->firstParag();
    m_cursor->place( pos,  s );
    updateUI( true );
}

void KoTextView::blinkCursor()
{
    //kdDebug() << "KoTextView::blinkCursor m_cursorVisible=" << m_cursorVisible
    //          << " blinkCursorVisible=" << blinkCursorVisible << endl;
    if ( !m_cursorVisible )
        return;
    bool cv = m_cursorVisible;
    blinkCursorVisible = !blinkCursorVisible;
    drawCursor( blinkCursorVisible );
    m_cursorVisible = cv;
}

void KoTextView::drawCursor( bool visible )
{
    m_cursorVisible = visible;
    // The rest is up to the app ;)
}

void KoTextView::focusInEvent()
{
    blinkTimer->start( QApplication::cursorFlashTime() / 2 );
    showCursor();
}

void KoTextView::focusOutEvent()
{
    blinkTimer->stop();
    hideCursor();
}

void KoTextView::setFormat( KoTextFormat * newFormat, int flags, bool zoomFont )
{
    textObject()->setFormat( m_cursor, m_currentFormat, newFormat, flags, zoomFont );
}

void KoTextView::dragStarted()
{
    mightStartDrag = FALSE;
    inDoubleClick = FALSE;
}

void KoTextView::applyStyle( const KoStyle * style )
{
    textObject()->applyStyle( m_cursor, style );
    showCurrentFormat();
}

void KoTextView::updateUI( bool updateFormat, bool /*force*/ )
{
    // Update UI - only for those items which have changed

    if ( updateFormat )
    {
        int i = cursor()->index();
        if ( i > 0 )
            --i;
#ifdef DEBUG_FORMATS
        if ( currentFormat() )
            kdDebug(32003) << "KoTextView::updateUI old currentFormat=" << currentFormat()
                           << " " << currentFormat()->key()
                           << " parag format=" << cursor()->parag()->at( i )->format()->key() << endl;
        else
            kdDebug(32003) << "KoTextView::updateUI old currentFormat=0" << endl;
#endif
        if ( !currentFormat() || currentFormat()->key() != cursor()->parag()->at( i )->format()->key() )
        {
            if ( currentFormat() )
                currentFormat()->removeRef();
#ifdef DEBUG_FORMATS
            kdDebug(32003) << "Setting currentFormat from format " << cursor()->parag()->at( i )->format()
                      << " ( character " << i << " in paragraph " << cursor()->parag()->paragId() << " )" << endl;
#endif
            setCurrentFormat( static_cast<KoTextFormat *>( textDocument()->formatCollection()->format( cursor()->parag()->at( i )->format() ) ) );
            if ( currentFormat()->isMisspelled() ) {
                currentFormat()->removeRef();
                // ## this forgets the background color, etc.
                setCurrentFormat( static_cast<KoTextFormat *>( textDocument()->formatCollection()->format( currentFormat()->font(), currentFormat()->color() ) ) );
            }
            showCurrentFormat();
        }
    }
}

void KoTextView::showCurrentFormat()
{
    //kdDebug() << "KoTextView::showCurrentFormat currentFormat=" << currentFormat() << " " << currentFormat()->key() << endl;
    KoTextFormat format = *currentFormat();
    format.setPointSize( textObject()->docFontSize( currentFormat() ) ); // "unzoom" the font size
    showFormat( &format );
}

KCommand * KoTextView::setCounterCommand( const KoParagCounter & counter )
{
     return textObject()->setCounterCommand( m_cursor, counter );
}
KCommand * KoTextView::setAlignCommand( int align )
{
     return textObject()->setAlignCommand( m_cursor, align );
}
KCommand * KoTextView::setLineSpacingCommand( double spacing )
{
     return textObject()->setLineSpacingCommand( m_cursor, spacing );
}
KCommand * KoTextView::setBordersCommand( const KoBorder& leftBorder, const KoBorder& rightBorder, const KoBorder& bottomBorder, const KoBorder& topBorder )
{
    return textObject()->setBordersCommand( m_cursor, leftBorder, rightBorder, bottomBorder, topBorder );
}
KCommand * KoTextView::setMarginCommand( QStyleSheetItem::Margin m, double margin )
{
    return textObject()->setMarginCommand( m_cursor, m, margin );
}
KCommand * KoTextView::setTabListCommand( const KoTabulatorList & tabList )
{
    return textObject()->setTabListCommand( m_cursor, tabList );
}

KoTextDocument * KoTextView::textDocument() const
{
    return textObject()->textDocument();
}

QPtrList<KAction> KoTextView::dataToolActionList(KInstance * instance)
{
    m_singleWord = false;
    m_wordUnderCursor = QString::null;
    m_refLink= QString::null;
    QString text;
    if ( textObject()->hasSelection() )
    {
        text = textObject()->selectedText();
        if ( text.find(' ') == -1 && text.find('\t') == -1 && text.find(KoTextObject::customItemChar()) == -1 )
        {
            m_singleWord = true;
            textObject()->textSelectedIsAnLink(m_refLink);
        }
    }
    else // No selection -> get word under cursor
    {
        selectWordUnderCursor();
        text = textObject()->selectedText();
        if(text.find(KoTextObject::customItemChar()) == -1)
        {
            textObject()->textSelectedIsAnLink(m_refLink);
            textDocument()->removeSelection( QTextDocument::Standard );
            m_singleWord = true;
            m_wordUnderCursor = text;
        }
        else
        {
            text = "";
            m_refLink=QString::null;
        }
    }

#if 0

    // EEEEEK. Please, no KSpell-specific things in here. This is the generic support
    // for any datatool!
    if(!text.isEmpty() && doc->dontCheckTitleCase() && text==text.upper())
    {
        text="";
        m_singleWord = false;
    }
    else if(!text.isEmpty() && doc->dontCheckUpperWord() && text[0]==text[0].upper())
    {
        QString tmp=text[0]+text.right(text.length()-1).lower();
        if(text==tmp)
        {
            text="";
            m_singleWord = false;
        }
    }

#endif

    if ( text.isEmpty() ) // Nothing to apply a tool to
        return QPtrList<KAction>();

    // Any tool that works on plain text is relevant
    QValueList<KDataToolInfo> tools = KDataToolInfo::query( "QString", "text/plain", instance );

    // Add tools that work on a single word if that is the case
    if ( m_singleWord )
        tools += KDataToolInfo::query( "QString", "application/x-singleword", instance );

    // Maybe one day we'll have tools that use libkotext (or qt3's qrt), to act on formatted text
    tools += KDataToolInfo::query( "QTextString", "application/x-qrichtext", instance );

    return KDataToolAction::dataToolActionList( tools, this, SLOT( slotToolActivated( const KDataToolInfo &, const QString & ) ) );
}

void KoTextView::slotToolActivated( const KDataToolInfo & info, const QString & command )
{
    KDataTool* tool = info.createTool( );
    if ( !tool )
    {
        kdWarning() << "Could not create Tool !" << endl;
        return;
    }

    kdDebug() << "KWTextFrameSetEdit::slotToolActivated command=" << command
              << " dataType=" << info.dataType() << endl;

    QString text;
    if ( textObject()->hasSelection() )
        text = textObject()->selectedText();
    else
        text = m_wordUnderCursor;

    // Preferred type is richtext
    QString mimetype = "application/x-qrichtext";
    QString datatype = "QTextString";
    // If unsupported, try text/plain
    if ( !info.mimeTypes().contains( mimetype ) )
    {
        mimetype = "text/plain";
        datatype = "QString";
    }
    // If unsupported (and if we have a single word indeed), try application/x-singleword
    if ( !info.mimeTypes().contains( mimetype ) && m_singleWord )
        mimetype = "application/x-singleword";

    kdDebug() << "Running tool with datatype=" << datatype << " mimetype=" << mimetype << endl;

    QString origText = text;
    if ( tool->run( command, &text, datatype, mimetype) )
    {
        kdDebug() << "Tool ran. Text is now " << text << endl;
        if ( origText != text )
        {
            if ( !textObject()->hasSelection() )
                selectWordUnderCursor();
            // replace selection with 'text'
            textObject()->emitNewCommand( textObject()->replaceSelectionCommand(
                cursor(), text, QTextDocument::Standard, i18n("Replace word") ));
        }
    }

    delete tool;
}

void KoTextView::openLink()
{
    if(m_refLink.find("http://")!=-1 || m_refLink.find("mailto:")!=-1
       || m_refLink.find("ftp://")!=-1 || m_refLink.find("file:")!=-1)
        (void) new KRun(m_refLink  );
    else
        KMessageBox::sorry(0L,i18n("%1 is not a valid link.").arg(m_refLink));//TODO FIX english
}


void KoTextView::insertSoftHyphen()
{
    textObject()->insert( cursor(), currentFormat(), QChar(0xad) /* see QRichText */,
                          false /* no newline */, true, i18n("Insert Soft Hyphen") );
}

void KoTextView::insertSpecialChar(QChar _c)
{
    if(textObject()->hasSelection() )
        textObject()->emitNewCommand(textObject()->replaceSelectionCommand(
            cursor(), _c, QTextDocument::Standard, i18n("Insert Special Char")));
    else
        textObject()->insert( cursor(), currentFormat(), _c, false /* no newline */, true, i18n("Insert Special Char") );
}

void KoTextView::insertLink(const QString &_linkName, const QString & hrefName)
{
    KoTextFormat format=*currentFormat();
    format.setAnchorName(_linkName);
    format.setAnchorHref( hrefName);
    textObject()->insert( cursor(), &format, _linkName+" " , false , true, i18n("Insert Link") );
}

const KoParagLayout * KoTextView::currentParagLayoutFormat() const
{
    KoTextParag * parag = static_cast<KoTextParag *>(m_cursor->parag());
    return &(parag->paragLayout());
}

void KoTextView::setParagLayoutFormat( KoParagLayout *newLayout,int flags)
{
    KCommand *cmd =0L;
    switch(flags)
    {
    case KoParagLayout::Alignment:
    {
        cmd = textObject()->setAlignCommand( m_cursor, newLayout->alignment );
        break;
    }
    case KoParagLayout::Tabulator:
        cmd= textObject()->setTabListCommand( m_cursor, newLayout->tabList() );
        break;
    default:
        break;
    }
    if (cmd)
       textObject()->emitNewCommand( cmd );
}

#include "kotextview.moc"
