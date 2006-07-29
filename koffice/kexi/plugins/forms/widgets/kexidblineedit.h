/* This file is part of the KDE project
   Copyright (C) 2005 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004-2006 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KexiDBLineEdit_H
#define KexiDBLineEdit_H

#include <klineedit.h>
#include <qvalidator.h>

#include "kexiformdataiteminterface.h"
#include "kexidbtextwidgetinterface.h"
#include "kexidbutils.h"
#include <widget/utils/kexidatetimeformatter.h>

class KexiDateFormatter;
class KexiTimeFormatter;
class KexiDBWidgetContextMenuExtender;

/*! @internal Utility: alter background color to be a blended color 
 of the background and base (usually lighter gray). Used for read-only mode. */
void setLighterGrayBackgroundColor(QWidget* widget);

//! @short Line edit widget for Kexi forms
/*! Handles many data types. User input is validated by using validators 
 and/or input masks.
*/
class KEXIFORMUTILS_EXPORT KexiDBLineEdit :
	public KLineEdit,
	protected KexiDBTextWidgetInterface,
	public KexiFormDataItemInterface,
	public KexiSubwidgetInterface
{
	Q_OBJECT
	Q_PROPERTY(QString dataSource READ dataSource WRITE setDataSource DESIGNABLE true)
	Q_PROPERTY(QCString dataSourceMimeType READ dataSourceMimeType WRITE setDataSourceMimeType DESIGNABLE true)
	Q_OVERRIDE(bool readOnly READ isReadOnly WRITE setReadOnly DESIGNABLE true)

	public:
		KexiDBLineEdit(QWidget *parent, const char *name=0);
		virtual ~KexiDBLineEdit();

		inline QString dataSource() const { return KexiFormDataItemInterface::dataSource(); }
		inline QCString dataSourceMimeType() const { return KexiFormDataItemInterface::dataSourceMimeType(); }
		virtual QVariant value();
		virtual void setInvalidState( const QString& displayText );

		//! \return true if editor's value is null (not empty)
		//! Used for checking if a given constraint within table of form is met.
		virtual bool valueIsNull();

		//! \return true if editor's value is empty (not necessary null).
		//! Only few data types can accept "EMPTY" property
		//! (use KexiDB::Field::hasEmptyProperty() to check this).
		//! Used for checking if a given constraint within table or form is met.
		virtual bool valueIsEmpty();

		/*! \return true if the value is valid */
		virtual bool valueIsValid();

		/*! \return 'readOnly' flag for this widget. */
		virtual bool isReadOnly() const;

		/*! \return the view widget of this item, e.g. line edit widget. */
		virtual QWidget* widget();

		virtual bool cursorAtStart();
		virtual bool cursorAtEnd();
		virtual void clear();

		virtual void setColumnInfo(KexiDB::QueryColumnInfo* cinfo);

		/*! Handles action having standard name \a actionName. 
		 Action could be: "edit_copy", "edit_paste", etc.
		 Reimplemented after KexiDataItemChangesListener. */
		virtual void handleAction(const QString& actionName);

	public slots:
		inline void setDataSource(const QString &ds) { KexiFormDataItemInterface::setDataSource(ds); }
		inline void setDataSourceMimeType(const QCString &ds) { KexiFormDataItemInterface::setDataSourceMimeType(ds); }
		virtual void setReadOnly( bool readOnly );

	protected slots:
		void slotTextChanged(const QString&);

	protected:
		virtual void paintEvent ( QPaintEvent * );
		virtual void setValueInternal(const QVariant& add, bool removeOld);
		virtual bool event ( QEvent * );

		inline KexiDateFormatter* dateFormatter() {
			return m_dateFormatter ? m_dateFormatter : m_dateFormatter = new KexiDateFormatter();
		}

		inline KexiTimeFormatter* timeFormatter() {
			return m_timeFormatter ? m_timeFormatter : m_timeFormatter = new KexiTimeFormatter();
		}

		virtual QPopupMenu * createPopupMenu();

		//! Implemented for KexiSubwidgetInterface
		virtual bool appendStretchRequired(KexiDBAutoField* autoField) const;

		//! Used for date and date/time types
		KexiDateFormatter* m_dateFormatter;

		//! Used for time and date/time types
		KexiTimeFormatter* m_timeFormatter;

		//! Used for read only flag to disable editing
		QGuardedPtr<const QValidator> m_readOnlyValidator;

		//! Used to remember the previous validator used forf r/w mode, after setting the read only flag
		QGuardedPtr<const QValidator> m_readWriteValidator;

		//! Used for extending context menu
		KexiDBWidgetContextMenuExtender m_menuExtender;

		//! Used in isReadOnly, as sometimes we want to have the flag set tot true when KLineEdit::isReadOnly 
		//! is still false.
		bool m_internalReadOnly : 1;

		//! Used in slotTextChanged()
		bool m_slotTextChanged_enabled : 1;
};

#endif
