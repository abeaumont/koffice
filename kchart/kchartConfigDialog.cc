/*
 * $Id$
 *
 * Copyright 1999 by Kalle Dalheimer, released under Artistic License.
 */


#include "kchartConfigDialog.h"
#include "kchartConfigDialog.moc"

#include <kapp.h>
#include <klocale.h>

KChartConfigDialog::KChartConfigDialog( KChartParameters* params,
					QWidget* parent ) :
    QTabDialog( parent, "Chart config dialog", true ),
    _params( params )
{
    // Geometry page
    // PENDING(kalle) _xstep only for axes charts
    //_geompage = new KChartGeometryConfigPage( this );
    //addTab( _geompage, i18n( "&Geometry" ) );
    setCaption(i18n( "Chart config dialog" ));
	// Color page
    _colorpage = new KChartColorConfigPage( this );
    addTab( _colorpage, i18n( "&Colors" ) );

    if(!_params->isPie())
    	{
    	_parameterpage = new KChartParameterConfigPage(_params,this );
    	addTab( _parameterpage, i18n( "&Parameter" ) );

        }
    else
    	{
    	_parameterpiepage = new KChartParameterPieConfigPage(_params,this );
    	addTab( _parameterpiepage, i18n( "&Parameter" ) );

    	_piepage = new KChartPieConfigPage(_params, this );
    	addTab( _piepage, i18n( "&Pie" ) );
        }

    _parameterfontpage = new KChartFontConfigPage(_params,this );
    addTab( _parameterfontpage, i18n( "&Font" ) );

    if(!_params->isPie()&&_params->threeD())
    	{
        _parameter3dpage = new KChartParameter3dConfigPage(_params,this );
        addTab( _parameter3dpage,i18n("3D Parameters"));
        }
    //For the moment juste for 3 type of chart
    if((_params->type==KCHARTTYPE_BAR) || ((_params->type==KCHARTTYPE_3DBAR)
        ||(_params->type==KCHARTTYPE_3DLINE)))
                {
                _subTypePage = new KChartSubTypeChartPage(_params, this );
    	        addTab( _subTypePage, i18n( "Sub Type Chart" ) );
                }
    //init
    defaults();

    // setup buttons
    setDefaultButton( i18n( "Defaults" ) );
    setCancelButton( i18n( "Cancel" ) );

    connect( this, SIGNAL( applyButtonPressed() ), this, SLOT( apply() ) );
    connect( this, SIGNAL( defaultButtonPressed() ), this, SLOT( defaults() ) );
}


void KChartConfigDialog::apply()
{
    // Copy application data from dialog into parameter structure that is also
    // being used by the application.

    // color page
    _params->BGColor = _colorpage->backgroundColor();
    _params->GridColor = _colorpage->gridColor();
    _params->LineColor = _colorpage->lineColor();
    _params->PlotColor = _colorpage->plotColor();
    _params->VolColor = _colorpage->volColor();
    _params->TitleColor = _colorpage->titleColor();
    _params->XTitleColor = _colorpage->xTitleColor();
    _params->YTitleColor = _colorpage->yTitleColor();
    _params->YTitle2Color = _colorpage->yTitle2Color();
    _params->XLabelColor = _colorpage->xLabelColor();
    _params->YLabelColor = _colorpage->yLabelColor();
    _params->YLabel2Color = _colorpage->yLabel2Color();
    //_params->EdgeColor = _colorpage->edgeColor();

    if(!_params->isPie())
        {
        _parameterpage->apply();
        if(_params->threeD())
                _parameter3dpage->apply();

        }
    else
    	{
    	_parameterpiepage->apply();
    	_piepage->apply();
    	}

    _parameterfontpage->apply();
    if((_params->type==KCHARTTYPE_BAR) || ((_params->type==KCHARTTYPE_3DBAR)
        ||(_params->type==KCHARTTYPE_3DLINE)))
        {
        _subTypePage->apply();
        }
//     for( uint i = 0; i < NUMDATACOLORS; i++ )
// 	_params->_datacolors.setColor( i, _colorpage->dataColor( i ) );
}


void KChartConfigDialog::defaults()
{
    // Fill pages with values

    // color page
    _colorpage->setBackgroundColor( _params->BGColor );
    _colorpage->setGridColor( _params->GridColor );
    _colorpage->setLineColor( _params->LineColor );
    _colorpage->setPlotColor( _params->PlotColor );
    _colorpage->setVolColor( _params->VolColor );
    _colorpage->setTitleColor( _params->TitleColor );
    _colorpage->setXTitleColor( _params->XTitleColor );
    _colorpage->setYTitleColor( _params->YTitleColor );
    _colorpage->setYTitle2Color( _params->YTitle2Color );
    _colorpage->setXLabelColor( _params->XLabelColor );
    _colorpage->setYLabelColor( _params->YLabelColor );
    _colorpage->setYLabel2Color( _params->YLabel2Color );
    //_colorpage->setEdgeColor( _params->EdgeColor );

    if(!_params->isPie())
        {
    	_parameterpage->init();
        if(_params->threeD())
                _parameter3dpage->init();

        }
    else
    	{
    	_parameterpiepage->init();
    	_piepage->init();
    	}

    _parameterfontpage->init();
    if((_params->type==KCHARTTYPE_BAR) || ((_params->type==KCHARTTYPE_3DBAR)
        ||(_params->type==KCHARTTYPE_3DLINE)))
        {
        _subTypePage->init();
        }
//     for( uint i = 0; i < NUMDATACOLORS; i++ )
// 	_colorpage->setDataColor( i, _params->_datacolors.color( i ) );
}

