/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1998                   */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* needs c++ library Qt (http://www.troll.no)                     */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* written for KDE (http://www.kde.org)                           */
/* KPresenter is under GNU GPL                                    */
/******************************************************************/
/* Module: KPresenter Document                                    */
/******************************************************************/

#include "kpresenter_doc.h"
#include "kpresenter_doc.moc"

/******************************************************************/
/* class BackPic                                                  */
/******************************************************************/

/*======================= constructor ============================*/
BackPic::BackPic(QWidget* parent=0,const char* name=0)
  : QWidget(parent,name)
{
  pic = 0;
  pic = new QPicture;
}

/*======================= destructor =============================*/
BackPic::~BackPic()
{
  delete pic;
}

/*==================== set clipart ===============================*/
void BackPic::setClipart(const char* fn)
{
  fileName = qstrdup(fn);
  wmf.load(fileName);
  wmf.paint(pic);
  repaint();
}

/*======================= get pic ================================*/
QPicture* BackPic::getPic()
{
  return pic;
}

/******************************************************************/
/* class KPresenterChild                                          */
/******************************************************************/

/*====================== constructor =============================*/
KPresenterChild::KPresenterChild(KPresenterDocument_impl *_kpr,
				 const QRect& _rect,OPParts::Document_ptr _doc)
{
  m_pKPresenterDoc = _kpr;
  m_rDoc = OPParts::Document::_duplicate(_doc);
  m_geometry = _rect;
}

/*====================== destructor ==============================*/
KPresenterChild::~KPresenterChild()
{
  m_rDoc = 0L;
}

/******************************************************************/
/* class KPresenterDocument_impl                                  */
/******************************************************************/

/*====================== constructor =============================*/
KPresenterDocument_impl::KPresenterDocument_impl()
{
  // Use CORBA mechanism for deleting views
  m_lstViews.setAutoDelete(false);
  m_lstChildren.setAutoDelete(true);

  m_bModified = false;

  // init
  _pageList.setAutoDelete(true);
  _objList.setAutoDelete(true);
  _objNums = 0;
  _pageNums = 0;
  _spInfinitLoop = false;
  _spManualSwitch = true;
  _spPageConfig.setAutoDelete(true);
  _rastX = 20;
  _rastY = 20;
  _xRnd = 20;
  _yRnd = 20;
  _txtBackCol.operator=(white);
  _txtSelCol.operator=(lightGray);
  _pageLayout.format = PG_SCREEN;
  _pageLayout.orientation = PG_PORTRAIT;
  _pageLayout.width = PG_SCREEN_WIDTH;
  _pageLayout.height = PG_SCREEN_HEIGHT;
  _pageLayout.left = 0;
  _pageLayout.right = 0;
  _pageLayout.top = 0;
  _pageLayout.bottom = 0;
  _pageLayout.unit = PG_MM;
  setPageLayout(_pageLayout,0,0);
  insertNewPage(0,0);
}

/*====================== constructor =============================*/
KPresenterDocument_impl::KPresenterDocument_impl(const CORBA::BOA::ReferenceData &_refdata)
  : KPresenter::KPresenterDocument_skel(_refdata)
{
  // Use CORBA mechanism for deleting views
  m_lstViews.setAutoDelete(false);
  m_lstChildren.setAutoDelete(true);

  m_bModified = false;

  // init
  _pageList.setAutoDelete(true);
  _objList.setAutoDelete(true);
  _objNums = 0;
  _pageNums = 0;
  _spInfinitLoop = false;
  _spManualSwitch = true;
  _spPageConfig.setAutoDelete(true);
  _rastX = 20;
  _rastY = 20;
  _xRnd = 20;
  _yRnd = 20;
  _txtBackCol.operator=(white);
  _txtSelCol.operator=(lightGray);
  _pageLayout.format = PG_SCREEN;
  _pageLayout.orientation = PG_PORTRAIT;
  _pageLayout.width = PG_SCREEN_WIDTH;
  _pageLayout.height = PG_SCREEN_HEIGHT;
  _pageLayout.left = 0;
  _pageLayout.right = 0;
  _pageLayout.top = 0;
  _pageLayout.bottom = 0;
  _pageLayout.unit = PG_MM;
  setPageLayout(_pageLayout,0,0);
  insertNewPage(0,0);
}

/*====================== destructor ==============================*/
KPresenterDocument_impl::~KPresenterDocument_impl()
{
  sdeb("KPresenterDocument_impl::~KPresenterDocument_impl()\n");
  cleanUp();
  edeb("...KPresenterDocument_impl::~KPresenterDocument_impl() %i\n",_refcnt());
}

/*======================= clean up ===============================*/
void KPresenterDocument_impl::cleanUp()
{
  if (m_bIsClean) return;

  assert(m_lstViews.count() == 0);
  
  m_lstChildren.clear();

  Document_impl::cleanUp();
}

/*========================== save ===============================*/
bool KPresenterDocument_impl::save(const char *_url)
{
  KURL u(_url);
  if (u.isMalformed())
    {
      cerr << "malformed URL" << endl;
      return false;
    }
  
  if (!u.isLocalFile())
    {
      QMessageBox::critical((QWidget*)0L,i18n("KSpread Error"),i18n("Can not save to remote URL\n"),i18n("OK"));
      return false;
    }

  ofstream out(u.path());
  if (!out)
    {
      QString tmp;
      tmp.sprintf(i18n("Could not write to\n%s"),u.path());
      cerr << tmp << endl;
      return false;
    }

  out << "<?xml version=\"1.0\"?>" << endl;
  
  save(out);
  
  m_strFileURL = _url;
    
  return true;
}

/*========================== save ===============================*/
bool KPresenterDocument_impl::save(ostream& out)
{
  out << otag << "<DOC author=\"" << "Reginals Stadlbauer" << "\" email=" << "reggie@kde.org" << " editor=" << "KPresenter"
      << " mime=" << "\"application/x-kpresenter\"" << " >" << endl;
  
  out << otag << "<PAPER format=" << paperFormatString() << " orientation=" << orientationString() << '>' << endl;
  out << indent << "<PAPERBORDERS left=" << pageLayout().left << " top=" << pageLayout().top << " right=" << pageLayout().right
      << " bottom=" << pageLayout().bottom << "/>" << endl;
  out << etag << "</PAPER>" << endl;
  
  out << etag << "</DOC>" << endl;
    
  setModified(false);
    
  return true;
}

/*========================== load ===============================*/
bool KPresenterDocument_impl::load(const char *_url)
{
//   KURL u( _url );
//   if ( u.isMalformed() )
//     return FALSE;
  
//   if ( !u.isLocalFile() )
//   {
//     cerr << "Can not save to remote URL" << endl;
//     return false;
//   }

//   ifstream in( u.path() );
//   if ( !in )
//   {
//     cerr << "Could not open" << u.path() << endl;
//     return false;
//   }

//   KOMLStreamFeed feed( &in );
//   KOMLParser parser( &feed );
  
//   string tag;
//   vector<KOMLAttrib> lst;
//   string name;

//   // DOC
//   if ( !parser.open( "DOC", tag ) )
//   {
//     cerr << "Missing DOC" << endl;
//     return false;
//   }
  
//   KOMLParser::parseTag( tag.c_str(), name, lst );
//   vector<KOMLAttrib>::const_iterator it = lst.begin();
//   for( ; it != lst.end(); it++ )
//   {
//     if ( (*it).m_strName == "mime" )
//     {
//       if ( (*it).m_strValue != "application/x-kspread" )
//       {
// 	cerr << "Unknown mime type " << (*it).m_strValue << endl;
// 	return false;
//       }
//     }
//   }

//   if ( !load( parser ) )
//     return false;
    
//   parser.close( tag );

//   m_strFileURL = _url;
  
//   return true;
  return false;
}

/*========================== load ===============================*/
bool KPresenterDocument_impl::load(KOMLParser& parser)
{
//   string tag;
//   vector<KOMLAttrib> lst;
//   string name;
  
//   // PAPER, MAP
//   while( parser.open( 0L, tag ) )
//   {
//     KOMLParser::parseTag( tag.c_str(), name, lst );
 
//     if ( name == "PAPER" )
//     {
//       KOMLParser::parseTag( tag.c_str(), name, lst );
//       vector<KOMLAttrib>::const_iterator it = lst.begin();
//       for( ; it != lst.end(); it++ )
//       {
// 	if ( (*it).m_strName == "format" )
// 	{
// 	}
// 	else if ( (*it).m_strName == "orientation" )
// 	{
// 	}
// 	else
// 	  cerr << "Unknown attrib PAPER:'" << (*it).m_strName << "'" << endl;
//       }

//       // PAPERBORDERS, HEAD, FOOT
//       while( parser.open( 0L, tag ) )
//       {
// 	KOMLParser::parseTag( tag.c_str(), name, lst );

// 	if ( name == "PAPERBORDERS" )
// 	{    
// 	  KOMLParser::parseTag( tag.c_str(), name, lst );
// 	  vector<KOMLAttrib>::const_iterator it = lst.begin();
// 	  for( ; it != lst.end(); it++ )
// 	  {
// 	    if ( (*it).m_strName == "left" )
// 	    {
// 	    }
// 	    else if ( (*it).m_strName == "top" )
// 	    {
// 	    }
// 	    else if ( (*it).m_strName == "right" )
// 	    {
// 	    }
// 	    else if ( (*it).m_strName == "bottom" )
// 	    {
// 	    }
// 	    else
// 	      cerr << "Unknown attrib 'PAPERBORDERS:" << (*it).m_strName << "'" << endl;
// 	  } 
// 	}
//       	else if ( name == "HEAD" )
// 	{
// 	  KOMLParser::parseTag( tag.c_str(), name, lst );
// 	  vector<KOMLAttrib>::const_iterator it = lst.begin();
// 	  for( ; it != lst.end(); it++ )
// 	  {
// 	    if ( (*it).m_strName == "left" )
// 	    {
// 	    }
// 	    else if ( (*it).m_strName == "center" )
// 	    {
// 	    }
// 	    else if ( (*it).m_strName == "right" )
// 	    {
// 	    }
// 	    else
// 	      cerr << "Unknown attrib 'HEAD:" << (*it).m_strName << "'" << endl;
// 	  } 
// 	}
//       	else if ( name == "FOOT" )
// 	{
// 	  KOMLParser::parseTag( tag.c_str(), name, lst );
// 	  vector<KOMLAttrib>::const_iterator it = lst.begin();
// 	  for( ; it != lst.end(); it++ )
// 	  {
// 	    if ( (*it).m_strName == "left" )
// 	    {
// 	    }
// 	    else if ( (*it).m_strName == "center" )
// 	    {
// 	    }
// 	    else if ( (*it).m_strName == "right" )
// 	    {
// 	    }
// 	    else
// 	      cerr << "Unknown attrib 'FOOT:" << (*it).m_strName << "'" << endl;
// 	  } 
// 	}
// 	else
// 	  cerr << "Unknown tag '" << tag << "' in PAPER" << endl;    
	
// 	if ( !parser.close( tag ) )
//         {
// 	  cerr << "ERR: Closing Child" << endl;
// 	  return false;
// 	}
//       }
//     }
//     else if ( name == "MAP" )
//     {
//       if ( !m_pMap->load( parser, lst ) )
// 	return false;
//     }
//     else
//       cerr << "Unknown tag '" << tag << "' in TABLE" << endl;    

//     if ( !parser.close( tag ) )
//     {
//       cerr << "ERR: Closing Child" << endl;
//       return false;
//     }
//   }

//   return true;

//   /*
//     KSpreadMap *map2 = new XclMap( this );

//     printf("Loading map ....\n");

//     // For use as values in the ObjectType property
//     TYPE t_map = _korb->findType( "KDE:kxcl:Map" );
//     if ( !t_map || !KPart::load( _korb, _map, t_map ) )
// 	return FALSE;
    
//     bool ret = map2->load( _korb, _map );

//     if ( ret )
//     {
// 	tableId = 1;
	
// 	printf("Adding to gui\n");

// 	if ( pGui )
// 	    pGui->removeAllKSpreadTables();
// 	delete pMap;

// 	pMap = map2;

// 	if ( pGui )
// 	{
// 	    KSpreadTable *t;
// 	    for ( t = pMap->firstKSpreadTable(); t != 0L; t = pMap->nextTable() )
// 		pGui->addKSpreadTable( t );
// 	}
	
// 	pMap->initAfterLoading();
// 	tableId = pMap->count() + 1;
	
// 	if ( pGui )
// 	    pGui->setActiveKSpreadTable( pMap->firstTable() );
//     }
//     else
// 	delete map2;

//     printf("... Done map\n");

//     return TRUE;
// */
  return false;
}


/*========================== open ================================*/
CORBA::Boolean KPresenterDocument_impl::open(const char *_filename)
{
  return open(_filename);;
}

/*========================== save as =============================*/
CORBA::Boolean KPresenterDocument_impl::saveAs(const char *_filename,const char *_format)
{
  return save(_filename);
}
  
/*========================= create a view ========================*/
OPParts::View_ptr KPresenterDocument_impl::createView()
{
  KPresenterView_impl *p = new KPresenterView_impl(0L);
  p->setDocument(this);
  
  return OPParts::View::_duplicate(p);
}

/*========================== view list ===========================*/
void KPresenterDocument_impl::viewList(OPParts::Document::ViewList*& _list)
{
  (*_list).length(m_lstViews.count());

  int i = 0;
  QListIterator<KPresenterView_impl> it(m_lstViews);
  for(;it.current();++it)
    (*_list)[i++] = OPParts::View::_duplicate(it.current());
}

/*======================== paperformatstring =====================*/
QString KPresenterDocument_impl::paperFormatString()
{
  switch(pageLayout().format)
    {
    case PG_DIN_A5:
      return QString("A5");
    case PG_DIN_A4:
      return QString("A4");
    case PG_DIN_A3:
      return QString("A3");
    case PG_US_LETTER:
      return QString("Letter");
    case PG_US_LEGAL:
	return QString("Legal");
    case PG_SCREEN:
      return QString("Screen");
    case PG_CUSTOM:
      QString tmp;
      tmp.sprintf("%fx%f",pageLayout().width,pageLayout().height);
      return QString(tmp);
    }
  assert(0);
}

/*=================== orientation ================================*/
const char* KPresenterDocument_impl::orientationString()
{
  switch(pageLayout().orientation)
    {
    case PG_PORTRAIT:
      return "Portrait";
    case PG_LANDSCAPE:
      return "Landscape";
    }
  assert(0);
}

/*========================== output formats ======================*/
QStrList KPresenterDocument_impl::outputFormats()
{
  return new QStrList();
}

/*========================== input formats =======================*/
QStrList KPresenterDocument_impl::inputFormats()
{
  return new QStrList();
}

/*========================= add view =============================*/
void KPresenterDocument_impl::addView(KPresenterView_impl *_view)
{
  m_lstViews.append(_view);
}

/*======================== remove view ===========================*/
void KPresenterDocument_impl::removeView(KPresenterView_impl *_view)
{
  m_lstViews.setAutoDelete(false);
  m_lstViews.removeRef(_view);
  m_lstViews.setAutoDelete(true);
}

/*========================= insert an object =====================*/
void KPresenterDocument_impl::insertObject(const QRect& _rect,const char* _part_name)
{
  OPParts::Document_var doc = imr_newdoc(_part_name);
  if (CORBA::is_nil(doc))
    return;
  
  KPresenterChild* ch = new KPresenterChild(this,_rect,doc);
  m_lstChildren.append(ch);
  
  emit sig_insertObject(ch);
}

/*======================= change child geometry ==================*/
void KPresenterDocument_impl::changeChildGeometry(KPresenterChild *_child,const QRect& _rect)
{
  _child->setGeometry(_rect);

  emit sig_updateChildGeometry(_child);
}

/*======================= child iterator =========================*/
QListIterator<KPresenterChild> KPresenterDocument_impl::childIterator()
{
  return QListIterator<KPresenterChild> (m_lstChildren);
}

/*===================== set page layout ==========================*/
void KPresenterDocument_impl::setPageLayout(KoPageLayout pgLayout,int diffx,int diffy)
{
  _pageLayout = pgLayout;
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    {
       if ((!pagePtr->backPix.isNull()) && (pagePtr->backPicView == BV_ZOOM))
	 {
	  QWMatrix m;
 	  m.scale((float)getPageSize(pagePtr->pageNum,diffx,diffy).width()/pagePtr->backPix.width(),
 		  (float)getPageSize(pagePtr->pageNum,diffx,diffy).height()/pagePtr->backPix.height());
 	  pagePtr->backPix.operator=(pagePtr->backPix.xForm(m));
	 }
       if (pagePtr->backType == BT_CLIP)
	 {
	   pagePtr->pic->resize(getPageSize(pagePtr->pageNum,diffx,diffy).width(),
				getPageSize(pagePtr->pageNum,diffx,diffy).height());
	 }
       pagePtr->cPix->resize(getPageSize(pagePtr->pageNum,diffx,diffy).width(),
			     getPageSize(pagePtr->pageNum,diffx,diffy).height());
       emit restoreBackColor(pagePtr->pageNum-1);
    }

  repaint(true);
}

/*==================== insert a new page =========================*/
unsigned int KPresenterDocument_impl::insertNewPage(int diffx,int diffy)
{
  _pageNums++;

  pagePtr = new Background;
  pagePtr->pageNum = _pageNums;
  pagePtr->backType = BT_COLOR;
  pagePtr->backPicView = BV_CENTER;
  pagePtr->backPic = 0;
  pagePtr->backClip = 0;
  pagePtr->pic = new BackPic(0);
  pagePtr->pic->resize(getPageSize(pagePtr->pageNum,diffx,diffy).width(),
		       getPageSize(pagePtr->pageNum,diffx,diffy).height());
  pagePtr->pic->move(getPageSize(pagePtr->pageNum,diffx,diffy).x(),
 		     getPageSize(pagePtr->pageNum,diffx,diffy).y());
  pagePtr->pic->hide();
  pagePtr->backColor1.operator=(white);
  pagePtr->backColor2.operator=(white);
  pagePtr->bcType = BCT_PLAIN;
  pagePtr->cPix = new QPixmap(getPageSize(pagePtr->pageNum,diffx,diffy).width(),
			      getPageSize(pagePtr->pageNum,diffx,diffy).height()); 
  _pageList.append(pagePtr);

  spPCPtr = new SpPageConfiguration;
  spPCPtr->time = 0;
  _spPageConfig.append(spPCPtr);

  emit restoreBackColor(_pageNums-1);
  repaint(true);

  return _pageNums;
}

/*==================== set background color ======================*/
void KPresenterDocument_impl::setBackColor(unsigned int pageNum,QColor backColor1,QColor backColor2,BCType bcType)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    {
      if (pagePtr->pageNum == pageNum)
	{
	  pagePtr->backColor1.operator=(backColor1);
	  pagePtr->backColor2.operator=(backColor2);
	  pagePtr->bcType = bcType;
	  emit restoreBackColor(pageNum-1);
	  return;
	}
    }
}

