/*
 * Copyright (C) 2007 Igor Stepin <igor_for_os@stepin.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Generic.h"
#include <QDomDocument>
#include <QDomElement>
using namespace kcollaborate::Message;

Generic::Generic( QObject *parent )
        : QObject( parent )
{}

Generic::~Generic()
{}

const QString Generic::toString() const
{
    QDomDocument document;
    QDomElement element = document.createElement( tagName() );
    document.appendChild(element);
    toXML( document, element );
    return document.toString();
}

#include "Generic.moc"
