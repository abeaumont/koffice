#include "koMainWindow.h"
#include "koFrame.h"
#include "koDocument.h"
#include "koView.h"

#include <opMainWindowIf.h>
#include <opApplication.h>
#include <opMenuBarManager.h>
#include <opToolBarManager.h>
#include <opMenu.h>

#include <qkeycode.h>

#include <kapp.h>
#include <kiconloader.h>

KoMainWindow::KoMainWindow( const char *_name )
{
  m_pFileMenu = 0L;
  m_pHelpMenu = 0L;
  m_pFrame = 0L;
  m_pKoInterface = 0L;
  
  connect( this, SIGNAL( activePartChanged( unsigned long, unsigned long ) ),
	   this, SLOT( slotActivePartChanged( unsigned long, unsigned long ) ) );

  // create the menu bar
  (void)menuBarManager();
  
  // create the toolbar manager to handle the toolbars of the embedded parts
  (void)toolBarManager();

  // build a toolbar and insert some buttons
  opToolBar()->insertButton(Icon("filenew.xpm"),TOOLBAR_NEW, SIGNAL( clicked() ), this, SLOT( slotFileNew() ), true,i18n("New"));
  opToolBar()->insertButton(Icon("fileopen.xpm"),TOOLBAR_OPEN, SIGNAL( clicked() ), this, SLOT( slotFileOpen() ),
			    true,i18n("Open File"));
  opToolBar()->insertButton(Icon("filefloppy.xpm"), TOOLBAR_SAVE, SIGNAL( clicked() ), this, SLOT( slotFileSave() ),
			    true,i18n("Save File"));
  opToolBar()->setItemEnabled( TOOLBAR_SAVE, false );
  opToolBar()->insertButton(Icon("fileprint.xpm"), TOOLBAR_PRINT, SIGNAL( clicked() ), this, SLOT( slotFilePrint() ),
			    true,i18n("Print"));
  opToolBar()->setItemEnabled( TOOLBAR_PRINT, false );

  m_pFrame = new KoFrame( this );
  setView( m_pFrame );

  // Build a default menubar with at least file and help menu
  menuBarManager()->create( 0 );
}

KoMainWindow::~KoMainWindow()
{
  menuBarManager()->clear();
}

void KoMainWindow::cleanUp()
{
  setRootPart( 0 );

  interface()->cleanUp();
}

OPMainWindowIf* KoMainWindow::interface()
{
  if ( m_pInterface == 0L )
    m_pInterface = m_pKoInterface = new KoMainWindowIf( this );
  return m_pInterface;
}

KoMainWindowIf* KoMainWindow::koInterface()
{    
  if ( m_pInterface == 0L )
    m_pInterface = m_pKoInterface = new KoMainWindowIf( this );
  return m_pKoInterface;
}

void KoMainWindow::setRootPart( unsigned long _part_id )
{
  OpenParts::Part_var part;
  if ( _part_id != 0 )
  {    
    part = interface()->findPart( _part_id );
    assert( !CORBA::is_nil( part ) );
  }
  
  m_pFrame->detach();
  
  if ( _part_id != 0 )
  {
    KOffice::View_var view = KOffice::View::_narrow( part );    
    m_pFrame->attachView( view );
  }
}

void KoMainWindow::slotActivePartChanged( unsigned long _new_part_id, unsigned long _old_part_id )
{
  menuBarManager()->clear();
  toolBarManager()->clear();
  menuBarManager()->create( _new_part_id );
  toolBarManager()->create( _new_part_id );
}

void KoMainWindow::createFileMenu( OPMenuBar* _menubar )
{
  // Do we loose control over the menubar ?
  if ( _menubar == 0L )
  {
    m_pFileMenu = 0L;
    return;
  }
  
  m_pFileMenu = new OPMenu( _menubar );

  m_idMenuFile_New = m_pFileMenu->insertItem( Icon( "filenew.xpm" ) , i18n( "&New" ), this, SLOT( slotFileNew() ), CTRL + Key_N );
  m_idMenuFile_Open = m_pFileMenu->insertItem( Icon( "fileopen.xpm" ), i18n( "&Open..." ), this, SLOT( slotFileOpen() ), CTRL + Key_O );
  m_pFileMenu->insertSeparator(-1);
  m_idMenuFile_Save = m_pFileMenu->insertItem( Icon( "filefloppy.xpm" ), i18n( "&Save" ), this, SLOT( slotFileSave() ), CTRL + Key_S );
  m_pFileMenu->setItemEnabled( m_idMenuFile_Save, false );
  
  m_idMenuFile_SaveAs = m_pFileMenu->insertItem( i18n( "&Save as..." ), this, SLOT( slotFileSaveAs() ) );
  m_pFileMenu->setItemEnabled( m_idMenuFile_SaveAs, false );

  m_pFileMenu->insertSeparator(-1);
  m_idMenuFile_Print = m_pFileMenu->insertItem( i18n( "&Print..." ), this, SLOT( slotFilePrint() ) );
  m_pFileMenu->setItemEnabled( m_idMenuFile_Print, false );
  
  m_pFileMenu->insertSeparator(-1);
  m_idMenuFile_Close = m_pFileMenu->insertItem( i18n( "&Close" ), this, SLOT( slotFileClose() ), CTRL + Key_W );
  m_pFileMenu->setItemEnabled( m_idMenuFile_Close, false );
  
  m_idMenuFile_Quit = m_pFileMenu->insertItem( i18n( "&Quit" ), this, SLOT( slotFileQuit() ), CTRL + Key_Q );

  _menubar->insertItem( i18n( "&File" ), m_pFileMenu );
}

