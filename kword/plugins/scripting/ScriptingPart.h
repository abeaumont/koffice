/*
 * This file is part of KWord
 *
 * Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCRIPTINGPART_H
#define SCRIPTINGPART_H

#include <kparts/plugin.h>

/**
* The ScriptingPart class implements a KPart component
* to integrate scripting into KWord.
*/
class ScriptingPart : public KParts::Plugin
{
		Q_OBJECT
	public:
		ScriptingPart(QObject* parent, const QStringList&);
		virtual ~ScriptingPart();
	private:
		/// \internal d-pointer class.
		class Private;
		/// \internal d-pointer instance.
		Private* const d;
};

#endif
