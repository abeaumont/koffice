/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

// We want to access the signals
#include <qobjectdefs.h>
#ifdef signals
#undef signals
#define signals public
#endif

#include "kscript_ext_qhboxlayout.h"
#include "kscript_context.h"
#include "kscript_util.h"

#include <qlayout.h>

#undef OBJECT
#define OBJECT ((QHBoxLayout*)object())
#undef CHECKTYPE
#define CHECKTYPE( context, v, type ) if ( !checkType( context, v, type ) ) return FALSE;
#define RETURN_LEFTEXPR( n, value ) if ( name == n ) { KSValue::Ptr ptr = value; ptr->setMode( KSValue::LeftExpr ); return ptr; }
#define RETURN_RIGHTEXPR( n, value ) if ( name == n ) { return value; }
#define CHECK_LEFTEXPR( context, name ) if ( context.leftExpr() ) return KSObject::member( context, name );
#define SET_PROP( __n, __expr, __t ) if ( name == __n ) { CHECKTYPE( context, v, __t ); __expr; return TRUE; }

KSClass_QHBoxLayout::KSClass_QHBoxLayout( KSModule* m, const char* name ) : KSClass_QBoxLayout( m, name )
{
  nameSpace()->insert( "QHBoxLayout", new KSValue( (KSBuiltinMethod)&KSObject_QHBoxLayout::ksQHBoxLayout ) );
}

KSScriptObject* KSClass_QHBoxLayout::createObject( KSClass* c )
{
  return new KSObject_QHBoxLayout( c );
}

// ------------------------------------------------------

KSObject_QHBoxLayout::KSObject_QHBoxLayout( KSClass* c ) : KSObject_QBoxLayout( c )
{
}

bool KSObject_QHBoxLayout::ksQHBoxLayout( KSContext& context )
{
  qDebug("QHBoxLayout\n");

  if ( !checkDoubleConstructor( context, "QHBoxLayout" ) )
    return false;

  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  QWidget* parent = 0;
  QString name;

  if ( args.count() >= 1 )
  {
      if ( !checkArguments( context, context.value(), "QHBoxLayout::QHBoxLayout", KS_Qt_Object::WidgetType ) )
	  return FALSE;
      parent = KSObject_QWidget::convert( args[0] );
  }
  if ( args.count() >= 2 )
  {
      if ( !checkArguments( context, context.value(), "QHBoxLayout::QHBoxLayout", KS_Qt_Object::StringType ) )
	  return FALSE;
      name = args[1]->stringValue();
  }
  if ( args.count() > 2 )
  {
      KSUtil::tooFewArgumentsError( context, "QHBoxLayout::QHBoxLayout" );
      return FALSE;
  }

  setObject( new QHBoxLayout( parent, 6, 6, name.latin1() ) );

  return true;
}

KSValue::Ptr KSObject_QHBoxLayout::member( KSContext& context, const QString& name )
{
  CHECK_LEFTEXPR( context, name );

  return KS_Qt_Object::member( context, name );
}

bool KSObject_QHBoxLayout::setMember( KSContext& context, const QString& name, const KSValue::Ptr& v )
{
  return KS_Qt_Object::setMember( context, name, v );
}
