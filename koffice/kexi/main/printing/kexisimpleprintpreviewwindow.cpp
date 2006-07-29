/* This file is part of the KDE project
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
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
*/

#include "kexisimpleprintpreviewwindow.h"
#include "kexisimpleprintingengine.h"
#include "kexisimpleprintpreviewwindow_p.h"
#include <kexi_version.h>

#include <qlayout.h>
#include <qaccel.h>
#include <qtimer.h>
#include <qlabel.h>

#include <kdialogbase.h>
#include <ktoolbarbutton.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kapplication.h>

KexiSimplePrintPreviewView::KexiSimplePrintPreviewView(
	QWidget *parent, KexiSimplePrintPreviewWindow *window)
 : QWidget(parent, "KexiSimplePrintPreviewView", WStaticContents)//|WNoAutoErase)
 , m_window(window)
{
	enablePainting = false;
//			resize(300,400);
//			resizeContents(200, 400);
}

void KexiSimplePrintPreviewView::paintEvent( QPaintEvent *pe )
{
	Q_UNUSED(pe);
	if (!enablePainting)
		return;
	QPixmap pm(size()); //dbl buffered
	QPainter p;
	p.begin(&pm, this);
//! @todo only for screen!
	p.fillRect(QRect(QPoint(0,0),pm.size()), QBrush(white));//pe->rect(), QBrush(white));
	if (m_window->currentPage()>=0)
		m_window->m_engine.paintPage(m_window->currentPage(), p);
//		emit m_window->paintingPageRequested(m_window->currentPage(), p);
	p.end();
	bitBlt(this, 0, 0, &pm);
}

//--------------------------

#define KexiSimplePrintPreviewScrollView_MARGIN KDialogBase::marginHint()

KexiSimplePrintPreviewScrollView::KexiSimplePrintPreviewScrollView(
	KexiSimplePrintPreviewWindow *window)
 : QScrollView(window, "scrollview", WStaticContents|WNoAutoErase)
 , m_window(window)
{
//			this->settings = settings;
	widget = new KexiSimplePrintPreviewView(viewport(), m_window);

/*			int widthMM = KoPageFormat::width( 
		settings.pageLayout.format, settings.pageLayout.orientation);
	int heightMM = KoPageFormat::height( 
		settings.pageLayout.format, settings.pageLayout.orientation);
//			int constantHeight = 400;
//			widget->resize(constantHeight * widthMM / heightMM, constantHeight ); //keep aspect
*/
	addChild(widget);
}

void KexiSimplePrintPreviewScrollView::resizeEvent( QResizeEvent *re )
{
	QScrollView::resizeEvent(re);
//	kdDebug() << re->size().width() << " " << re->size().height() << endl;
//	kdDebug() << contentsWidth() << " " << contentsHeight() << endl;
//	kdDebug() << widget->width() << " " << widget->height() << endl;
	setUpdatesEnabled(false);
	if (re->size().width() > (widget->width()+2*KexiSimplePrintPreviewScrollView_MARGIN)
		|| re->size().height() > (widget->height()+2*KexiSimplePrintPreviewScrollView_MARGIN)) {
		resizeContents(
			QMAX(re->size().width(), widget->width()+2*KexiSimplePrintPreviewScrollView_MARGIN),
			QMAX(re->size().height(), widget->height()+2*KexiSimplePrintPreviewScrollView_MARGIN));
		int vscrbarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
		moveChild(widget, (contentsWidth() - vscrbarWidth - widget->width())/2, 
			(contentsHeight() - widget->height())/2);
	}
	setUpdatesEnabled(true);
}

void KexiSimplePrintPreviewScrollView::setFullWidth()
{
	viewport()->setUpdatesEnabled(false);
	double widthMM = KoPageFormat::width( 
		m_window->settings().pageLayout.format, 
		m_window->settings().pageLayout.orientation);
	double heightMM = KoPageFormat::height( 
		m_window->settings().pageLayout.format, m_window->settings().pageLayout.orientation);
//	int constantWidth = m_window->width()- KexiSimplePrintPreviewScrollView_MARGIN*6;
	double constantWidth = width()- KexiSimplePrintPreviewScrollView_MARGIN*6;
	double heightForWidth = constantWidth * heightMM / widthMM;
//	heightForWidth = QMIN(kapp->desktop()->height()*4/5, heightForWidth);
	kdDebug() << "1: " << heightForWidth << endl;
#if 0 //todo we can use this if we want to fix the height to width of the page
	heightForWidth = QMIN(height(), heightForWidth);
	kdDebug() << "2: " << heightForWidth << endl;
#endif
	constantWidth = heightForWidth * widthMM / heightMM;
	widget->resize((int)constantWidth, (int)heightForWidth); //keep aspect
	resizeContents(int(widget->width() + 2*KexiSimplePrintPreviewScrollView_MARGIN), 
		int(widget->height() + 2*KexiSimplePrintPreviewScrollView_MARGIN));
	moveChild(widget, (contentsWidth()-widget->width())/2, 
		(contentsHeight()-widget->height())/2);
	viewport()->setUpdatesEnabled(true);
	resize(size()+QSize(1,1)); //to update pos.
	widget->enablePainting = true;
	widget->repaint();
}

