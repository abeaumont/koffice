/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "KoTextShapeData.h"
#include <KoTextShapeDataBase.h>
#include <KoTextShapeDataBase_p.h>
#include "KoTextDocument.h"
#include "KoTextEditor.h"
#include "KoTextDocumentLayout.h"
#include "styles/KoStyleManager.h"
#include "styles/KParagraphStyle.h"
#include "KoDeleteChangeMarker.h"

#include <KDebug>
#include <QUrl>
#include <QTextDocumentFragment>
#include <QTextDocument>
#include <QTextBlock>
#include <QMetaObject>
#include <QTextCursor>

#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>

#include <KXmlWriter.h>

#include "KoTextPage.h"

#include "opendocument/KoTextLoader.h"
#include "opendocument/KoTextWriter.h"

#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoTextAnchor.h>
#include <KInlineTextObjectManager.h>
#include <KCanvasBase.h>
#include <KoShapeController.h>
#include <KoShapeContainer.h>
#include <KUndoStack>
#include <QUndoCommand>

class KoTextShapeDataPrivate : public KoTextShapeDataBasePrivate
{
public:
    KoTextShapeDataPrivate()
            : ownsDocument(true),
            dirty(true),
            inRelayoutForPage(false),
            offset(0.0),
            position(-1),
            endPosition(-1),
            direction(KoText::AutoDirection),
            textpage(0)
    {
    }

    virtual ~KoTextShapeDataPrivate()
    {
        if (ownsDocument) {
            delete document;
        }
        delete textpage;
    }

    bool ownsDocument;
    bool dirty;
    bool inRelayoutForPage;
    qreal offset;
    int position, endPosition;
    KoText::Direction direction;
    KoTextPage *textpage;
    KoInsets padding; // Distance between shape border and text
};


KoTextShapeData::KoTextShapeData()
    : KoTextShapeDataBase(*(new KoTextShapeDataPrivate()))
{
    setDocument(new QTextDocument, true);
}

KoTextShapeData::~KoTextShapeData()
{
}

void KoTextShapeData::setDocument(QTextDocument *document, bool transferOwnership)
{
    Q_D(KoTextShapeData);
    Q_ASSERT(document);
    if (d->ownsDocument && document != d->document)
        delete d->document;
    d->ownsDocument = transferOwnership;
    if (d->document == document)
        return;
    d->document = document;
    // The following avoids the normal case where the glyph metrices are rounded to integers and
    // hinted to the screen by freetype, which you of course don't want for WYSIWYG
    if (! d->document->useDesignMetrics())
        d->document->setUseDesignMetrics(true);

    if (d->document->isEmpty() && d->document->allFormats().count() == 2) {
        // apply app default style for first parag
        KoTextDocument doc(d->document);
        KoStyleManager *sm = doc.styleManager();
        if (sm)
            sm->defaultParagraphStyle()->applyStyle(document->begin());
    }

    KoTextDocument kodoc(d->document);
    if (kodoc.textEditor() == 0)
        kodoc.setTextEditor(new KoTextEditor(d->document));
}

qreal KoTextShapeData::documentOffset() const
{
    Q_D(const KoTextShapeData);
    return d->offset;
}

void KoTextShapeData::setDocumentOffset(qreal offset)
{
    Q_D(KoTextShapeData);
    d->offset = offset;
}

int KoTextShapeData::position() const
{
    Q_D(const KoTextShapeData);
    return d->position;
}

void KoTextShapeData::setPosition(int position)
{
    Q_D(KoTextShapeData);
    d->position = position;
}

int KoTextShapeData::endPosition() const
{
    Q_D(const KoTextShapeData);
    return d->endPosition;
}

void KoTextShapeData::setEndPosition(int position)
{
    Q_D(KoTextShapeData);
    d->endPosition = position;
}

void KoTextShapeData::foul()
{
    Q_D(KoTextShapeData);
    d->dirty = true;
}

void KoTextShapeData::wipe()
{
    Q_D(KoTextShapeData);
    d->dirty = false;
}

bool KoTextShapeData::isDirty() const
{
    Q_D(const KoTextShapeData);
    return d->dirty;
}

void KoTextShapeData::fireResizeEvent()
{
    emit relayout();
}

void KoTextShapeData::setPageDirection(KoText::Direction direction)
{
    Q_D(KoTextShapeData);
    d->direction = direction;
}

KoText::Direction KoTextShapeData::pageDirection() const
{
    Q_D(const KoTextShapeData);
    return d->direction;
}

void KoTextShapeData::setPage(KoTextPage *textpage)
{
    Q_D(KoTextShapeData);
    if (d->inRelayoutForPage)
        return;
    delete d->textpage;
    d->textpage = textpage;
}

KoTextPage* KoTextShapeData::page() const
{
    Q_D(const KoTextShapeData);
    return d->textpage;
}

bool KoTextShapeData::loadOdf(const KXmlElement &element, KoShapeLoadingContext &context, KoDocumentRdfBase *rdfData, KoShape *shape)
{
    Q_UNUSED(rdfData);
    KoTextLoader loader(context, shape);

    QTextCursor cursor(document());
    loader.loadBody(element, cursor);   // now let's load the body from the ODF KXmlElement.
    KoTextEditor *editor = KoTextDocument(document()).textEditor();
    if (editor) // at one point we have to get the position from the odf doc instead.
        editor->setPosition(0);
    editor->finishedLoading();

    return true;
}

