/* This file is part of the KDE project
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

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

#include <kexidb/roweditbuffer.h>

#include <kdebug.h>

using namespace KexiDB;


RowEditBuffer::RowEditBuffer(bool dbAwareBuffer)
: m_simpleBuffer(dbAwareBuffer ? 0 : new SimpleMap())
, m_simpleBufferIt(dbAwareBuffer ? 0 : new SimpleMap::ConstIterator())
, m_dbBuffer(dbAwareBuffer ? new DBMap() : 0)
, m_dbBufferIt(dbAwareBuffer ? new DBMap::ConstIterator() : 0)
{
}

RowEditBuffer::~RowEditBuffer()
{
	delete m_simpleBuffer;
	delete m_simpleBufferIt;
	delete m_dbBuffer;
	delete m_dbBufferIt;
}

const QVariant* RowEditBuffer::at( QueryColumnInfo& fi ) const
{ 
	if (!m_dbBuffer) {
		KexiDBWarn << "RowEditBuffer::at(QueryColumnInfo&): not db-aware buffer!" << endl;
		return 0;
	}
	*m_dbBufferIt = m_dbBuffer->find( &fi );
	if (*m_dbBufferIt==m_dbBuffer->constEnd())
		return 0;
	return &(*m_dbBufferIt).data();
}

const QVariant* RowEditBuffer::at( Field& f ) const
{
	if (!m_simpleBuffer) {
		KexiDBWarn << "RowEditBuffer::at(Field&): this is db-aware buffer!" << endl;
		return 0;
	}
	*m_simpleBufferIt = m_simpleBuffer->find( f.name() );
	if (*m_simpleBufferIt==m_simpleBuffer->constEnd())
		return 0;
	return &(*m_simpleBufferIt).data();
}

const QVariant* RowEditBuffer::at( const QString& fname ) const
{
	if (!m_simpleBuffer) {
		KexiDBWarn << "RowEditBuffer::at(Field&): this is db-aware buffer!" << endl;
		return 0;
	}
	*m_simpleBufferIt = m_simpleBuffer->find( fname );
	if (*m_simpleBufferIt==m_simpleBuffer->constEnd())
		return 0;
	return &(*m_simpleBufferIt).data();
}

void RowEditBuffer::clear() {
	if (m_dbBuffer)
		m_dbBuffer->clear(); 
	if (m_simpleBuffer)
		m_simpleBuffer->clear();
}

bool RowEditBuffer::isEmpty() const
{
	if (m_dbBuffer)
		return m_dbBuffer->isEmpty(); 
	if (m_simpleBuffer)
		return m_simpleBuffer->isEmpty();
	return true;
}

void RowEditBuffer::debug()
{
	if (isDBAware()) {
		kdDebug() << "RowEditBuffer type=DB-AWARE, " << m_dbBuffer->count() <<" items"<< endl;
		for (DBMap::ConstIterator it = m_dbBuffer->constBegin(); it!=m_dbBuffer->constEnd(); ++it) {
			kdDebug() << "* field name=" <<it.key()->field->name()<<" val="
				<< (it.data().isNull() ? QString("<NULL>") : it.data().toString()) <<endl;
		}
		return;
	}
	kdDebug() << "RowEditBuffer type=SIMPLE, " << m_simpleBuffer->count() <<" items"<< endl;
	for (SimpleMap::ConstIterator it = m_simpleBuffer->constBegin(); it!=m_simpleBuffer->constEnd(); ++it) {
		kdDebug() << "* field name=" <<it.key()<<" val="
			<< (it.data().isNull() ? QString("<NULL>") : it.data().toString()) <<endl;
	}
}