void KexiSimplePrintPreviewScrollView::setContentsPos(int x, int y)
{
//	kdDebug() << "############" << x << " " << y << " " << contentsX()<< " " <<contentsY() << endl;
	if (x<0 || y<0) //to avoid endless loop on Linux
		return;
	QScrollView::setContentsPos(x,y);
}

//------------------

KexiSimplePrintPreviewWindow::KexiSimplePrintPreviewWindow(
	KexiSimplePrintingEngine &engine, const QString& previewName, 
	QWidget *parent, WFlags f)
 : QWidget(parent, "KexiSimplePrintPreviewWindow", f)
 , m_engine(engine)
 , m_settings(*m_engine.settings())
 , m_pageNumber(-1)
{
//	m_pagesCount = INT_MAX;

	setCaption(i18n("%1 - Print Preview - %2").arg(previewName).arg(KEXI_APP_NAME));
	setIcon(DesktopIcon("filequickprint"));
	QVBoxLayout *lyr = new QVBoxLayout(this, 6);

	int id;
	m_toolbar = new KToolBar(0, this);
	m_toolbar->setLineWidth(0);
	m_toolbar->setFrameStyle(QFrame::NoFrame);
	m_toolbar->setIconText(KToolBar::IconTextRight);
	lyr->addWidget(m_toolbar);

	id = m_toolbar->insertWidget( -1, 0, new KPushButton(KStdGuiItem::print(), m_toolbar) );
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotPrintClicked()));
	static_cast<KPushButton*>(m_toolbar->getWidget(id))->setAccel(Qt::CTRL|Qt::Key_P);
	m_toolbar->insertSeparator();

	id = m_toolbar->insertWidget(-1, 0, new KPushButton(i18n("Page Set&up..."), m_toolbar));
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotPageSetup()));
	m_toolbar->insertSeparator();


#ifndef KEXI_NO_UNFINISHED 
//! @todo unfinished
	id = m_toolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("viewmag+"), i18n("Zoom In"), m_toolbar));
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotZoomInClicked()));
	m_toolbar->insertSeparator();

	id = m_toolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("viewmag-"), i18n("Zoom Out"), m_toolbar));
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotZoomOutClicked()));
	m_toolbar->insertSeparator();
#endif

	id = m_toolbar->insertWidget(-1, 0, new KPushButton(KStdGuiItem::close(), m_toolbar));
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(close()));
	m_toolbar->alignItemRight(id);

	m_scrollView = new KexiSimplePrintPreviewScrollView(this);
	m_scrollView->setUpdatesEnabled(false);
	m_view = m_scrollView->widget;
	m_scrollView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	lyr->addWidget(m_scrollView);

	m_navToolbar = new KToolBar(0, this);
//	m_navToolbar->setFullWidth(true);
	m_navToolbar->setLineWidth(0);
	m_navToolbar->setFrameStyle(QFrame::NoFrame);
	m_navToolbar->setIconText(KToolBar::IconTextRight);
	lyr->addWidget(m_navToolbar);

	m_idFirst = m_navToolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("start"), i18n("First Page"), m_navToolbar));
	m_navToolbar->addConnection(m_idFirst, SIGNAL(clicked()), this, SLOT(slotFirstClicked()));
	m_navToolbar->insertSeparator();

	m_idPrevious = m_navToolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("previous"), i18n("Previous Page"), m_navToolbar));
	m_navToolbar->addConnection(m_idPrevious, SIGNAL(clicked()), this, SLOT(slotPreviousClicked()));
	m_navToolbar->insertSeparator();

	m_idPageNumberLabel = m_navToolbar->insertWidget( -1, 0, new QLabel(m_navToolbar));
	m_navToolbar->insertSeparator();

	m_idNext = m_navToolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("next"), i18n("Next Page"), m_navToolbar));
	m_navToolbar->addConnection(m_idNext, SIGNAL(clicked()), this, SLOT(slotNextClicked()));
	m_navToolbar->insertSeparator();

	m_idLast = m_navToolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("finish"), i18n("Last Page"), m_navToolbar));
	m_navToolbar->addConnection(m_idLast, SIGNAL(clicked()), this, SLOT(slotLastClicked()));
	m_navToolbar->insertSeparator();

	resize(width(), kapp->desktop()->height()*4/5);

