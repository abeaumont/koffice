/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>

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

#ifndef KEXIREPORTFACTORY_H
#define KEXIREPORTFACTORY_H

#include <widgetfactory.h>

//! Kexi Factory (DB widgets + subform)
class KexiReportFactory : public KFormDesigner::WidgetFactory
{
	Q_OBJECT

	public:
		KexiReportFactory(QObject *parent, const char *name, const QStringList &args);
		virtual ~KexiReportFactory();

		virtual QString  name();
		virtual QWidget *createWidget(const QCString &classname, QWidget *parent, const char *name, KFormDesigner::Container *container,
			int options = DefaultOptions);

		virtual bool createMenuActions(const QCString &classname, QWidget *w, QPopupMenu *menu,
		   KFormDesigner::Container *container);
		virtual bool startEditing(const QCString &classname, QWidget *w, KFormDesigner::Container *container);
		virtual bool previewWidget(const QCString &, QWidget *, KFormDesigner::Container *);

		//virtual void		saveSpecialProperty(const QString &classname, const QString &name, const QVariant &value, QWidget *w,
		         //QDomElement &parentNode, QDomDocument &parent) {}
		//virtual void            readSpecialProperty(const QCString &classname, QDomElement &node, QWidget *w, KFormDesigner::ObjectTreeItem *item) {}
		virtual QValueList<QCString> autoSaveProperties(const QCString &classname);

	public slots:
		void editText();

	protected:
		virtual bool isPropertyVisibleInternal(const QCString &, QWidget *, const QCString &, bool isTopLevel);
//		virtual void changeText(const QString &newText);
//		virtual void resizeEditor(QWidget *widget, const QCString &classname);

	private:
		QWidget *m_widget;
		KFormDesigner::Container *m_container;
};

#endif

