/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "webpresentation.h"
#include "webpresentation.moc"

#include "kpresenter_doc.h"
#include "kpresenter_view.h"
#include "page.h"

#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpalette.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qframe.h>
#include <qprogressbar.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qheader.h>
#include <qwmatrix.h>

#include <klocale.h>
#include <kcolorbtn.h>
#include <kfiledialog.h>
#include <kbuttonbox.h>
#include <ksimpleconfig.h>
#include <kimgio.h>

#include <kstddirs.h>
#include <kglobal.h>

/******************************************************************/
/* Class: KPWebPresentation					  */
/******************************************************************/

/*================================================================*/
KPWebPresentation::KPWebPresentation( KPresenterDoc *_doc, KPresenterView *_view )
    : config( QString::null )
{
    doc = _doc;
    view = _view;
    init();
}

/*================================================================*/
KPWebPresentation::KPWebPresentation( const QString &_config, KPresenterDoc *_doc, KPresenterView *_view )
    : config( _config )
{
    doc = _doc;
    view = _view;
    init();
    loadConfig();
}

/*================================================================*/
KPWebPresentation::KPWebPresentation( const KPWebPresentation &webPres )
    : config( webPres.config ), author( webPres.author ), title( webPres.title ), email( webPres.email ),
      slideTitles( webPres.slideTitles ), backColor( webPres.backColor ), titleColor( webPres.titleColor ),
      textColor( webPres.textColor ), path( webPres.path ), imgFormat( webPres.imgFormat ), zoom( webPres.zoom )
{
    doc = webPres.doc;
    view = webPres.view;
}

/*================================================================*/
void KPWebPresentation::loadConfig()
{
    if ( config.isEmpty() )
	return;

    KSimpleConfig cfg( config );
    cfg.setGroup( "General" );

    author = cfg.readEntry( "Author", author );
    title = cfg.readEntry( "Title", title );
    email = cfg.readEntry( "EMail", email );
    unsigned int num = cfg.readNumEntry( "Slides", doc->getPageNums() );

    if ( num <= doc->getPageNums() ) {
	for ( unsigned int i = 0; i < num; i++ )
	    slideTitles[ i ] = cfg.readEntry( QString( "SlideTitle[%1]" ).arg( i ), slideTitles[ i ] );
    }

    backColor = cfg.readColorEntry( "BackColor", &backColor );
    titleColor = cfg.readColorEntry( "TitleColor", &titleColor );
    textColor = cfg.readColorEntry( "TextColor", &textColor );
    path = cfg.readEntry( "Path", path );
    imgFormat = static_cast<ImageFormat>( cfg.readNumEntry( "ImageFormat", static_cast<int>( imgFormat ) ) );
    zoom = cfg.readNumEntry( "Zoom", zoom );
}

/*================================================================*/
void KPWebPresentation::saveConfig()
{
    KSimpleConfig cfg( config );
    cfg.setGroup( "General" );

    cfg.writeEntry( "Author", author );
    cfg.writeEntry( "Title", title );
    cfg.writeEntry( "EMail", email );
    cfg.writeEntry( "Slides", doc->getPageNums() );

    for ( unsigned int i = 0; i < doc->getPageNums(); i++ )
	cfg.writeEntry( QString( "SlideTitle[ %1 ]" ).arg( i ), slideTitles[ i ] );

    cfg.writeEntry( "BackColor", backColor );
    cfg.writeEntry( "TitleColor", titleColor );
    cfg.writeEntry( "TextColor", textColor );
    cfg.writeEntry( "Path", path );
    cfg.writeEntry( "ImageFormat", static_cast<int>( imgFormat ) );
    cfg.writeEntry( "Zoom", zoom );
}

/*================================================================*/
void KPWebPresentation::initCreation( QProgressBar *progressBar )
{
    QString cmd;
    int p;

    QDir( path ).mkdir( path + "/html" );

    p = progressBar->progress();
    progressBar->setProgress( ++p );
    kapp->processEvents();

    QDir( path ).mkdir( path + "/pics" );

    p = progressBar->progress();
    progressBar->setProgress( ++p );
    kapp->processEvents();

    QString format = "." + imageFormat( imgFormat );

    const char *pics[] = { "home", "first", "next", "prev", "last", 0 };
    uint index = 0;

    QString filename;

    while ( pics[ index ] ) {
	filename = pics[index] + format;
	system( QString( "cp %1 %2/pics/%3" ).
		arg( locate( "appdata", "slideshow/" + filename ) ).
		arg( path ).arg( filename ) );
	p = progressBar->progress();
	progressBar->setProgress( ++p );
	kapp->processEvents();
	index++;
    }
}

