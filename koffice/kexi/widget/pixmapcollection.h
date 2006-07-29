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

#ifndef KFORMDESIGNERPIXMAPCOLLECTION_H
#define KFORMDESIGNERPIXMAPCOLLECTION_H

#include <qobject.h>
#include <qmap.h>
#include <qpair.h>
#include <qintdict.h>
#include <qtoolbutton.h>

#include <kicontheme.h>
#include <kdialogbase.h>
#include <kiconview.h>
#include <kurl.h>

class QPixmap;
class KIconView;
class KIconButton;
class KLineEdit;
class QDomNode;

typedef QMap<QString, QPair<QString, int> > PixmapMap;

//! A class that store pixmaps (by path or by name for KDE icons)
class KEXIEXTWIDGETS_EXPORT PixmapCollection : public QObject
{
	Q_OBJECT

	public:
		PixmapCollection(const QString &collectionName, QObject *parent = 0, const char *name = 0);
		~PixmapCollection() {;}

		QString addPixmapPath(const KURL &url);
		QString addPixmapName(const QString &name, int size = KIcon::SizeMedium);
		void removePixmap(const QString &name);

		bool   contains(const QString &name);
		QPixmap  getPixmap(const QString &name);

		void save(QDomNode parentNode);
		void load(QDomNode node);

		QString collectionName() {return m_name; }

	signals:
		void itemRenamed(const QString &oldName, const QString &newName);
		void itemRemoved(const QString &name);

	protected:
		QString  m_name;
		PixmapMap  m_pixmaps;

	friend class PixmapCollectionEditor;
	friend class PixmapCollectionChooser;
};

//! A dialog to edit the contents of a PixmapCollection
class KEXIEXTWIDGETS_EXPORT PixmapCollectionEditor : public KDialogBase
{
	Q_OBJECT

	public:
		PixmapCollectionEditor(PixmapCollection *collection, QWidget *parent = 0);
		~PixmapCollectionEditor() {;}

	protected:
		QPixmap getPixmap(const QString &name);
		void createIconViewItem(const QString &name);

	protected slots:
		void newItemByPath();
		void newItemByName();
		void removeItem();
		void renameItem();
		void renameCollectionItem(QIconViewItem *item, const QString &name);
		void displayMenu(QIconViewItem *item, const QPoint &p);

	private:
		enum { BNewItemPath = 101, BNewItemName, BDelItem};
		KIconView  *m_iconView;
		QIntDict<QToolButton>  m_buttons;
		PixmapCollection  *m_collection;
};

//! A dialog to choose an icon in a PixmapCollection
class KEXIEXTWIDGETS_EXPORT PixmapCollectionChooser : public KDialogBase
{
	Q_OBJECT

	public:
		PixmapCollectionChooser(PixmapCollection *collection, const QString &selectedItem, QWidget *parent = 0);
		~PixmapCollectionChooser() {;}

		QPixmap  pixmap();
		QString  pixmapName();

	protected:
		QPixmap getPixmap(const QString &name);
	protected slots:
		virtual void slotUser1();

	private:
		PixmapCollection  *m_collection;
		KIconView  *m_iconView;
};

//! A simple dialog to choose a KDE icon
class KEXIEXTWIDGETS_EXPORT LoadIconDialog : public KDialogBase
{
	Q_OBJECT

	public:
		LoadIconDialog(QWidget *parent = 0);
		~LoadIconDialog() {;}

		int iconSize();
		QString iconName();

	protected slots:
		void changeIconSize(int);
		void updateIconName(QString);
		void setIcon(const QString &);

	private:
		KLineEdit *m_nameInput;
		KIconButton *m_button;
};

//! A Special KIconViewItem that holds the name of its associated pixmap (to allow renaming)
class KEXIEXTWIDGETS_EXPORT PixmapIconViewItem : public KIconViewItem
{
	public:
		PixmapIconViewItem(KIconView *parent, const QString &text, const QPixmap &icon)
		: KIconViewItem(parent, text, icon)  { m_name = text; }
		~PixmapIconViewItem() {;}

		void setName(const QString &name) { m_name = name; }
		QString name() { return m_name;}

	private:
		QString  m_name;
};

#endif
