/******************************************************************/ 
/* KWord - (c) by Reginald Stadlbauer and Torben Weis 1997-1998   */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer, Torben Weis                       */
/* E-Mail: reggie@kde.org, weis@kde.org                           */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Document (header)                                      */
/******************************************************************/

#ifndef kword_doc_h
#define kword_doc_h

class KWordDocument;
class KWPage;
class QPrinter;

#include <koDocument.h>
#include <koPrintExt.h>

#include <iostream>

#include "kword_view.h"
#include "defs.h"
#include "font.h"
#include "fc.h"
#include "parag.h"
#include "paraglayout.h"
#include "formatcollection.h"
#include "imagecollection.h"
#include "image.h"
#include "char.h"
#include "frame.h"

#include <qlist.h>
#include <qobject.h>
#include <krect.h>
#include <qevent.h>
#include <qpainter.h>
#include <qregion.h>
#include <qstring.h>
#include <qclipboard.h>
#include <qstrlist.h>
#include <qmessagebox.h>
#include <qstrlist.h>

#include <kurl.h>

#include <koPageLayoutDia.h>
#include <koTemplateChooseDia.h>

#define MIME_TYPE "application/x-kword"
#define KWordRepoID "IDL:KWord/KWordDocument:1.0"

/******************************************************************/
/* Class: KWordChild                                              */
/******************************************************************/

class KWordChild : public KoDocumentChild
{
public:
  KWordChild( KWordDocument *_wdoc,const KRect& _rect,KOffice::Document_ptr _doc,int diffx,int diffy);
  KWordChild( KWordDocument *_wdoc);
  ~KWordChild();
  
  KWordDocument* parent() 
    { return m_pKWordDoc; }
  
protected:
  KWordDocument *m_pKWordDoc;

};

/******************************************************************/
/* Class: KWordDocument                                           */
/******************************************************************/