/*==================== set background picture ====================*/
void KPresenterDocument_impl::setBackPic(unsigned int pageNum,const char* backPic)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    {
      if (pagePtr->pageNum == pageNum)
	{
	  pagePtr->backPic = qstrdup(backPic);
	  pagePtr->backPix.load(pagePtr->backPic);
	  emit restoreBackColor(pageNum-1);
	  return;
	}
    }
}

/*==================== set background clipart ====================*/
void KPresenterDocument_impl::setBackClip(unsigned int pageNum,const char* backClip)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    {
      if (pagePtr->pageNum == pageNum)
	{
	  pagePtr->backClip = qstrdup(backClip);
	  if (backClip)
	    pagePtr->pic->setClipart(backClip);
	  emit restoreBackColor(pageNum-1);
	  return;
	}
    }
}

/*================= set background pic view ======================*/
void KPresenterDocument_impl::setBPicView(unsigned int pageNum,BackView picView)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    {
      if (pagePtr->pageNum == pageNum)
	{
	  pagePtr->backPicView = picView;
	  if ((picView == BV_ZOOM) && (!pagePtr->backPix.isNull()))
	    {
	      QWMatrix m;
	      m.scale((float)getPageSize(pagePtr->pageNum,0,0).width()/pagePtr->backPix.width(),
		      (float)getPageSize(pagePtr->pageNum,0,0).height()/pagePtr->backPix.height());
	      pagePtr->backPix.operator=(pagePtr->backPix.xForm(m));
	    }
	  emit restoreBackColor(pageNum-1);
	  return;
	}
    }
}

