/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <qcursor.h>
#include <qiconset.h>
#include <qpainter.h>
#include <qbuttongroup.h>

#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kstatusbar.h>

#include "karbon_factory.h"
#include "karbon_part.h"
#include "karbon_view.h"
#include "vccmd_text.h"
#include "vmcmd_group.h"
#include "vctool_ellipse.h"
#include "vctool_polygon.h"
#include "vctool_rectangle.h"
#include "vctool_roundrect.h"
#include "vctool_sinus.h"
#include "vctool_spiral.h"
#include "vctool_star.h"
#include "vmcmd_delete.h"
#include "vmcmd_fill.h"
#include "vmcmd_stroke.h"
#include "vmdlg_solidfill.h"
#include "vmdlg_stroke.h"
#include "vmpanel_color.h"
#include "vmtool_handle.h"
#include "vmtool_select.h"
#include "vmtool_rotate.h"
#include "vmtool_scale.h"
#include "vmtool_shear.h"
#include "vpainter.h"
#include "vpainterfactory.h"
#include "vtoolcontainer.h"
#include "vtext.h"

#include <kdebug.h>
#include <koMainWindow.h>

#include "karbon_view_iface.h"

KarbonView::KarbonView( KarbonPart* part, QWidget* parent, const char* name )
	: KoView( part, parent, name ), m_part( part )
{
	setInstance( KarbonFactory::instance() );

	setXMLFile( QString::fromLatin1( "karbon.rc" ) );
	initActions();
	m_dcop = 0;
	dcopObject(); // build it


	m_canvas = new VCanvas( this, part );
	m_canvas->viewport()->installEventFilter( this );
	m_canvas->setGeometry( 0, 0, width(), height() );

	// set up factory
	m_painterFactory = new VPainterFactory;
	m_painterFactory->setPainter( canvasWidget()->viewport(), width(), height() );
	m_painterFactory->setEditPainter( canvasWidget()->viewport(), width(), height() );

	// set up status bar message
	m_status = new KStatusBarLabel( QString::null, 0, statusBar() );
	m_status->setAlignment( AlignLeft | AlignVCenter );
	m_status->setMinimumWidth( 300 );
	addStatusBarItem( m_status, 0 );

	// initial tool is select-tool:
	selectTool();
}

KarbonView::~KarbonView()
{
	delete m_dcop;
	delete m_painterFactory;
	delete m_canvas;
	m_canvas = 0L;
	//delete m_toolbox;
	delete m_status;
}

DCOPObject* KarbonView::dcopObject()
{
    if ( !m_dcop )
	m_dcop = new KarbonViewIface( this );

    return m_dcop;
}


void
KarbonView::updateReadWrite( bool /*rw*/ )
{
}

void
KarbonView::resizeEvent( QResizeEvent* /*event*/ )
{
	m_painterFactory->painter()->resize( width(), height() );
	m_painterFactory->editpainter()->resize( width(), height() );
	m_canvas->resize( width(), height() );
}

void
KarbonView::editCut()
{
}

void
KarbonView::editCopy()
{
}

void
KarbonView::editPaste()
{
	VObjectListIterator itr( m_part->selection() );
	VObjectList selection;
	for ( ; itr.current() ; ++itr )
	{
		VObject *temp = itr.current()->clone();
		temp->transform( QWMatrix().translate( Karbon::m_copyOffset, Karbon::m_copyOffset ) );
		selection.append( temp );
	}
	m_part->deselectAllObjects();
	// calc new selection
	VObjectListIterator itr2( selection );
	for ( ; itr2.current() ; ++itr2 )
	{
		m_part->insertObject( itr2.current() );
		m_part->selectObject( *( itr2.current() ) );
	}
	m_part->repaintAllViews();
}

void
KarbonView::editSelectAll()
{
	m_part->selectAllObjects();
	//handleTool();
	if( m_part->selection().count() > 0 )
		m_part->repaintAllViews();
}

void
KarbonView::editDeselectAll()
{
	if( m_part->selection().count() > 0 )
	{
		m_part->deselectAllObjects();
		m_part->repaintAllViews();
	}
}

void
KarbonView::editDeleteSelection()
{
	m_part->addCommand( new VMCmdDelete( m_part ), true );

	selectTool();
}

void
KarbonView::editPurgeHistory()
{
// TODO: check for history size != 0
	if( KMessageBox::warningContinueCancel( this,
		i18n( "This action cannot be undone later. Do you really want to continue?" ),
		i18n( "Purge History" ),
		i18n( "C&ontinue" ),	// TODO: is there a constant for this?
		"edit_purge_history" ) )
	{
		m_part->purgeHistory();
	}
}



