/* This file is part of the KDE project
   Copyright (c) 2004 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#include "liststylestack.h"
#include "ooutils.h" // for ooNS

ListStyleStack::ListStyleStack()
        : m_initialLevel(0)
{
}

ListStyleStack::~ListStyleStack()
{
}

void ListStyleStack::pop()
{
    m_stack.pop();
}

void ListStyleStack::push(const KXmlElement& style)
{
    m_stack.push(style);
}

void ListStyleStack::setInitialLevel(int initialLevel)
{
    Q_ASSERT(m_stack.isEmpty());
    m_initialLevel = initialLevel;
}

KXmlElement ListStyleStack::currentListStyle() const
{
    Q_ASSERT(!m_stack.isEmpty());
    return m_stack.top();
}

KXmlElement ListStyleStack::currentListStyleProperties() const
{
    KXmlElement style = currentListStyle();
    return KoXml::namedItemNS(style, ooNS::style, "properties");
}
