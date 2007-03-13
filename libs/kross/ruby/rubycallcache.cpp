/***************************************************************************
 * rubycallcache.cpp
 * This file is part of the KDE project
 * copyright (C)2006 by Cyrille Berger (cberger@cberger.net)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "rubycallcache.h"
#include "rubyvariant.h"
#include "rubyinterpreter.h"

#include <QVariant>
#include <QMetaObject>
#include <QMetaMethod>

namespace Kross {

    struct RubyCallCachePrivate
    {
        RubyCallCachePrivate(QObject* nobject, int nmethodindex, bool nhasreturnvalue, QVarLengthArray<int> ntypes, QVarLengthArray<int> nmetatypes)
            : object(nobject), methodindex(nmethodindex), hasreturnvalue(nhasreturnvalue), types(ntypes), metatypes(nmetatypes)
        {
        }
        QObject* object;
        int methodindex;
        bool hasreturnvalue;
        QVarLengthArray<int> types, metatypes;
        static VALUE s_rccObject;
    };

    VALUE RubyCallCachePrivate::s_rccObject = 0;

    RubyCallCache::RubyCallCache(QObject* object, int methodindex, bool hasreturnvalue, QVarLengthArray<int> ntypes, QVarLengthArray<int> nmetatypes)
        : d(new RubyCallCachePrivate(object, methodindex, hasreturnvalue, ntypes, nmetatypes)), m_self(0)
    {
    }

    RubyCallCache::~RubyCallCache()
    {
        delete d;
    }

    VALUE RubyCallCache::execfunction( int argc, VALUE *argv )
    {
        int typelistcount = d->types.count();
        QVarLengthArray<MetaType*> variantargs( typelistcount );
        QVarLengthArray<void*> voidstarargs( typelistcount );

        #ifdef KROSS_RUBY_EXTENSION_DEBUG
            QMetaMethod metamethod = d->object->metaObject()->method(d->methodindex);
            krossdebug( QString("RubyCallCache::execfunction signature=%1 typeName=%2 argc=%3 typelistcount=%4").arg(metamethod.signature()).arg(metamethod.typeName()).arg(argc).arg(typelistcount) );
            for(int i = 0; i < d->types.count(); ++i)
                krossdebug( QString("  argument index=%1 typeId=%2 typeName=%3 metaTypeId=%4").arg(i).arg(d->types[i]).arg(QVariant::typeToName( (QVariant::Type)d->types[i] )).arg(d->metatypes[i]) );
        #endif

        Q_ASSERT(argc >= typelistcount);

        // set the return value
        if(d->hasreturnvalue)
        {
            MetaType* returntype = RubyMetaTypeFactory::create( d->types[0], d->metatypes[0] );
            variantargs[0] = returntype;
            voidstarargs[0] = returntype->toVoidStar();
        }
        else
        {
            variantargs[0] = 0;
            voidstarargs[0] = (void*)0;
        }

        // set the arguments values
        for(int idx = 1; idx < typelistcount; ++idx)
        {
            //krossdebug( QString("-----------> %1").arg( STR2CSTR( rb_inspect(argv[idx]) ) ) );
            MetaType* metatype = RubyMetaTypeFactory::create( d->types[idx], d->metatypes[idx], argv[idx] );
            if(! metatype) { // Seems RubyMetaTypeFactory::create returned an invalid RubyType.
                krosswarning( QString("RubyExtension::callMetaMethod Aborting cause RubyMetaTypeFactory::create returned NULL.") );
                for(int i = 0; i < idx; ++i) // Clear already allocated instances.
                    delete variantargs[i];
                return Qfalse; // abort execution.
            }
            variantargs[idx] = metatype;
            voidstarargs[idx] = metatype->toVoidStar();
        }

        // call the method now
        int r = d->object->qt_metacall(QMetaObject::InvokeMetaMethod, d->methodindex, &voidstarargs[0]);
        #ifdef KROSS_RUBY_EXTENSION_DEBUG
            krossdebug( QString("RESULT nr=%1").arg(r) );
        #else
            Q_UNUSED(r);
        #endif

        // free the arguments
        for(int idx = 1; idx < typelistcount; ++idx)
        {
            delete variantargs[idx];
        }

        // eval the return-value
        if(d->hasreturnvalue)
        {
            QVariant result(variantargs[0]->typeId(), variantargs[0]->toVoidStar());
            #ifdef KROSS_RUBY_EXTENSION_DEBUG
                QMetaMethod metamethod = d->object->metaObject()->method(d->methodindex);
                krossdebug( QString("RubyExtension::callMetaMethod Returnvalue typeId=%1 metamethod.typename=%2 variant.toString=%3 variant.typeName=%4").arg(variantargs[0]->typeId()).arg(metamethod.typeName()).arg(result.toString()).arg(result.typeName()) );
            #endif
            // free the return argument
            delete variantargs[0];
            // return the result
            return result.isNull() ? 0 : RubyType<QVariant>::toVALUE(result);
        }
        return 0;
    }

    void RubyCallCache::delete_object(void* object)
    {
        #ifdef KROSS_RUBY_EXTENSION_DEBUG
            krossdebug("RubyCallCache::delete_object");
        #endif
        RubyCallCache* extension = static_cast< RubyCallCache* >(object);
        delete extension;
        extension = 0;
    }

    VALUE RubyCallCache::method_cacheexec(int argc, VALUE *argv, VALUE self)
    {
        #ifdef KROSS_RUBY_EXTENSION_DEBUG
            krossdebug("RubyCallCache::method_cacheexec");
        #endif
        RubyCallCache* callcache;
        Data_Get_Struct(self, RubyCallCache, callcache);
        return callcache->execfunction(argc, argv);
    }

    VALUE RubyCallCache::toValue()
    {
        if(m_self == 0)
        {
            if(RubyCallCachePrivate::s_rccObject  == 0)
            {
                RubyCallCachePrivate::s_rccObject = rb_define_class_under(RubyInterpreter::krossModule(), "CallCache", rb_cObject);
                rb_define_method(RubyCallCachePrivate::s_rccObject, "cacheexec",  (VALUE (*)(...))RubyCallCache::method_cacheexec, -1);
            }
            m_self = Data_Wrap_Struct(RubyCallCachePrivate::s_rccObject, 0, RubyCallCache::delete_object, this);
        }
        return m_self;
    }

}
