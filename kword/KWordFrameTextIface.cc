/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>

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

#include "KWordFrameTextIface.h"

#include "kwtextframeset.h"
#include <kotextobject.h>
#include <kapplication.h>
#include <dcopclient.h>

KWordFrameTextIface::KWordFrameTextIface( KWTextFrameSet *_frame )
    : DCOPObject()
{
   m_frametext = _frame;
}

bool KWordFrameTextIface::hasSelection() const
{
    return m_frametext->hasSelection();
}

int KWordFrameTextIface::numberOfParagraphs()
{
    return m_frametext->paragraphs();
}

QString KWordFrameTextIface::name() const
{
    return m_frametext->getName();
}

bool KWordFrameTextIface::isVisible() const
{
    return m_frametext->isVisible();
}

bool KWordFrameTextIface::isFloating() const
{
    return m_frametext->isFloating();
}

bool KWordFrameTextIface::isAHeader() const
{
    return m_frametext->isAHeader();
}

bool KWordFrameTextIface::isAFooter() const
{
    return m_frametext->isAFooter();
}

QString KWordFrameTextIface::selectedText() const
{
    return m_frametext->selectedText();
}

void KWordFrameTextIface::selectAll( bool select )
{
    m_frametext->textObject()->selectAll(select);
}