/*==================== set background type =======================*/
void KPresenterDocument_impl::setBackType(unsigned int pageNum,BackType backType)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    {
      if (pagePtr->pageNum == pageNum)
	{
	  pagePtr->backType = backType;
	  emit restoreBackColor(pageNum-1);
	  repaint(true);
	  return;
	}
    }
}

/*===================== set pen and brush ========================*/
bool KPresenterDocument_impl::setPenBrush(QPen pen,QBrush brush,int diffx,int diffy)
{
  bool ret = false;
  unsigned int i;

  if (!_objList.isEmpty())
    {
      for (i = 0;i <= _objList.count()-1;i++)
	{
	  if (_objList.at(i)->isSelected)
	    {
	      objPtr = _objList.at(i);
	      if (objPtr->objType != OT_TEXT && objPtr->objType != OT_PICTURE
		  && objPtr->objType != OT_CLIPART)
		{
		  objPtr->graphObj->setObjPen(pen);
		  objPtr->graphObj->setObjBrush(brush);
		  repaint(objPtr->ox,objPtr->oy,
			  objPtr->ow,objPtr->oh,TRUE);
		  ret = true;
		}
	    }      
	}
    }
  return ret;
}

/*=================== get background type ========================*/
BackType KPresenterDocument_impl::getBackType(unsigned int pageNum)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    if (pagePtr->pageNum == pageNum) return pagePtr->backType;
  return BT_COLOR;
}

