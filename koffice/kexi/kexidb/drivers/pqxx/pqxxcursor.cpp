/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>

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

#include "pqxxcursor.h"
#include "pqxxconnection.h"
#include "pqxxconnection_p.h"

#include <kexidb/error.h>
#include <kexidb/global.h>

#include <klocale.h>
#include <kdebug.h>

#include <migration/pqxx/pg_type.h>
using namespace KexiDB;


unsigned int pqxxSqlCursor_trans_num=0; //!< debug helper

//! @todo move to KexiDB utils
QByteArray processBinaryData(const pqxx::result::field r)
{
	const int size = r.size();
	QByteArray array;
	int output=0;
	for (int pass=0; pass<2; pass++) {//2 passes to avoid allocating buffer twice:
		                                //  0: count #of chars; 1: copy data
		const char* s = r.c_str();
		const char* end = s + size;
		if (pass==1) {
			KexiDBDrvDbg << "processBinaryData(): real size == " << output << endl;
			array.resize(output);
			output=0;
		}
		for (int input=0; s < end; output++) {
	//		KexiDBDrvDbg<<(int)s[0]<<" "<<(int)s[1]<<" "<<(int)s[2]<<" "<<(int)s[3]<<" "<<(int)s[4]<<endl;
			if (s[0]=='\\' && (s+1)<end) {
				//special cases as in http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
				if (s[1]=='\'') {// \'
					if (pass==1)
						array[output] = '\'';
					s+=2;
				}
				else if (s[1]=='\\') { // 2 backslashes 
					if (pass==1)
						array[output] = '\\';
					s+=2;
				}
				else if ((input+3)<size) {// \\xyz where xyz are 3 octal digits
					if (pass==1)
						array[output] = char( (int(s[1]-'0')*8+int(s[2]-'0'))*8+int(s[3]-'0') );
					s+=4;
				}
				else {
					KexiDBDrvWarn << "processBinaryData(): no octal value after backslash" << endl;
					s++;
				}
			}
			else {
				if (pass==1)
					array[output] = s[0];
				s++;
			}
	//		KexiDBDrvDbg<<output<<": "<<(int)array[output]<<endl;
		}
	}
	return array;
}
/*old
QByteArray pqxxSqlCursor::processBinaryData(pqxx::binarystring *bs) const
{
	QByteArray ba;
	kdDebug() << "pqxxSqlCursor::processBinaryData(): bs->data()=="<<(int)bs->data()[0]
	 <<" "<<(int)bs->data()[1]<<" bs->size()=="<<bs->size()<<endl;
	ba.setRawData((const char *)(bs->data()), bs->size());
	ba.detach();
	return ba;
}*/

//==================================================================================
//Constructor based on query statement
pqxxSqlCursor::pqxxSqlCursor(KexiDB::Connection* conn, const QString& statement, uint options):
	Cursor(conn,statement, options)
{
//	KexiDBDrvDbg << "PQXXSQLCURSOR: constructor for query statement" << endl;
	my_conn = static_cast<pqxxSqlConnection*>(conn)->d->m_pqxxsql;
	m_options = Buffered;
	m_res = 0;
//	m_tran = 0;
	m_implicityStarted = false;
}

//==================================================================================
//Constructor base on query object
pqxxSqlCursor::pqxxSqlCursor(Connection* conn, QuerySchema& query, uint options )
	: Cursor( conn, query, options )
{
//	KexiDBDrvDbg << "PQXXSQLCURSOR: constructor for query schema" << endl;
	my_conn = static_cast<pqxxSqlConnection*>(conn)->d->m_pqxxsql;
	m_options = Buffered;
	m_res = 0;
//	m_tran = 0;
	m_implicityStarted = false;
}

//==================================================================================
//Destructor
pqxxSqlCursor::~pqxxSqlCursor()
{
	close();
}

