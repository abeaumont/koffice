/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KFORMULAMIMESOURCE_H
#define KFORMULAMIMESOURCE_H

#include <qmime.h>
#include <qdom.h>

#include "kformulacontainer.h"
#include "symboltable.h"

KFORMULA_NAMESPACE_BEGIN
class FormulaElement;


class MimeSource : public QMimeSource, public FormulaDocument
{
public:
    MimeSource(QDomDocument formula);
    ~MimeSource();

    virtual const char* format ( int n = 0 ) const;
    virtual bool provides ( const char * ) const;
    virtual QByteArray encodedData ( const char * ) const;

    virtual void elementRemoval(BasicElement* child);
    virtual void changed();
    virtual void cursorHasMoved( FormulaCursor* );
    virtual const SymbolTable& getSymbolTable() const;

private:

    QDomDocument document;
    QByteArray latexString;
    SymbolTable table;

    FormulaElement* rootElement;
};

KFORMULA_NAMESPACE_END

#endif // KFORMULAMIMESOURCE_H
