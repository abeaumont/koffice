
#include "engine.h"
#include "engine2.h"
#include "enginehelper.h"
#include "enginedraw.h"

#include "kchartparams.h"
#include "kchartEngine.h"

#include <qfont.h>
#include <qcolor.h>
#include <qpainter.h>

#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h> //PENDING(kalle) Remove?


void kchartEngine::computeSize() {
  int	title_hgt  = !params->title.isEmpty()? 2 /* title? horizontal text line(s) */
    + cnt_nl(params->title.latin1(),(int*)NULL)*params->titleFontHeight()
    + 2:
    2;
  int	xlabel_hgt = 0;
  int	xtitle_hgt = !params->xtitle.isEmpty()? 1+params->xTitleFontHeight()+1: 0; 
  int	ytitle_hgt = !params->ytitle.isEmpty()? 1+params->yTitleFontHeight()+1: 0;
  int	vtitle_hgt = params->do_vol() && !params->ytitle2.isEmpty()? 1+params->yTitleFontHeight()+1: 0;
  int	ylabel_wth = 0;
  int	vlabel_wth = 0;
  
  int	xtics       = params->grid||params->xaxis? 1+2: 0;
  int	ytics       = params->grid||params->yaxis? 1+3: 0;
  int	vtics       = params->yaxis&& params->do_vol() ? 3+1: 0;
  
  qDebug( "done width, height, tics computation" );
  
  
  xdepth_3D      = params->threeD() ? (int)( cos(RAD_DEPTH) * HYP_DEPTH ): 0;
  ydepth_3D      = params->threeD() ? (int)( sin(RAD_DEPTH) * HYP_DEPTH ): 0;
  xdepth_3Dtotal = xdepth_3D*(params->stack_type==KCHARTSTACKTYPE_DEPTH? num_hlc_sets? num_hlc_sets:
			      num_sets:
			      1 );
  ydepth_3Dtotal = ydepth_3D*(params->stack_type==KCHARTSTACKTYPE_DEPTH? num_hlc_sets? num_hlc_sets:
			      num_sets:
			      1 );
  annote_hgt = params->annotation && *(params->annotation->note)?
    1 +											/* space to note */
    (1+params->annotationFontHeight()) *		/* number of '\n' substrs */
    cnt_nl(params->annotation->note,&annote_len) +
    1 +											/* space under note */
    2: 0;										/* space to chart */
  annote_len *= params->annotationFontWidth();
  
  qDebug( "done annote_len computation" );
  
  // for the time being: no x labels
  hasxlabels = 0;
  /*	
	if( params->xaxis && hasxlabels ) {
	int biggest     = -MAXINT;
	
	int		len = 0;
	for( i=0; i<num_points; ++i ) {
	biggest = MAX( len, xlbl[i].length() );
	}
	
	xlabel_hgt = 1+ biggest*params->xAxisFontWidth() +1;
	}
  */
  
  grapheight = imageheight - ( xtics          +
			       xtitle_hgt     +
			       xlabel_hgt     +
			       title_hgt      +
									 annote_hgt     +
			       ydepth_3Dtotal +
			       2 );
  if( params->hard_size && params->hard_graphheight )				/* user wants to use his */
    grapheight = params->hard_graphheight;
  params->hard_graphheight = grapheight;
  
  qDebug( "done grapheight computation" );
  
  // before width can be known...
  /* ----- y labels intervals ----- */
  {
    float	tmp_highest;
    /* possible y gridline points */
    float	ypoints[] = { 1.0/64.0, 1.0/32.0, 1.0/16.0, 1.0/8.0, 1.0/4.0, 1.0/2.0,
				1.0,      2.0,      3.0,      5.0,     10.0,    25.0,
				50.0,     100.0,    250.0,    500.0,   1000.0,  2500,    5000.0,
				10000.0,  25000.0,  50000.0,  100000.0,500000.0,1000000, 5000000,
				10000000 };
    int		max_num_ylbls;
    int		longest_ylblen = 0;
    /* maximum y lables that'll fit... */
    max_num_ylbls = grapheight / (3+params->yAxisFontHeight());
    if( max_num_ylbls < 3 ) {
				/* gdImageDestroy(im);		haven't yet created it */
      debug( "Insufficient Height" );
      //      return 2;
      return;
    }
    cerr << "Here it is still ok!\n";
    /* one "space" interval above + below */
    int jumpout_value = NUM_YPOINTS-1;
    for(int i=1; i<NUM_YPOINTS; ++i ) {
				// if( ypoints[i] > ylbl_interval )
				//	break;
      if( (highest-lowest)/ypoints[i] < ((float)max_num_ylbls-(1.0+1.0))
	  * (float)params->ylabel_density/100.0 ) {
	jumpout_value = i;
	break;
      }
    }
    //#warning "Hack - setting yinterval to 1.0"
    //    params->requested_yinterval = 1.0;

    /* gotta go through the above loop to catch the 'tweeners :-| */
    ylbl_interval = params->requested_yinterval != -MAXDOUBLE &&
      params->requested_yinterval > ypoints[jumpout_value-1] ? 
      params->requested_yinterval:
      ypoints[jumpout_value-1];
    cout << "ylbl_interval:" << ylbl_interval << "\n";
    // FIXME: This seems to be a total mess: != ???
    
    /* perform floating point remainders */
    /* gonculate largest interval-point < lowest */
    if( lowest != 0.0 &&
	lowest != params->requested_ymin ) {
      if( lowest < 0.0 )
	lowest -= ylbl_interval;
				// lowest = (lowest-ypoints[0]) -
				// 			( ( ((lowest-ypoints[0])/ylbl_interval)*ylbl_interval ) -
				// 			   ( (float)((int)((lowest-ypoints[0])/ylbl_interval))*ylbl_interval ) );
      lowest = ylbl_interval * (float)(int)((lowest-ypoints[0])/ylbl_interval);
    }
    cerr << "Alive and healthy!\n";
    
    /* find smallest interval-point > highest */
    tmp_highest = lowest;
#warning "Kalle take a look at this, this is an infinite cycle for some reason"
    int maxcount = 0;
    do {	// while( (tmp_highest += ylbl_interval) <= highest )
      int		nmrtr, dmntr, whole;
      char	*price_to_str( float, int*, int*, int*, const char* );
      int		lbl_len;
      char	foo[32];
      
      if( params->yaxis )	{ /* XPG2 compatibility */
	//cerr << "At least I am doing something\n";
	sprintf( foo, do_ylbl_fractions ? QString( "%.0f" ): params->ylabel_fmt, tmp_highest );
	lbl_len = ylbl_interval<1.0? strlen( price_to_str(tmp_highest,
							  &nmrtr,
							  &dmntr,
							  &whole,
							  do_ylbl_fractions? QString::null: params->ylabel_fmt) ):
	  strlen( foo );
	longest_ylblen = MAX( longest_ylblen, lbl_len );
      }
      cerr << "tmp_highest" <<  tmp_highest << " highest " << highest << "ylbl_interval" << ylbl_interval << "\n";
      
      tmp_highest += ylbl_interval;
      //break; // force it
    } while( tmp_highest <= highest); // BL.
    
    ylabel_wth = longest_ylblen * params->yAxisFontWidth();
    highest = params->requested_ymax==-MAXDOUBLE? tmp_highest:
      MAX( params->requested_ymax, highest );
    
    if( params->do_vol() ) {
      float	num_yintrvls = (highest-lowest) / ylbl_interval;
				/* no skyscrapers */
      if( vhighest != 0.0 )
	vhighest += (vhighest-vlowest) / (num_yintrvls*2.0);
      if( vlowest != 0.0 )
	vlowest -= (vhighest-vlowest) / (num_yintrvls*2.0);
      
      if( params->yaxis2 ) {
	char	svlongest[32];
	int		lbl_len_low  = sprintf( svlongest, !params->ylabel2_fmt.isEmpty()? params->ylabel2_fmt: QString( "%.0f" ), vlowest );
	int		lbl_len_high = sprintf( svlongest, !params->ylabel2_fmt.isEmpty()? params->ylabel2_fmt: QString( "%.0f" ), vhighest );
	vlabel_wth = 1
	  + MAX( lbl_len_low,lbl_len_high ) * params->yAxisFontWidth();
      }
    }
  }
  
  graphwidth = imagewidth - ( ( (params->hard_size && params->hard_xorig)? params->hard_xorig:
				( ytitle_hgt +
				  ylabel_wth +
				  ytics ) )
			      + vtics
			      + vtitle_hgt
			      + vlabel_wth
			      + xdepth_3Dtotal );
  if( params->hard_size && params->hard_graphwidth )				/* user wants to use his */
    graphwidth = params->hard_graphwidth;
  params->hard_graphwidth = graphwidth;
  
  qDebug( "done graphwidth computation" );
  
  /* ----- scale to gif size ----- */
  /* offset to 0 at lower left (where it should be) */
  xscl = (float)(graphwidth-xdepth_3Dtotal) / (float)(num_points + (params->do_bar()?2:0));
  yscl = -((float)grapheight) / (float)(highest-lowest);
  if( params->do_vol() ) {
    float	hilow_diff = vhighest-vlowest==0.0? 1.0: vhighest-vlowest;
    
    vyscl = -((float)grapheight) / hilow_diff;
    vyorig = (float)grapheight
      + ABS(vyscl) * MIN(vlowest,vhighest)
      + ydepth_3Dtotal
      + title_hgt
      + annote_hgt;
  }
  xorig = (float)( imagewidth - ( graphwidth +
				  vtitle_hgt +
				  vtics      +
				  vlabel_wth ) );
  if( params->hard_size && params->hard_xorig )
    xorig = params->hard_xorig;
  params->hard_xorig = (int)xorig;
  //	yorig = (float)grapheight + ABS(yscl * lowest) + ydepth_3Dtotal + title_hgt;
  yorig = (float)grapheight
    + ABS(yscl) * MIN(lowest,highest)
    + ydepth_3Dtotal
    + title_hgt
    + annote_hgt;
  //????	if( params->hard_size && params->hard_yorig )					/* vyorig too? */
  //????		yorig = params->hard_yorig;
  params->hard_yorig = (int)yorig;
  
  hlf_barwdth     = (int)( (float)(PX(2)-PX(1)) * (((float)params->bar_width/100.0)/2.0) );	// used only for bars
  hlf_hlccapwdth  = (int)( (float)(PX(2)-PX(1)) * (((float)params->hlc_cap_width/100.0)/2.0) );
}




