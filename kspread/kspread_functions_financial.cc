// built-in financial functions
#include "kspread_util.h"
#include "kspread_doc.h"
#include "kspread_table.h"

#include <koscript_parser.h>
#include <koscript_util.h>
#include <koscript_func.h>
#include <koscript_synext.h>

#include <stdlib.h>
#include <math.h>
#include <float.h>

// Function: FV
/* Returns future value, given current value, interest rate and time */
bool kspreadfunc_fv( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "FV", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;

  double present = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();

  context.setValue( new KSValue( present * pow(1+interest, periods)));
  return true;
}

// Function: compound
/* Returns value after compounded interest, given principal, rate, periods
per year and year */
 bool kspreadfunc_compound( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 4, "compound", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;
  double principal = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();
  double years = args[3]->doubleValue();

  context.setValue( new KSValue( principal * pow(1+(interest/periods),
periods*years)));

  return true;
}

// Function: continuous
/* Returns value after continuous compounding of interest, given prinicpal,
rate and years */
bool kspreadfunc_continuous( KSContext& context )
{
    // If you still don't understand this, let me know!  ;-)  jsinger@leeta.net
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "continuous", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double principal = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double years = args[2]->doubleValue();


  context.setValue( new KSValue( principal * exp(interest * years)));
  return true;
}

// Function: PV
bool kspreadfunc_pv( KSContext& context )
{
/* Returns presnt value, given future value, interest rate and years */
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "PV", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double future = args[0]->doubleValue();
  double interest = args[1]->doubleValue();
  double periods = args[2]->doubleValue();


  context.setValue( new KSValue( future / pow(1+interest, periods)));
  return true;
}

// Function: PV_annuity
bool kspreadfunc_pv_annuity( KSContext& context )
{
    /* Returns present value of an annuity or cash flow, given payment,
       interest rate,
       periods, initial amount and whether payments are made at the start (TRUE)
       or end of a period */

    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 3, "PV_annuity", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
	return false;
    if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
	return false;
    if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
	return false;
    double amount = args[0]->doubleValue();
    double interest = args[1]->doubleValue();
    double periods = args[2]->doubleValue();

    double result;
    result = amount * (1 - 1/(pow( (1+interest), periods ))) / interest ;

  context.setValue( new KSValue( result ) );

  return true;
}

// Function: FV_annnuity
bool kspreadfunc_fv_annuity( KSContext& context )
{
    /* Returns future value of an annuity or cash flow, given payment, interest
       rate and periods */

    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 3, "FV_annuity", true ) )
	return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
	return false;
    if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
	return false;
    if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
	return false;
    double amount= args[0]->doubleValue();
    double interest = args[1]->doubleValue();
    double periods = args[2]->doubleValue();

    double result;

    result = amount * ((pow( (1+interest),periods))/interest - 1/interest)   ;

    context.setValue( new KSValue( result ) );

    return true;
}

// Function: effective
bool kspreadfunc_effective( KSContext& context )
{
/* Returns effective interest rate given nominal rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "effective", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  double nominal = args[0]->doubleValue();
  double periods = args[1]->doubleValue();

  context.setValue( new KSValue(  pow( 1 + (nominal/periods), periods )- 1 ) );

  return true;
}

// Function: zero_coupon
bool kspreadfunc_zero_coupon( KSContext& context )
{
/* Returns effective interest rate given nominal rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "zero_coupon", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  double face = args[0]->doubleValue();
  double rate = args[1]->doubleValue();
  double years = args[2]->doubleValue();

  context.setValue( new KSValue(  face / pow( (1 + rate), years )  ) );

  return true;
}

// Function: level_coupon
bool kspreadfunc_level_coupon( KSContext& context )
{
/* Returns effective interest rate given nominal rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 5, "level_coupon", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[2], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[3], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[4], KSValue::DoubleType, true ) )
    return false;
  double face = args[0]->doubleValue();
  double coupon_rate = args[1]->doubleValue();
  double coupon_year = args[2]->doubleValue();
  double years = args[3]->doubleValue();
  double market_rate = args[4]->doubleValue();

  double coupon = coupon_rate * face / coupon_year;
  double interest =  market_rate/coupon_year;
  double pv_annuity = (1 - 1/(pow( (1+interest), (years*coupon_year) ))) / interest ;
  context.setValue( new KSValue( coupon * pv_annuity + (face/ pow( (1+interest), (years*coupon_year) ) ) ) );

  return true;
}

// Function: nominal
bool kspreadfunc_nominal( KSContext& context )
{
/* Returns nominal interest rate given effective rate and periods per year */

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "nominal", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;
  double effective = args[0]->doubleValue();
  double periods = args[1]->doubleValue();

  if ( periods == 0.0 ) // Check null
      return false;

  context.setValue( new KSValue( periods * (pow( (effective + 1), (1 / periods) ) -1) ) );

  return true;
}
