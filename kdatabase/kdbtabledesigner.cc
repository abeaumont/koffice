/***************************************************************************
                          clstbldesigner2.cpp  -  description
                             -------------------
    begin                : Mon Apr 29 2002
    copyright            : (C) 2002 by Chris Machemer
    email                : machey@ceinetworks.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License.         *
 *                                                                         *
 ***************************************************************************/

#include "kdbtabledesigner.h"
#include <qheader.h>
#include <qtable.h>
#include <qlayout.h>
#include <kdebug.h>
#include <kdatabase_struct.h>

KDBTableDesigner::KDBTableDesigner(QWidget *parent, KDBStruct *KDBStruct) : QWidget(parent, 0) {
    myKDBStruct = KDBStruct;
	QGridLayout *g = new QGridLayout(this);

	this->resize(647,430);
	
	m_table = new QTable(1,6, this, "myQTable");
	m_table->resize(575,420);

	QHeader *myHeader = m_table->horizontalHeader();

	myHeader->setLabel(0,"Primary Key");
	myHeader->setLabel(1,"Name");
	myHeader->setLabel(2,"Data Type");
	myHeader->setLabel(3,"Size");
	myHeader->setLabel(4,"Default");
	myHeader->setLabel(5,"Allow Null");
  
//	m_table->setItem( 0, 0, new QCheckTableItem( m_table, "" ) );
//	m_table->setItem( 0, 5, new QCheckTableItem( m_table, "" ) );

//	QStringList m_comboEntries;
	m_comboEntries << "int" << "char" << "varchar" << "float" << "boolean";

//	m_table->setItem( 0, 2, new QComboTableItem( m_table, comboEntries, FALSE ));

	g->addWidget(m_table,	0,	0);

	m_rows = 0;

	
//	this->addRow(true, "id", t_int, 12, "", true);
//	this->addRow(false, "name", t_char, 10, "nobody");
	kdDebug() << "kdbTblDesigner::constructed tblDesigner" << endl;
}

KDBTableDesigner::~KDBTableDesigner(){
}


bool KDBTableDesigner::populateTblDesigner(QString tblName){

    KDBTable *myTableInfo = myKDBStruct->getTable(tblName);
    QString *msg=NULL;
	kdDebug() << "kdbTblDesigner::populateTblDesigner: at top, looking for " << tblName << endl;

    QPtrList<TableStructureRow> *columnList = myTableInfo->getColumns(&tblName, msg);
	kdDebug() << "kdbTblDesigner::populateTblDesigner: after getColumns" << endl;
    unsigned int rowCounter=1;

    tblName.append(" - Table Designer");
    setCaption(tblName);

   TableStructureRow *aColumn = columnList->first();
	kdDebug() << "kdbTblDesigner::populateTblDesigner: after columnList.first" << endl;

	this->addRow(aColumn->primary_key, aColumn->name, aColumn->type, aColumn->size, aColumn->Default,aColumn->allow_null);
   aColumn = columnList->next();
    while(rowCounter < columnList->count()) {
  	    this->addRow(aColumn->primary_key, aColumn->name, aColumn->type, aColumn->size, aColumn->Default,aColumn->allow_null);
       aColumn = columnList->next();
       rowCounter++;
       }
    return(true);
}

void KDBTableDesigner::addRow(bool primary_key, QString name, DataType type, int size, QString default_v, bool allow_null)
{
	QComboTableItem *dataTypeView = new QComboTableItem(m_table, m_comboEntries, false);
	QCheckTableItem *primary_keyView = new QCheckTableItem(m_table, "");
	QCheckTableItem *allow_nullView = new QCheckTableItem(m_table, "");
	QString sizestr;
	sizestr = sizestr.setNum(size);
	
	m_table->setNumRows(m_rows + 1);
	
	m_table->setItem(m_rows, 0, primary_keyView);
	primary_keyView->setChecked(primary_key);
	m_table->setText(m_rows, 1, name);
	m_table->setText(m_rows, 3, sizestr.latin1());
	m_table->setText(m_rows, 4, default_v.latin1());
	m_table->setItem(m_rows, 2, dataTypeView);
	m_table->setItem(m_rows, 5, allow_nullView);
	allow_nullView->setChecked(allow_null);
	
	switch(type)
	{
	
		case t_int:
			dataTypeView->setCurrentItem(0);
			break;
	
		case t_char:
			dataTypeView->setCurrentItem(1);
			break;
			
		case t_vchar:
			dataTypeView->setCurrentItem(2);
			break;
		
		case t_float:
			dataTypeView->setCurrentItem(3);
			break;
			
		case t_boolen:
			dataTypeView->setCurrentItem(4);
			break;
		
	}
	m_rows++;
}

#include "kdbtabledesigner.moc"