/*================================================================*/
void KPWebPresentation::createSlidesPictures( QProgressBar *progressBar )
{
    QPixmap pix( QSize( doc->getPageSize( 0, 0, 0 ).width(), doc->getPageSize( 0, 0, 0 ).height() ) );
    QString filename;
    QString format = imageFormat( imgFormat );
    int p;

    for ( unsigned int i = 0; i < doc->getPageNums(); i++ ) {
	pix.resize( QSize( doc->getPageSize( 0, 0, 0 ).width(), doc->getPageSize( 0, 0, 0 ).height() ) );
	pix.fill( Qt::white );
	view->getPage()->drawPageInPix2( pix, i * doc->getPageSize( 0, 0, 0 ).height(), i );
	filename = QString( "%1/pics/slide_%2.%3" ).arg( path ).arg( i + 1 ).arg( format );
	if ( zoom != 100 ) {
	    QWMatrix m;
	    m.scale( ( (float)zoom ) / 100.0, ( (float)zoom ) / 100.0 );
	    pix = pix.xForm( m );
	}
	pix.save( filename, format.upper() );

	p = progressBar->progress();
	progressBar->setProgress( ++p );
	kapp->processEvents();
    }
}

/*================================================================*/
void KPWebPresentation::createSlidesHTML( QProgressBar *progressBar )
{
    unsigned int pgNum;
    int p;
    QString format = imageFormat( imgFormat );

    QString html;
    for ( unsigned int i = 0; i < doc->getPageNums(); i++ ) {
	pgNum = i + 1;
	html = QString( "<HTML><HEAD><TITLE>%1 - %2</TITLE></HEAD>\n" ).arg( title ).arg( slideTitles[ i ] );

	QString c1;
	c1.sprintf( "%02X%02X%02X", backColor.red(), backColor.green(), backColor.blue() );
	QString c2;
	c2.sprintf( "%02X%02X%02X", textColor.red(), textColor.green(), textColor.blue() );
	QString c3;
	c3.sprintf( "%02X%02X%02X", titleColor.red(), titleColor.green(), titleColor.blue() );

	html += QString( "<BODY bgcolor=\"%1\" text=\"%2\">\n" ).arg( c1 ).arg( c2 );

	html += QString( "  <CENTER>\n" );
	if ( i > 0 )
	    html += QString( "	  <A HREF=\"slide_1.html\">" );
	html += QString( "<IMG src=\"../pics/first.%1\" border=\"0\">" ).arg( format );
	if ( i > 0 )
	    html += "</A>\n";
	else
	    html += "\n";

	if ( i > 0 )
	    html += QString( "	  <A HREF=\"slide_%1.html\">" ).arg( pgNum - 1 );
	html += QString( "<IMG src=\"../pics/prev.%1\" border=\"0\">" ).arg( format );
	if ( i > 0 )
	    html += "</A>\n";
	else
	    html += "\n";

	if ( i < doc->getPageNums() - 1 )
	    html += QString( "	  <A HREF=\"slide_%1.html\">" ).arg( pgNum + 1 );
	html += QString( "<IMG src=\"../pics/next.%1\" border=\"0\">" ).arg( format );
	if ( i < doc->getPageNums() - 1 )
	    html += "</A>\n";
	else
	    html += "\n";

	if ( i < doc->getPageNums() - 1 )
	    html += QString( "	  <A HREF=\"slide_%1.html\">" ).arg( doc->getPageNums() );
	html += QString( "<IMG src=\"../pics/last.%1\" border=\"0\">" ).arg( format );
	if ( i < doc->getPageNums() - 1 )
	    html += "</A>\n";
	else
	    html += "\n";

	html += "    &nbsp; &nbsp; &nbsp; &nbsp; \n";

	html += "    <A HREF=\"../index.html\">";
	html += QString( "<IMG src=\"../pics/home.%1\" border=\"0\">" ).arg( format );
	html += "</A>\n";

	html += "  </CENTER><BR><HR noshade>\n";

	html += QString( "  <FONT color=\"%1\">\n" ).arg( c3 );
	html += QString( "  <CENTER><B>%1</B> - <I>%2</I></CENTER>\n" ).arg( title ).arg( slideTitles[ i ] );

	html += "  </FONT><HR noshade><BR>\n";

	html += "  <CENTER>\n";
	html += QString( "    <IMG src=\"../pics/slide_%1.%2\">" ).arg( pgNum ).arg( format );
	html += "  </CENTER><BR><HR noshade>\n";
	html += "  <CENTER>\n";
	html += "    <B>Author: </B>";
	if ( !email.isEmpty() )
	    html += QString( "<A HREF=\"mailto:%1\">" ).arg( email );
	html += QString( "<I>%1</I>" ).arg( author );
	if ( !email.isEmpty() )
	    html += "</A>";

	html += " - created with <A HREF=\"http://koffice.kde.org/kpresenter.html\">KPresenter</A>\n";
	html += "  </CENTER><HR noshade>\n";
	html += "</BODY></HTML>\n";

	QFile file( QString( "%1/html/slide_%2.html" ).arg( path ).arg( pgNum ) );
	file.open( IO_WriteOnly );
	QTextStream t( &file );
	t << html;
	file.close();

	p = progressBar->progress();
	progressBar->setProgress( ++p );
	kapp->processEvents();
    }
}