//==================================================================================
//Create a cursor result set 
bool pqxxSqlCursor::drv_open()
{
//	KexiDBDrvDbg << "pqxxSqlCursor::drv_open:" << m_sql << endl;

	if (!my_conn->is_open())
	{
//! @todo this check should be moved to Connection! when drv_prepareQuery() arrive
		//should never happen, but who knows
		setError(ERR_NO_CONNECTION,i18n("No connection for cursor open operation specified"));
		return false;
	}
		
	QCString cur_name;
	//Set up a transaction
	try
	{
		//m_tran = new pqxx::work(*my_conn, "cursor_open");
		cur_name.sprintf("cursor_transaction%d", pqxxSqlCursor_trans_num++);
		
//		m_tran = new pqxx::nontransaction(*my_conn, (const char*)cur_name);
		if (!((pqxxSqlConnection*)connection())->m_trans) {
//			my_conn->drv_beginTransaction();
//		if (implicityStarted)
			(void)new pqxxTransactionData((pqxxSqlConnection*)connection(), true);
			m_implicityStarted = true;
		}

		m_res = new pqxx::result(((pqxxSqlConnection*)connection())->m_trans->data->exec(m_sql.utf8()));
		((pqxxSqlConnection*)connection())
			->drv_commitTransaction(((pqxxSqlConnection*)connection())->m_trans);
//		my_conn->m_trans->commit();
//		KexiDBDrvDbg << "pqxxSqlCursor::drv_open: trans. commited: " << cur_name <<endl;

		//We should now be placed before the first row, if any
		m_fieldCount = m_res->columns() - (m_containsROWIDInfo ? 1 : 0);
//js		m_opened=true;
		m_afterLast=false;
		m_records_in_buf = m_res->size();
		m_buffering_completed = true;
		return true;
	}
	catch (const std::exception &e)
	{
		setError(ERR_DB_SPECIFIC, QString::fromUtf8( e.what()) );
		KexiDBDrvWarn << "pqxxSqlCursor::drv_open:exception - " << QString::fromUtf8( e.what() ) << endl;
	}
	catch(...)
	{
		setError();
	}
//	delete m_tran;
//	m_tran = 0;
	if (m_implicityStarted) {
		delete ((pqxxSqlConnection*)connection())->m_trans;
		m_implicityStarted = false;
	}
//	KexiDBDrvDbg << "pqxxSqlCursor::drv_open: trans. rolled back! - " << cur_name <<endl;
	return false;
}

//==================================================================================
//Delete objects
bool pqxxSqlCursor::drv_close()
{
//js	m_opened=false;

	delete m_res;
	m_res = 0;

//	if (m_implicityStarted) {
//		delete m_tran;
//		m_tran = 0;
//		m_implicityStarted = false;
//	}

	return true;
}

//==================================================================================
//Gets the next record...doesnt need to do much, just return fetchend if at end of result set
void pqxxSqlCursor::drv_getNextRecord()
{
//	KexiDBDrvDbg << "pqxxSqlCursor::drv_getNextRecord, size is " <<m_res->size() << " Current Position is " << (long)at() << endl;
	if(at() < m_res->size() && at() >=0)
	{	
		m_result = FetchOK;
	}
	else if (at() >= m_res->size())
	{
		m_result = FetchEnd;
	}
	else
	{
		m_result = FetchError;
	}
}

//==================================================================================
//Check the current position is within boundaries
void pqxxSqlCursor::drv_getPrevRecord()
{
//	KexiDBDrvDbg << "pqxxSqlCursor::drv_getPrevRecord" << endl;

	if(at() < m_res->size() && at() >=0)
	{	
		m_result = FetchOK;
	}
	else if (at() >= m_res->size())
	{
		m_result = FetchEnd;
	}
	else
	{
		m_result = FetchError;
	}
}

//==================================================================================
//Return the value for a given column for the current record
QVariant pqxxSqlCursor::value(uint pos)
{
	if (pos < m_fieldCount)
		return pValue(pos);
	else
		return QVariant();
}