void
KarbonView::selectionMoveToTop()
{
	m_part->moveSelectionToTop();
}

void
KarbonView::selectionMoveUp()
{
	m_part->moveSelectionUp();
}

void
KarbonView::selectionMoveDown()
{
	m_part->moveSelectionDown();
}

void
KarbonView::selectionMoveToBottom()
{
	m_part->moveSelectionToBottom();
}

void
KarbonView::groupSelection()
{
	m_part->addCommand( new VMCmdGroup( m_part ), true );
}

void
KarbonView::ungroupSelection()
{
}

void
KarbonView::objectTrafoTranslate()
{
}

void
KarbonView::objectTrafoScale()
{
}

void
KarbonView::objectTrafoRotate()
{
}

void
KarbonView::objectTrafoShear()
{
}



void
KarbonView::ellipseTool()
{
	m_status->setText( i18n( "Ellipse" ) );
	s_currentTool = VCToolEllipse::instance( m_part );
	m_canvas->viewport()->setCursor( QCursor( crossCursor ) );
}

void
KarbonView::polygonTool()
{
	if( shell()->rootView() == this )
	{
		if( s_currentTool == VCToolPolygon::instance( m_part ) )
			( (VShapeTool *) s_currentTool )->showDialog();
		else
		{
			s_currentTool = VCToolPolygon::instance( m_part );
			m_canvas->viewport()->setCursor( QCursor( crossCursor ) );
		}
	}
}

void
KarbonView::rectangleTool()
{
	s_currentTool = VCToolRectangle::instance( m_part );
	m_canvas->viewport()->setCursor( QCursor( crossCursor ) );
}

void
KarbonView::roundRectTool()
{
	if( shell()->rootView() == this )
	{
		if( s_currentTool == VCToolRoundRect::instance( m_part ) )
			( (VShapeTool *) s_currentTool )->showDialog();
		else
		{
			s_currentTool = VCToolRoundRect::instance( m_part );
			m_canvas->viewport()->setCursor( QCursor( crossCursor ) );
		}
	}
}

void
KarbonView::selectTool()
{
	m_status->setText( i18n( "Selection" ) );
	s_currentTool = VMToolSelect::instance( m_part );
	m_canvas->viewport()->setCursor( QCursor( arrowCursor ) );
	m_selectToolAction->setChecked( true );
}

void
KarbonView::rotateTool()
{
	s_currentTool = VMToolRotate::instance( m_part );
	m_canvas->viewport()->setCursor( QCursor( arrowCursor ) );
	m_rotateToolAction->setChecked( true );
}

void
KarbonView::scaleTool()
{
	kdDebug() << "KarbonView::scaleTool()" << endl;
	s_currentTool = VMToolScale::instance( m_part );
	m_canvas->viewport()->setCursor( QCursor( arrowCursor ) );
	m_scaleToolAction->setChecked( true );
}

void
KarbonView::textTool()
{
    kdDebug() << "KarbonView::textTool()" << endl;
	QFont f;

	f.setFamily( m_setFontFamily->font() );
	f.setPointSize( m_setFontSize->fontSize() );
	f.setBold( m_setFontBold->isChecked() );
	f.setItalic( m_setFontItalic->isChecked() );

	// TODO : find a way to edit the text, no predefined strings
	m_part->addCommand( new VCCmdText( m_part, f, "KARBON" ), true );
}

void
KarbonView::shearTool()
{
	kdDebug() << "KarbonView::shearTool()" << endl;
	s_currentTool = VMToolShear::instance( m_part );
	m_canvas->viewport()->setCursor( QCursor( arrowCursor ) );
	m_shearToolAction->setChecked( true );
}

void
KarbonView::sinusTool()
{
	if( shell()->rootView() == this )
	{
		if( s_currentTool == VCToolSinus::instance( m_part ) )
			( (VShapeTool *) s_currentTool )->showDialog();
		else
		{
			s_currentTool = VCToolSinus::instance( m_part );
			m_canvas->viewport()->setCursor( QCursor( crossCursor ) );
		}
	}
}

void
KarbonView::spiralTool()
{
	if( shell()->rootView() == this )
	{
		if( s_currentTool == VCToolSpiral::instance( m_part ) )
			( (VShapeTool *) s_currentTool )->showDialog();
		else
		{
			s_currentTool = VCToolSpiral::instance( m_part );
			m_canvas->viewport()->setCursor( QCursor( crossCursor ) );
		}
	}
}

