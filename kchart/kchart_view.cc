/**
 * $Id$
 *
 * Kalle Dalheimer <kalle@kde.org>
 */

#include "kchart_view.h"
#include "kchart_factory.h"
#include "kchart_part.h"
#include "kchartWizard.h"
#include "kchartDataEditor.h"
#include "kchartparams.h"
#include "kchartBarConfigDialog.h"

#include <qpainter.h>
#include <kaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

using namespace std;

//#include "sheetdlg.h"

KChartView::KChartView( KChartPart* part, QWidget* parent, const char* name )
  : KoView( part, parent, name )
{
  setInstance( KChartFactory::global() );
  setXMLFile( "kchart.rc" );

  m_wizard = new KAction( i18n("Customize with &Wizard"),
			  "wizard", 0,
			  this, SLOT( wizard() ),
			  actionCollection(), "wizard");
  m_edit = new KAction( i18n("&Edit Data"), "data", 0,
			this, SLOT( edit() ),
			actionCollection(), "edit");
  m_config = new KAction( i18n( "&Config" ), "options", 0,
			  this, SLOT( config() ),
			  actionCollection(), "config" );
  m_loadconfig = new KAction( i18n("&Load Config"),
			      "loadconfig", 0, this,
			      SLOT( loadConfig() ),
			      actionCollection(), "loadconfig");
  m_saveconfig = new KAction( i18n("&Save Config"),
			      "saveconfig",	0, this,
			      SLOT( saveConfig() ),
			      actionCollection(), "saveconfig");
  m_defaultconfig = new KAction( i18n("&Default Config"),
				 "defaultconfig", 0, this,
				 SLOT( defaultConfig() ),
				 actionCollection(), "defaultconfig");

  m_chartpie = new KToggleAction( i18n("&Pie"), "cakes", 0, this,
				  SLOT( pieChart() ), actionCollection(),
				  "piechart");
  m_chartpie->setExclusiveGroup( "charttypes" );
  m_chartline = new KToggleAction( i18n("&Line"), "lines", 0, this,
				   SLOT( lineChart() ), actionCollection(),
				   "linechart");
  m_chartline->setExclusiveGroup( "charttypes" );
  m_chartareas = new KToggleAction( i18n("&Areas"), "areas", 0, this,
				    SLOT( areasChart() ), actionCollection(),
				    "areaschart");
  m_chartareas->setExclusiveGroup( "charttypes" );
  m_chartbars = new KToggleAction( i18n("&Bars"), "bars", 0, this,
				   SLOT( barsChart() ), actionCollection(),
				   "barschart");
  m_chartbars->setExclusiveGroup( "charttypes" );
  m_chartbars->setChecked( true );

  // initialize the configuration
  //    loadConfig();

  // make sure there is always some test data
  createTempData();
}

void KChartView::paintEvent( QPaintEvent* /*ev*/ )
{
  QPainter painter;
  painter.begin( this );

  // ### TODO: Scaling

  // Let the document do the drawing
  // PENDING(kalle) Do double-buffering if we are a widget
  //    part()->paintEverything( painter, ev->rect(), FALSE, this );
  // paint everything
  koDocument()->paintEverything( painter, rect(), FALSE, this );


  painter.end();
}

void KChartView::updateReadWrite( bool /*readwrite*/ )
{
#ifdef __GNUC__
#warning TODO
#endif
}

