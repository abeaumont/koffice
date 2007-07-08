/* This file is part of the KDE project
   Copyright 2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2004 Laurent Montel <montel@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_UNDO_WRAPPER_COMMAND
#define KSPREAD_UNDO_WRAPPER_COMMAND

#include <QString>
#include <QUndoCommand>

/**
 * The KSpread namespace.
 */
namespace KSpread
{
class UndoAction;

/**
 * Class UndoWrapperCommand is used to help migration from custom
 * UndoAction to QUndoCommand-based system.
 * See Doc::addCommand for more information.
 */
class UndoWrapperCommand : public QUndoCommand
{
public:
  explicit UndoWrapperCommand( UndoAction* undoAction );

  virtual void redo();
  virtual void undo();

protected:
  UndoAction* undoAction;
};

} // namespace KSpread

#endif // KSPREAD_UNDO_WRAPPER_COMMAND
