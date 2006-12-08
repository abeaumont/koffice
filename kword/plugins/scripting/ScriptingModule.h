/*
 * This file is part of KWord
 *
 * Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCRIPTINGMODULE_H
#define SCRIPTINGMODULE_H

#include <QString>
#include <QStringList>
#include <QObject>

/**
* The ScriptingModule class enables access to the KWord
* functionality from within the scripting backends.
*/
class ScriptingModule : public QObject
{
		Q_OBJECT
	public:
		ScriptingModule();
		virtual ~ScriptingModule();

		//KSpread::Doc* doc();
		//void setView(KSpread::View* view = 0);

	public slots:

#if 0
		/**
		* Returns the \a KoApplicationAdaptor object.
		*/
		QObject* application();

		/**
		* Returns the \a KoDocumentAdaptor object.
		*/
		QObject* document();
#endif

	private:
		/// \internal d-pointer class.
		class Private;
		/// \internal d-pointer instance.
		Private* const d;
};

#endif
