/*
** A program to convert the XML rendered by KWord into LATEX.
**
** Copyright (C) 2000 Robert JACOLIN
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** To receive a copy of the GNU Library General Public License, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
*/

#include <kdebug.h>		/* for kdDebug stream */
#include "listtable.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
ListTable::ListTable()
{

}

/*******************************************/
/* Destructor                              */
/*******************************************/
ListTable::~ListTable()
{
	/* Just call the parent destructor */
}

/*******************************************/
/* IsNewTable                              */
/*******************************************/
Table* ListTable::isNewTable(QString grpMgr)
{
	Table *current = 0;

	/* Parcourir les tables et tester chaque nom de table */
	for(current = first(); current != 0; current = next())
	{
		if(current->getGrpMgr() == grpMgr)
			return current;
	}
	return 0;
}

/*******************************************/
/* add                                     */
/*******************************************/
void ListTable::add(Element* elt)
{
	Table* newTable = 0;
	/* If the GrpMng exist in one element 
	 * update it
	 * else
	 * add
	 */
	if((newTable = isNewTable(elt->getGrpMgr())) == 0)
	{
		kdDebug() << "NEW TABLE !!" << endl;
		newTable = new Table(elt->getGrpMgr());
		newTable->append(elt);
		append(newTable);
	}
	else
	{
		kdDebug() << "UPDATE TABLE : " << elt->getGrpMgr() << endl;
		newTable->append(elt);
	}
}