/*================================================================*/
void KPWebPresentation::createMainPage( QProgressBar *progressBar )
{
    QString html;

    html = QString( "<HTML><HEAD><TITLE>%1 - Table of Contents</TITLE></HEAD>\n" ).arg( title );

    QString c1;
    c1.sprintf( "%02X%02X%02X", backColor.red(), backColor.green(), backColor.blue() );
    QString c2;
    c2.sprintf( "%02X%02X%02X", textColor.red(), textColor.green(), textColor.blue() );
    QString c3;
    c3.sprintf( "%02X%02X%02X", titleColor.red(), titleColor.green(), titleColor.blue() );

    html += QString( "<BODY bgcolor=\"%1\" text=\"%2\">\n" ).arg( c1 ).arg( c2 );

    html += QString( "<FONT color=\"%1\">\n" ).arg( c3 );
    html += QString( "<BR><CENTER><H1>%1</H1></CENTER>\n" ).arg( title );
    html += "</FONT>\n";

    html += "<BR><BR><CENTER><H3><A HREF=\"html/slide_1.html\">Click here to start the Slideshow</A></H3></CENTER><BR>\n";

    html += "<HR noshade><BR><BR>\n";

    if ( email.isEmpty() )
	html += QString( "Created on %1 by <I>%2</I><BR><BR>\n" ).arg( QDate::currentDate().toString() ).arg( author );
    else
	html += QString( "Created on %1 by <I><A HREF=\"mailto:%2\">%3</A></I><BR><BR>\n" ).arg( QDate::currentDate().toString() ).arg( email ).arg( author );

    html += "<B>Table of Contents</B><BR>\n";
    html += "<OL>\n";

    for ( unsigned int i = 0; i < doc->getPageNums(); i++ )
	html += QString( "  <LI><A HREF=\"html/slide_%1.html\">%2</A><BR>\n" ).arg( i + 1 ).arg( slideTitles[ i ] );

    html += "</OL></BODY></HTML>\n";

    QFile file( QString( "%1/index.html" ).arg( path ) );
    file.open( IO_WriteOnly );
    QTextStream t( &file );
    t << html;
    file.close();

    progressBar->setProgress( progressBar->totalSteps() );
    kapp->processEvents();
}

/*================================================================*/
void KPWebPresentation::init()
{
    struct passwd* pw;
    char str[ 80 ];

    pw = getpwuid( getuid() );
    if ( pw ) {
	author = QString( pw->pw_gecos );
	int i = author.find( ',' );
	if ( i > 0 ) author.truncate( i );
    }

    pw = getpwuid( getuid() );
    if ( pw ) {
	gethostname( str, 79 );
	email = QString( pw->pw_name ) + "@" + str;
    }

    title = "Slideshow";

    for ( unsigned int i = 0; i < doc->getPageNums(); i++ )
	slideTitles.append( doc->getPageTitle( i, QString( "Slide %1" ).arg( i ) ) );

    backColor = Qt::white;
    textColor = Qt::black;
    titleColor = Qt::red;
    if (KImageIO::canWrite("JPEG"))
	imgFormat = JPEG;
    else
	imgFormat = PNG;

    path = getenv( "HOME" );
    path += "/www";

    zoom = 100;
}

