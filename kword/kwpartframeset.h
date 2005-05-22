/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2, as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KWPARTFRAMESET_H
#define KWPARTFRAMESET_H

#include "kwframe.h"
class KWChild;
/**
 * @short A frameset that contains an embedded object (a KWChild)
 */
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

    KWChild *getChild() const { return m_child; }
    void setChild( KWChild* child );

    void updateChildGeometry( KWViewMode* viewMode );

    virtual void drawFrameContents( KWFrame * frame, QPainter *painter, const QRect & crect,
                                    const QColorGroup &cg, bool onlyChanged, bool resetChanged,
                                    KWFrameSetEdit * edit, KWViewMode *viewMode );

    // Embedded parts can be transparent
    virtual void createEmptyRegion( const QRect &, QRegion &, KWViewMode * ) { }

    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true );
    virtual void load( QDomElement &attributes, bool loadFrames = true );
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context, bool saveFrames ) const;

    virtual MouseMeaning getMouseMeaning( const QPoint &nPoint, int keyState );


    /**
     * Called by KWView when the object is being edited (activated).
     * Allows to store the initial position.
     */
    void startEditing();
    /**
     * Called by KWView when the object is not being edited anymore (deactivated).
     * Allows to create a frame-move command if the frame was moved.
     */
    void endEditing();

    /**
     * Move the frame 'frameNum' to the given position and
     * move the document child window with it
     * This is called when the frame is anchored and the anchor moves (see KWAnchor).
    */
    virtual void moveFloatingFrame( int frameNum, const KoPoint &position );

#ifndef NDEBUG
    virtual void printDebug();
#endif

    /**
    *   Delete a frame from the set of frames this frameSet has.
    *   @param num The frameNumber to be removed.
    *   @param remove passing true means that there can not be an undo of the action.
    *   @param recalc do an updateFrames()
    */
    void delFrame( unsigned int _num, bool remove = true, bool recalc = true );

    void setDeleted( bool on = true );

    virtual void storeInternal();

    virtual void setProtectContent (  bool protect ) { m_protectContent = protect; }
    virtual bool protectContent() const { return m_protectContent; }

private slots:
    void slotChildChanged();

private:
    KWChild *m_child;
    KWFramePartMoveCommand *m_cmdMoveChild;
    bool m_protectContent;
};

#if 0
class KWPartFrameSetEdit :  public QObject, public KWFrameSetEdit
{
    //Q_OBJECT
public:
    KWPartFrameSetEdit( KWPartFrameSet * fs, KWCanvas * canvas );
    virtual ~KWPartFrameSetEdit();

    virtual DCOPObject* dcopObject();


    KWPartFrameSet * partFrameSet() const
    {
        return static_cast<KWPartFrameSet*>(frameSet());
    }

    // Events forwarded by the canvas (when being in "edit" mode)
    //virtual void mousePressEvent( QMouseEvent *, const QPoint &, const KoPoint & );
    //virtual void mouseDoubleClickEvent( QMouseEvent *, const QPoint &, const KoPoint & );
//private slots:
    void slotChildActivated(bool);
private:
    DCOPObject *m_dcop;
};
#endif

#include <koDocumentChild.h>
class KWDocument;

/******************************************************************/
/* Class: KWChild                                              */
/******************************************************************/
class KWChild : public KoDocumentChild
{
public:
    KWChild( KWDocument *_wdoc, const QRect& _rect, KoDocument *_doc );
    KWChild( KWDocument *_wdoc );
    ~KWChild();

    KWDocument* parent()const
    { return m_pKWordDoc; }

    void setPartFrameSet( KWPartFrameSet* fs ) { m_partFrameSet = fs; }
    KWPartFrameSet * partFrameSet() const { return m_partFrameSet; }
    virtual KoDocument* hitTest( const QPoint& p, const QWMatrix& _matrix = QWMatrix() );

protected:
    KWDocument *m_pKWordDoc;
    KWPartFrameSet *m_partFrameSet;
};

#endif // KWPARTFRAMESET_H
