// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>
   Copyright (C) 2004-2005 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KPRPAGE_H
#define KPRPAGE_H

#include <qwidget.h>
#include <qptrlist.h>
#include "global.h"
#include <koPoint.h>
#include <koRect.h>
#include <kostyle.h> // for KoStyleChangeDefMap
#include <qvariant.h>
#include <koDocumentChild.h>
#include <koPictureCollection.h>

class KMacroCommand;
class KoSavingContext;
class KURL;
class KPTextView;
class KPObject;
class KPresenterDoc;
class KPresenterView;
class KoDocumentEntry;
class KoPageLayout;
class KCommand;
class KoPointArray;
class DCOPObject;
class KPTextObject;
class KoTextObject;
class KPPixmapObject;
class KPPartObject;
class KPBackGround;

class KoXmlWriter;
class KoGenStyles;


class KPrPage
{
public:
    /**
     * @brief Constructor
     *
     * @param doc pointer to the kpresenter document
     * @param masterPage master page, 0 if it is a master page
     */
    KPrPage(KPresenterDoc *_doc, KPrPage *masterPage = 0 );

    /**
     * @brief Desctructor
     */
    virtual ~KPrPage();

    /**
     * @brief Return a DCOP interface for this page
     */
    virtual DCOPObject* dcopObject();

    /**
     * @brief Return a DCOP interface for this page
     */
    KPresenterDoc * kPresenterDoc() const {return m_doc; }

    /**
     * @brief Get the manual title
     *
     * @return the manual title
     */
    QString manualTitle()const;

    /**
     * @brief Set the manual title
     *
     * @param title which sould be set
     */
    void insertManualTitle(const QString & title);

    /**
     * @brief Get the page title.
     *
     * The page title is found out like following:
     * - If a manual title is set the manual title is returned.
     * - If there are text objects on the page the title is generated
     *   for the first paragraph of top most text object.
     * - If a title is passed to the function it is returned
     * - Slide %PAGENUMBER or Master Slide
     */
    QString pageTitle( const QString &_title = QString() ) const;

    /**
     * @brief Set the note text
     *
     * @param text which should be set
     */
    void setNoteText( const QString &_text );

    /**
     * @brief Get the note text
     *
     * @return the text of the note
     */
    QString noteText( )const;

    /**
     * @brief Set the master page to masterPage
     *
     * @param masterpage to set
     */
    void setMasterPage( KPrPage *_masterPage ) { m_masterPage = _masterPage; }

    /**
     * @brief Get the master page
     *
     * @return the master page, 0 if it is a master page
     */
    KPrPage * masterPage() const { return m_masterPage; }

    /**
     * @brief Check if page is a master page
     *
     * @return true if master page
     * @return false otherwise
     */
    bool isMasterPage() const;

    /**
     * @brief Get object list
     *
     * @return object list
     */
    const QPtrList<KPObject> & objectList() const { return m_objectList; }

    /**
     * @brief Get object at position
     *
     * @param pos position of object
     *
     * @return object at position num
     */
    KPObject *getObject( int num );

    /**
     * @brief Appends object to page
     *
     * Appends the object to the page. The name of the object
     * is unified.
     *
     * @param obj object to append
     */
    void appendObject( KPObject *_obj );

    /**
     * @brief Replaces an object
     *
     * The object oldObject by the object newObject
     *
     * @param oldObject object which will be removed from the page
     * @param newObject object which will take the place of the oldObject
     */
    void replaceObject( KPObject *oldObject, KPObject *newObject );

    /**
     * @brief Take object from page
     *
     * @param object to be take from page
     *
     * @return the position of the taken object
     */
    int takeObject( KPObject *object );

    /**
     * @brief Delete selected objects from page
     *
     * The returned command will be executed.
     *
     * @return KCommand the command
     * @return 0 if no object is selected
     */
    KCommand * deleteSelectedObjects();

    /**
     * @brief Insert object in page
     *
     * Inserts the object at pos to the page. The name of the object
     * is unified.
     *
     * @param object to insert
     * @param position the object should be insert
     */
    void insertObject( KPObject *object, int pos );

    /**
     * @brief
     */
    void completeLoading( bool _clean, int lastObj );

    /**
     * @brief Create a uniq name for an object.
     *
     * Create a uniq name for the object. If no name is set for the object
     * a name according to its type is created. If the name already exists
     * append ' (x)'. // FIXME: not allowed by I18N
     *
     * @param object to work on
     */
    void unifyObjectName( KPObject *object );

