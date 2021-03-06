/* This file is part of the KDE project
* Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
* Copyright (C) 2009 Thomas Zander <zander@kde.org>
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
#ifndef KODFTEXTEDITOR_P_H
#define KODFTEXTEDITOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KOdfText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "KoTextEditor.h"

#include "KTextDocument.h"
#include "styles/KParagraphStyle.h"
#include "styles/KStyleManager.h"
#include "changetracker/KChangeTracker.h"

#include <KLocale>
#include <kdebug.h>

#include <QTextBlock>
#include <QTextDocument>
#include <QTimer>

class KoTextEditorPrivate
{
public:
    enum State {
        NoOp,
        KeyPress,
        Delete,
        Format,
        Custom
    };

    explicit KoTextEditorPrivate(KoTextEditor *qq, QTextDocument *document);

    ~KoTextEditorPrivate() {}

    void documentCommandAdded();
    void updateState(State newState, QString title = QString());

    bool deleteInlineObjects(bool backwards = false);
    void deleteSelection();
    void runDirectionUpdater();
    void clearCharFormatProperty(int propertyId);

    KoTextEditor *q;
    QTextCursor caret;
    QTextDocument *document;
    QUndoCommand *headCommand;
    QString commandTitle;
    KOdfText::Direction direction;
    bool isBidiDocument;

    State editorState;

    QTimer updateRtlTimer;
    QList<int> dirtyBlocks;
};

class BlockFormatVisitor
{
public:
    BlockFormatVisitor() {}
    virtual ~BlockFormatVisitor() {}

    virtual void visit(QTextBlockFormat &format) const = 0;

    static void visitSelection(KoTextEditor *editor, const BlockFormatVisitor &visitor, QString title = i18n("Format"), bool resetProperties = false, bool registerChange = true) {
        int start = qMin(editor->position(), editor->anchor());
        int end = qMax(editor->position(), editor->anchor());

        QTextBlock block = editor->block();
        if (block.position() > start)
            block = block.document()->findBlock(start);

        // now loop over all blocks that the selection contains and alter the text fragments where applicable.
        while (block.isValid() && block.position() <= end) {
            QTextBlockFormat format = block.blockFormat();
            if (resetProperties) {
                if (KTextDocument(editor->document()).styleManager()) {
                    KParagraphStyle *old = KTextDocument(editor->document()).styleManager()->paragraphStyle(block.blockFormat().intProperty(KParagraphStyle::StyleId));
                    if (old)
                        old->unapplyStyle(block);
                }
            }
            visitor.visit(format);
            QTextCursor cursor(block);
            QTextBlockFormat prevFormat = cursor.blockFormat();
            if (registerChange)
                editor->registerTrackedChange(cursor, KOdfGenericChange::FormatChange, title, format, prevFormat, true);
            cursor.setBlockFormat(format);
            block = block.next();
        }
    }
};

class CharFormatVisitor
{
public:
    CharFormatVisitor() {}
    virtual ~CharFormatVisitor() {}

    virtual void visit(QTextCharFormat &format) const = 0;

    static void visitSelection(KoTextEditor *editor, const CharFormatVisitor &visitor, QString title = i18n("Format"), bool registerChange = true) {
        int start = qMin(editor->position(), editor->anchor());
        int end = qMax(editor->position(), editor->anchor());
        if (start == end) { // just set a new one.
            QTextCharFormat format = editor->charFormat();
            visitor.visit(format);

            if (registerChange && KTextDocument(editor->document()).changeTracker() && KTextDocument(editor->document()).changeTracker()->recordChanges()) {
                QTextCharFormat prevFormat(editor->charFormat());

                int changeId = KTextDocument(editor->document()).changeTracker()->formatChangeId(title, format, prevFormat, editor->charFormat().property( KCharacterStyle::ChangeTrackerId ).toInt());
                format.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
            }

            editor->cursor()->setCharFormat(format);
            return;
        }

        QTextBlock block = editor->block();
        if (block.position() > start)
            block = block.document()->findBlock(start);

        QList<QTextCursor> cursors;
        QList<QTextCharFormat> formats;
        // now loop over all blocks that the selection contains and alter the text fragments where applicable.
        while (block.isValid() && block.position() < end) {
            QTextBlock::iterator iter = block.begin();
            while (! iter.atEnd()) {
                QTextFragment fragment = iter.fragment();
                if (fragment.position() > end)
                    break;
                if (fragment.position() + fragment.length() <= start) {
                    iter++;
                    continue;
                }

                QTextCursor cursor(block);
                cursor.setPosition(fragment.position() + 1);
                QTextCharFormat format = cursor.charFormat(); // this gets the format one char after the postion.
                visitor.visit(format);

                if (registerChange && KTextDocument(editor->document()).changeTracker() && KTextDocument(editor->document()).changeTracker()->recordChanges()) {
                    QTextCharFormat prevFormat(cursor.charFormat());

                    int changeId = KTextDocument(editor->document()).changeTracker()->formatChangeId(title, format, prevFormat, cursor.charFormat().property( KCharacterStyle::ChangeTrackerId ).toInt());
                    format.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
                }

                cursor.setPosition(qMax(start, fragment.position()));
                int to = qMin(end, fragment.position() + fragment.length());
                cursor.setPosition(to, QTextCursor::KeepAnchor);
                cursors.append(cursor);
                formats.append(format);

                QTextCharFormat prevFormat(cursor.charFormat());
                if (registerChange)
                    editor->registerTrackedChange(cursor,KOdfGenericChange::FormatChange,title, format, prevFormat, false); //this will lead to every fragment having a different change untill the change merging in registerTrackedChange checks also for formatChange or not?

                iter++;
            }
            block = block.next();
        }
        QList<QTextCharFormat>::Iterator iter = formats.begin();
        foreach(QTextCursor cursor, cursors) {
            cursor.setCharFormat(*iter);
            ++iter;
        }
    }
};

#endif //KODFTEXTEDITOR_P_H