/*=================== get background pic view ====================*/
BackView KPresenterDocument_impl::getBPicView(unsigned int pageNum)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    if (pagePtr->pageNum == pageNum) return pagePtr->backPicView;
  return BV_TILED;
}

/*=================== get background picture =====================*/
const char* KPresenterDocument_impl::getBackPic(unsigned int pageNum)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    if (pagePtr->pageNum == pageNum)
      return pagePtr->backPic;
  return 0;
}

/*=================== get background clipart =====================*/
const char* KPresenterDocument_impl::getBackClip(unsigned int pageNum)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    if (pagePtr->pageNum == pageNum) return pagePtr->backClip;
  return 0;
}

/*=================== get background color 1 ======================*/
QColor KPresenterDocument_impl::getBackColor1(unsigned int pageNum)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    if (pagePtr->pageNum == pageNum) return pagePtr->backColor1;
  return white;
}

/*=================== get background color 2 ======================*/
QColor KPresenterDocument_impl::getBackColor2(unsigned int pageNum)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    if (pagePtr->pageNum == pageNum) return pagePtr->backColor2;
  return white;
}

/*=================== get background color type ==================*/
BCType KPresenterDocument_impl::getBackColorType(unsigned int pageNum)
{
  for (pagePtr=_pageList.first();pagePtr != 0;pagePtr=_pageList.next())
    if (pagePtr->pageNum == pageNum) return pagePtr->bcType;
  return BCT_PLAIN;
}