/******************************************************************/
/* Class: KPWebPresentationWizard				  */
/******************************************************************/

/*================================================================*/
KPWebPresentationWizard::KPWebPresentationWizard( const QString &_config, KPresenterDoc *_doc,
						  KPresenterView *_view )
    : QWizard( 0, "", FALSE ), config( _config ), webPres( config, _doc, _view )
{
    doc = _doc;
    view = _view;

    setupPage1();
    setupPage2();
    setupPage3();

    connect( nextButton(), SIGNAL( clicked() ), this, SLOT( pageChanged() ) );
    connect( backButton(), SIGNAL( clicked() ), this, SLOT( pageChanged() ) );
    connect( finishButton(), SIGNAL( clicked() ), this, SLOT( finish() ) );
}

/*================================================================*/
KPWebPresentationWizard::~KPWebPresentationWizard()
{
    view->enableWebPres();
}

/*================================================================*/
void KPWebPresentationWizard::createWebPresentation( const QString &_config, KPresenterDoc *_doc,
						     KPresenterView *_view )
{
    KPWebPresentationWizard *dlg = new KPWebPresentationWizard( _config, _doc, _view );

    dlg->setCaption( i18n( "Create Web-Presentation (HTML Slideshow)" ) );
    dlg->resize( 640, 350 );
    dlg->show();
}

/*================================================================*/
void KPWebPresentationWizard::setupPage1()
{
    page1 = new QHBox( this );
    page1->setSpacing( 5 );
    page1->setMargin( 5 );

    QLabel *helptext = new QLabel( page1 );
    helptext->setMargin( 5 );
    helptext->setBackgroundMode( PaletteLight );
    helptext->setText( i18n( "Enter here your Name, your Email\n"
			     "Address and the Title of the Web-\n"
			     "Presentation. Also enter the Path\n"
			     "into which the Web-Presentation\n"
			     "should be created. This must be a\n"
			     "directory." ) );
    helptext->setMaximumWidth( helptext->sizeHint().width() );

    QVBox *canvas = new QVBox( page1 );

    QHBox *row1 = new QHBox( canvas );
    QHBox *row2 = new QHBox( canvas );
    QHBox *row3 = new QHBox( canvas );
    QHBox *row4 = new QHBox( canvas );

    QLabel *label1 = new QLabel( " Author: ", row1 );
    label1->setAlignment( Qt::AlignVCenter );
    QLabel *label2 = new QLabel( " Title: ", row2 );
    label2->setAlignment( Qt::AlignVCenter );
    QLabel *label3 = new QLabel( " Email-Address: ", row3 );
    label3->setAlignment( Qt::AlignVCenter );
    QLabel *label4 = new QLabel( " Path: ", row4 );
    label4->setAlignment( Qt::AlignVCenter );

    label1->setMinimumWidth( label3->sizeHint().width() );
    label2->setMinimumWidth( label3->sizeHint().width() );
    label3->setMinimumWidth( label3->sizeHint().width() );
    label4->setMinimumWidth( label3->sizeHint().width() );
    label1->setMaximumWidth( label3->sizeHint().width() );
    label2->setMaximumWidth( label3->sizeHint().width() );
    label3->setMaximumWidth( label3->sizeHint().width() );
    label4->setMaximumWidth( label3->sizeHint().width() );

    author = new QLineEdit( webPres.getAuthor(), row1 );
    title = new QLineEdit( webPres.getTitle(), row2 );
    email = new QLineEdit( webPres.getEmail(), row3 );

    QHBox *hbox = new QHBox( row4 );
    path = new QLineEdit( webPres.getPath(), hbox );
    choosePath = new QPushButton( "Choose...", hbox );
    choosePath->setMaximumSize( choosePath->sizeHint() );

    addPage( page1, i18n( "General Information" ) );

    connect( choosePath, SIGNAL( clicked() ), this, SLOT( slotChoosePath() ) );
}