//==================================================================================
//Return the value for a given column for the current record - Private const version
QVariant pqxxSqlCursor::pValue(uint pos)const
{	
	if (m_res->size() <= 0)
	{
		KexiDBDrvWarn << "pqxxSqlCursor::value - ERROR: result size not greater than 0" << endl;
		return QVariant();
	}

	if (pos>=(m_fieldCount+(m_containsROWIDInfo ? 1 : 0)))
	{
//		KexiDBDrvWarn << "pqxxSqlCursor::value - ERROR: requested position is greater than the number of fields" << endl;
		return QVariant();
	}

	KexiDB::Field *f = (m_fieldsExpanded && pos<QMIN(m_fieldsExpanded->count(), m_fieldCount)) 
		? m_fieldsExpanded->at(pos)->field : 0;

//	KexiDBDrvDbg << "pqxxSqlCursor::value(" << pos << ")" << endl;

	//from most to least frequently used types:
	if (f) //We probably have a schema type query so can use kexi to determin the row type
	{
		if ((f->isIntegerType()) || (/*ROWID*/!f && m_containsROWIDInfo && pos==m_fieldCount))
		{
			return QVariant((*m_res)[at()][pos].as(int()));
		}
		else if (f->isTextType())
		{
			return QVariant((*m_res)[at()][pos].c_str());
		}
		else if (f->isFPNumericType())
		{
			return QVariant((*m_res)[at()][pos].as(double()));
		}
		else if (f->typeGroup() == Field::BLOBGroup)
		{
//			pqxx::result::field r = (*m_res)[at()][pos];
//			kdDebug() << r.name() << ", " << r.c_str() << ", " << r.type() << ", " << r.size() << endl;
			return QVariant(processBinaryData( (*m_res)[at()][pos] ));
		}
	}
	else // We probably have a raw type query so use pqxx to determin the column type
	{
		
		switch((*m_res)[at()][pos].type())
		{
			case BOOLOID:
				return QVariant((*m_res)[at()][pos].c_str()); //TODO check formatting
			case INT2OID:
			case INT4OID:
			case INT8OID:
				return QVariant((*m_res)[at()][pos].as(int()));
			case FLOAT4OID:
			case FLOAT8OID:
			case NUMERICOID:
				return QVariant((*m_res)[at()][pos].as(double()));
			case DATEOID:
				return QVariant((*m_res)[at()][pos].c_str()); //TODO check formatting
			case TIMEOID:
				return QVariant((*m_res)[at()][pos].c_str()); //TODO check formatting
			case TIMESTAMPOID:
				return QVariant((*m_res)[at()][pos].c_str()); //TODO check formatting
			case BYTEAOID:
				return QVariant(processBinaryData( (*m_res)[at()][pos] ));
			case BPCHAROID:
			case VARCHAROID:
			case TEXTOID:
				return QVariant((*m_res)[at()][pos].c_str());
			default:
				return QVariant((*m_res)[at()][pos].c_str());
		}
		
	}
	
	//If all else fails just return a string
	return QVariant((*m_res)[at()][pos].c_str());
	
}

//==================================================================================
//Return the current record as a char**
//who'd have thought we'd be using char** in this day and age :o)
const char** pqxxSqlCursor::rowData() const
{
//	KexiDBDrvDbg << "pqxxSqlCursor::recordData" << endl;
	
	const char** row;
	
	row = (const char**)malloc(m_res->columns()+1);
	row[m_res->columns()] = NULL;
	if (at() >= 0 && at() < m_res->size())
	{
		for(int i = 0; i < (int)m_res->columns(); i++)
		{
			row[i] = (char*)malloc(strlen((*m_res)[at()][i].c_str())+1);
			strcpy((char*)(*m_res)[at()][i].c_str(), row[i]);
//			KexiDBDrvDbg << row[i] << endl;
		}
	}
	else
	{
		KexiDBDrvWarn << "pqxxSqlCursor::recordData: m_at is invalid" << endl;
	}
	return row;
}

//==================================================================================
//Store the current record in [data]
void pqxxSqlCursor::storeCurrentRow(RowData &data) const
{
//	KexiDBDrvDbg << "pqxxSqlCursor::storeCurrentRow: POSITION IS " << (long)m_at<< endl;

	if (m_res->size()<=0)
		return;

	const uint realCount = m_fieldCount + (m_containsROWIDInfo ? 1 : 0);
	data.resize(realCount);

	for( uint i=0; i<realCount; i++)
	{
		data[i] = pValue(i);
	}
}

//==================================================================================
//
void pqxxSqlCursor::drv_clearServerResult()
{
//! @todo pqxxSqlCursor: stuff with server results
}

//==================================================================================
//Add the current record to the internal buffer
//Implementation required but no need in this driver
//Result set is a buffer so dont need another
void pqxxSqlCursor::drv_appendCurrentRecordToBuffer()
{

}

//==================================================================================
//Move internal pointer to internal buffer +1
//Implementation required but no need in this driver
void pqxxSqlCursor::drv_bufferMovePointerNext()
{

}

//==================================================================================
//Move internal pointer to internal buffer -1
//Implementation required but no need in this driver
void pqxxSqlCursor::drv_bufferMovePointerPrev()
{

}

//==================================================================================
//Move internal pointer to internal buffer to N
//Implementation required but no need in this driver
void pqxxSqlCursor::drv_bufferMovePointerTo(Q_LLONG to)
{
	Q_UNUSED(to);
}