/*========================= get pen ==============================*/
QPen KPresenterDocument_impl::getPen(QPen pen)
{
  if (!_objList.isEmpty())
    {
      for (unsigned int i = 0;i <= _objList.count()-1;i++)
	{
	  if (_objList.at(i)->isSelected)
	    {
	      objPtr = _objList.at(i);
	      if (objPtr->objType != OT_TEXT && objPtr->objType != OT_PICTURE
		  && objPtr->objType != OT_CLIPART)
		return objPtr->graphObj->getObjPen();
	    }      
	}
    }
  return pen;
}

/*========================= get brush =============================*/
QBrush KPresenterDocument_impl::getBrush(QBrush brush)
{
  if (!_objList.isEmpty())
    {
      for (unsigned int i = 0;i <= _objList.count()-1;i++)
	{
	  if (_objList.at(i)->isSelected)
	    {
	      objPtr = _objList.at(i);
	      if (objPtr->objType != OT_TEXT && objPtr->objType != OT_PICTURE
		  && objPtr->objType != OT_CLIPART)
		return objPtr->graphObj->getObjBrush();
	    }      
	}
    }
  return brush;
}

/*======================== raise objects =========================*/
void KPresenterDocument_impl::raiseObjs(int diffx,int diffy)
{
  if (!_objList.isEmpty())
    {
      for (unsigned int i = 0;i <= _objList.count()-1;i++)
	{
	  objPtr = _objList.at(i);
	  if (objPtr->isSelected)
	    {
	      _objList.take(i);
	      _objList.append(objPtr);
	      repaint(objPtr->ox,objPtr->oy,
		      objPtr->ow,objPtr->oh,TRUE);
	    
	    }
	}      
      reArrangeObjs();
    }
}