/*================================================================*/
void KPWebPresentationWizard::setupPage2()
{
    page2 = new QHBox( this );
    page2->setSpacing( 5 );
    page2->setMargin( 5 );

    QLabel *helptext = new QLabel( page2 );
    helptext->setMargin( 5 );
    helptext->setBackgroundMode( PaletteLight );
    QString help = i18n("Here you can configure the style\n"
			"of the webpages ( colors ). You also\n"
			"need to specify the picture format\n"
			"which should be used for the slides.\n"
			"PNG is a very optimized and well\n"
			"compressed format, but may not be\n"
			"supported by some old Web-Browsers.\n"
			"BMP is a picture format with a bad\n"
			"compression, but is supported also by\n"
			"old Web-Browsers.\n");

    if ( KImageIO::canWrite( "JPEG" ) )
	help += i18n("JPEG is a picture format with a quite good\n"
		     "compression and which is also supported by\n"
		     "all Web-Browsers.\n");

    help += i18n( "\n"
		  "Finally you also can specify the zoom\n"
		  "for the slides." );
    helptext->setText(help);
    helptext->setMaximumWidth( helptext->sizeHint().width() );

    QVBox *canvas = new QVBox( page2 );

    QHBox *row1 = new QHBox( canvas );
    QHBox *row2 = new QHBox( canvas );
    QHBox *row3 = new QHBox( canvas );
    QHBox *row4 = new QHBox( canvas );
    QHBox *row5 = new QHBox( canvas );

    QLabel *label1 = new QLabel( " Text Color: ", row1 );
    label1->setAlignment( Qt::AlignVCenter );
    QLabel *label2 = new QLabel( " Title Color: ", row2 );
    label2->setAlignment( Qt::AlignVCenter );
    QLabel *label3 = new QLabel( " Background Color: ", row3 );
    label3->setAlignment( Qt::AlignVCenter );
    QLabel *label4 = new QLabel( " Picture Format: ", row4 );
    label4->setAlignment( Qt::AlignVCenter );
    QLabel *label5 = new QLabel( " Zoom: ", row5 );
    label5->setAlignment( Qt::AlignVCenter );

    label1->setMinimumWidth( label3->sizeHint().width() );
    label2->setMinimumWidth( label3->sizeHint().width() );
    label3->setMinimumWidth( label3->sizeHint().width() );
    label4->setMinimumWidth( label3->sizeHint().width() );
    label5->setMinimumWidth( label3->sizeHint().width() );
    label1->setMaximumWidth( label3->sizeHint().width() );
    label2->setMaximumWidth( label3->sizeHint().width() );
    label3->setMaximumWidth( label3->sizeHint().width() );
    label4->setMaximumWidth( label3->sizeHint().width() );
    label5->setMaximumWidth( label3->sizeHint().width() );

    textColor = new KColorButton( webPres.getTextColor(), row1 );
    titleColor = new KColorButton( webPres.getTitleColor(), row2 );
    backColor = new KColorButton( webPres.getBackColor(), row3 );
    format = new QComboBox( FALSE, row4 );
    format->insertItem( "BMP", -1 );
    format->insertItem( "PNG", -1 );
    if ( KImageIO::canWrite( "JPEG" ) )
	format->insertItem( "JPEG", -1 );
    format->setCurrentItem( static_cast<int>( webPres.getImageFormat() ) );
    zoom = new QSpinBox( 1, 1000, 1, row5 );
    zoom->setSuffix( " %" );
    zoom->setValue( webPres.getZoom() );
    zoom->setMaximumHeight( zoom->sizeHint().height() );

    addPage( page2, i18n( "Style" ) );
}

/*================================================================*/
void KPWebPresentationWizard::setupPage3()
{
    page3 = new QHBox( this );
    page3->setSpacing( 5 );
    page3->setMargin( 5 );

    QLabel *helptext = new QLabel( page3 );
    helptext->setMargin( 5 );
    helptext->setBackgroundMode( PaletteLight );
    helptext->setText( i18n( "Here you can specify titles for\n"
			     "each slide. Click in the list on\n"
			     "on a slide and then enter in the\n"
			     "editbox below the title. If you\n"
			     "click on a title the KPresenter\n"
			     "mainview scrolls to this slide,\n"
			     "so that you can see the slide." ) );
    helptext->setMaximumWidth( helptext->sizeHint().width() );

    QVBox *canvas = new QVBox( page3 );

    QHBox *row = new QHBox( canvas );
    QLabel *label = new QLabel( i18n( " Slide Title: " ), row );
    label->setAlignment( Qt::AlignVCenter );
    label->setMinimumWidth( label->sizeHint().width() );
    label->setMaximumWidth( label->sizeHint().width() );

    slideTitle = new QLineEdit( row );
    connect( slideTitle, SIGNAL( textChanged( const QString & ) ), this,
	     SLOT( slideTitleChanged( const QString & ) ) );

    slideTitles = new QListView( canvas );
    slideTitles->addColumn( i18n( "Slide Nr." ) );
    slideTitles->addColumn( i18n( "Slide Title" ) );
    connect( slideTitles, SIGNAL( selectionChanged( QListViewItem * ) ), this,
	     SLOT( slideTitleChanged( QListViewItem * ) ) );
    slideTitles->setSorting( -1 );
    slideTitles->header()->setMovingEnabled( FALSE );

    for ( int i = doc->getPageNums() - 1; i >= 0; --i ) {
	QListViewItem *item = new QListViewItem( slideTitles );
	item->setText( 0, QString( "%1" ).arg( i + 1 ) );
	item->setText( 1, (int)webPres.getSlideTitles().count() > i ?
		       QString( webPres.getSlideTitles()[ i ] ) : QString( "No Title" ) );
    }

    addPage( page3, i18n( "Slide Titles" ) );
    setFinish( page3, TRUE );
}


