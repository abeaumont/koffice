/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOPAPAGEINSERTCOMMAND_H
#define KOPAPAGEINSERTCOMMAND_H

#include <QUndoCommand>

#include "kopageapp_export.h"

class KoPADocument;
class KoPAPage;

/**
 * Command for inserting a page into a Document
 */
class KOPAGEAPP_TEST_EXPORT KoPAPageInsertCommand : public QUndoCommand
{
public:
    KoPAPageInsertCommand(KoPADocument *document, KoPAPage *page, KoPAPage *after, QUndoCommand *parent = 0);
    virtual ~KoPAPageInsertCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KoPADocument * m_document;
    KoPAPage * m_page;
    KoPAPage * m_after;
    bool m_deletePage;
};

#endif // KOPAPAGEINSERTCOMMAND_H