/*======================== lower objects =========================*/
void KPresenterDocument_impl::lowerObjs(int diffx,int diffy)
{
  if (!_objList.isEmpty())
    {
      for (unsigned int i = 0;i <= _objList.count()-1;i++)
	{
	  objPtr = _objList.at(i);
	  if (objPtr->isSelected)
	    {
	      _objList.take(i);
	      _objList.insert(0,objPtr);
	      repaint(objPtr->ox,objPtr->oy,
		      objPtr->ow,objPtr->oh,false);
	    }
	}      
      reArrangeObjs();
    }
}

/*=================== insert a picture ==========================*/
void KPresenterDocument_impl::insertPicture(const char *filename,int diffx,int diffy)
{
  QApplication::setOverrideCursor(waitCursor);
  
  if (filename)
    {
      QPixmap pix(filename);
      if (!pix.isNull())
	{
	  _objNums++;
	  objPtr = new PageObjects;
	  objPtr->isSelected = true;
	  objPtr->objNum = _objNums;
	  objPtr->objType = OT_PICTURE;
	  objPtr->ox = diffx + 10;
	  objPtr->oy = diffy + 10;
	  objPtr->ow = pix.width();
	  objPtr->oh = pix.height();
	  objPtr->graphObj = new GraphObj(0,"graphObj",OT_PICTURE,QString(filename));
	  objPtr->graphObj->resize(objPtr->ow,objPtr->oh);
	  objPtr->graphObj->loadPixmap();
	  _objList.append(objPtr);
	  repaint(objPtr->ox,objPtr->oy,
		  objPtr->ow,objPtr->oh,false);
	}
    }
  QApplication::restoreOverrideCursor();
}

/*=================== insert a clipart ==========================*/
void KPresenterDocument_impl::insertClipart(const char *filename,int diffx,int diffy)
{
  QApplication::setOverrideCursor(waitCursor);
  
  if (filename)
    {
      _objNums++;
      objPtr = new PageObjects;
      objPtr->isSelected = true;
      objPtr->objNum = _objNums;
      objPtr->objType = OT_CLIPART;
      objPtr->ox = diffx + 10;
      objPtr->oy = diffy + 10;
      objPtr->ow = 150;
      objPtr->oh = 150;
      objPtr->graphObj = new GraphObj(0,"graphObj",OT_CLIPART,QString(filename));
      objPtr->graphObj->resize(objPtr->ow,objPtr->oh);
      objPtr->graphObj->loadClipart();
      _objList.append(objPtr);
      repaint(objPtr->ox,objPtr->oy,
	      objPtr->ow,objPtr->oh,false);
    }
  QApplication::restoreOverrideCursor();
}