/*================================================================*/
void KPWebPresentationWizard::finish()
{
    webPres.setAuthor( author->text() );
    webPres.setEMail( email->text() );
    webPres.setTitle( title->text() );

    QStringList slides = webPres.getSlideTitles();
    QListViewItemIterator it( slideTitles );
    for ( ; it.current(); ++it )
	slides[ atoi( it.current()->text( 0 ) ) - 1 ] = QString( it.current()->text( 1 ) );
    webPres.setSlideTitles( slides );

    webPres.setBackColor( backColor->color() );
    webPres.setTitleColor( titleColor->color() );
    webPres.setTextColor( textColor->color() );
    webPres.setImageFormat( static_cast<KPWebPresentation::ImageFormat>( format->currentItem() ) );
    webPres.setPath( path->text() );
    webPres.setZoom( zoom->value() );

    close();
    KPWebPresentationCreateDialog::createWebPresentation( doc, view, webPres );
}

/*================================================================*/
void KPWebPresentationWizard::slotChoosePath()
{
    QFileInfo fi( path->text() );
    QString url = QString::null;
    if ( fi.exists() && fi.isDir() )
	url = path->text();

    url = KFileDialog::getDirectory( url );

    if ( QFileInfo( url ).exists() && QFileInfo( url ).isDir() )
	path->setText( url );
}

/*================================================================*/
bool KPWebPresentationWizard::isPathValid()
{
    QFileInfo fi( path->text() );

    if ( fi.exists() && fi.isDir() )
	return TRUE;

    return FALSE;
}

/*================================================================*/
void KPWebPresentationWizard::pageChanged()
{
    if ( currentPage() != page3 ) {
	if ( !isPathValid() ) {
	    QMessageBox::critical( 0, i18n( "Invalid Path" ),
				   i18n( "The path you entered is not a valid directory!\n"
					 "Please correct this." ),
				   i18n( "OK" ) );
	    showPage( page1 );
	    path->setFocus();
	}
    } else
	finishButton()->setEnabled( TRUE );
}

/*================================================================*/
void KPWebPresentationWizard::slideTitleChanged( const QString &s )
{
    if ( slideTitles->currentItem() )
	slideTitles->currentItem()->setText( 1, s );
}

/*================================================================*/
void KPWebPresentationWizard::slideTitleChanged( QListViewItem *i )
{
    if ( !i ) return;

    slideTitle->setText( i->text( 1 ) );
    view->skipToPage( atoi( i->text( 0 ) ) - 1 );
}

/*================================================================*/
void KPWebPresentationWizard::closeEvent( QCloseEvent *e )
{
    view->enableWebPres();
    QWizard::closeEvent( e );
}

/******************************************************************/
/* Class: KPWebPresentationCreateDialog				  */
/******************************************************************/

/*================================================================*/
KPWebPresentationCreateDialog::KPWebPresentationCreateDialog( KPresenterDoc *_doc, KPresenterView *_view,
							      const KPWebPresentation &_webPres )
    : QDialog( 0, "", FALSE ), webPres( _webPres )
{
    doc = _doc;
    view = _view;

    setupGUI();
}

/*================================================================*/
KPWebPresentationCreateDialog::~KPWebPresentationCreateDialog()
{
    view->enableWebPres();
}