void kchartEngine::computeMinMaxValues() {
  switch(params->stack_type) {
  case KCHARTSTACKTYPE_SUM:	// need to walk sideways
    qDebug("Sum stacktype");
    for(int j=0; j<num_points; ++j ) {
      float set_sum = 0.0;
      for(int i=0; i<num_sets; ++i ) {
	//debug( "vor dem crash" );
	if( CELLEXISTS( i, j ) ) {
	  //debug( "nach dem crash" );
	  set_sum += CELLVALUE( i, j );
	  highest = MAX( highest, set_sum );
	  lowest  = MIN( lowest,  set_sum );
	}
      }
    }
    break;
  case  KCHARTSTACKTYPE_LAYER: // need to walk sideways
    qDebug("Layer stacktype");
    for(int j=0; j<num_points; ++j ) {
      float neg_set_sum = 0.0, pos_set_sum = 0.0;
      for(int i=0; i<num_sets; ++i )
	if( CELLEXISTS( i, j ) )
	  if( CELLVALUE( i, j ) < 0.0 )
	    neg_set_sum += CELLVALUE( i, j );
	  else
	    pos_set_sum += CELLVALUE( i, j );
      lowest  = MIN( lowest,  MIN(neg_set_sum,pos_set_sum) );
      highest = MAX( highest, MAX(neg_set_sum,pos_set_sum) );
    }
    break;
  default:
    qDebug("Other stacktype");
    for(int i=0; i<num_sets; ++i ) {
      for(int j=0; j<num_points; ++j ) {
	//debug( "Vor dem crash" );
	if( CELLEXISTS( i, j ) ) {
	  //debug( "nach dem crash" );
	  cerr << "cellvalue " << i << "," << j << "=" << CELLVALUE(i,j) << "\n";
	  highest = MAX( CELLVALUE( i, j ), highest );
	  lowest  = MIN( CELLVALUE( i, j ), lowest );
	}
      }
    }
  }
  qDebug( "done computation highest and lowest value" );
  cerr << "Highest:" << highest << " lowest " << lowest << "\n";
    
  if( params->scatter )
    cerr << "doing scattering?\n";
    for(int i=0; i<params->num_scatter_pts; ++i ) {
      highest = MAX( ((params->scatter)+i)->val, highest );
      lowest  = MIN( ((params->scatter)+i)->val, lowest  );
    }

  cerr << "Highest:" << highest << " lowest " << lowest << "\n";

  if( params->do_vol() ) { // for now only one combo set allowed
    // vhighest = 1.0;
    // vlowest  = 0.0;
    for(int j=0; j<num_points; ++j )
      if( uvol[j] != GDC_NOVALUE ) {
	vhighest = MAX( uvol[j], vhighest );
	vlowest  = MIN( uvol[j], vlowest );
      }
    if( vhighest == -MAXFLOAT )			// no values
      vhighest = 1.0;						// for scaling, need a range
    else if( vhighest < 0.0 )
      vhighest = 0.0;
    if( vlowest > 0.0 || vlowest == MAXFLOAT )
      vlowest = 0.0;						// vol should always start at 0
  }
    
  debug( "done vlowest computation" );
  
  if( lowest == MAXFLOAT )
    lowest = 0.0;
  if( highest == -MAXFLOAT )
    highest = 1.0;							// need a range
  if( params->type == KCHARTTYPE_AREA  ||					// bars and area should always start at 0
      params->type == KCHARTTYPE_BAR   ||
      params->type == KCHARTTYPE_3DBAR ||
      params->type == KCHARTTYPE_3DAREA )
    if( highest < 0.0 )
      highest = 0.0;
    else if( lowest > 0.0 )						// negs should be drawn from 0
      lowest = 0.0;
  
  if( params->requested_ymin != -MAXDOUBLE && params->requested_ymin < lowest )
    lowest = params->requested_ymin;
  if( params->requested_ymax != -MAXDOUBLE && params->requested_ymax > highest )
    highest = params->requested_ymax;
  
  qDebug( "done requested_* computation" );
}
