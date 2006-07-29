/* This file is part of the KDE project
   Copyright (C) 2005 Adam Pigg <adam@piggz.co.uk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
//
// C++ Implementation: pqxxsqlconnectioninternal
//
// Description: 
//
//
// Author: Adam Pigg <adam@piggz.co.uk>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pqxxconnection_p.h"
#include <kdebug.h>

using namespace KexiDB;
pqxxSqlConnectionInternal::pqxxSqlConnectionInternal(Connection *conn)
 : ConnectionInternal(conn)
 , m_pqxxsql(0)
 , m_res(0)
{
}


pqxxSqlConnectionInternal::~pqxxSqlConnectionInternal()
{

}

void pqxxSqlConnectionInternal::storeResult()
{
	errmsg = "";
}