//! @todo progress bar...

	QTimer::singleShot(50, this, SLOT(initLater()));
}

void KexiSimplePrintPreviewWindow::initLater()
{
	setFullWidth();
	updatePagesCount();
	goToPage(0);
}

KexiSimplePrintPreviewWindow::~KexiSimplePrintPreviewWindow()
{
}

/*void KexiSimplePrintPreviewWindow::setPagesCount(int pagesCount)
{
	m_pagesCount = pagesCount;
	goToPage(0);
}*/

void KexiSimplePrintPreviewWindow::slotPrintClicked()
{
	hide();
	emit printRequested();
	show();
	raise();
}

void KexiSimplePrintPreviewWindow::slotPageSetup()
{
	lower();
	emit pageSetupRequested();
}

void KexiSimplePrintPreviewWindow::slotZoomInClicked()
{
	//! @todo
}

void KexiSimplePrintPreviewWindow::slotZoomOutClicked()
{
	//! @todo
}

void KexiSimplePrintPreviewWindow::slotFirstClicked()
{
	goToPage(0);
}

void KexiSimplePrintPreviewWindow::slotPreviousClicked()
{
	goToPage(m_pageNumber-1);
}

void KexiSimplePrintPreviewWindow::slotNextClicked()
{
	goToPage(m_pageNumber+1);
}

void KexiSimplePrintPreviewWindow::slotLastClicked()
{
	goToPage(m_engine.pagesCount()-1);
}

void KexiSimplePrintPreviewWindow::goToPage(int pageNumber)
{
	if (pageNumber==m_pageNumber || pageNumber < 0 || pageNumber > ((int)m_engine.pagesCount()-1))
		return;
	m_pageNumber = pageNumber;

	m_view->repaint(); //this will automatically paint a new page
//	if (m_engine.eof())
//		m_pagesCount = pageNumber+1;

	m_navToolbar->setItemEnabled(m_idNext, pageNumber < ((int)m_engine.pagesCount()-1));
	m_navToolbar->setItemEnabled(m_idLast, pageNumber < ((int)m_engine.pagesCount()-1));
	m_navToolbar->setItemEnabled(m_idPrevious, pageNumber > 0);
	m_navToolbar->setItemEnabled(m_idFirst, pageNumber > 0);
	static_cast<QLabel*>(m_navToolbar->getWidget(m_idPageNumberLabel))->setText(
		i18n("Page (number) of (total)", "Page %1 of %2").arg(m_pageNumber+1).arg(m_engine.pagesCount()));
}

void KexiSimplePrintPreviewWindow::setFullWidth()
{
	m_scrollView->setFullWidth();
}

void KexiSimplePrintPreviewWindow::updatePagesCount()
{
	QPixmap pm(m_view->size()); //dbl buffered
	QPainter p(m_view);
	//p.begin(&pm, this);
////! @todo only for screen!
//	p.fillRect(pe->rect(), QBrush(white));
	m_engine.calculatePagesCount(p);
	p.end();
}

bool KexiSimplePrintPreviewWindow::event( QEvent * e )
{
	QEvent::Type t = e->type();
	if (t==QEvent::KeyPress) {
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);
		const int k = ke->key();
		bool ok = true;
		if (k==Qt::Key_Equal || k==Qt::Key_Plus)
			slotZoomInClicked();
		else if (k==Qt::Key_Minus)
			slotZoomOutClicked();
		else if (k==Qt::Key_Home)
			slotFirstClicked();
		else if (k==Qt::Key_End)
			slotLastClicked();
		else
			ok = false;

		if (ok) {
			ke->accept();
			return true;
		}
	}
	else if (t==QEvent::AccelOverride) {
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);
		const int k = ke->key();
		bool ok = true;
		if (k==Qt::Key_PageUp)
			slotPreviousClicked();
		else if (k==Qt::Key_PageDown)
			slotNextClicked();
		else
			ok = false;

		if (ok) {
			ke->accept();
			return true;
		}
	}
	return QWidget::event(e);
}


#include "kexisimpleprintpreviewwindow.moc"
#include "kexisimpleprintpreviewwindow_p.moc"