void
KarbonView::starTool()
{
	if( shell()->rootView() == this )
	{
		if( s_currentTool == VCToolStar::instance( m_part ) )
			( (VShapeTool *) s_currentTool )->showDialog();
		else
		{
			s_currentTool = VCToolStar::instance( m_part );
			m_canvas->viewport()->setCursor( QCursor( crossCursor ) );
		}
	}
}


void
KarbonView::handleTool()
{
	s_currentTool = VMToolHandle::instance( m_part );
	m_canvas->viewport()->setCursor( QCursor( arrowCursor ) );
}

void
KarbonView::viewModeChanged()
{
	if( m_viewAction->currentItem() == 1 )
	{
		m_painterFactory->setWireframePainter( canvasWidget()->viewport(), width(), height() );
	}
	else
	{
		m_painterFactory->setPainter( canvasWidget()->viewport(), width(), height() );
	}
	m_canvas->repaintAll();
}

void
KarbonView::zoomChanged()
{
	bool bOK;
	double zoomFactor = m_zoomAction->currentText().toDouble( &bOK ) / 100.0;
	m_canvas->setZoomFactor( zoomFactor );
	// TODO : I guess we should define a document size member at this point...
	m_canvas->resizeContents( 800 * zoomFactor, 600 * zoomFactor );
	m_canvas->repaintAll();
}

void
KarbonView::solidFillClicked()
{
	if( shell()->rootView() == this )
	{
		VMDlgSolidFill* m_solidFillDialog = new VMDlgSolidFill( m_part );
		m_solidFillDialog->show();
	}
}

void
KarbonView::strokeClicked()
{
	if( shell()->rootView() == this )
	{
		VMDlgStroke* m_strokeDialog = new VMDlgStroke( m_part );
		m_strokeDialog->show();
	}
}

void
KarbonView::slotStrokeColorChanged( const QColor &c )
{
	kdDebug() << "In KarbonView::slotStrokeColorChanged" << endl;
	VColor color;
	float r = float( c.red() ) / 255.0, g = float( c.green() ) / 255.0, b = float( c.blue() ) / 255.0;

	color.setValues( &r, &g, &b, 0L );
	m_part->setDefaultStrokeColor( color );

	m_part->addCommand( new VMCmdStroke( m_part, color ), true );
}

void
KarbonView::slotFillColorChanged( const QColor &c )
{
	kdDebug() << "In KarbonView::slotFillColorChanged" << endl;
	VColor color;
	float r = float( c.red() ) / 255.0, g = float( c.green() ) / 255.0, b = float( c.blue() ) / 255.0;

	color.setValues( &r, &g, &b, 0L );
	m_part->setDefaultFillColor( color );

	m_part->addCommand( new VMCmdFill( m_part, color ), true );
}

void
KarbonView::viewColorManager()
{
	VColorPanel* m_ColorPanel = new VColorPanel ( this );
	m_ColorPanel->show();
}

void
KarbonView::refreshView()
{
	m_canvas->repaintAll();
}

