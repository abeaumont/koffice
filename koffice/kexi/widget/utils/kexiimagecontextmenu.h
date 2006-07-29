/* This file is part of the KDE project
   Copyright (C) 2006 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KexiImageContextMenu_H
#define KexiImageContextMenu_H

#include <kpopupmenu.h>
#include <kurl.h>

class KActionCollection;
class KexiDataItemInterface;

//! @short A context menu used for images within form and table views
/*! Used in KexiDBImageBox and KexiBlobTableEdit.
 Contains actions like insert, save, copy, paste, clear.

 Signals like insertFromFileRequested() are all connected to
 handlers in KexiDBImageBox and KexiBlobTableEdit so these objects can 
 respond on requests for data handling.
*/
class KEXIGUIUTILS_EXPORT KexiImageContextMenu : public KPopupMenu
{
	Q_OBJECT

	public:
		KexiImageContextMenu(QWidget *parent);
		virtual ~KexiImageContextMenu();

		KActionCollection* actionCollection() const;

		/*! Updates title for context menu.
		 Used in KexiDBWidgetContextMenuExtender::createTitle(QPopupMenu *menu) and KexiDBImageBox.
		 \return true if the title has been added. */
		static bool updateTitle(QPopupMenu *menu, const QString& title, const QString& icon = QString::null);

	public slots:
		void updateActionsAvailability();

		virtual void insertFromFile();
		virtual void saveAs();
		virtual void cut();
		virtual void copy();
		virtual void paste();
		virtual void clear();
		virtual void showProperties();

	signals:
		//! Emitted when actions availability should be performed. Just connect this signal
		//! to a slot and set \a valueIsNull and \a valueIsReadOnly.
		void updateActionsAvailabilityRequested(bool& valueIsNull, bool& valueIsReadOnly);

		/*! Emitted before "insertFromFile" action was requested. */
		void insertFromFileRequested(const KURL &url);

		/*! Emitted before "saveAs" action was requested. 
		 You should fill \a origFilename, \a fileExtension and \a dataIsEmpty values.
		 If \a dataIsEmpty is false, saving will be cancelled. */
		void aboutToSaveAsRequested(QString& origFilename, QString& fileExtension, bool& dataIsEmpty);

		//! Emitted when "saveAs" action was requested
		void saveAsRequested(const QString& fileName);

		//! Emitted when "cut" action was requested
		void cutRequested();

		//! Emitted when "copy" action was requested
		void copyRequested();

		//! Emitted when "paste" action was requested
		void pasteRequested();

		//! Emitted when "clear" action was requested
		void clearRequested();

		//! Emitted when "showProperties" action was requested
		void showPropertiesRequested();

	protected:
		class Private;
		Private *d;
};

#endif