/*================================================================*/
void KPWebPresentationCreateDialog::createWebPresentation( KPresenterDoc *_doc, KPresenterView *_view,
							   const KPWebPresentation &_webPres )
{
    KPWebPresentationCreateDialog *dlg = new KPWebPresentationCreateDialog( _doc, _view, _webPres );

    dlg->setCaption( i18n( "Create Web-Presentation (HTML Slideshow)" ) );
    dlg->resize( 400, 300 );
    dlg->show();
    dlg->start();
}

/*================================================================*/
void KPWebPresentationCreateDialog::start()
{
    setCursor( waitCursor );
    initCreation();
    createSlidesPictures();
    createSlidesHTML();
    createMainPage();
    setCursor( arrowCursor );

    bDone->setEnabled( TRUE );
    bSave->setEnabled( TRUE );
}

/*================================================================*/
void KPWebPresentationCreateDialog::initCreation()
{
    QFont f = step1->font(), f2 = step1->font();
    f.setBold( TRUE );
    step1->setFont( f );

    progressBar->reset();
    progressBar->setTotalSteps( webPres.initSteps() );

    webPres.initCreation( progressBar );

    step1->setFont( f2 );
    progressBar->setProgress( progressBar->totalSteps() );
}

/*================================================================*/
void KPWebPresentationCreateDialog::createSlidesPictures()
{
    QFont f = step2->font(), f2 = step2->font();
    f.setBold( TRUE );
    step2->setFont( f );

    progressBar->reset();
    progressBar->setTotalSteps( webPres.slides1Steps() );

    webPres.createSlidesPictures( progressBar );

    step2->setFont( f2 );
    progressBar->setProgress( progressBar->totalSteps() );
}

/*================================================================*/
void KPWebPresentationCreateDialog::createSlidesHTML()
{
    QFont f = step3->font(), f2 = step3->font();
    f.setBold( TRUE );
    step3->setFont( f );

    progressBar->reset();
    progressBar->setTotalSteps( webPres.slides2Steps() );

    webPres.createSlidesHTML( progressBar );

    step3->setFont( f2 );
    progressBar->setProgress( progressBar->totalSteps() );
}

/*================================================================*/
void KPWebPresentationCreateDialog::createMainPage()
{
    QFont f = step4->font(), f2 = step4->font();
    f.setBold( TRUE );
    step4->setFont( f );

    progressBar->reset();
    progressBar->setTotalSteps( webPres.mainSteps() );

    webPres.createMainPage( progressBar );

    step4->setFont( f2 );
    progressBar->setProgress( progressBar->totalSteps() );
}

/*================================================================*/
void KPWebPresentationCreateDialog::setupGUI()
{
    back = new QVBox( this );
    back->setMargin( 10 );

    QFrame *line;

    line = new QFrame( back );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    line->setMaximumHeight( 20 );

    step1 = new QLabel( i18n( "Initialize ( create file structure, etc. )" ), back );
    step2 = new QLabel( i18n( "Create Pictures of the Slides" ), back );
    step3 = new QLabel( i18n( "Create HTML pages for the slides" ), back );
    step4 = new QLabel( i18n( "Create Main Page ( Table of Contents )" ), back );

    line = new QFrame( back );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    line->setMaximumHeight( 20 );

    progressBar = new QProgressBar( back );

    line = new QFrame( back );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    line->setMaximumHeight( 20 );

    KButtonBox *bb = new KButtonBox( back );
    bSave = bb->addButton( i18n( "Save Configuration..." ) );
    bb->addStretch();
    bDone = bb->addButton( i18n( "Done" ) );

    bSave->setEnabled( FALSE );
    bDone->setEnabled( FALSE );

    connect( bDone, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( bSave, SIGNAL( clicked() ), this, SLOT( saveConfig() ) );
}

/*================================================================*/
void KPWebPresentationCreateDialog::resizeEvent( QResizeEvent *e )
{
    QDialog::resizeEvent( e );
    back->resize( size() );
}

/*================================================================*/
void KPWebPresentationCreateDialog::saveConfig()
{
    QString filename = webPres.getConfig();
    if ( QFileInfo( filename ).exists() )
	filename = QFileInfo( filename ).absFilePath();
    else
	filename = QString::null;

    filename = KFileDialog::getOpenFileName( filename, "*.kpweb|KPresenter Web-Presentation" );

    if ( !filename.isEmpty() ) {
	webPres.setConfig( filename );
	webPres.saveConfig();
    }
}