class InsertDeleteChangesCommand:public QUndoCommand
{
    public:
        InsertDeleteChangesCommand(QTextDocument *document, QUndoCommand *parent=0);
        void redo();

    private:
        QTextDocument *m_document;
        void insertDeleteChanges();
};

InsertDeleteChangesCommand::InsertDeleteChangesCommand(QTextDocument *document,QUndoCommand *parent):QUndoCommand("Insert Delete Changes",parent),m_document(document)
{
}

void InsertDeleteChangesCommand::redo()
{
    insertDeleteChanges();
}

static bool isPositionLessThan(KoChangeTrackerElement *element1, KoChangeTrackerElement *element2)
{
    return element1->deleteChangeMarker()->position() < element2->deleteChangeMarker()->position();
}

void InsertDeleteChangesCommand::insertDeleteChanges()
{
    int numAddedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    KoTextDocument(m_document).changeTracker()->deletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach (KoChangeTrackerElement *element, elementVector) {
        if (element->isValid() && element->deleteChangeMarker()) {
            QTextCursor caret(element->deleteChangeMarker()->document());
            caret.setPosition(element->deleteChangeMarker()->position() + numAddedChars +  1);
            QTextCharFormat f = caret.charFormat();
            f.clearProperty(KoCharacterStyle::InlineInstanceId);
            caret.setCharFormat(f);
            KoChangeTracker::insertDeleteFragment(caret, element->deleteChangeMarker());
            numAddedChars += KoChangeTracker::fragmentLength(element->deleteData());
        }
    }
}

class RemoveDeleteChangesCommand:public QUndoCommand
{
    public:
        RemoveDeleteChangesCommand(QTextDocument *document, QUndoCommand *parent=0);
        void redo();

    private:
        QTextDocument *m_document;
        void removeDeleteChanges();
};

RemoveDeleteChangesCommand::RemoveDeleteChangesCommand(QTextDocument *document,QUndoCommand *parent):QUndoCommand("Insert Delete Changes",parent),m_document(document)
{
}

void RemoveDeleteChangesCommand::redo()
{
    removeDeleteChanges();
}

void RemoveDeleteChangesCommand::removeDeleteChanges()
{
    int numDeletedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    KoTextDocument(m_document).changeTracker()->deletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach(KoChangeTrackerElement *element, elementVector) {
        if (element->isValid() && element->deleteChangeMarker()) {
            QTextCursor caret(element->deleteChangeMarker()->document());
            QTextCharFormat f;
            int deletePosition = element->deleteChangeMarker()->position() + 1 - numDeletedChars;
            caret.setPosition(deletePosition);
            int deletedLength = KoChangeTracker::fragmentLength(element->deleteData());
            caret.setPosition(deletePosition + deletedLength, QTextCursor::KeepAnchor);
            caret.removeSelectedText();
            numDeletedChars += KoChangeTracker::fragmentLength(element->deleteData());
        }
    }
}

void KoTextShapeData::saveOdf(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData, int from, int to) const
{
    Q_D(const KoTextShapeData);
    InsertDeleteChangesCommand *insertCommand = new InsertDeleteChangesCommand(document());
    RemoveDeleteChangesCommand *removeCommand = new RemoveDeleteChangesCommand(document());

    KoChangeTracker *changeTracker = KoTextDocument(document()).changeTracker();
    KoChangeTracker::ChangeSaveFormat changeSaveFormat;
    if (changeTracker) {
        changeSaveFormat = changeTracker->saveFormat();
        if (!changeTracker->displayChanges() && (changeSaveFormat == KoChangeTracker::DELTAXML)) {
            KoTextDocument(document()).textEditor()->addCommand(insertCommand, false);
        }

        if (changeTracker->displayChanges() && (changeSaveFormat == KoChangeTracker::ODF_1_2)) {
            KoTextDocument(document()).textEditor()->addCommand(removeCommand, false);
        }
    }

    KoTextWriter writer(context, rdfData);
    writer.write(d->document, from, to);

    if (changeTracker) {
        changeSaveFormat = changeTracker->saveFormat();
        if (!changeTracker->displayChanges() && (changeSaveFormat == KoChangeTracker::DELTAXML)) {
            insertCommand->undo();
            delete insertCommand;
        }

        if (changeTracker->displayChanges() && (changeSaveFormat == KoChangeTracker::ODF_1_2)) {
            removeCommand->undo();
            delete removeCommand;
        }
    }
}

void KoTextShapeData::relayoutFor(KoTextPage &textPage)
{
    Q_D(KoTextShapeData);
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(d->document->documentLayout());
    if (layout == 0)
        return;
    KoTextPage *oldPage = d->textpage;
    d->dirty = true;
    d->inRelayoutForPage = true;
    d->textpage = &textPage;
    layout->setProperty("KoTextRelayoutForPage", LayoutCopyShape);
    layout->interruptLayout();
    layout->relayout();
    layout->setProperty("KoTextRelayoutForPage", LayoutOrig); // relayout (if triggered by usage of variables)
    layout->interruptLayout(); // make sure it will relayout the orig shape cleanly
    d->textpage = oldPage;
    d->dirty = true;
    d->inRelayoutForPage = false;
}

KoInsets KoTextShapeData::insets() const
{
    Q_D(const KoTextShapeData);
    return d->padding;
}

void KoTextShapeData::setInsets(const KoInsets &insets)
{
    Q_D(KoTextShapeData);
    d->padding = insets;
}

#include <KoTextShapeData.moc>
