/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2001 Igor Janssen (rm@linux.ru.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "CreateOvalCmd.h"

#include <klocale.h>

#include "GDocument.h"
#include "GPage.h"
#include "GOval.h"

CreateOvalCmd::CreateOvalCmd(GDocument *aGDoc, GOval *oval):
Command(aGDoc, i18n("Create Oval"))
{
  object = oval;
}

CreateOvalCmd::CreateOvalCmd(GDocument *aGDoc, const KoPoint &p0, const KoPoint &p1, bool flag):
Command(aGDoc, i18n("Create Oval"))
{
  object = 0L;
  spos = p0;
  epos = p1;
  cflag = flag;
}

CreateOvalCmd::~CreateOvalCmd()
{
//  if(object)
//    object->unref();
}

void CreateOvalCmd::execute()
{
  if(!object)
  {
    /* Create oval */
    object = new GOval(cflag);
    object->startPoint(spos);
    object->endPoint(epos);
    //object->ref();
  }
  document()->activePage()->insertObject(object);
}

void CreateOvalCmd::unexecute()
{
  document()->activePage()->deleteObject(object);
}