/*======================= change picture ========================*/
void KPresenterDocument_impl::changePicture(const char *filename,int diffx,int diffy)
{
  QApplication::setOverrideCursor(waitCursor);

  if (filename)
    {
      QPixmap pix(filename);
      if (!_objList.isEmpty() && !pix.isNull())
	{
	  for (unsigned int i = 0;i <= _objList.count()-1;i++)
	    {
	      if (_objList.at(i)->isSelected)
		{
		  objPtr = _objList.at(i);
		  if (objPtr->objType == OT_PICTURE)
		    {
		      objPtr->graphObj->setFileName(QString(filename));
		      objPtr->graphObj->loadPixmap();
		      objPtr->isSelected = false;
		      objPtr->graphObj->resize(pix.size());
		      repaint(objPtr->ox,objPtr->oy,
			      objPtr->ow,objPtr->oh,false);
		      objPtr = _objList.at(i);
		      objPtr->ow = pix.width();
		      objPtr->oh = pix.height();
		      objPtr->isSelected = true;
		      repaint(objPtr->ox,objPtr->oy,
			      objPtr->ow,objPtr->oh,false);
		      break;
		    }
		}      
	    }
	}
    }
 
  QApplication::restoreOverrideCursor();
}

/*======================= change clipart ========================*/
void KPresenterDocument_impl::changeClipart(const char *filename,int diffx,int diffy)
{
  QApplication::setOverrideCursor(waitCursor);

  if (filename)
    {
      if (!_objList.isEmpty())
	{
	  for (unsigned int i = 0;i <= _objList.count()-1;i++)
	    {
	      if (_objList.at(i)->isSelected)
		{
		  objPtr = _objList.at(i);
		  if (objPtr->objType == OT_CLIPART)
		    {
		      objPtr->graphObj->setFileName(QString(filename));
		      objPtr->graphObj->loadClipart();
		      objPtr->isSelected = false;
		      repaint(objPtr->ox,objPtr->oy,
			      objPtr->ow,objPtr->oh,false);
		      objPtr = _objList.at(i);
		      objPtr->isSelected = true;
		      repaint(objPtr->ox,objPtr->oy,
			      objPtr->ow,objPtr->oh,false);
		      break;
		    }
		}      
	    }
	}
    }
 
  QApplication::restoreOverrideCursor();
}

/*===================== insert a line ===========================*/
void KPresenterDocument_impl::insertLine(QPen pen,LineType lt,int diffx,int diffy)
{
  _objNums++;
  objPtr = new PageObjects;
  objPtr->isSelected = true;
  objPtr->objType = OT_LINE;
  objPtr->objNum = _objNums;
  objPtr->ox = diffx + 10;
  objPtr->oy = diffy + 10;
  objPtr->ow = 150;
  objPtr->oh = 150;
  objPtr->graphObj = new GraphObj(0,"graphObj",OT_LINE);
  objPtr->graphObj->setObjPen(pen);
  objPtr->graphObj->setLineType(lt);
  objPtr->graphObj->resize(objPtr->ow,objPtr->oh);
  _objList.append(objPtr);
  repaint(objPtr->ox,objPtr->oy,
	  objPtr->ow,objPtr->oh,false);
}

/*===================== insert a rectangle =======================*/
void KPresenterDocument_impl::insertRectangle(QPen pen,QBrush brush,RectType rt,int diffx,int diffy)
{
  _objNums++;
  objPtr = new PageObjects;
  objPtr->isSelected = true;
  objPtr->objType = OT_RECT;
  objPtr->objNum = _objNums;
  objPtr->ox = diffx + 10;
  objPtr->oy = diffy + 10;
  objPtr->ow = 150;
  objPtr->oh = 150;
  objPtr->graphObj = new GraphObj(0,"graphObj",OT_RECT);
  objPtr->graphObj->setObjPen(pen);
  objPtr->graphObj->setObjBrush(brush);
  objPtr->graphObj->setRectType(rt);
  objPtr->graphObj->setRnds(_xRnd,_yRnd);
  objPtr->graphObj->resize(objPtr->ow,objPtr->oh);
  _objList.append(objPtr);
  repaint(objPtr->ox,objPtr->oy,
	  objPtr->ow,objPtr->oh,false);
}

/*===================== insert a circle or ellipse ===============*/
void KPresenterDocument_impl::insertCircleOrEllipse(QPen pen,QBrush brush,int diffx,int diffy)
{
  _objNums++;
  objPtr = new PageObjects;
  objPtr->isSelected = true;
  objPtr->objType = OT_CIRCLE;
  objPtr->objNum = _objNums;
  objPtr->ox = diffx + 10;
  objPtr->oy = diffy + 10;
  objPtr->ow = 150;
  objPtr->oh = 150;
  objPtr->graphObj = new GraphObj(0,"graphObj",OT_CIRCLE);
  objPtr->graphObj->setObjPen(pen);
  objPtr->graphObj->setObjBrush(brush);
  objPtr->graphObj->resize(objPtr->ow,objPtr->oh);
  _objList.append(objPtr);
  repaint(objPtr->ox,objPtr->oy,
	  objPtr->ow,objPtr->oh,false);
}

