/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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

#include "KoTextEditor.h"
#include "KoTextEditor_p.h"

#include "KoBookmark.h"
#include "KInlineTextObjectManager.h"
#include <KOdf.h>
#include "KTextDocument.h"
#include "KTextDocumentLayout.h"
#include "KTextDrag.h"
#include "KTextLocator.h"
#include "KTextOdfSaveHelper.h"
#include "changetracker/KChangeTracker.h"
#include "changetracker/KChangeTrackerElement.h"
#include "changetracker/KDeleteChangeMarker.h"
#include "styles/KCharacterStyle.h"
#include "styles/KParagraphStyle.h"
#include "styles/KStyleManager.h"
#include "styles/KTableCellStyle.h"
#include "styles/KTableColumnStyle.h"
#include "styles/KTableRowStyle.h"
#include "KTableColumnAndRowStyleManager.h"
#include "commands/DeleteTableRowCommand_p.h"
#include "commands/DeleteTableColumnCommand_p.h"
#include "commands/InsertTableRowCommand_p.h"
#include "commands/InsertTableColumnCommand_p.h"

#include <KLocale>
#include <KUndoStack>

#include <QApplication>
#include <QFontDatabase>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFormat>
#include <QTextTable>
#include <QTimer>
#include <QString>
#include <QUndoCommand>

#include <kdebug.h>

static bool isRightToLeft(const QString &text)
{
    int ltr = 0, rtl = 0;

    QString::const_iterator iter = text.begin();
    while (iter != text.end()) {
        switch (QChar::direction((*iter).unicode())) {
        case QChar::DirL:
        case QChar::DirLRO:
        case QChar::DirLRE:
            ltr++;
            break;
        case QChar::DirR:
        case QChar::DirAL:
        case QChar::DirRLO:
        case QChar::DirRLE:
            rtl++;
        default:
            break;
        }
        ++iter;
    }
    return ltr < rtl;
}

/*Private*/

KoTextEditorPrivate::KoTextEditorPrivate(KoTextEditor *qq, QTextDocument *document)
    : q(qq),
    document (document),
    headCommand(0),
    isBidiDocument(false)
{
    caret = QTextCursor(document);
    editorState = NoOp;
    updateRtlTimer.setSingleShot(true);
    updateRtlTimer.setInterval(250);
    QObject::connect(&updateRtlTimer, SIGNAL(timeout()), q, SLOT(runDirectionUpdater()));
}

void KoTextEditorPrivate::documentCommandAdded()
{
    class UndoTextCommand : public QUndoCommand
    {
    public:
        UndoTextCommand(QTextDocument *document, QUndoCommand *parent = 0)
        : QUndoCommand(i18n("Text"), parent),
        m_document(document)
        {}

        void undo() {
            QTextDocument *doc = m_document.data();
            if (doc == 0)
                return;
            doc->undo(KTextDocument(doc).textEditor()->cursor());
        }

        void redo() {
            QTextDocument *doc = m_document.data();
            if (doc == 0)
                return;
            doc->redo(KTextDocument(doc).textEditor()->cursor());
        }

        QWeakPointer<QTextDocument> m_document;
    };

    //kDebug() << "editor state: " << editorState << " headcommand: " << headCommand;
    if (!headCommand || editorState == NoOp) {
        headCommand = new QUndoCommand(commandTitle);
        if (KTextDocument(document).undoStack()) {
            //kDebug() << "pushing head: " << headCommand->text();
            KTextDocument(document).undoStack()->push(headCommand);
        }
    }
    else if ((editorState == KeyPress || editorState == Delete) && headCommand->childCount()) {
        headCommand = new QUndoCommand(commandTitle);
        if (KTextDocument(document).undoStack()) {
            //kDebug() << "pushing head: " << headCommand->text();
            KTextDocument(document).undoStack()->push(headCommand);
        }
    }

    new UndoTextCommand(document, headCommand);
}

void KoTextEditorPrivate::updateState(KoTextEditorPrivate::State newState, QString title)
{
    if (editorState == Custom && newState !=NoOp)
        return;
    //kDebug() << "updateState from: " << editorState << " to: " << newState;
    if (editorState != newState || commandTitle != title) {
        if (headCommand /*&& headCommand->childCount() && KTextDocument(document).undoStack()*/) {
            //kDebug() << "reset headCommand";
            //            KTextDocument(document).undoStack()->push(headCommand);
            headCommand = 0;
        }
    }
    editorState = newState;
    if (!title.isEmpty())
        commandTitle = title;
    else
        commandTitle = i18n("Text");
    //kDebug() << "commandTitle is now: " << commandTitle;
}

