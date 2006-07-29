/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2003,2005 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KEXISTATICPART_H
#define KEXISTATICPART_H

#include "kexipart.h"
#include "kexipartinfo.h"

namespace KexiPart
{

/**
 * @short Information about a static Kexi Part (plugin).
 */
class KEXICORE_EXPORT StaticInfo : public Info
{
	public:
		StaticInfo(const QCString& mimeType, const QString& itemIcon, const QString& objectName);
		~StaticInfo();

	protected:
};

/**
 * @short Static Kexi Part (plugin).
 */
class KEXICORE_EXPORT StaticPart : public Part
{
	public:
		StaticPart(const QCString& mimeType, const QString& itemIcon, const QString& objectName);
		virtual ~StaticPart();

		/*! Creates a new view for mode \a viewMode, \a item and \a parent. The view will be 
		 used inside \a dialog. \a args arguments can be passed. */
		virtual KexiViewBase* createView(QWidget *parent, KexiDialogBase* dialog, 
			KexiPart::Item &item, int viewMode, QMap<QString,QString>* args) = 0;

	protected:
		//! unused by static parts
		KexiViewBase* createView(QWidget *parent, KexiDialogBase* dialog, 
			KexiPart::Item &item, int viewMode = Kexi::DataViewMode);
};

}

#endif