    /**
     * @brief Get the page rect
     *
     * @return the page rect
     */
    KoRect getPageRect() const;

    /**
     * @brief Get the page rect with applied zoom
     *
     * @return the page rect
     */
    QRect getZoomPageRect()const;

    /**
     * @brief Set object for the page
     *
     * @param list of objects
     */
    void setObjectList( QPtrList<KPObject> objectLlist ) {
        m_objectList.setAutoDelete( false ); m_objectList = objectLlist; m_objectList.setAutoDelete( false );
    }

    /**
     * @brief Get the number ob objects on the page
     *
     * @return the number of objects on the page
     */
    unsigned int objNums() const { return m_objectList.count(); }

    /**
     * @brief Get the number of text objects
     *
     * The functions in only used by KPresenterPageIface.
     *
     * @return the number os text object on the page
     */
    int numTextObject() const;
    /**
     * @brief Get the text object
     *
     * The functions in only used by KPresenterPageIface.
     *
     * @param num the number of the text object to get
     *
     * @return the text object with the number
     * @return 0 if no such object exists
     */
    KPTextObject *textFrameSet( unsigned int _num ) const;

    /**
     * @brief Get the amount of selected objects
     *
     * @return the amount of select objects
     */
    int numSelected() const;

    /**
     * @brief
     */
    void pasteObjs( const QByteArray & data, int nbCopy = 1, double angle = 0.0 ,
                    double _increaseX=0.0, double increaseY = 0.0, double moveX=0.0, double moveY=0.0);

    /**
     * @brief
     */
    KCommand * replaceObjs( bool createUndoRedo, double _orastX, double _orastY,
                            const QColor & _txtBackCol, const QColor & _otxtBackCol);

    /**
     * @brief
     */
    void copyObjs(QDomDocument &doc, QDomElement &presenter, QValueList<KoPictureKey> & lst) const;

    /**
     * @brief Get the first selected object
     *
     * @return the first selected object of this page
     * @return O if none exists
     */
    KPObject* getSelectedObj() const;

    /**
     * @brief Get a list of all selected objects
     *
     * @param withoutHeaderFooter if true a selected header
     *        or footer will not be in the list returned.
     *
     * @return list of selected objets.
     */
    QPtrList<KPObject> getSelectedObjects( bool withoutHeaderFooter = false ) const;

    /**
     * @brief Get the first selected pictur
     *
     * @return first selected picture
     * @return 0 if none is selected
     */
    KPPixmapObject* getSelectedImage() const;

    /**
     * @brief Set the image effect for all selected pictures
     *
     * @param eff image effect
     * @param param1 effect parameter 1
     * @param param1 effect parameter 2
     * @param param1 effect parameter 3
     */
    KCommand * setImageEffect(ImageEffect eff, QVariant param1, QVariant param2, QVariant param3);

    /**
     * @brief Group selected objects.
     *
     * This will create a GroupObjCmd, executes it and adds it to the history.
     * If less than 2 objects are selected nothing happens.
     */
    void groupObjects();

    /**
     * @brief Ungroup all selected group objects
     *
     * @param macro to store the commands in
     */
    void ungroupObjects( KMacroCommand ** macro );

    /**
     * @brief
     */
    void raiseObjs( bool forward );

    /**
     * @brief
     */
    void lowerObjs( bool backward );

    KCommand* setPen( const QPen &pen, LineEnd lb, LineEnd le, int flags );
    KCommand* setBrush( const QBrush &brush, FillType ft, const  QColor& g1, const QColor &g2,
                        BCType gt, bool unbalanced, int xfactor, int yfactor, int flags );

    /**
     * Insert an object into the page
     */
    KCommand * insertObject( const QString &name, KPObject * object, const KoRect &r, bool addCommand = true );

    virtual KPPartObject* insertObject( const KoRect&, KoDocumentEntry& );

    void insertRectangle( const KoRect &r, const QPen & pen, const QBrush &brush, FillType ft,
                          const QColor &g1, const QColor & g2,BCType gt, int rndX, int rndY,
                          bool unbalanced, int xfactor, int yfactor );

    void insertCircleOrEllipse( const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft,
                                const QColor &g1, const QColor &g2, BCType gt, bool unbalanced, int xfactor, int yfactor );

