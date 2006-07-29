/* This file is part of the KDE project
   Copyright (C) 2005-2006 Jaroslaw Staniek <js@iidea.pl>

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

#include "kexifieldcombobox.h"

#include <qheader.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcursor.h>
#include <qpoint.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qstyle.h>
#include <qlistbox.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include <kexidb/tableschema.h>
#include <kexidb/queryschema.h>
#include <kexidb/utils.h>
#include <kexidragobjects.h>
#include <kexiproject.h>

//! @internal
class KexiFieldComboBox::Private
{
	public:
		Private()
//		 : schema(0)
		 : table(true)
		{
		}
		~Private()
		{
//			delete schema;
		}
		QGuardedPtr<KexiProject> prj;
//		KexiDB::TableOrQuerySchema* schema;
		QPixmap keyIcon, noIcon;
		QCString tableOrQueryName;
		QString fieldOrExpression;
		QMap<QCString, QString> captions;
		bool table;
};

//------------------------

KexiFieldComboBox::KexiFieldComboBox(QWidget *parent, const char *name)
 : KComboBox(true/*rw*/, parent, name)
 , d(new Private())
{
	d->keyIcon = SmallIcon("key");
	d->noIcon = QPixmap(d->keyIcon.size());
	QBitmap bmp(d->noIcon.size());
	bmp.fill(Qt::color0);
	d->noIcon.setMask(bmp);

	setInsertionPolicy(NoInsertion);
	setCompletionMode(KGlobalSettings::CompletionPopupAuto);
	setSizeLimit( 16 );
	connect(this, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
	connect(this, SIGNAL(returnPressed(const QString &)), this, SLOT(slotReturnPressed(const QString &)));

//	setAcceptDrops(true);
//	viewport()->setAcceptDrops(true);
}

KexiFieldComboBox::~KexiFieldComboBox()
{
	delete d;
}

void KexiFieldComboBox::setProject(KexiProject *prj)
{
	if ((KexiProject*)d->prj==prj)
		return;
	d->prj = prj;
	setTableOrQuery("", true);
}

void KexiFieldComboBox::setTableOrQuery(const QCString& name, bool table)
{
	d->tableOrQueryName = name;
	d->table = table;
	clear();
	d->captions.clear();
	insertItem("");
//	delete d->schema;
	if (d->tableOrQueryName.isEmpty() || !d->prj)
		return;

	KexiDB::TableOrQuerySchema tableOrQuery(d->prj->dbConnection(), d->tableOrQueryName, table);
	if (!tableOrQuery.table() && !tableOrQuery.query())
		return;

//	bool hasPKeys = true; //t->hasPrimaryKeys();
	KexiDB::QueryColumnInfo::Vector columns = tableOrQuery.columns();
	const int count = columns.count();
	for(int i=0; i < count; i++)
	{
		KexiDB::QueryColumnInfo *colinfo = columns[i];
		insertItem(
			(colinfo && (colinfo->field->isPrimaryKey() || colinfo->field->isUniqueKey()))
			? d->keyIcon
			: d->noIcon
			, colinfo->aliasOrName());
		completionObject()->addItem(colinfo->aliasOrName());
		//store user-friendly caption (used by fieldOrExpressionCaption())
		d->captions.insert( colinfo->aliasOrName(), colinfo->captionOrAliasOrName() );
	}

	//update selection
	setFieldOrExpression(d->fieldOrExpression);
}

QCString KexiFieldComboBox::setTableOrQueryName() const
{
	return d->tableOrQueryName;
}

void KexiFieldComboBox::setFieldOrExpression(const QString& string)
{
	const QString name(string); //string.stripWhiteSpace().lower());
	const int pos = name.find('.');
	if (pos!=-1) {
		QCString objectName = name.left(pos).latin1();
		if (d->tableOrQueryName!=objectName) {
			d->fieldOrExpression = name;
			setCurrentItem(0);
			setCurrentText(name);
//! @todo show error
			kexiwarn << "KexiFieldComboBox::setField(): invalid table/query name in '" << name << "'" << endl;
			return;
		}
		d->fieldOrExpression = name.mid(pos+1);
	}
	else
		d->fieldOrExpression = name;

	QListBoxItem *item = listBox()->findItem(d->fieldOrExpression);
	if (!item) {
		setCurrentItem(0);
		setCurrentText(d->fieldOrExpression);
		//todo: show 'the item doesn't match' info?
		return;
	}
	setCurrentItem( listBox()->index(item) );
}

QString KexiFieldComboBox::fieldOrExpression() const
{
	return d->fieldOrExpression;
}

QString KexiFieldComboBox::fieldOrExpressionCaption() const
{
	return d->captions[ d->fieldOrExpression.latin1() ];
}

void KexiFieldComboBox::slotActivated(int i)
{
	d->fieldOrExpression = text(i);
	emit selected();
}

void KexiFieldComboBox::slotReturnPressed(const QString & text)
{
	//text is available: select item for this text:
	int index;
	if (text.isEmpty()) {
		index = 0;
	}
	else {
		QListBoxItem *item = listBox()->findItem( text, Qt::ExactMatch );
		if (!item)
			return;
		index = listBox()->index( item );
		if (index < 1)
			return;
	}
	setCurrentItem( index );
	slotActivated( index );
}

void KexiFieldComboBox::focusOutEvent( QFocusEvent *e )
{
	KComboBox::focusOutEvent( e );
	slotReturnPressed(currentText());
}

#include "kexifieldcombobox.moc"