void
KarbonView::initActions()
{
	// edit ----->
	KStdAction::cut( this,
		SLOT( editCut() ), actionCollection(), "edit_cut" );
	KStdAction::copy( this,
		SLOT( editCopy() ), actionCollection(), "edit_copy");
	KStdAction::paste( this,
		SLOT( editPaste() ), actionCollection(), "edit_paste" );
  	KStdAction::selectAll( this,
		SLOT( editSelectAll() ), actionCollection(), "edit_select_all" );
	new KAction(
		i18n( "&Deselect All" ), QKeySequence( "Ctrl+D" ), this,
		SLOT( editDeselectAll() ), actionCollection(), "edit_deselect_all" );
  	new KAction(
		i18n( "D&elete" ), "editdelete", QKeySequence( "Del" ), this,
		SLOT( editDeleteSelection() ), actionCollection(), "edit_delete" );
	new KAction(
		i18n( "&History" ), 0, 0, this,
		SLOT( editPurgeHistory() ), actionCollection(), "edit_purge_history" );
	// edit <-----

	// object ----->
	new KAction(
		i18n( "Bring To &Front" ), 0, QKeySequence("Shift+PgUp"), this,
		SLOT( selectionMoveToTop() ), actionCollection(), "object_move_totop" );
	new KAction(
		i18n( "&Raise" ), 0, QKeySequence("Ctrl+PgUp"), this,
		SLOT( selectionMoveUp() ), actionCollection(), "object_move_up" );
	new KAction(
		i18n( "&Lower" ), 0, QKeySequence("Ctrl+PgDown"), this,
		SLOT( selectionMoveDown() ), actionCollection(), "object_move_down" );
	new KAction(
		i18n( "Send To &Back" ), 0, QKeySequence("Shift+PgDown"), this,
		SLOT( selectionMoveToBottom() ), actionCollection(), "object_move_tobottom" );
	new KAction(
		i18n( "Group objects" ), 0, QKeySequence("Shift+G"), this,
		SLOT( groupSelection() ), actionCollection(), "selection_group" );
	new KAction(
		i18n( "Ungroup objects" ), 0, QKeySequence("Shift+U"), this,
		SLOT( ungroupSelection() ), actionCollection(), "selection_ungroup" );
	new KAction(
		i18n( "&Translate" ), "translate", 0, this,
		SLOT( objectTrafoTranslate() ), actionCollection(), "object_trafo_translate" );
	new KAction(
		i18n( "&Scale" ), "scale", 0, this,
		SLOT( objectTrafoScale() ), actionCollection(), "object_trafo_scale" );
	new KAction(
		i18n( "&Rotate" ), "rotate", 0, this,
		SLOT( objectTrafoRotate() ), actionCollection(), "object_trafo_rotate" );
	new KAction(
		i18n( "S&hear" ), "shear", 0, this,
		SLOT( objectTrafoShear() ), actionCollection(), "object_trafo_shear" );
	// object <-----

	// tools ----->
	m_ellipseToolAction = new KToggleAction(
		i18n( "&Ellipse" ), "ellipse", 0, this,
		SLOT( ellipseTool() ), actionCollection(), "tool_ellipse" );
	m_polygonToolAction = new KToggleAction(
		i18n( "&Polygon" ), "polygon", 0, this,
		SLOT( polygonTool() ), actionCollection(), "tool_polygon" );
	m_rectangleToolAction = new KToggleAction(
		i18n( "&Rectangle" ), "rectangle", 0, this,
		SLOT( rectangleTool() ), actionCollection(), "tool_rectangle" );
	m_roundRectToolAction = new KToggleAction(
		i18n( "&Round Rectangle" ), "roundrect", 0, this,
		SLOT( roundRectTool() ), actionCollection(), "tool_roundrect" );
	m_sinusToolAction = new KToggleAction(
		i18n( "S&inus" ), "sinus", 0, this,
		SLOT( sinusTool() ), actionCollection(), "tool_sinus" );
	m_selectToolAction = new KToggleAction(
		i18n( "&Select Objects" ), "select", 0, this,
		SLOT( selectTool() ), actionCollection(), "tool_select" );
	m_rotateToolAction = new KToggleAction(
		i18n( "&Rotate Objects" ), "rotate", 0, this,
		SLOT( rotateTool() ), actionCollection(), "tool_rotate" );
	m_scaleToolAction = new KToggleAction(
		i18n( "&Scale Objects" ), "scale", 0, this,
		SLOT( scaleTool() ), actionCollection(), "tool_scale" );
	m_shearToolAction = new KToggleAction(
		i18n( "&Shear Objects" ), "shear", 0, this,
		SLOT( shearTool() ), actionCollection(), "tool_shear" );
	m_spiralToolAction = new KToggleAction(
		i18n( "S&piral" ), "spiral", 0, this,
		SLOT( spiralTool() ), actionCollection(), "tool_spiral" );
	m_starToolAction = new KToggleAction(
		i18n( "S&tar" ), "star", 0, this,
		SLOT( starTool() ), actionCollection(), "tool_star" );
	m_textToolAction = new KToggleAction(
		i18n( "Text" ), "text", 0, this,
		SLOT( textTool() ), actionCollection(), "tool_text" );

	m_ellipseToolAction->setExclusiveGroup( "Tools" );
	m_polygonToolAction->setExclusiveGroup( "Tools" );
	m_rectangleToolAction->setExclusiveGroup( "Tools" );
	m_roundRectToolAction->setExclusiveGroup( "Tools" );
	m_sinusToolAction->setExclusiveGroup( "Tools" );
	m_selectToolAction->setExclusiveGroup( "Tools" );
	m_rotateToolAction->setExclusiveGroup( "Tools" );
	m_scaleToolAction->setExclusiveGroup( "Tools" );
	m_spiralToolAction->setExclusiveGroup( "Tools" );
	m_starToolAction->setExclusiveGroup( "Tools" );
	m_textToolAction->setExclusiveGroup( "Tools" );
	// tools <-----

	// text

	m_setFontFamily = new KFontAction( i18n( "Set Font Family" ), 0, actionCollection(), "setFontFamily" );
	m_setFontFamily->setCurrentItem( 0 );

	//connect( m_setFontFamily, SIGNAL(activated(const QString&)), SLOT(setFontFamily(const QString&)) );

	m_setFontSize = new KFontSizeAction( i18n( "Set Font Size" ), 0, actionCollection(), "setFontSize" );
	m_setFontSize->setCurrentItem( 0 );
	//connect( m_setFontSize, SIGNAL(activated(const QString&)), SLOT(setFontSize(const QString&)) );

	m_setFontItalic = new KToggleAction( i18n( "&Italic" ), "text_italic", 0, actionCollection(), "setFontItalic" );
	m_setFontBold = new KToggleAction(	i18n( "&Bold" ), "text_bold", 0, actionCollection(), "setFontBold" );

	//m_setTextColor = new TKSelectColorAction( i18n("Set Text Color"), TKSelectColorAction::TextColor, actionCollection(), "setTextColor" );
	//connect( m_setTextColor, SIGNAL(activated()), SLOT(setTextColor()) );

	// view ----->
	m_viewAction = new KSelectAction(
		i18n( "View &Mode" ), 0, this,
		SLOT( viewModeChanged() ), actionCollection(), "view_mode" );

	m_zoomAction = new KSelectAction(
		i18n( "&Zoom" ), 0, this,
		SLOT( zoomChanged() ), actionCollection(), "view_zoom" );

	QStringList mstl;
	mstl
		<< i18n( "Normal" )
		<< i18n( "Wireframe" );
	m_viewAction->setItems( mstl );
	m_viewAction->setCurrentItem( 0 );
	m_viewAction->setEditable( false );

	QStringList stl;
	stl
		<< i18n( "25%" )
		<< i18n( "50%" )
		<< i18n( "100%" )
		<< i18n( "200%" )
		<< i18n( "300%" )
		<< i18n( "400%" )
		<< i18n( "800%" );
	m_zoomAction->setItems( stl );
	m_zoomAction->setEditable( true );
	m_zoomAction->setCurrentItem( 2 );

	new KAction(
		i18n( "&Color Manager" ), "colorman", 0, this,
		SLOT( viewColorManager() ), actionCollection(), "view_color_manager" );

	new KAction(
		i18n( "&Refresh" ), 0, QKeySequence("Ctrl+W"), this,
		SLOT( refreshView() ), actionCollection(), "view_refresh" );
	// view <-----

	m_toolbox = VToolContainer::instance( m_part, this );

	connect( m_toolbox, SIGNAL(selectToolActivated()), this, SLOT(selectTool()) );
	connect( m_toolbox, SIGNAL(scaleToolActivated()), this, SLOT(scaleTool()) );
	connect( m_toolbox, SIGNAL(rotateToolActivated()), this, SLOT(rotateTool()) );
	connect( m_toolbox, SIGNAL(shearToolActivated()), this, SLOT(shearTool()) );
	connect( m_toolbox, SIGNAL(ellipseToolActivated()), this, SLOT(ellipseTool()) );
	connect( m_toolbox, SIGNAL(rectangleToolActivated()), this, SLOT(rectangleTool()) );
	connect( m_toolbox, SIGNAL(roundRectToolActivated()), this, SLOT(roundRectTool()) );
	connect( m_toolbox, SIGNAL(polygonToolActivated()), this, SLOT(polygonTool()) );
	connect( m_toolbox, SIGNAL(starToolActivated()), this, SLOT(starTool()) );
	connect( m_toolbox, SIGNAL(sinusToolActivated()), this, SLOT(sinusTool()) );
	connect( m_toolbox, SIGNAL(spiralToolActivated()), this, SLOT(spiralTool()) );
	connect( m_toolbox, SIGNAL(textToolActivated()), this, SLOT(textTool()) );
	connect( m_toolbox, SIGNAL(solidFillActivated()), this, SLOT(solidFillClicked()) );
	connect( m_toolbox, SIGNAL(strokeActivated()), this, SLOT(strokeClicked()) );
	connect( m_toolbox, SIGNAL(strokeColorChanged(const QColor &)), this, SLOT(slotStrokeColorChanged(const QColor &)) );
	connect( m_toolbox, SIGNAL(fillColorChanged(const QColor &)), this, SLOT(slotFillColorChanged(const QColor &)) );
	shell()->moveDockWindow( m_toolbox, Qt::DockLeft );
	m_toolbox->show();
}

void
KarbonView::paintEverything( QPainter& /*p*/, const QRect& /*rect*/,
	bool /*transparent*/)
{
	kdDebug() << "view->paintEverything()" << endl;
}

bool
KarbonView::eventFilter( QObject* object, QEvent* event )
{
	if ( object == m_canvas->viewport() )
	{
		s_currentTool->m_part = m_part;
		return s_currentTool->eventFilter( this, event );
	}
	else
		return false;
}

#include "karbon_view.moc"

