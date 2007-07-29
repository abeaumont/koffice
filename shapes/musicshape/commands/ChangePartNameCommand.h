/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef CHANGEPARTNAMECOMMAND_H
#define CHANGEPARTNAMECOMMAND_H

#include <QUndoCommand>

namespace MusicCore {
    class Sheet;
    class Part;
}
class MusicShape;

class ChangePartNameCommand : public QUndoCommand {
public:
    ChangePartNameCommand(MusicShape* shape, MusicCore::Part* part, const QString& name);
    virtual void redo();
    virtual void undo();
private:
    MusicCore::Sheet* m_sheet;
    MusicCore::Part* m_part;
    MusicShape* m_shape;
    QString m_name, m_oldName;
};

#endif // ADDPARTCOMMAND_H