class KWordDocument : public QObject,
		      virtual public KoDocument,
		      virtual public KoPrintExt,
		      virtual public KWord::KWordDocument_skel
{
  Q_OBJECT

public:
  KWordDocument();
  ~KWordDocument();

  virtual void cleanUp();

  enum ProcessingType {WP = 0,DTP = 1};
  static const int U_FONT_FAMILY_SAME_SIZE = 1;
  static const int U_FONT_ALL_SAME_SIZE = 2;
  static const int U_COLOR = 4;
  static const int U_INDENT = 8;
  static const int U_BORDER = 16;
  static const int U_ALIGN = 32;
  static const int U_NUMBERING = 64;
  static const int U_FONT_FAMILY_ALL_SIZE = 128;
  static const int U_FONT_ALL_ALL_SIZE = 256;
  static const int U_TABS = 512;
  
protected:
  virtual bool hasToWriteMultipart();
  
public:
  // IDL
  virtual CORBA::Boolean init();

  // C++
  virtual bool loadXML( KOMLParser& parser, KOStore::Store_ptr _store );
  virtual bool loadChildren( KOStore::Store_ptr _store );
  virtual bool save( ostream& out, const char *_format );

  virtual bool loadTemplate(const char *_url);
  
  // IDL
  virtual OpenParts::View_ptr createView();
  // C++
  KWordView* createWordView();

  // IDL
  virtual void viewList(KOffice::Document::ViewList*& _list);

  virtual char* mimeType() 
    { return CORBA::string_dup(MIME_TYPE); }
  
  virtual CORBA::Boolean isModified() 
    { return m_bModified; }

  // C++
  virtual void setModified( bool _c ) { m_bModified = _c; if ( _c ) m_bEmpty = false; }
  virtual bool isEmpty() { return m_bEmpty; }

  unsigned int viewCount() { return m_lstViews.count(); }

  virtual QStrList outputFormats();
  virtual QStrList inputFormats();

  virtual void addView(KWordView *_view);
  virtual void removeView(KWordView *_view);

  virtual void insertObject(const KRect& _rect,const char *_part_name,int diffx,int diffy);
  virtual void changeChildGeometry(KWordChild *_child,const KRect&);
  
  virtual QListIterator<KWordChild> childIterator();

  /**
   * Sets the paper size and recalculates the papers width and height.
   */
  void setPageLayout(KoPageLayout _layout,KoColumns _cl);

  void getPageLayout(KoPageLayout& _layout,KoColumns& _cl)
    { _layout = pageLayout; _cl = pageColumns; }

  KWFrameSet *getFrameSet(unsigned int _num)
    { return frames.at(_num); }
  unsigned int getNumFrameSets()
    { return frames.count(); }
  void addFrameSet(KWFrameSet *f)
    { frames.append(f); updateAllRanges(); updateAllFrames(); /*updateAllViews(0L);*/ }


  /**
   * Returns the first parag of the frameset <i>_num</i>.
   */
  KWParag *getFirstParag(unsigned int _num) { 
    if (frames.at(_num)->getFrameType() == FT_TEXT)
      return dynamic_cast<KWTextFrameSet*>(frames.at(_num))->getFirstParag();
    else
      return 0L;
  }
      
  /**
   * @return the default user font.
   *
   * @see KWUserFont
   */
  KWUserFont* getDefaultUserFont() 
    { return defaultUserFont; }
  KWParagLayout *getDefaultParagLayout() 
    { return defaultParagLayout; }
    
  /**
   * List of all available user fonts
   */    
  QList<KWUserFont> userFontList;

  /**
   * List of all available display fonts
   */
  QList<KWDisplayFont> displayFontList;

  /**
   * List of all available parag layouts.
   */
  QList<KWParagLayout> paragLayoutList;

  /**
   * @return a pointer to the user font with the given name or 0L if not found.
   *
   * @see KWUserFont
   */
  KWUserFont* findUserFont(QString _fontname);

  /**
   * @return a display font matching the criteriums or 0L if none is found.
   */
  KWDisplayFont* findDisplayFont(KWUserFont* _font,unsigned int _size,int _weight,bool _italic,bool _underline);

  /**
   * @return a pointer to the parag with the given name or 0L if not found.
   *
   * @see KWParagLayout
   */
  KWParagLayout* findParagLayout(QString _name);

  KWParag* findFirstParagOfPage(unsigned int _page,unsigned int _frameset);
  KWParag* findFirstParagOfRect(unsigned int _ypos,unsigned int _page,unsigned int _frameset);
    

  float getMMTopBorder() { return pageLayout.top; }
  float getMMBottomBorder() { return pageLayout.bottom; }
  float getMMLeftBorder() { return pageLayout.left; }
  float getMMRightBorder() { return pageLayout.right; }
  float getMMPaperWidth() { return pageLayout.width; }
  float getMMPaperHeight() { return pageLayout.height; }
  
  unsigned int getPTTopBorder() { return static_cast<unsigned int>(pageLayout.ptTop); }
  unsigned int getPTBottomBorder() { return static_cast<unsigned int>(pageLayout.ptBottom); }
  unsigned int getPTLeftBorder() { return static_cast<unsigned int>(pageLayout.ptLeft); }
  unsigned int getPTRightBorder() { return static_cast<unsigned int>(pageLayout.ptRight); }
  unsigned int getPTPaperHeight() { return static_cast<unsigned int>(pageLayout.ptHeight); }
  unsigned int getPTPaperWidth() { return static_cast<unsigned int>(pageLayout.ptWidth); }
  unsigned int getPTColumnWidth() { return static_cast<unsigned int>(ptColumnWidth); }
  unsigned int getPTColumnSpacing() { return MM_TO_POINT(pageColumns.columnSpacing); }
    
  unsigned int getColumns() { return pageColumns.columns; }
  
  void print() {}
  
  /**
   * @param _ypos is relative to the upper left corner of the FIRST
   *              page of the document.
   *
   * @return TRUE if '_ypos' is within the printable area of the
   *         page with number '_page'. The borders of the paper
   *         are not considered to belong to the printable area.
   */
  bool isPTYInFrame(unsigned int _frameSet,unsigned int _frame,unsigned int _ypos);
  
  bool printLine(KWFormatContext &_fc,QPainter &_painter,int xOffset,int yOffset,int _w,int _h,bool _viewFormattingChars = false);
  
  void drawMarker(KWFormatContext &_fc,QPainter *_painter,int xOffset,int yOffset);

  void updateAllViews(KWordView *_view);
  void updateAllRanges();
  void updateAllCursors();
  void drawAllBorders(QPainter *_painter = 0);
  void recalcWholeText();
  void hideAllFrames();

  int getPages() { return pages; }

  void setPages(int _pages)
    { pages = _pages; updateAllRanges(); }

  KWFormatCollection *getFormatCollection()
    { return &formatCollection; }
  KWImageCollection *getImageCollection()
    { return &imageCollection; }

  void insertPicture(QString _filename,KWPage *_paperWidget);

  void setSelStart(KWFormatContext &_fc) 
    { selStart = _fc; }
  KWFormatContext *getSelStart()
    { return &selStart; }
  void setSelEnd(KWFormatContext &_fc) 
    { selEnd = _fc; }
  KWFormatContext *getSelEnd()
    { return &selEnd; }
  void drawSelection(QPainter &_painter,int xOffset,int yOffset);
  void setSelection(bool _has)
    { hasSelection = _has; }
  bool has_selection()
    { return hasSelection; }

  void deleteSelectedText(KWFormatContext *_fc,QPainter &_painter);
  void copySelectedText();
  void setFormat(KWFormat &_format);

  void paste(KWFormatContext *_fc,QString _string,KWPage *_page);

  void appendPage(unsigned int _page,QPainter &_painter);

  ProcessingType getProcessingType()
    { return processingType; }
  
  int getFrameSet(unsigned int mx,unsigned int my);
  /**
   * Return 1, if a frame gets selected which was not selected before,
   * 2, if a frame gets selected which was already selected, and
   * 0 if no frame got selected.
   */
  int selectFrame(unsigned int mx,unsigned int my);
  void deSelectFrame(unsigned int mx,unsigned int my);
  void deSelectAllFrames();
  QCursor getMouseCursor(unsigned int mx,unsigned int my);
  KWFrame *getFirstSelectedFrame();
  KWFrame *getFirstSelectedFrame(int &_frameset);

  void print(QPainter *painter,QPrinter *printer,float left_margin,float top_margin);
  
  void updateAllFrames();

  int getRastX() { return rastX; }
  int getRastY() { return rastY; }

  int getApplyStyleTemplate() { return applyStyleTemplate; }
  void setApplyStyleTemplate(int _f) { applyStyleTemplate = _f; }

  void updateAllStyles();
  void updateAllStyleLists();

  void setStyleChanged(QString _name);
  bool isStyleChanged(QString _name);

  bool loaded() { return _loaded; }

  bool hasHead() { return _head; }
  bool hasFoot() { return _foot; }
  void setHead(bool h) { _head = h; recalcFrames(); }
  void setFoot(bool f) { _foot = f; recalcFrames(); }

signals:
  void sig_imageModified();
  void sig_insertObject(KWordChild *_child,KWPartFrameSet*);
  void sig_updateChildGeometry(KWordChild *_child);
  void sig_removeObject(KWordChild *_child);

protected:
  virtual void insertChild(KWordChild*);
  virtual void makeChildListIntern(KOffice::Document_ptr _doc,const char *_path);
  
  virtual void draw(QPaintDevice*,CORBA::Long _width,CORBA::Long _height);
  QPen setBorderPen(KWParagLayout::Border _brd);

  void loadFrameSets(KOMLParser&,vector<KOMLAttrib>&);
  void loadStyleTemplates(KOMLParser&,vector<KOMLAttrib>&);
  void recalcFrames();

  void addStyleTemplate(KWParagLayout *pl);

  QList<KWordView> m_lstViews;
  QList<KWordChild> m_lstChildren;

  bool m_bModified;

  /**
   * Calculates the width of a column.
   *
   * @see #columnWidth
   * @see #ptColumnWidth
   */
  //void calcColumnWidth();
    
  KoPageLayout pageLayout;
  KoColumns pageColumns;
    
  /**
   * Width of a column in unzoomed points. This value is automatically calculated,
   * so dont write to it if you dont know exactly what you are doing.
   *
   * @see #calcColumnWidth
   */
  unsigned int ptColumnWidth;
  
  /**
   * List of all framesets.
   *
   * @see KWFrame
   */
  QList<KWFrameSet> frames;

  /**
   * The default user font.
   *
   * @see KWUserFont
   */
  KWUserFont *defaultUserFont;
  /**
   * The default parag layout
   *
   * @see KWParagLayout
   */
  KWParagLayout *defaultParagLayout;
  
  int pages;

  KWFormatCollection formatCollection;
  KWImageCollection imageCollection;

  KWFormatContext selStart,selEnd;
  bool hasSelection;

  ProcessingType processingType;
  int rastX,rastY;

  bool m_bEmpty;

  int applyStyleTemplate;
  
  QStrList changedStyles;

  bool _loaded;

  QPixmap ret_pix;

  bool _head,_foot;
  
};

#endif