void KChartView::createTempData()
{
  int row, col;
  int nbrow,nbcol;

  KChartData *dat = ((KChartPart*)koDocument())->data();

  // initialize some data, if there is none
  nbrow=4;
  nbcol=4;
  if (dat->rows() == 0)
    {
	  kdDebug(35001) << "Initialize with some data!!!" << endl;
	  dat->expand(4,4);
	  for (row = 0;row < nbrow;row++)
	    for (col = 0;col < nbcol;col++)
		  {
			//	  _widget->fillCell(row,col,row+col);
			KChartValue t;
			t.exists= true;
			t.value = (double)row+col;
			kdDebug(35001) << "Set cell for " << row << "," << col << endl;
			dat->setCell(row,col,t);
		  }
	  //      _dlg->exec();
    }

  KChartParameters *params = ((KChartPart*)koDocument())->params();
  if(params->legend.isEmpty())
	{
	  for(unsigned int i=0;i<dat->rows();i++)
		{
		  QString tmp;
		  tmp="Legend "+tmp.setNum(i);
		  params->legend+=tmp;
		}
	}

  if(params->xlbl.isEmpty())
	{
	  for(unsigned int i=0;i<dat->cols();i++)
		{
		  QString tmp;
		  tmp="Year 200"+tmp.setNum(i);
		  params->xlbl+=tmp;
		}
	}

  QArray<int> tmpExp(dat->cols()*dat->rows());
  QArray<bool> tmpMissing(dat->cols()*dat->rows());

  for(unsigned int i=0; i<(dat->cols()*dat->rows()); ++i )
	{
	  tmpExp[i]=0;
	  tmpMissing[i]=FALSE;
	}
  if(params->missing.isEmpty())
	{
	  params->missing=tmpMissing;
  	}
  if(params->explode.isEmpty())
	{
	  params->explode=tmpExp;
	}

}


void KChartView::edit()
{
  kchartDataEditor ed;
  KChartParameters* params=((KChartPart*)koDocument())->params();

  KChartData *dat = (( (KChartPart*)koDocument())->data());
  ed.setData(dat);
  ed.setLegend(params->legend);
  ed.setXLabel(params->xlbl);
  if (ed.exec() != QDialog::Accepted) {
    return;
  }
  ed.getData(dat);
  ed.getLegend(params);
  ed.getXLabel(params);
  repaint();
}

void KChartView::wizard()
{
  kdDebug(35001) << "Wizard called" << endl;
  KChartWizard *wiz =
	new KChartWizard((KChartPart*)koDocument(), this, "KChart Wizard", true);
  kdDebug(35001) << "Executed. Now, display it" << endl;
  wiz->exec();
  repaint();
  kdDebug(35001) << "Ok, executed..." << endl;
}


void KChartView::config()
{
  // open a config dialog depending on the chart type
  KChartParameters* params = ((KChartPart*)koDocument())->params();

  KChartConfigDialog* d = new KChartConfigDialog( params, this );
  connect( d, SIGNAL( dataChanged() ),
		   this, SLOT( slotRepaint() ) );
  d->exec();
  delete d;
}


void KChartView::slotRepaint()
{
  repaint();
}


void KChartView::saveConfig() {
  kdDebug(35001) << "Save config..." << endl;
  ((KChartPart*)koDocument())->saveConfig( KGlobal::config() );
}

void KChartView::loadConfig() {
  kdDebug(35001) << "Load config..." << endl;
  KGlobal::config()->reparseConfiguration();
  ((KChartPart*)koDocument())->loadConfig( KGlobal::config() );
  //refresh chart when you load config
  repaint();
}

void KChartView::defaultConfig() {
  ((KChartPart*)koDocument())->defaultConfig(  );
  repaint();
}


void KChartView::pieChart() {
  KChartParameters* params = ((KChartPart*)koDocument())->params();
  params->type=KCHARTTYPE_2DPIE;
  params->stack_type=KCHARTSTACKTYPE_DEPTH;
  repaint();
}

void KChartView::lineChart() {
  KChartParameters* params = ((KChartPart*)koDocument())->params();
  params->type=KCHARTTYPE_3DLINE;
  params->stack_type=KCHARTSTACKTYPE_DEPTH;
  repaint();
}

void KChartView::barsChart() {
  KChartParameters* params = ((KChartPart*)koDocument())->params();
  params->type=KCHARTTYPE_3DBAR;
  params->stack_type=KCHARTSTACKTYPE_DEPTH;
  repaint();
}

void KChartView::areasChart() {
  KChartParameters* params = ((KChartPart*)koDocument())->params();
  params->type=KCHARTTYPE_3DAREA;
  params->stack_type=KCHARTSTACKTYPE_DEPTH;
  repaint();
}

#include "kchart_view.moc"