    void insertPie( const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft,
                    const QColor &g1, const QColor &g2,BCType gt, PieType pt, int _angle, int _len,
                    LineEnd lb,LineEnd le,bool unbalanced, int xfactor, int yfactor );

    KPTextObject*  insertTextObject( const KoRect& r, const QString& text = QString::null, KPresenterView *_view = 0L );
    void insertLine( const KoRect &r, const QPen &pen, LineEnd lb, LineEnd le, LineType lt );

    void insertAutoform( const KoRect &r, const QPen &pen, const QBrush &brush, LineEnd lb, LineEnd le,
                         FillType ft,const QColor &g1, const QColor &g2, BCType gt, const QString &fileName,
                         bool unbalanced,int xfactor, int yfactor );

    void insertFreehand( const KoPointArray &points, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );
    void insertPolyline( const KoPointArray &points, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );
    void insertQuadricBezierCurve( const KoPointArray &points, const KoPointArray &allPoints, const KoRect &r,
                                   const QPen &pen,LineEnd lb, LineEnd le );
    void insertCubicBezierCurve( const KoPointArray &points, const KoPointArray &allPoints, const KoRect &r,
                                 const QPen &pen,LineEnd lb, LineEnd le );

    void insertPolygon( const KoPointArray &points, const KoRect &r, const QPen &pen, const QBrush &brush,
                        FillType ft,const QColor &g1, const QColor &g2, BCType gt, bool unbalanced,
                        int xfactor, int yfactor, bool _checkConcavePolygon, int _cornersValue, int _sharpnessValue );

    void insertClosedLine( const KoPointArray &points, const KoRect &r, const QPen &pen, const QBrush &brush,
                           FillType ft,const QColor &g1, const QColor &g2,
                           BCType gt, bool unbalanced, int xfactor, int yfactor, ToolEditMode _mode );

    void slotRepaintVariable();
    void recalcPageNum();
    void changePicture( const KURL & url, QWidget *parent );
    void insertPicture( const QString &, int _x = 10, int _y = 10 );
    void insertPicture( const QString &_file, const KoRect &_rect );

    void enableEmbeddedParts( bool f );

    KPBackGround *background() { return m_kpbackground; }

    void makeUsedPixmapList();

    void setBackColor( const QColor &backColor1, const QColor &backColor2, BCType bcType,
                       bool unbalanced, int xfactor, int yfactor );
    void setBackPicture( const KoPictureKey & key );
    bool getBackUnbalanced(  )const;
    void setBackView( BackView backView );
    void setBackType( BackType backType );

    void setPageEffect(  PageEffect pageEffect );
    void setPageEffectSpeed( EffectSpeed pageEffectSpeed );
    void setPageTimer(  int pageTimer );
    void setPageSoundEffect(  bool soundEffect );
    void setPageSoundFileName(  const QString &fileName );
    BackType getBackType(  ) const ;
    BackView getBackView( )const ;
    KoPictureKey getBackPictureKey( )const ;
    KoPicture getBackPicture( )const ;
    QColor getBackColor1( )const ;
    QColor getBackColor2()const ;
    int getBackXFactor()const ;
    int getBackYFactor( )const;
    BCType getBackColorType( )const;
    PageEffect getPageEffect( )const;
    EffectSpeed getPageEffectSpeed() const;
    int getPageTimer(  )const;
    bool getPageSoundEffect( )const;
    QString getPageSoundFileName()const;


    void setUseMasterBackground( bool useMasterBackground );
    bool useMasterBackground() const;

    /**
     * return the list of steps where objects appear/disappear.
     */
    QValueList<int> getEffectSteps() const;

    bool isSlideSelected() const {return  m_selectedSlides;}
    void slideSelected(bool _b){m_selectedSlides=_b;}

    void setInsPictureFile( const QString &_file ) { m_pictureFile = _file; }

    QString insPictureFile() const { return m_pictureFile; }

    void deSelectAllObj();
    void deSelectObj( KPObject *kpobject );
    QDomElement saveObjects( QDomDocument &doc, QDomElement &objects, double yoffset, KoZoomHandler* zoomHandler,
                             int saveOnlyPage ) const;

    bool oneObjectTextExist(bool forceAllTextObject = true) ;
    bool oneObjectTextSelected();
    bool isOneObjectSelected();
    bool haveASelectedPartObj();
    bool haveASelectedGroupObj();
    bool haveASelectedPixmapObj();

