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
// C++ Implementation: pqxxpreparedstatement
//
// Description: 
//
//
// Author: Adam Pigg <adam@piggz.co.uk>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
#include "pqxxpreparedstatement.h"
#include <kdebug.h>
using namespace KexiDB;

pqxxPreparedStatement::pqxxPreparedStatement(
	StatementType type, ConnectionInternal& conn, FieldList& fields) 
 : KexiDB::PreparedStatement(type, conn, fields)
 , m_conn(conn.connection)
{
//	KexiDBDrvDbg << "pqxxPreparedStatement: Construction" << endl;
}


pqxxPreparedStatement::~pqxxPreparedStatement()
{
}

bool pqxxPreparedStatement::execute()
{
//	KexiDBDrvDbg << "pqxxPreparedStatement::execute()" << endl;
	m_resetRequired = true;
	if (m_conn->insertRecord(*m_fields, m_args)) {
		return true;
	}
	return false;
}