void KoMainWindow::createHelpMenu( OPMenuBar* _menubar )
{
  // Do we loose control over the menubar ?
  if ( _menubar == 0L )
  {
    m_pHelpMenu = 0L;
    return;
  }
  
  m_pHelpMenu = _menubar->helpMenu();
  // No help menu yet ?
  if ( m_pHelpMenu == 0L )
  {    
    m_pHelpMenu = new OPMenu( _menubar );

    _menubar->insertSeparator();
    _menubar->insertItem( i18n( "&Help" ), m_pHelpMenu );
  }
  else
    m_pHelpMenu->insertSeparator();

  // Insert our item
  m_idMenuHelp_About = m_pHelpMenu->insertItem( i18n( "&About KOffice" ), this, SLOT( slotHelpAbout() ) );
}

void KoMainWindow::slotFileNew()
{
}

void KoMainWindow::slotFileOpen()
{
}

void KoMainWindow::slotFileSave()
{
}

void KoMainWindow::slotFileSaveAs()
{
}

void KoMainWindow::slotFileClose()
{
}

void KoMainWindow::slotFilePrint()
{
}

void KoMainWindow::slotFileQuit()
{
}

void KoMainWindow::slotHelpAbout()
{
}

/**************************************************
 *
 * KoMainWindowIf
 *
 **************************************************/

KoMainWindowIf::KoMainWindowIf( KoMainWindow* _main ) : OPMainWindowIf( _main )
{
  m_pKoMainWindow = _main;
  m_iMarkedPart = 0;
}

KoMainWindowIf::~KoMainWindowIf()
{
}

void KoMainWindowIf::setMarkedPart( OpenParts::Id id )
{
  m_iMarkedPart = id;
}

KOffice::Document_ptr KoMainWindowIf::document()
{
  return KOffice::Document::_duplicate( m_pKoMainWindow->document() );  
}

KOffice::View_ptr KoMainWindowIf::view()
{
  return KOffice::View::_duplicate( m_pKoMainWindow->view() );  
}

void KoMainWindowIf::partClicked( OpenParts::Id _part_id )
{
  assert( _part_id != 0 );
  
  OpenParts::Part_var part;

  // Find it
  part = findPart( _part_id );
  if( CORBA::is_nil( part ) )
  {
    cerr << "ERROR: void OPMainWindowIf::setActivePart( OpenParts::Id _id )" << endl;
    cerr << "       id " << _part_id << " is unknown" << endl;
    return;
  }
 
  KOffice::View_var view = KOffice::View::_narrow( part );
  if ( CORBA::is_nil( view ) )
  {    
    setActivePart( _part_id );
    return;
  }
  
  // Special handling for the root view, since the root view is
  // never marked
  if ( view->mode() == KOffice::View::RootMode )
  {
    if ( _part_id != m_iMarkedPart )
      unmarkPart();
    setActivePart( _part_id );
    return;
  }
  
  if ( view->isMarked() )
  {
    if ( _part_id != m_iMarkedPart )
      unmarkPart();
    setActivePart( _part_id );
    return;
  }
  else if ( !view->hasFocus() )
  {
    if ( _part_id != m_iMarkedPart )
      unmarkPart();
    view->setMarked( true );
    m_iMarkedPart = _part_id;
    return;
  }
}

void KoMainWindowIf::unmarkPart()
{
  if ( m_iMarkedPart == 0 )
    return;
  
  // Find it
  OpenParts::Part_var part = findPart( m_iMarkedPart );
  assert( !CORBA::is_nil( part ) );
  
  KOffice::View_var view = KOffice::View::_narrow( part );
  assert( !CORBA::is_nil( view ) );
  
  view->setMarked( false );

  m_iMarkedPart = 0;
}