bool KoTextEditorPrivate::deleteInlineObjects(bool backwards)
{
    Q_UNUSED(backwards)
    return false;
    // TODO don't just blindly delete, make this a command so we can undo it later.
    // Also note that the below code needs unit testing since I found some issues already
    /*
    QTextCursor cursor(*d->caret);
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(d->textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    KInlineTextObjectManager *manager = layout->inlineObjectTextManager();
    KInlineObject *object;
    bool found = false;

    if (d->caret->hasSelection()) {
   QString selected = cursor.selectedText();
   cursor.setPosition(cursor.selectionStart() + 1);
   int position = cursor.position();
   const QChar *data = selected.constData();
   for (int i = 0; i < selected.length(); i++) {
   if (data->unicode() == QChar::ObjectReplacementCharacter) {
   found = true;
   cursor.setPosition(position);
   object = manager->inlineTextObject(cursor);

   if (object)
   manager->removeInlineObject(cursor);
}
// if there is an inline object, the InlineTextObjectManager will also delete the char
// so only need to update position if inline object not found
else
    position++;
data++;
}
} else {
    if (!backward)
    cursor.movePosition(QTextCursor::Right);
    object = manager->inlineTextObject(cursor);

    if (object) {
   manager->removeInlineObject(cursor);
   found = true;
}
}
return found;
*/
}

void KoTextEditorPrivate::deleteSelection()
{
#ifndef NDEBUG
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(layout);
#endif
    QTextCursor delText = QTextCursor(caret);
    if (!delText.hasSelection())
        delText.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    // XXX: is there are reason for these two unused variables? Side effects? (boud)
    QString text = delText.selectedText();
    Q_UNUSED(text);
    QTextDocumentFragment selection = delText.selection();
    Q_UNUSED(selection);
    caret.deleteChar();
}

void KoTextEditorPrivate::runDirectionUpdater()
{
    while (! dirtyBlocks.isEmpty()) {
        const int blockNumber = dirtyBlocks.first();
        dirtyBlocks.removeAll(blockNumber);
        QTextBlock block = document->findBlockByNumber(blockNumber);
        if (block.isValid()) {
            KOdfText::Direction newDirection = KOdfText::AutoDirection;
            QTextBlockFormat format = block.blockFormat();
            KOdfText::Direction dir =
                static_cast<KOdfText::Direction>(format.intProperty(KParagraphStyle::TextProgressionDirection));

            if (dir == KOdfText::AutoDirection || dir == KOdfText::PerhapsLeftRightTopBottom
                    || dir == KOdfText::PerhapsRightLeftTopBottom
                    || dir == KOdfText::InheritDirection) {
                bool rtl = isRightToLeft(block.text());
                if (rtl && (dir != KOdfText::AutoDirection || QApplication::isLeftToRight()))
                    newDirection = KOdfText::PerhapsRightLeftTopBottom;
                else if (!rtl && (dir != KOdfText::AutoDirection || QApplication::isRightToLeft())) // remove previously set one if needed.
                    newDirection = KOdfText::PerhapsLeftRightTopBottom;

                QTextCursor cursor(block);
                if (format.property(KParagraphStyle::TextProgressionDirection).toInt() != newDirection) {
                    format.setProperty(KParagraphStyle::TextProgressionDirection, newDirection);
                    cursor.setBlockFormat(format); // note that setting this causes a re-layout.
                }
                if (!isBidiDocument) {
                    if ((QApplication::isLeftToRight() && (newDirection == KOdfText::RightLeftTopBottom
                                    || newDirection == KOdfText::PerhapsRightLeftTopBottom))
                            || (QApplication::isRightToLeft() && (newDirection == KOdfText::LeftRightTopBottom
                                    || newDirection == KOdfText::PerhapsLeftRightTopBottom))) {
                        isBidiDocument = true;
                        emit q->isBidiUpdated();
                    }
                }
            }
        }
    }
}

void KoTextEditorPrivate::clearCharFormatProperty(int property)
{
    class PropertyWiper : public CharFormatVisitor
    {
    public:
        PropertyWiper(int propertyId) : propertyId(propertyId) {};
        void visit(QTextCharFormat &format) const {
            format.clearProperty(propertyId);
        }

    int propertyId;
    };
    PropertyWiper wiper(property);
    CharFormatVisitor::visitSelection(q, wiper,QString(), false);
}

/*KoTextEditor*/

//TODO factor out the changeTracking charFormat setting from all individual slots to a public slot, which will be available for external commands (TextShape)

//The BlockFormatVisitor and CharFormatVisitor are used when a property needs to be modified relative to its current value (which could be different over the selection). For example: increase indentation by 10pt.
//The BlockFormatVisitor is also used for the change tracking of a blockFormat. The changeTracker stores the information about the changeId in the charFormat. The BlockFormatVisitor ensures that thd changeId is set on the whole block (even if only a part of the block is actually selected).
//Should such mechanisms be later provided directly by Qt, we could dispose of these classes.


KoTextEditor::KoTextEditor(QTextDocument *document)
    : KToolSelection(document),
    d (new KoTextEditorPrivate(this, document))
{
    connect (d->document, SIGNAL (undoCommandAdded()), this, SLOT (documentCommandAdded()));
}

KoTextEditor::~KoTextEditor()
{
    delete d;
}

void KoTextEditor::updateDefaultTextDirection(KOdfText::Direction direction)
{
    d->direction = direction;
}

QTextCursor* KoTextEditor::cursor()
{
    return &(d->caret);
}

