// built-in logical functions
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

// Function: NOT
bool kspreadfunc_not( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "NOT", true ) || !KSUtil::checkArgumentsCount( context, 1, "not", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::BoolType, true ) )
    return false;

  bool toto = !args[0]->boolValue();
  context.setValue( new KSValue(toto));
  return true;
}

bool kspreadfunc_or_helper( KSContext& context, QValueList<KSValue::Ptr>& args, bool& first )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_or_helper( context, (*it)->listValue(), first ) )
        return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::BoolType, true ) )
      first = (first || (*it)->boolValue());
    else
      return false;
  }

  return true;
}

// Function: OR
bool kspreadfunc_or( KSContext& context )
{
  bool first = false;
  bool b = kspreadfunc_or_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( first ) );

  return b;
}

// Function: NOR
bool kspreadfunc_nor( KSContext& context )
{
  bool first = false;
  bool b = kspreadfunc_or_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( !first ) );

  return b;
}


bool kspreadfunc_and_helper( KSContext& context, QValueList<KSValue::Ptr>& args, bool& first )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_and_helper( context, (*it)->listValue(), first ) )
        return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::BoolType, true ) )
      first = first && (*it)->boolValue();
    else
      return false;
  }

  return true;
}

// Function: AND
bool kspreadfunc_and( KSContext& context )
{
  bool first = true;
  bool b = kspreadfunc_and_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( first ) );

  return b;
}

// Function: NAND
bool kspreadfunc_nand( KSContext& context )
{
  bool first = true;
  bool b = kspreadfunc_and_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( !first ) );

  return b;
}

static bool kspreadfunc_xor_helper( KSContext& context, QValueList<KSValue::Ptr>& args, bool& first )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();

  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_and_helper( context, (*it)->listValue(), first ) )
        return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::BoolType, true ) )
      first = first ^ (*it)->boolValue();
    else
      return false;
  }

  return true;
}

// Function: XOR
bool kspreadfunc_xor( KSContext& context )
{
  bool first = true;
  bool b = kspreadfunc_xor_helper( context, context.value()->listValue(), first );

  if ( b )
    context.setValue( new KSValue( first ) );

  return b;
}

// Function: IF
bool kspreadfunc_if( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 3, "if", true ) || !KSUtil::checkArgumentsCount( context, 3, "IF", true ))
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::BoolType, true ) )
      return false;

    if (  args[0]->boolValue() == true )
      context.setValue( new KSValue( *(args[1]) ) );
    else
      context.setValue( new KSValue( *(args[2]) ) );

    return true;
}