    KoRect getBoundingRect(const KoRect &rect) const;
    KoRect getBoundingAllObjectRect(const KoRect &rect) const;
    bool chPic( KPresenterView *_view);

    //return command when we move object
    KCommand *moveObject(KPresenterView *_view, double diffx, double diffy);
    KCommand *moveObject(KPresenterView *m_view,const KoPoint &_move,bool key);

    KCommand *rotateSelectedObjects(float _newAngle, bool addAngle=false);
    KCommand *shadowObj(ShadowDirection dir,int dist, const QColor &col);
    KCommand *stickyObj(bool _sticky, KPrPage * currentPage);

    QPtrList<KoTextObject> allTextObjects() const;
    void addTextObjects(QPtrList<KoTextObject>& lst) const;

    void repaintObj();

    KPObject * getCursor(const QPoint &pos );
    KPObject * getCursor(const KoPoint &pos );

    /**
     * Returns the object at position pos
     * @param pos the position in the document on which to look
     * @param withoutProtected if set to true only unprotecred objects are returned
     * @return the object at position pos
     */
    KPObject* getObjectAt( const KoPoint &pos, bool withoutProtected = false ) const;
    KPPixmapObject * picViewOrigHelper() const;
    void applyStyleChange( KoStyleChangeDefMap changed );

    void reactivateBgSpellChecking(bool refreshTextObj);

    bool canMoveOneObject() const;
    KCommand *alignVertical( VerticalAlignmentType _type );
    void changeTabStopValue ( double _tabStop );
    bool savePicture( KPresenterView *_view ) const;
    bool findTextObject( KPObject *obj );
    KPObject *nextTextObject(KPTextObject *obj);

    void getAllObjectSelectedList(QPtrList<KPObject> &lst,bool force = false );
    void getAllEmbeddedObjectSelected(QPtrList<KoDocumentChild> &embeddedObjects );

    void load( const QDomElement &element );
    void loadOasis( KoOasisContext & context );

    QDomElement save( QDomDocument &doc, const bool saveAsKOffice1Dot1 );

    bool saveOasisPage( KoStore *store, KoXmlWriter &xmlWriter, int posPage, KoSavingContext& context, int & indexObj, int &partIndexObj, KoXmlWriter* manifestWriter ) const;
    QString saveOasisPageStyle( KoStore *store, KoGenStyles& mainStyles ) const;
    QString saveOasisPageEffect() const;
    bool saveOasisNote( KoXmlWriter &xmlWriter ) const;
    bool saveOasisStickyPage( KoStore *store, KoXmlWriter &xmlWriter, KoSavingContext& context, int & indexObj, int &partIndexObj, KoXmlWriter* manifestWriter ) const;

    bool hasHeader() const { return m_bHasHeader; }
    bool hasFooter() const { return m_bHasFooter; }
    void setHeader( bool b );
    void setFooter( bool b );

    void setDisplayObjectFromMasterPage( bool _b );
    bool displayObjectFromMasterPage() const { return m_displayObjectFromMasterPage;}

    QString oasisNamePage( int posPage ) const;

private:
    void makeUsedPixmapListForGroupObject( KPObject *_obj );
    void completeLoadingForGroupObject( KPObject *_obj );
    bool objectNameExists( KPObject *object, QPtrList<KPObject> &list );
    void saveOasisObject( KoStore *store, KoXmlWriter &xmlWriter, KoSavingContext& context, int & indexObj, int &partIndexObj,  KoXmlWriter* manifestWriter, bool stickyObj=false ) const;


    // list of objects
    QPtrList<KPObject> m_objectList;
    KPresenterDoc *m_doc;
    KPrPage *m_masterPage;
    KPBackGround *m_kpbackground;
    QString m_manualTitle;
    QString m_noteText;
    DCOPObject *m_dcop;
    bool m_selectedSlides;

    QString m_pictureFile;
    bool m_bHasHeader, m_bHasFooter;
    bool m_useMasterBackground;


    bool m_displayObjectFromMasterPage;

    /**
     * The page effect
     */
    PageEffect m_pageEffect;
    /**
     * The speed of the page effect.
     */
    EffectSpeed m_pageEffectSpeed;

    /**
     * true when a sound effect will be played when going to this page
     */
    // ### This could be simply !soundFileName.isEmpty()...
    bool m_soundEffect;
    /**
     * Sound played when showing this page
     */
    QString m_soundFileName;
    /**
     * TODO
     */
    int m_pageTimer;
};
#endif //KPRPAGE_H