void KoTextEditor::addCommand(QUndoCommand *command, bool addCommandToStack)
{
    d->updateState(KoTextEditorPrivate::Custom, (!command->text().isEmpty())?command->text():i18n("Text"));
    //kDebug() << "will push the custom command: " << command->text();
    d->headCommand = command;
    QUndoStack *stack = KTextDocument(d->document).undoStack();
    if (stack && addCommandToStack)
        stack->push(command);
    else
        command->redo();
    //kDebug() << "custom command pushed";
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::registerTrackedChange(QTextCursor &selection, KOdfGenericChange::Type changeType, QString title, QTextFormat& format, QTextFormat& prevFormat, bool applyToWholeBlock)
{
    if (!KTextDocument(d->document).changeTracker() || !KTextDocument(d->document).changeTracker()->recordChanges()) {
        // clear the ChangeTrackerId from the passed in selection, without recursively registring
        // change tracking again  ;)
        int start = qMin(selection.position(), selection.anchor());
        int end = qMax(selection.position(), selection.anchor());

        QTextBlock block = selection.block();
        if (block.position() > start)
            block = block.document()->findBlock(start);

        while (block.isValid() && block.position() < end) {
            QTextBlock::iterator iter = block.begin();
            while (!iter.atEnd()) {
                QTextFragment fragment = iter.fragment();
                if (fragment.position() > end) {
                    break;
                }

                if (fragment.position() + fragment.length() <= start) {
                    ++iter;
                    continue;
                }

                QTextCursor cursor(block);
                cursor.setPosition(fragment.position());
                QTextCharFormat fm = fragment.charFormat();
                fm.clearProperty(KCharacterStyle::ChangeTrackerId);
                int to = qMin(end, fragment.position() + fragment.length());
                cursor.setPosition(to, QTextCursor::KeepAnchor);
                cursor.setCharFormat(fm);
                ++iter;
            }
            block = block.next();
        }
    } else {
        if (changeType != KOdfGenericChange::DeleteChange) {
            //first check if there already is an identical change registered just before or just after the selection. If so, merge appropriatly.
            //TODO implement for format change. handle the prevFormat/newFormat check.
            QTextCursor checker = QTextCursor(selection);
            int idBefore = 0;
            int idAfter = 0;
            int changeId = 0;
            int selectionBegin = qMin(checker.anchor(), checker.position());
            int selectionEnd = qMax(checker.anchor(), checker.position());

            checker.setPosition(selectionBegin);
            KChangeTracker *changeTracker = KTextDocument(d->document).changeTracker();
            if (!checker.atBlockStart()) {
                int changeId = checker.charFormat().property(KCharacterStyle::ChangeTrackerId).toInt();
                if (changeId && changeTracker->elementById(changeId)->changeType() == changeType)
                    idBefore = changeId;
            } else {
                if (!checker.currentTable()) {
                    int changeId = checker.blockFormat().intProperty(KCharacterStyle::ChangeTrackerId);
                    if (changeId && changeTracker->elementById(changeId)->changeType() == changeType)
                        idBefore = changeId;
                } else {
                    idBefore = checker.currentTable()->format().intProperty(KCharacterStyle::ChangeTrackerId);
                    if (!idBefore) {
                        idBefore = checker.currentTable()->cellAt(checker).format().intProperty(KCharacterStyle::ChangeTrackerId);
                    }
                }
            }

            checker.setPosition(selectionEnd);
            if (!checker.atEnd()) {
                checker.movePosition(QTextCursor::NextCharacter);
                idAfter = KTextDocument(d->document).changeTracker()->mergeableId(changeType, title, checker.charFormat().property( KCharacterStyle::ChangeTrackerId ).toInt());
            }
            changeId = (idBefore)?idBefore:idAfter;

            switch (changeType) {//TODO: this whole thing actually needs to be done like a visitor. If the selection contains several change regions, the parenting needs to be individualised.
                case KOdfGenericChange::InsertChange:
                    if (!changeId)
                        changeId = KTextDocument(d->document).changeTracker()->insertChangeId(title, 0);
                break;
                case KOdfGenericChange::FormatChange:
                    if (!changeId)
                        changeId = KTextDocument(d->document).changeTracker()->formatChangeId(title, format, prevFormat, 0);
                break;
                case KOdfGenericChange::DeleteChange:
                    //this should never be the case
                break;
            }

            if (applyToWholeBlock) {
                selection.movePosition(QTextCursor::StartOfBlock);
                selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            }

            QTextCharFormat f;
            f.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
            selection.mergeCharFormat(f);

            QTextBlock startBlock = selection.document()->findBlock(selection.anchor());
            QTextBlock endBlock = selection.document()->findBlock(selection.position());

            if (startBlock != endBlock) {
                do {
                    startBlock = startBlock.next();
                    QTextCursor cursor(startBlock);
                    QTextBlockFormat blockFormat;
                    blockFormat.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
                    cursor.mergeBlockFormat(blockFormat);

                    QTextCharFormat blockCharFormat = cursor.blockCharFormat();
                    if (blockCharFormat.hasProperty(KCharacterStyle::ChangeTrackerId)) {
                        blockCharFormat.clearProperty(KCharacterStyle::ChangeTrackerId);
                        cursor.setBlockCharFormat(blockCharFormat);
                    }
                } while(startBlock != endBlock);
            }
        }
    }
}

void KoTextEditor::bold(bool bold)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Bold"));
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);

    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Bold"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::italic(bool italic)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Italic"));
    QTextCharFormat format;
    format.setFontItalic(italic);

    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Italic"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::underline(bool underline)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Underline"));
    QTextCharFormat format;
    if (underline) {
        format.setProperty(KCharacterStyle::UnderlineType, KCharacterStyle::SingleLine);
        format.setProperty(KCharacterStyle::UnderlineStyle, KCharacterStyle::SolidLine);
    } else {
        format.setProperty(KCharacterStyle::UnderlineType, KCharacterStyle::NoLineType);
        format.setProperty(KCharacterStyle::UnderlineStyle, KCharacterStyle::NoLineStyle);
    }

    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Underline"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::strikeOut(bool strikeout)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Strike Out"));
    QTextCharFormat format;
    if (strikeout) {
        format.setProperty(KCharacterStyle::StrikeOutType, KCharacterStyle::SingleLine);
        format.setProperty(KCharacterStyle::StrikeOutStyle, KCharacterStyle::SolidLine);
    } else {
        format.setProperty(KCharacterStyle::StrikeOutType, KCharacterStyle::NoLineType);
        format.setProperty(KCharacterStyle::StrikeOutStyle, KCharacterStyle::NoLineStyle);
    }
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Strike Out"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setHorizontalTextAlignment(Qt::Alignment align)
{
    class Aligner : public BlockFormatVisitor
    {
    public:
        Aligner(Qt::Alignment align) : alignment(align) {}
        void visit(QTextBlockFormat &format) const {
            format.setAlignment(alignment);
        }
        Qt::Alignment alignment;
    };

    Aligner aligner(align);
    d->updateState(KoTextEditorPrivate::Format, i18n("Set Horizontal Alignment"));
    BlockFormatVisitor::visitSelection(this, aligner, i18n("Set Horizontal Alignment"));
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setVerticalTextAlignment(Qt::Alignment align)
{
    QTextCharFormat::VerticalAlignment charAlign = QTextCharFormat::AlignNormal;
    if (align == Qt::AlignTop)
        charAlign = QTextCharFormat::AlignSuperScript;
    else if (align == Qt::AlignBottom)
        charAlign = QTextCharFormat::AlignSubScript;

    d->updateState(KoTextEditorPrivate::Format, i18n("Set Vertical Alignment"));
    QTextCharFormat format;
    format.setVerticalAlignment(charAlign);
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Set Vertical Alignment"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::decreaseIndent()
{
    class Indenter : public BlockFormatVisitor
    {
    public:
        void visit(QTextBlockFormat &format) const {
            // TODO make the 10 configurable.
            format.setLeftMargin(qMax(qreal(0.0), format.leftMargin() - 10));
        }
        Qt::Alignment alignment;
    };

    Indenter indenter;
    d->updateState(KoTextEditorPrivate::Format, i18n("Decrease Indent"));
    BlockFormatVisitor::visitSelection(this, indenter, i18n("Decrease Indent"));
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::increaseIndent()
{
    class Indenter : public BlockFormatVisitor
    {
    public:
        void visit(QTextBlockFormat &format) const {
            // TODO make the 10 configurable.
            format.setLeftMargin(format.leftMargin() + 10);
        }
        Qt::Alignment alignment;
    };

    Indenter indenter;
    d->updateState(KoTextEditorPrivate::Format, i18n("Increase Indent"));
    BlockFormatVisitor::visitSelection(this, indenter, i18n("Increase Indent"));
    d->updateState(KoTextEditorPrivate::NoOp);
}

class FontResizer : public CharFormatVisitor
{
public:
    enum Type { Grow, Shrink };
    FontResizer(Type type_) : type(type_) {
        QFontDatabase fontDB;
        defaultSizes = fontDB.standardSizes();
    }
    void visit(QTextCharFormat &format) const {
        const qreal current = format.fontPointSize();
        int prev = 1;
        foreach(int pt, defaultSizes) {
            if ((type == Grow && pt > current) || (type == Shrink && pt >= current)) {
                format.setFontPointSize(type == Grow ? pt : prev);
                return;
            }
            prev = pt;
        }
    }

    QList<int> defaultSizes;
    const Type type;
};

void KoTextEditor::decreaseFontSize()
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Decrease font size"));
    FontResizer sizer(FontResizer::Shrink);
    CharFormatVisitor::visitSelection(this, sizer, i18n("Decrease font size"));
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::increaseFontSize()
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Increase font size"));
    FontResizer sizer(FontResizer::Grow);
    CharFormatVisitor::visitSelection(this, sizer, i18n("Increase font size"));
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setFontFamily(const QString &font)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Set Font"));
    QTextCharFormat format;
    format.setFontFamily(font);
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Set Font"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setFontSize(int size)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Set Font Size"));
    QTextCharFormat format;
    format.setFontPointSize(size);
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Set Font Size"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setTextBackgroundColor(const QColor &color)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Set Background Color"));
    QTextCharFormat format;
    format.setBackground(QBrush(color));
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Set Background Color"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setTextColor(const QColor &color)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Set Text Color"));
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Set Text Color"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setStyle(KCharacterStyle *style)
{
    Q_ASSERT(style);
    d->updateState(KoTextEditorPrivate::Format, i18n("Set Character Style"));
    QTextCharFormat format;
    style->applyStyle(format);
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.setCharFormat(format);
    registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Set Character Style"), format, prevFormat, false);
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setStyle(KParagraphStyle *style)
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Set Paragraph Style"));
    const int start = qMin(position(), anchor());
    const int end = qMax(position(), anchor());
    QTextBlock block = d->document->findBlock(start);
    KStyleManager *styleManager = KTextDocument(d->document).styleManager();
    while (block.isValid() && block.position() <= end) { // now loop over all blocks
        QTextBlockFormat bf = block.blockFormat();
        if (styleManager) {
            KParagraphStyle *old = styleManager->paragraphStyle(bf.intProperty(KParagraphStyle::StyleId));
            if (old)
                old->unapplyStyle(block);
        }
        style->applyStyle(block);
        block = block.next();
    }
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::setDefaultFormat()
{
    d->updateState(KoTextEditorPrivate::Format, i18n("Set default format"));
    if (KStyleManager *styleManager = KTextDocument(d->document).styleManager()) {
        KCharacterStyle *defaultCharStyle = styleManager->defaultParagraphStyle()->characterStyle();
        QTextCharFormat format;
        defaultCharStyle->applyStyle(format);
        QTextCharFormat prevFormat(d->caret.charFormat());
        d->caret.setCharFormat(format);
        registerTrackedChange(d->caret, KOdfGenericChange::FormatChange, i18n("Set default format"), format, prevFormat, false);
    }
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::addBookmark(const QString &name)
{//TODO changeTracking
    d->updateState(KoTextEditorPrivate::Custom, i18n("Insert Bookmark"));
    KoBookmark *bookmark = new KoBookmark(name);
    int startPos = -1, endPos = -1, caretPos = -1;

    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());
    if (d->caret.hasSelection()) {
        startPos = d->caret.selectionStart();
        endPos = d->caret.selectionEnd();
        caretPos = d->caret.position();

        d->caret.setPosition(endPos);
        KoBookmark *endBookmark = new KoBookmark(name);
        bookmark->setType(KoBookmark::StartBookmark);
        endBookmark->setType(KoBookmark::EndBookmark);
        layout->inlineTextObjectManager()->insertInlineObject(d->caret, endBookmark);
        bookmark->setEndBookmark(endBookmark);
        d->caret.setPosition(startPos);
    } else {
        bookmark->setType(KoBookmark::SinglePosition);
    }
    // TODO the macro & undo things
    layout->inlineTextObjectManager()->insertInlineObject(d->caret, bookmark);
    if (startPos != -1) {
        // TODO repaint selection properly
        if (caretPos == startPos) {
            startPos = endPos + 1;
            endPos = caretPos;
        } else {
            endPos += 2;
        }
        d->caret.setPosition(startPos);
        d->caret.setPosition(endPos, QTextCursor::KeepAnchor);
    }
    d->updateState(KoTextEditorPrivate::NoOp);
}

bool KoTextEditor::insertIndexMarker()
{//TODO changeTracking
    QTextBlock block = d->caret.block();
    if (d->caret.position() >= block.position() + block.length() - 1)
        return false; // can't insert one at end of text
    if (block.text()[ d->caret.position() - block.position()].isSpace())
        return false; // can't insert one on a whitespace as that does not indicate a word.

    d->updateState(KoTextEditorPrivate::Custom, i18n("Insert Index"));
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());
    KTextLocator *tl = new KTextLocator();
    layout->inlineTextObjectManager()->insertInlineObject(d->caret, tl);
    d->updateState(KoTextEditorPrivate::NoOp);
    return true;
}