/*===================== insert a textobject =====================*/
void KPresenterDocument_impl::insertText(int diffx,int diffy)
{
  _objNums++;
  objPtr = new PageObjects;
  objPtr->isSelected = true;
  objPtr->objNum = _objNums;
  objPtr->objType = OT_TEXT;
  objPtr->ox = diffx + 10;
  objPtr->oy = diffy + 10;
  objPtr->ow = 150;
  objPtr->oh = 150;
  objPtr->textObj = new KTextObject(0,"textObj",KTextObject::PLAIN);
  objPtr->textObj->setBackgroundColor(_txtBackCol);
  //objPtr->textObj->breakLines(objPtr->ow);
  objPtr->textObj->resize(objPtr->ow,objPtr->oh);
  objPtr->textObj->setShowCursor(false);
  _objList.append(objPtr);
  repaint(objPtr->ox,objPtr->oy,
	  objPtr->ow,objPtr->oh,false);
}

/*======================= insert an autoform ====================*/
void KPresenterDocument_impl::insertAutoform(QPen pen,QBrush brush,const char *fileName,int diffx,int diffy)
{
  _objNums++;
  objPtr = new PageObjects;
  objPtr->objType = OT_AUTOFORM;
  objPtr->isSelected = true;
  objPtr->objNum = _objNums;
  objPtr->ox = diffx + 10;
  objPtr->oy = diffy + 10;
  objPtr->ow = 150;
  objPtr->oh = 150;
  objPtr->graphObj = new GraphObj(0,"graphObj",OT_AUTOFORM,QString(fileName));
  objPtr->graphObj->setObjPen(pen);
  objPtr->graphObj->setObjBrush(brush);
  _objList.append(objPtr);
  objPtr->graphObj->resize(objPtr->ow,objPtr->oh);
  repaint(objPtr->ox,objPtr->oy,
 	  objPtr->ow,objPtr->oh,false);
}

/*=================== repaint all views =========================*/
void KPresenterDocument_impl::repaint(bool erase)
{
  if (!m_lstViews.isEmpty())
    {
      for (viewPtr = m_lstViews.first();viewPtr != 0;viewPtr = m_lstViews.next())
	viewPtr->repaint(erase);
    }
}

/*=================== repaint all views =========================*/
void KPresenterDocument_impl::repaint(unsigned int x,unsigned int y,unsigned int w,
				      unsigned int h,bool erase)
{
  if (!m_lstViews.isEmpty())
    {
      for (viewPtr = m_lstViews.first();viewPtr != 0;viewPtr = m_lstViews.next())
	{
	  x -= viewPtr->getDiffX();
	  y -= viewPtr->getDiffY();
	  viewPtr->repaint(x,y,w,h,erase);
	}
    }
}

/*================== get size of page ===========================*/
QRect KPresenterDocument_impl::getPageSize(unsigned int num,int diffx,int diffy)
{
  double fact = 1;
  if (_pageLayout.unit == PG_CM) fact = 10;
  if (_pageLayout.unit == PG_INCH) fact = 25.4;

  int pw,ph,bl = (int)(_pageLayout.left * fact * 100)/100,br = (int)(_pageLayout.right * fact * 100)/100;
  int bt = (int)(_pageLayout.top * fact * 100)/100,bb = (int)(_pageLayout.bottom * fact * 100)/100;
  int wid = (int)(_pageLayout.width * fact * 100)/100,hei = (int)(_pageLayout.height * fact * 100)/100;
  
  pw = wid*(int)(MM_TO_POINT * 100) / 100 - 
    (bl + br);
  ph = hei*(int)(MM_TO_POINT * 100) / 100 -
    (bt + bb);
  QRect rect(10 - diffx,(10 + ph * (num - 1) +
			 (num - 1) * 10) - diffy,pw,ph);
  return rect;
}

/*==================== rearrange objects =========================*/
void KPresenterDocument_impl::reArrangeObjs()
{
  _objNums = 0;
  if (!_objList.isEmpty())
    {
      for (objPtr=_objList.first();objPtr != 0;objPtr=_objList.next())
	{
	  _objNums++;
	  objPtr->objNum = _objNums;
	}
    }
}

/*======================= delete objects =========================*/
void KPresenterDocument_impl::deleteObjs()
{
  bool changed = false;
  
  if (!_objList.isEmpty())
    {
      for (unsigned int i=0;i < _objList.count();i++)
	{
	  objPtr = _objList.at(i);
	  if (objPtr->isSelected)
	    {
	      _objList.remove(i);
	      i--;
	      changed = true;
	    }
	}
      if (changed) repaint(true);
      reArrangeObjs();
    }      
}

/*======================= rotate objects =========================*/
void KPresenterDocument_impl::rotateObjs()
{
}