void KoTextEditor::insertInlineObject(KInlineObject *inliner)
{
    d->updateState(KoTextEditorPrivate::Custom, i18n("Insert Variable"));
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());

    int startPosition = d->caret.position();
    QTextCharFormat format = d->caret.charFormat();
    if (format.hasProperty(KCharacterStyle::ChangeTrackerId)) {
        format.clearProperty(KCharacterStyle::ChangeTrackerId);
    }

    layout->inlineTextObjectManager()->insertInlineObject(d->caret, inliner, d->caret.charFormat());

    int endPosition = d->caret.position();
    d->caret.setPosition(startPosition);
    d->caret.setPosition(endPosition, QTextCursor::KeepAnchor);
    registerTrackedChange(d->caret, KOdfGenericChange::InsertChange, i18n("Key Press"), format, format, false);
    d->caret.clearSelection();

    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::insertFrameBreak()
{
    d->updateState(KoTextEditorPrivate::KeyPress, i18n("Insert Break"));
    QTextBlock block = d->caret.block();
    if (d->caret.position() == block.position() && block.length() > 0) { // start of parag
        QTextBlockFormat bf = d->caret.blockFormat();
        bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
        d->caret.insertBlock(bf);
        if (block.textList())
            block.textList()->remove(block);
    } else {
        QTextBlockFormat bf = d->caret.blockFormat();
        newLine();
        bf = d->caret.blockFormat();
        bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); // TODO we should create an autostyle instead
        d->caret.setBlockFormat(bf);
    }
    d->updateState(KoTextEditorPrivate::NoOp);
}

bool KoTextEditor::deleteInlineObjects(bool backward)
{
    return d->deleteInlineObjects(backward);
}

int KoTextEditor::anchor() const
{
    return d->caret.anchor();
}

bool KoTextEditor::atBlockEnd() const
{
    return d->caret.atBlockEnd();
}

bool KoTextEditor::atBlockStart() const
{
    return d->caret.atBlockStart();
}

bool KoTextEditor::atEnd() const
{
    return d->caret.atEnd();
}

bool KoTextEditor::atStart() const
{
    return d->caret.atStart();
}

QTextBlock KoTextEditor::block() const
{
    return d->caret.block();
}

QTextCharFormat KoTextEditor::blockCharFormat() const
{
    return d->caret.blockCharFormat();
}

QTextBlockFormat KoTextEditor::blockFormat() const
{
    return d->caret.blockFormat();
}

int KoTextEditor::blockNumber() const
{
    return d->caret.blockNumber();
}

QTextCharFormat KoTextEditor::charFormat() const
{
    return d->caret.charFormat();
}

void KoTextEditor::clearSelection()
{
    d->caret.clearSelection();
}

int KoTextEditor::columnNumber() const
{
    return d->caret.columnNumber();
}

void KoTextEditor::deleteChar()
{
    if (!d->caret.hasSelection() && d->caret.atEnd())
        return;
    if (!d->deleteInlineObjects(false) || d->caret.hasSelection()) {
        d->updateState(KoTextEditorPrivate::Delete, i18n("Delete"));

        if (!d->caret.hasSelection())
            d->caret.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        d->deleteSelection();
    }
}

void KoTextEditor::deletePreviousChar()
{
    if (!d->caret.hasSelection() && d->caret.atStart())
        return;
    if (!d->deleteInlineObjects(false) || d->caret.hasSelection()) {
        d->updateState(KoTextEditorPrivate::Delete, i18n("Delete"));

        if (!d->caret.hasSelection())
            d->caret.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        d->deleteSelection();
    }
}

QTextDocument* KoTextEditor::document() const
{
    return d->caret.document();
}

bool KoTextEditor::hasComplexSelection() const
{
    return d->caret.hasComplexSelection();
}

bool KoTextEditor::hasSelection()
{
    return d->caret.hasSelection();
}

void KoTextEditor::insertBlock()
{
//TODO
}

void KoTextEditor::insertBlock(const QTextBlockFormat &format)
{
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat)
{
    Q_UNUSED(format)
    Q_UNUSED(charFormat)
//TODO
}

void KoTextEditor::insertFragment(const QTextDocumentFragment &fragment)
{
    Q_UNUSED(fragment)
//TODO
}

void KoTextEditor::insertTable(int rows, int columns)
{
    d->updateState(KoTextEditorPrivate::Custom, i18n("Insert Table"));
    QTextTableFormat tableFormat;

    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    tableFormat.setMargin(5);

    KChangeTracker *changeTracker = KTextDocument(d->document).changeTracker();
    if (changeTracker && changeTracker->recordChanges()) {
        QTextCharFormat charFormat = d->caret.charFormat();
        QTextBlockFormat blockFormat = d->caret.blockFormat();
        QString title = i18n("Key Press");

        int changeId;
        if (!d->caret.atBlockStart())
            changeId = changeTracker->mergeableId(KOdfGenericChange::InsertChange, title, charFormat.intProperty(KCharacterStyle::ChangeTrackerId));
        else
            changeId = changeTracker->mergeableId(KOdfGenericChange::InsertChange, title, blockFormat.intProperty(KCharacterStyle::ChangeTrackerId));

        if (!changeId)
            changeId = KTextDocument(d->document).changeTracker()->insertChangeId(title, 0);

        tableFormat.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
    }

    QTextTable *table = d->caret.insertTable(rows, columns, tableFormat);

    // Format the cells a bit.
    for (int row = 0; row < table->rows(); ++row) {
        for (int col = 0; col < table->columns(); ++col) {
            QTextTableCell cell = table->cellAt(row, col);
            QTextTableCellFormat format;
            KTableCellStyle cellStyle;
            cellStyle.setEdge(KTableCellStyle::Top, KTableCellStyle::BorderSolid, 2, QColor(Qt::black));
            cellStyle.setEdge(KTableCellStyle::Left, KTableCellStyle::BorderSolid, 2, QColor(Qt::black));
            cellStyle.setEdge(KTableCellStyle::Bottom, KTableCellStyle::BorderSolid, 2, QColor(Qt::black));
            cellStyle.setEdge(KTableCellStyle::Right, KTableCellStyle::BorderSolid, 2, QColor(Qt::black));
            cellStyle.setPadding(5);

            cellStyle.applyStyle(format);
            cell.setFormat(format);
        }
    }

    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::insertTableRowAbove()
{
    QTextTable *table = d->caret.currentTable();
    if (table) {
        int changeId = 0;
        KChangeTracker *changeTracker = KTextDocument(d->document).changeTracker();
        if (changeTracker && changeTracker->recordChanges()) {
            QString title(i18n("Insert Row Above"));
            changeId = changeTracker->insertChangeId(title, 0);
        }
        addCommand(new InsertTableRowCommand(this, table, false, changeId));
    }
}

void KoTextEditor::insertTableRowBelow()
{
    QTextTable *table = d->caret.currentTable();
    if (table) {
        int changeId = 0;
        KChangeTracker *changeTracker = KTextDocument(d->document).changeTracker();
        if (changeTracker && changeTracker->recordChanges()) {
            QString title(i18n("Insert Row Above"));
            changeId = changeTracker->insertChangeId(title, 0);
        }
        addCommand(new InsertTableRowCommand(this, table, true, changeId));
    }
}

void KoTextEditor::insertTableColumnLeft()
{
    QTextTable *table = d->caret.currentTable();
    if (table) {
        int changeId = 0;
        KChangeTracker *changeTracker = KTextDocument(d->document).changeTracker();
        if (changeTracker && changeTracker->recordChanges()) {
            QString title(i18n("Insert Column Left"));
            changeId = changeTracker->insertChangeId(title, 0);
        }
        addCommand(new InsertTableColumnCommand(this, table, false, changeId));
    }
}

void KoTextEditor::insertTableColumnRight()
{
    QTextTable *table = d->caret.currentTable();
    if (table) {
        int changeId = 0;
        KChangeTracker *changeTracker = KTextDocument(d->document).changeTracker();
        if (changeTracker && changeTracker->recordChanges()) {
            QString title(i18n("Insert Column Right"));
            changeId = changeTracker->insertChangeId(title, 0);
        }
        addCommand(new InsertTableColumnCommand(this, table, true, changeId));
    }
}

void KoTextEditor::deleteTableColumn()
{
    QTextTable *table = d->caret.currentTable();
    if (table) {
        int changeId = 0;
        KChangeTracker *changeTracker = KTextDocument(d->document).changeTracker();
        if (changeTracker && changeTracker->recordChanges()) {
            QString title(i18n("Delete Column"));
            changeId = changeTracker->deleteChangeId(title, QTextDocumentFragment(), 0);
        }

        addCommand(new DeleteTableColumnCommand(this, table, changeId));
    }
}

void KoTextEditor::deleteTableRow()
{
    QTextTable *table = d->caret.currentTable();
    if (table) {
        KChangeTracker *changeTracker = KTextDocument(d->document).changeTracker();
        int changeId = 0;
        if (changeTracker && changeTracker->recordChanges()) {
            QString title(i18n("Delete Row"));
            changeId = changeTracker->deleteChangeId(title, QTextDocumentFragment(), 0);
        }
        addCommand(new DeleteTableRowCommand(this, table, changeId));
    }
}

void KoTextEditor::mergeTableCells()
{
    d->updateState(KoTextEditorPrivate::Custom, i18n("Merge Cells"));

    QTextTable *table = d->caret.currentTable();

    if (table) {
        table->mergeCells(d->caret);
    }

    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::splitTableCells()
{
    d->updateState(KoTextEditorPrivate::Custom, i18n("Split Cells"));

    QTextTable *table = d->caret.currentTable();

    if (table) {
        QTextTableCell cell = table->cellAt(d->caret);
        table->splitCell(cell.row(), cell.column(),  cell.rowSpan(), cell.columnSpan());
    }

    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::insertText(const QString &text)
{
    d->updateState(KoTextEditorPrivate::KeyPress, i18n("Key Press"));

    //first we make sure that we clear the inlineObject charProperty, if we have no selection
    if (!d->caret.hasSelection() && d->caret.charFormat().hasProperty(KCharacterStyle::InlineInstanceId))
        d->clearCharFormatProperty(KCharacterStyle::InlineInstanceId);

    int startPosition = d->caret.position();
    QTextCharFormat format = d->caret.charFormat();
    if (format.hasProperty(KCharacterStyle::ChangeTrackerId)) {
        format.clearProperty(KCharacterStyle::ChangeTrackerId);
    }
    d->caret.insertText(text, format);
    int endPosition = d->caret.position();

    //Mark the inserted text
    d->caret.setPosition(startPosition);
    d->caret.setPosition(endPosition, QTextCursor::KeepAnchor);

    registerTrackedChange(d->caret, KOdfGenericChange::InsertChange, i18n("Key Press"), format, format, false);

    d->caret.clearSelection();

    int blockNumber = d->caret.blockNumber();
    while (blockNumber <= d->caret.blockNumber()) {
        d->dirtyBlocks << blockNumber;
        ++blockNumber;
    }
    d->updateRtlTimer.stop();
    d->updateRtlTimer.start();
}

void KoTextEditor::insertText(const QString &text, const QTextCharFormat &format)
{
    Q_UNUSED(text)
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::mergeBlockCharFormat(const QTextCharFormat &modifier)
{
    Q_UNUSED(modifier)
//TODO
}

void KoTextEditor::mergeBlockFormat(const QTextBlockFormat &modifier)
{
    Q_UNUSED(modifier)
//TODO
}

void KoTextEditor::mergeCharFormat(const QTextCharFormat &modifier)
{
    Q_UNUSED(modifier)
//TODO
}

bool KoTextEditor::movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode, int n)
{
    return d->caret.movePosition (operation, mode, n);
}

void KoTextEditor::newLine()
{
    d->updateState(KoTextEditorPrivate::Custom, i18n("Line Break"));
    if (d->caret.hasSelection())
        d->deleteInlineObjects();
    KTextDocument koDocument(d->document);
    KStyleManager *styleManager = koDocument.styleManager();
    KParagraphStyle *nextStyle = 0;
    KParagraphStyle *currentStyle = 0;
    if (styleManager) {
        int id = d->caret.blockFormat().intProperty(KParagraphStyle::StyleId);
        currentStyle = styleManager->paragraphStyle(id);
        if (currentStyle == 0) // not a style based parag.  Lets make the next one correct.
            nextStyle = styleManager->defaultParagraphStyle();
        else
            nextStyle = styleManager->paragraphStyle(currentStyle->nextStyle());
        Q_ASSERT(nextStyle);
        if (currentStyle == nextStyle)
            nextStyle = 0;
    }

    int startPosition = d->caret.position();
    QTextCharFormat format = d->caret.charFormat();
    if (format.hasProperty(KCharacterStyle::ChangeTrackerId)) {
        format.clearProperty(KCharacterStyle::ChangeTrackerId);
    }

    QTextBlockFormat bf = d->caret.blockFormat();
    QVariant direction = bf.property(KParagraphStyle::TextProgressionDirection);
    bf.clearProperty(QTextFormat::PageBreakPolicy);
    bf.clearProperty(KParagraphStyle::ListStartValue);
    bf.clearProperty(KParagraphStyle::UnnumberedListItem);
    bf.clearProperty(KParagraphStyle::IsListHeader);
    bf.clearProperty(KParagraphStyle::MasterPageName);
    d->caret.insertBlock(bf); // does not inherit list

    if (nextStyle) {
        QTextBlock block = d->caret.block();
        if (currentStyle)
            currentStyle->unapplyStyle(block);
        nextStyle->applyStyle(block);
    } else { // need to inherit list manually
        QTextBlock block = d->caret.block();
        QTextBlock prev = block.previous();
        QTextList *list = prev.textList();
        if (list) {
            if (prev.length() > 1) { // if not empty
                list->add(block);
            } else if (list->count() > 1 && list->itemNumber(prev) == list->count() -1) {
                // remove list from prevblock
                list->remove(prev);
            }
        }
    }

    bf = d->caret.blockFormat();
    if (d->direction != KOdfText::AutoDirection) { // inherit from shape
        KOdfText::Direction dir;
        switch (d->direction) {
        case KOdfText::RightLeftTopBottom:
            dir = KOdfText::PerhapsRightLeftTopBottom;
            break;
        case KOdfText::LeftRightTopBottom:
        default:
            dir = KOdfText::PerhapsLeftRightTopBottom;
        }
        bf.setProperty(KParagraphStyle::TextProgressionDirection, dir);
    } else if (! direction.isNull()) { // then we inherit from the previous paragraph.
        bf.setProperty(KParagraphStyle::TextProgressionDirection, direction);
    }
    d->caret.setBlockFormat(bf);

    int endPosition = d->caret.position();

    //Mark the inserted text
    d->caret.setPosition(startPosition);
    d->caret.setPosition(endPosition, QTextCursor::KeepAnchor);

    registerTrackedChange(d->caret, KOdfGenericChange::InsertChange, i18n("Key Press"), format, format, false);

    d->caret.clearSelection();

    d->updateState(KoTextEditorPrivate::NoOp);
}

int KoTextEditor::position() const
{
    return d->caret.position();
}

void KoTextEditor::removeSelectedText()
{
    d->caret.removeSelectedText();
}

void KoTextEditor::select(QTextCursor::SelectionType selection)
{
    //TODO add selection of previous/next char, and option about hasSelection
    d->caret.select(selection);
}

QString KoTextEditor::selectedText() const
{
    return d->caret.selectedText();
}

QTextDocumentFragment KoTextEditor::selection() const
{
    return d->caret.selection();
}

int KoTextEditor::selectionEnd() const
{
    return d->caret.selectionEnd();
}

int KoTextEditor::selectionStart() const
{
    return d->caret.selectionStart();
}

void KoTextEditor::setBlockCharFormat(const QTextCharFormat &format)
{
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::setBlockFormat(const QTextBlockFormat &format)
{
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::setCharFormat(const QTextCharFormat &format)
{
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::setPosition(int pos, QTextCursor::MoveMode m)
{
    d->caret.setPosition (pos, m);
}

void KoTextEditor::setVisualNavigation(bool b)
{
    d->caret.setVisualNavigation (b);
}

bool KoTextEditor::visualNavigation() const
{
    return d->caret.visualNavigation();
}

bool KoTextEditor::isBidiDocument() const
{
    return d->isBidiDocument;
}

void KoTextEditor::beginEditBlock()
{
    d->updateState(KoTextEditorPrivate::Custom);
    d->caret.beginEditBlock();
}

void KoTextEditor::endEditBlock()
{
    d->caret.endEditBlock();
    d->updateState(KoTextEditorPrivate::NoOp);
}

void KoTextEditor::finishedLoading()
{
    QTextBlock block = d->document->begin();
    while (!d->isBidiDocument && block.isValid()) {
        bool rtl = isRightToLeft(block.text());
        if ((QApplication::isLeftToRight() && rtl) || (QApplication::isRightToLeft() && !rtl)) {
            d->isBidiDocument = true;
            emit isBidiUpdated();
        }
        block = block.next();
    }
}

#include <KoTextEditor.moc>
