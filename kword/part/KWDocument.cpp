/* This file is part of the KDE project
 * Copyright (C) 2002-2006 David Faure <faure@kde.org>
 * Copyright (C) 2005-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2008 Sebastian Sauer <mail@dipe.org>
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

// kword includes
#include "KWDocument.h"
#include "KWDocument_p.h"
#include "KWFactory.h"
#include "KWView.h"
#include "KWCanvas.h"
#include "KWPageManager.h"
#include "KWPage.h"
#include "KWPageStyle.h"
#include "KWOdfLoader.h"
#include "KWDLoader.h"
#include "KWOdfWriter.h"
#include "frames/KWFrameSet.h"
#include "frames/KWTextFrameSet.h"
#include "frames/KWTextFrame.h"
#include "frames/KWFrameLayout.h"
#include "frames/KWOutlineShape.h"
#include "dialogs/KWFrameDialog.h"
#include "dialogs/KWStartupWidget.h"
#include "commands/KWFrameRemoveSilentCommand.h"
#include "commands/KWPageInsertCommand.h"
#include "commands/KWPageRemoveCommand.h"

// koffice libs includes
#include <KShapeManager.h>
#include <KTextDocument.h>
#include <KTextAnchor.h>
#include <KShapeContainer.h>
#include <KOdfWriteStore.h>
#include <KToolManager.h>
#include <KShapeRegistry.h>
#include <KShapeFactoryBase.h>
#include <KStyleManager.h>
#include <KResourceManager.h>
#include <KInteractionTool.h>
#include <KInlineTextObjectManager.h>
#include <KoDocumentInfo.h>
#include <KCharacterStyle.h>
#include <KParagraphStyle.h>
#include <KListStyle.h>
#include <KListLevelProperties.h>
#include <KTextShapeData.h>
#include <KChangeTracker.h>

#include <KDocumentRdfBase.h>
#ifdef SHOULD_BUILD_RDF
#include <rdf/KoDocumentRdf.h>
#endif

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

// KDE + Qt includes
#include <klocale.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kdebug.h>
#include <QIODevice>
#include <QTimer>
#include <QThread>
#include <QCoreApplication>
#include <QTextBlock>

/// \internal
// this class will be added to all views and be hidden by default.
// during loading any frames we find will be added here and only when they are positioned properly will
// those frames be re-shown.
class MagicCurtain : public KShapeContainer
{
public:
    // reimplemented pure virtual calls
    bool loadOdf(const KXmlElement&, KShapeLoadingContext&) { return false; }
    void saveOdf(KShapeSavingContext&) const { }
    void paintComponent(QPainter&, const KViewConverter&) { }

    /// add the frame to be hidden
    void addFrame(KWFrame *frame);
    /// add the shape to be hidden
    void addShape(KShape *shape);
    // reveal all the frames that were added before
    void revealFramesForPage(int pageNumber, qreal moveFrames);

private:
    QHash<int, QList<KWFrame*> > m_data;
};

void MagicCurtain::addFrame(KWFrame *frame)
{
    Q_ASSERT(frame->loadingPageNumber() > 0);
    QList<KWFrame*> frames = m_data.value(frame->loadingPageNumber());
    frames << frame;
    m_data.insert(frame->loadingPageNumber(), frames);
    frame->shape()->setParent(this);
}

void MagicCurtain::addShape(KShape *shape)
{
    shape->setParent(this);
}

void MagicCurtain::revealFramesForPage(int pageNumber, qreal moveFrames)
{
    QPointF offset(0, moveFrames);
    foreach (KWFrame *frame, m_data.value(pageNumber)) {
        frame->shape()->setPosition(frame->shape()->position() + offset);
        frame->shape()->setParent(0);
        frame->clearLoadingData();
    }
    m_data.remove(pageNumber);
}


// KWDocument
KWDocument::KWDocument(QWidget *parentWidget, QObject *parent, bool singleViewMode)
        : KoDocument(parentWidget, parent, singleViewMode),
        m_frameLayout(&m_pageManager, m_frameSets),
        m_magicCurtain(0),
        m_mainFramesetEverFinished(false),
        m_loadingTemplate(false),
        m_commandBeingAdded(0)
{
    m_frameLayout.setDocument(this);
    resourceManager()->setOdfDocument(this);

    setComponentData(KWFactory::componentData(), false);
    setTemplateType("kword_template");

    connect(&m_frameLayout, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFrameSet(KWFrameSet*)));
    connect(&m_frameLayout, SIGNAL(removedFrameSet(KWFrameSet*)), this, SLOT(removeFrameSet(KWFrameSet*)));

    resourceManager()->setUndoStack(undoStack());
    if (documentRdfBase()) {
        documentRdfBase()->linkToResourceManager(resourceManager());
    }

    QVariant variant;
    variant.setValue(new KChangeTracker(resourceManager()));
    resourceManager()->setResource(KOdfText::ChangeTracker, variant);

    connect(documentInfo(), SIGNAL(infoUpdated(const QString &, const QString &)),
            inlineTextObjectManager(), SLOT(documentInformationUpdated(const QString &, const QString &)));

    clear();
}

KWDocument::~KWDocument()
{
    delete m_magicCurtain;
    m_config.setUnit(unit());
    saveConfig();
    qDeleteAll(m_frameSets);
}

void KWDocument::addShape(KShape *shape)
{
    // notice that this call can come from the user inserting a shape, the undo framework
    // or from pasting.

    KWFrame *frame = dynamic_cast<KWFrame*>(shape->applicationData());
    if (frame == 0) {
        KWFrameSet *fs;
        if (shape->shapeId() == TextShape_SHAPEID) {
            KWTextFrameSet *tfs = new KWTextFrameSet(this);
            fs = tfs;
            fs->setName(uniqueFrameSetName("Text"));
            frame = new KWTextFrame(shape, tfs);
        } else {
            fs = new KWFrameSet();
            fs->setName(shape->shapeId());
            frame = new KWFrame(shape, fs);
        }
        // since we auto-decorate we can expect someone to add a shape that has
        // as a child an already existing shape we previous decorated with a frame.
        recurseFrameRemovalOn(dynamic_cast<KShapeContainer*>(shape), m_commandBeingAdded);
    }
    Q_ASSERT(frame->frameSet());
    addFrameSet(frame->frameSet());

    foreach (KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        canvas->shapeManager()->addShape(shape);
    }
}

void KWDocument::recurseFrameRemovalOn(KShapeContainer *container, QUndoCommand *parent)
{
    if (container == 0)
        return;
    foreach (KShape *shape, container->shapes()) {
        KWFrame *frame = dynamic_cast<KWFrame*>(shape->applicationData());
        if (frame) {
            QUndoCommand *cmd = new KWFrameRemoveSilentCommand(this, frame, m_commandBeingAdded);
            if (m_commandBeingAdded)
                cmd->redo();
            else
                addCommand(cmd);
        }
        recurseFrameRemovalOn(dynamic_cast<KShapeContainer*>(shape), parent);
    }
}

void KWDocument::removeShape(KShape *shape)
{
    KWFrame *frame = dynamic_cast<KWFrame*>(shape->applicationData());
    if (frame) { // not all shapes have to have a frame. Only top-level ones do.
        KWFrameSet *fs = frame->frameSet();
        Q_ASSERT(fs);
        if (fs->frameCount() == 1) // last frame on FrameSet
            removeFrameSet(fs); // frame and frameset will be deleted when the shape is deleted
        else
            fs->removeFrame(frame);
    } else { // not a frame, but we still have to remove it from views.
        foreach (KoView *view, views()) {
            KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
            canvas->shapeManager()->remove(shape);
        }
    }
}

void KWDocument::paintContent(QPainter&, const QRect &rect)
{
    Q_UNUSED(rect);
    // TODO: implement KWDocument::paintContent
}

KoView *KWDocument::createViewInstance(QWidget *parent)
{
    KWView *view = new KWView(m_viewMode, this, parent);
    if (m_magicCurtain)
        view->kwcanvas()->shapeManager()->addShape(m_magicCurtain, KShapeManager::AddWithoutRepaint);

    bool switchToolCalled = false;
    foreach (KWFrameSet *fs, m_frameSets) {
        if (fs->frameCount() == 0)
            continue;
        foreach (KWFrame *frame, fs->frames())
            view->kwcanvas()->shapeManager()->addShape(frame->shape(), KShapeManager::AddWithoutRepaint);
        if (switchToolCalled)
            continue;
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
        if (tfs && tfs->textFrameSetType() == KWord::MainTextFrameSet) {
            KShapeSelection *selection = view->kwcanvas()->shapeManager()->selection();
            selection->select(fs->frames().first()->shape());

            KToolManager::instance()->switchToolRequested(
                KToolManager::instance()->preferredToolForSelection(selection->selectedShapes()));
            switchToolCalled = true;
        }
    }
    if (!switchToolCalled)
        KToolManager::instance()->switchToolRequested(KoInteractionTool_ID);

    return view;
}

KWPage KWDocument::insertPage(int afterPageNum, const QString &masterPageName)
{
    KWPageInsertCommand *cmd = new KWPageInsertCommand(this, afterPageNum, masterPageName);
    addCommand(cmd);
    Q_ASSERT(cmd->page().isValid());
    return cmd->page();
}

KWPage KWDocument::appendPage(const QString &masterPageName)
{
    int number = 0;
    KWPage last = m_pageManager.last();
    if (last.isValid())
        number = last.pageNumber();
    return insertPage(number, masterPageName);
}

void KWDocument::removePage(int pageNumber)
{
    if (pageCount() <= 1)
        return;

    KWPage page = m_pageManager.page(pageNumber);
    if (! page.isValid()) {
        kWarning(32001) << "remove page requested for a non exiting page!" << pageNumber;
        return;
    }
    addCommand(new KWPageRemoveCommand(this, page));
}

void KWDocument::firePageSetupChanged()
{
    if (inlineTextObjectManager())
        inlineTextObjectManager()->setProperty(KInlineObject::PageCount, pageCount());
    resourceManager()->setResource(KWord::CurrentPageCount, pageCount());
    emit pageSetupChanged();
}

void KWDocument::removeFrameSet(KWFrameSet *fs)
{
    m_frameSets.removeAt(m_frameSets.indexOf(fs));
    setModified(true);
    foreach (KWFrame *frame, fs->frames())
        removeFrame(frame);
    resourceManager()->setResource(KWord::CurrentFrameSetCount, m_frameSets.count());
    KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
    if (tfs) {
        QList<QTextDocument*> docs = resourceManager()->textDocumentList();
        if (docs.removeAll(tfs->document()))
            resourceManager()->setTextDocumentList(docs);
    }
}

void KWDocument::addFrameSet(KWFrameSet *fs)
{
    if (m_frameSets.contains(fs)) return;
    setModified(true);
    m_frameSets.append(fs);
    foreach (KWFrame *frame, fs->frames())
        addFrame(frame);

    KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
    if (tfs) {
        tfs->setPageManager(pageManager());
        if (tfs->textFrameSetType() == KWord::MainTextFrameSet ||
                tfs->textFrameSetType() == KWord::OtherTextFrameSet) {
            connect(tfs, SIGNAL(moreFramesNeeded(KWTextFrameSet*)),
                    this, SLOT(requestMoreSpace(KWTextFrameSet*)));
            connect(tfs, SIGNAL(layoutDone()), this, SLOT(mainTextFrameSetLayoutDone()));
        }
        else {
            connect(tfs, SIGNAL(decorationFrameResize(KWTextFrameSet*)),
                    this, SLOT(updateHeaderFooter(KWTextFrameSet*)));
        }
    }

    connect(fs, SIGNAL(frameAdded(KWFrame*)), this, SLOT(addFrame(KWFrame*)));
    connect(fs, SIGNAL(frameRemoved(KWFrame*)), this, SLOT(removeFrame(KWFrame*)));
}

void KWDocument::addFrame(KWFrame *frame)
{
    foreach (KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        if (frame->outlineShape())
            canvas->shapeManager()->addShape(frame->outlineShape()->parent());
        else
            canvas->shapeManager()->addShape(frame->shape());
    }
    if (frame->loadingPageNumber() > 0) {
        if (m_magicCurtain == 0) {
            m_magicCurtain = new MagicCurtain();
            m_magicCurtain->setVisible(false);
            foreach (KoView *view, views())
                static_cast<KWView*>(view)->kwcanvas()->shapeManager()->addShape(m_magicCurtain);
        }
        m_magicCurtain->addFrame(frame);
    } else {
        frame->shape()->update();
    }
    resourceManager()->setResource(KWord::CurrentFrameSetCount, m_frameSets.count());

    KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(frame->frameSet());
    if (tfs) { // we add the textdoc here instead of in the addFrameSet because
                // only after the first frame is added are we sure the doc won't change.
        QList<QTextDocument*> docs = resourceManager()->textDocumentList();
        if (!docs.contains(tfs->document())) {
            docs.append(tfs->document());
            resourceManager()->setTextDocumentList(docs);
        }
    }
}

void KWDocument::removeFrame(KWFrame *frame)
{
    if (frame->shape() == 0) return;
    removeFrameFromViews(frame);
    KWPage page = pageManager()->page(frame->shape());
    if (!page.isValid()) return;
    if (!page.isAutoGenerated()) return;
    if (page != pageManager()->last() || page == pageManager()->begin())
        return; // can only delete last page.
    foreach (KWFrameSet *fs, m_frameSets) {
        foreach (KWFrame *f, fs->frames()) {
            if (f->isCopy())
                continue;
            if (page == pageManager()->page(f->shape()))
                return;
        }
    }

    KWPageRemoveCommand *cmd = new KWPageRemoveCommand(this, page);
    cmd->redo();
    delete cmd;
}

void KWDocument::mainTextFrameSetLayoutDone()
{
    m_mainFramesetEverFinished = true;
}

KWFrameSet *KWDocument::frameSetByName(const QString &name)
{
    foreach (KWFrameSet *fs, m_frameSets) {
        if (fs->name() == name)
            return fs;
    }
    return 0;
}

KWTextFrameSet *KWDocument::mainFrameSet() const
{
    return m_frameLayout.mainFrameSet();
}

KInlineTextObjectManager *KWDocument::inlineTextObjectManager() const
{
    QVariant var = resourceManager()->resource(KOdfText::InlineTextObjectManager);
    return var.value<KInlineTextObjectManager*>();
}

QString KWDocument::uniqueFrameSetName(const QString &suggestion)
{
    // make up a new name for the frameset, use "[base] [digits]" as template.
    // Fully translatable naturally :)
    return renameFrameSet("", suggestion);
}

QString KWDocument::suggestFrameSetNameForCopy(const QString &base)
{
    // make up a new name for the frameset, use Copy[digits]-[base] as template.
    // Fully translatable naturally :)
    return renameFrameSet(i18n("Copy"), base);
}

QString KWDocument::renameFrameSet(const QString &prefix, const QString &base)
{
    if (! frameSetByName(base))
        return base;
    QString before, after;
    QRegExp findDigits("\\d+");
    int pos = findDigits.indexIn(base);
    if (pos >= 0) {
        before = base.left(pos);
        after = base.mid(pos + findDigits.matchedLength());
    } else if (prefix.isEmpty())
        before = base + ' ';
    else {
        before = prefix;
        after = ' ' + base;
    }

    if (! before.startsWith(prefix)) {
        before = prefix + before;
    }

    int count = 0;
    while (true) {
        QString name = (before + (count == 0 ? "" : QString::number(count)) + after).trimmed();
        if (! frameSetByName(name))
            return name;
        count++;
    }
}

void KWDocument::clipToDocument(const KShape *shape, QPointF &move) const
{
    Q_ASSERT(shape);
    const QPointF absPos = shape->absolutePosition();
    const QPointF destination = absPos + move;
    qreal bottomOfPage = 0.0;
    KWPage page;
    foreach (const KWPage &p, pageManager()->pages()) {
        bottomOfPage += p.height();
        if (bottomOfPage >= absPos.y())
            page = p;
        if (bottomOfPage >= destination.y()) {
            page = p;
            break;
        }
    }
    if (!page.isValid()) { // shape was not in any page to begin with, can't propose anything sane...
        move.setX(0);
        move.setY(0);
        return;
    }
    QRectF pageRect(page.rect().adjusted(5, 5, -5, -5));
    QPainterPath path(shape->absoluteTransformation(0).map(shape->outline()));
    QRectF shapeBounds = path.boundingRect();
    shapeBounds.moveTopLeft(shapeBounds.topLeft() + move);
    if (!shapeBounds.intersects(pageRect)) {
        if (shapeBounds.left() > pageRect.right()) // need to move to the left some
            move.setX(move.x() + (pageRect.right() - shapeBounds.left()));
        else if (shapeBounds.right() < pageRect.left()) // need to move to the right some
            move.setX(move.x() + pageRect.left() - shapeBounds.right());

        if (shapeBounds.top() > pageRect.bottom()) // need to move up some
            move.setY(move.y() + (pageRect.bottom() - shapeBounds.top()));
        else if (shapeBounds.bottom() < pageRect.top()) // need to move down some
            move.setY(move.y() + pageRect.top() - shapeBounds.bottom());
    }
}

// *** LOADING

void KWDocument::initEmpty()
{
    clear();

    appendPage("Standard");

    Q_ASSERT(resourceManager()->hasResource(KOdfText::StyleManager));
    KStyleManager *styleManager = resourceManager()->resource(KOdfText::StyleManager).value<KStyleManager*>();
    Q_ASSERT(styleManager);

    QTextDocument document;
    KTextDocument doc(&document);
    doc.setStyleManager(styleManager);
    KOdfText::loadOpenDocument("/home/zander/work/kde/build-trunk/installed/share/apps/kword/templates/Normal/.source/A4.odt", &document);

    KParagraphStyle *parag = new KParagraphStyle();
    parag->setName(i18n("Head 1"));
    KCharacterStyle *character = parag->characterStyle();
    character->setFontPointSize(20);
    character->setFontWeight(QFont::Bold);
    styleManager->add(parag);

    parag = new KParagraphStyle();
    parag->setName(i18n("Head 2"));
    character = parag->characterStyle();
    character->setFontPointSize(16);
    character->setFontWeight(QFont::Bold);
    styleManager->add(parag);

    parag = new KParagraphStyle();
    parag->setName(i18n("Head 3"));
    character = parag->characterStyle();
    character->setFontPointSize(12);
    character->setFontWeight(QFont::Bold);
    styleManager->add(parag);

    parag = new KParagraphStyle();
    parag->setName(i18n("Bullet List"));
    KListStyle *list = new KListStyle(parag);
    KListLevelProperties llp = list->levelProperties(0);
    llp.setStyle(KListStyle::DiscItem);
    list->setLevelProperties(llp);
    parag->setListStyle(list);
    styleManager->add(parag);

    KoDocument::initEmpty();
    clearUndoHistory();
}

void KWDocument::clear()
{
    // document defaults
    foreach (const KWPage &page, m_pageManager.pages())
        m_pageManager.removePage(page);
    m_pageManager.clearPageStyles();

    m_config.load(this); // re-load values
    foreach (KWFrameSet *fs, m_frameSets) {
        removeFrameSet(fs);
        delete fs;
    }

    // industry standard for bleed
    KInsets padding;
    padding.top = MM_TO_POINT(3);
    padding.bottom = MM_TO_POINT(3);
    padding.left = MM_TO_POINT(3);
    padding.right = MM_TO_POINT(3);
    m_pageManager.setPadding(padding);

    if (inlineTextObjectManager())
        inlineTextObjectManager()->setProperty(KInlineObject::PageCount, pageCount());
}

void KWDocument::openTemplate(const KUrl &url)
{
    m_loadingTemplate = true;
    KoDocument::openTemplate(url);
    m_loadingTemplate = false;
}

bool KWDocument::loadOdf(KOdfStoreReader &odfStore)
{
    clear();
    KWOdfLoader loader(this);
    bool rc = loader.load(odfStore);
    if (rc) {
        if (m_loadingTemplate) {
            Q_ASSERT(resourceManager()->hasResource(KOdfText::StyleManager));
            KStyleManager *styleManager = resourceManager()->resource(KOdfText::StyleManager).value<KStyleManager*>();
            Q_ASSERT(styleManager);
            foreach (KParagraphStyle *style, styleManager->paragraphStyles()) {
                QString name = style->name();
                QByteArray bytes(name.toAscii());

                // qstring is utf16, don't loose data by going via 8 bits;
                if (QString::fromAscii(bytes.constData()) == name) { // check data consistency
                    QString newName(i18n(bytes.constData()));
                    style->setName(newName);
                }
            }
        }

        endOfLoading();
    }
    return rc;
}

bool KWDocument::loadXML(const KXmlDocument &doc, KOdfStore *store)
{
    clear();
    KXmlElement root = doc.documentElement();
    KWDLoader loader(this, store);
    bool rc = loader.load(root);
    if (rc)
        endOfLoading();
    return rc;
}

void KWDocument::endOfLoading() // called by both oasis and oldxml
{
    QPointer<KoUpdater> updater;
    if (progressUpdater()) {
        updater = progressUpdater()->startSubtask(1, "KWDocument::endOfLoading");
        updater->setProgress(0);
    }

    // Get the master page name of the first page.
    QString firstPageMasterName;
    if (mainFrameSet()) {
        QTextBlock block = mainFrameSet()->document()->firstBlock();
        firstPageMasterName = block.blockFormat().stringProperty(KParagraphStyle::MasterPageName);
    }

    KWPage lastpage = pageManager()->last();
    qreal docHeight = lastpage.isValid() ? (lastpage.offsetInDocument() + lastpage.height()) : 0.0;
    PageProcessingQueue *ppq = new PageProcessingQueue(this);

    // insert pages
    qreal maxBottom = 0;
    foreach (KWFrameSet *fs, m_frameSets) {
        foreach (KWFrame *frame, fs->frames())
        maxBottom = qMax(maxBottom, frame->shape()->boundingRect().bottom());
    }
    // The Document we loaded could have specified
    //  1) a number of pages
    //  2) a number of frames
    // At the end of loading we then end up in one of 3 situations.
    // a) we have exactly the amount of pages that the document needs.
    // b) we have absolute frames positioned on pages that don't exist.
    // c) we have so much text in any of our text-framesets that new pages
    //    may have to be generated at some time after loading is completed.

    if (m_magicCurtain) { // pages defined in the loaded-document
        foreach (const KWPage &page, m_pageManager.pages())
            m_magicCurtain->revealFramesForPage(page.pageNumber(), page.offsetInDocument());
    }

    // Here we look at point 'b'. We add pages so at least all frames have a page.
    // btw. the observent reader might notice that cases b and c are not mutually exclusive ;)
    while (docHeight <= maxBottom) {
        kDebug(32001) << "KWDocument::endOfLoading appends a page";
        if (m_pageManager.pageCount() == 0) // apply the firstPageMasterName only on the first page
            lastpage = m_pageManager.appendPage(m_pageManager.pageStyle(firstPageMasterName));
        else // normally this shouldn't happen cause that loop is only run once...
            lastpage = m_pageManager.appendPage();
        ppq->addPage(lastpage);
        docHeight += lastpage.height();
        if (m_magicCurtain) {
            m_magicCurtain->revealFramesForPage(lastpage.pageNumber(), lastpage.offsetInDocument());
        }
    }

    if (updater) updater->setProgress(50);

#if 0
    // do some sanity checking on document.
    for (int i = frameSetCount() - 1; i > -1; i--) {
        KWFrameSet *fs = frameSet(i);
        if (!fs) {
            kWarning() << "frameset " << i << " is NULL!!";
            m_lstFrameSet.remove(i);
            continue;
        }
        if (fs->type() == FT_TABLE) {
            static_cast<KWTableFrameSet *>(fs)->validate();
        } else if (fs->type() == FT_TEXT) {
            for (int f = fs->frameCount() - 1; f >= 0; f--) {
                KWFrame *frame = fs->frame(f);
                if (frame->left() < 0) {
                    kWarning() << fs->name() << " frame " << f << " pos.x is < 0, moving frame";
                    frame->moveBy(0 - frame->left(), 0);
                }
                if (frame->right() > m_pageLayout.ptWidth) {
                    kWarning() << fs->name() << " frame " << f << " rightborder outside page ("
                    << frame->right() << ">" << m_pageLayout.ptWidth << "), shrinking";
                    frame->setRight(m_pageLayout.ptWidth);
                }
                if (fs->isProtectSize())
                    continue; // don't make frames bigger of a protected frameset.
                if (frame->height() < s_minFrameHeight) {
                    kWarning() << fs->name() << " frame " << f << " height is so small no text will fit, adjusting (was: "
                    << frame->height() << " is: " << s_minFrameHeight << ")";
                    frame->setHeight(s_minFrameHeight);
                }
                if (frame->width() < s_minFrameWidth) {
                    kWarning() << fs->name() << " frame " << f << " width is so small no text will fit, adjusting (was: "
                    << frame->width() << " is: " << s_minFrameWidth  << ")";
                    frame->setWidth(s_minFrameWidth);
                }
            }
            if (fs->frameCount() == 0) {
                KWPage *page = pageManager()->page(startPage());
                KWFrame *frame = new KWFrame(fs, page->leftMargin(), page->topMargin(),
                                             page->width() - page->leftMargin() - page->rightMargin(),
                                             page->height() - page->topMargin() - page->bottomMargin());
                //kDebug(32001) <<"KWDocument::loadXML main-KWFrame created" << *frame;
                fs->addFrame(frame);
            }
        } else if (fs->frameCount() == 0) {
            kWarning() << "frameset " << i << " " << fs->name() << " has no frames";
            removeFrameSet(fs);
            if (fs->type() == FT_PART)
                delete static_cast<KWPartFrameSet *>(fs)->getChild();
            delete fs;
            continue;
        }
        if (fs->frameCount() > 0) {
            KWFrame *frame = fs->frame(0);
            if (frame->isCopy()) {
                kWarning() << "First frame in a frameset[" << fs->name() << "] was set to be a copy; resetting";
                frame->setCopy(false);
            }
        }
    }

    // Renumber footnotes
    KWTextFrameSet *frameset = dynamic_cast<KWTextFrameSet *>(m_lstFrameSet.getFirst());
    if (frameset)
        frameset->renumberFootNotes(false /*no repaint*/);

#endif
    // remove header/footer frames that are not visible.
    m_frameLayout.cleanupHeadersFooters();

    foreach (const KWPage &page, m_pageManager.pages())
        m_frameLayout.createNewFramesForPage(page.pageNumber());

    foreach (KWFrameSet *fs, m_frameSets) {
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
        if (!tfs)
            continue;
        KTextDocument textDoc(tfs->document());
        foreach (KInlineObject *inlineObject, textDoc.inlineTextObjectManager()->inlineTextObjects()) {
            KTextAnchor *anchor = dynamic_cast<KTextAnchor*>(inlineObject);
            if (anchor) {
                if (m_magicCurtain == 0) {
                    m_magicCurtain = new MagicCurtain();
                    m_magicCurtain->setVisible(false);
                    foreach (KoView *view, views())
                        static_cast<KWView*>(view)->kwcanvas()->shapeManager()->addShape(m_magicCurtain);
                }
                m_magicCurtain->addShape(anchor->shape());
            }
        }
        tfs->setAllowLayout(true);
    }

    if (updater) updater->setProgress(100);

    kDebug(32001) << "KWDocument::endOfLoading done";

    // Note that more stuff will happen in completeLoading
    firePageSetupChanged();
    setModified(false);
}

bool KWDocument::saveOdf(SavingContext &documentContext)
{
    KWOdfWriter writer(this);
    return writer.save(documentContext.odfStore, documentContext.embeddedSaver);
}

QStringList KWDocument::extraNativeMimeTypes(ImportExportType importExportType) const
{
    QStringList answer = KoDocument::extraNativeMimeTypes(importExportType);
    if (importExportType == KoDocument::ForExport)
        answer.removeAll("application/x-kword"); // we can't save this, only load.
    return answer;
}

void KWDocument::requestMoreSpace(KWTextFrameSet *fs)
{
    // kDebug(32002) << fs;
    Q_ASSERT(fs);
    Q_ASSERT(fs->frameCount() > 0);
    Q_ASSERT(QThread::currentThread() == thread());

    KWFrame *lastFrame = fs->frames().last();

    QString masterPageName;
    if (fs == mainFrameSet()) {
        KShape *shape = lastFrame->shape();
        if (shape) {
            KTextShapeData *data = qobject_cast<KTextShapeData*>(shape->userData());
            if (data) {
                QTextBlock block = fs->document()->findBlock(data->endPosition() + 1);
                if (block.isValid()) {
                    masterPageName = block.blockFormat().stringProperty(KParagraphStyle::MasterPageName);
                }
            }
        }
    }

    KWPage page = m_pageManager.page(lastFrame->shape());
    int pageDiff = m_pageManager.pageCount() - page.pageNumber();
    if (page.pageSide() == KWPage::PageSpread)
        pageDiff--;
    if (pageDiff >= (lastFrame->frameOnBothSheets() ? 1 : 2)) {
        // its enough to just create a new frame.
        m_frameLayout.createNewFrameForPage(fs, page.pageNumber()
                + (lastFrame->frameOnBothSheets() ? 1 : 2));
    } else {
        int afterPageNum = 0;
        KWPage last = m_pageManager.last();
        if (last.isValid())
            afterPageNum = last.pageNumber();
        KWPageInsertCommand cmd(this, afterPageNum, masterPageName);
        cmd.redo();
        KWPage newPage = cmd.page();
        Q_ASSERT(newPage.isValid());
        newPage.setAutoGenerated(true);
        if (m_magicCurtain)
            m_magicCurtain->revealFramesForPage(newPage.pageNumber(), newPage.offsetInDocument());
    }
}

void KWDocument::updateHeaderFooter(KWTextFrameSet *tfs)
{
    // find all pages that have the page style set and re-layout them.
    Q_ASSERT(tfs->pageStyle().isValid());
    updatePagesForStyle(tfs->pageStyle(), true);
}

void KWDocument::updatePagesForStyle(const KWPageStyle &style, bool delayed)
{
    PageProcessingQueue *ppq = 0;
    foreach (const KWPage &page, pageManager()->pages()) {
        if (page.pageStyle() == style) {
            if (ppq == 0)
                ppq = new PageProcessingQueue(this);
            ppq->addPage(page);
        }
    }
    if (!delayed && ppq)
        ppq->process();
}

void KWDocument::showStartUpWidget(KoMainWindow *parent, bool alwaysShow)
{
    // print error if kotext not available
    if (KShapeRegistry::instance()->value(TextShape_SHAPEID) == 0)
        // need to wait 1 event since exiting here would not work.
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
    else
        KoDocument::showStartUpWidget(parent, alwaysShow);
}

void KWDocument::showErrorAndDie()
{
    KMessageBox::error(widget(),
                       i18n("Can not find needed text component, KWord will quit now"),
                       i18n("Installation Error"));
    QCoreApplication::exit(10);
}

void KWDocument::removeFrameFromViews(KWFrame *frame)
{
    Q_ASSERT(frame);
    foreach (KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        canvas->shapeManager()->remove(frame->shape());
    }
}

#ifndef NDEBUG
void KWDocument::printDebug()
{
    static const char *headerFooterType[] = { "None", "EvenOdd", "Uniform", "ERROR" };
    static const char *fstype[] = { "BackgroundFrameSet", "TextFrameSet", "OtherFrameSet", "ERROR" };

    kDebug(32001) << "----------------------------------------";
    kDebug(32001) << "                 Debug info";
    kDebug(32001) << "Document:" << this;
    /*kDebug(32001) <<"Type of document:" << (m_pageStyle.hasMainTextFrame()?"WP":"DTP"); */
    kDebug(32001) << "Units:" << KUnit::unitName(unit());
    kDebug(32001) << "# Framesets:" << frameSetCount();
    int i = 0;
    foreach (KWFrameSet *fs, m_frameSets) {
        kDebug(32001) << "Frameset" << i++ << fstype[fs->type()] << ":" <<
        fs->name() << '(' << fs << ')';
        fs->printDebug();
    }

    kDebug(32001) << "PageManager holds" << pageCount() << " pages";
    KWPage page = m_pageManager.begin();
    while (page.isValid()) {
        int pgnum = page.pageNumber();
        QString side = "[Left] ";
        QString num = QString::number(pgnum);
        if (page.pageSide() == KWPage::Right)
            side = "[Right]";
        else if (page.pageSide() == KWPage::PageSpread) {
            side = "[PageSpread]";
            pgnum++;
            num += '-' + QString::number(pgnum);
        }
        kDebug(32001) << "Page" << num << side << " width:" << page.width() << " height:" << page.height() << "following" << page.pageStyle().name();
        if (page.isAutoGenerated())
            kDebug(32001) << "     Auto-generated page (inserted for text layout)";
        page = page.next();
    }

    foreach (const KWPageStyle &style, m_pageManager.pageStyles()) {
        kDebug(32001) << "PageStyle" << style.name();
        kDebug(32001) << "     Header:" << headerFooterType[style.headerPolicy()];
        kDebug(32001) << "     Footer:" << headerFooterType[style.footerPolicy()];
        KOdfColumnData columns = style.columns();
        if (columns.columns != 1)
            kDebug(32001) << " +-- Columns:" << columns.columns << columns.columnSpacing << "pt spacing";
        KOdfPageLayoutData layout = style.pageLayout();
        kDebug(32001) << "     PageSize: " << layout.width << "x" << layout.height;
        kDebug(32001) << "     Indents: (tlbr, edge, binding)"
                      << layout.topMargin << layout.leftMargin
                      << layout.bottomMargin << layout.rightMargin
                      << layout.pageEdge << layout.bindingSide;
    }

    kDebug(32001) << "  The height of the doc (in pt) is:" << pageManager()->bottomOfPage(pageManager()->pageCount() - 1);
}
#endif

QList<KoDocument::CustomDocumentWidgetItem> KWDocument::createCustomDocumentWidgets(QWidget *parent)
{
    KOdfColumnData columns;
    columns.columns = 1;
    columns.columnSpacing = m_config.defaultColumnSpacing();

    QList<KoDocument::CustomDocumentWidgetItem> widgetList;
    KoDocument::CustomDocumentWidgetItem item;
    item.widget = new KWStartupWidget(parent, this, columns);
    widgetList << item;
    return widgetList;
}

void KWDocument::saveConfig()
{
    if (!isReadWrite())
        return;
//   KConfigGroup group(KoGlobal::kofficeConfig(), "Spelling");
//   group.writeEntry("PersonalDict", m_spellCheckPersonalDict);

    if (isEmbedded())
        return;
    m_config.save();
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup interface = config->group("Interface");
    interface.writeEntry("ResolutionX", gridData().gridX());
    interface.writeEntry("ResolutionY", gridData().gridY());
}

void KWDocument::addCommand(QUndoCommand *command)
{
    m_commandBeingAdded = command;
    KoDocument::addCommand(command);
    m_commandBeingAdded = 0;
}

// ************* PageProcessingQueue ************
PageProcessingQueue::PageProcessingQueue(KWDocument *parent)
    : QObject(parent)
{
    m_document = parent;
    m_triggered = false;
}

void PageProcessingQueue::addPage(const KWPage &page)
{
    m_pages.append(page);
    if (! m_triggered)
        QTimer::singleShot(0, this, SLOT(process()));
    m_triggered = true;
}

void PageProcessingQueue::process()
{
    m_triggered = false;
    const bool docIsEmpty = m_document->isEmpty();
    const bool docIsModified = m_document->isModified();
    foreach (const KWPage &page, m_pages) {
        if (! page.isValid())
            continue;
        m_document->m_frameLayout.createNewFramesForPage(page.pageNumber());
    }
    if (docIsEmpty)
        m_document->setEmpty();
    if (!docIsModified)
        m_document->setModified(false);
    m_pages.clear();
    deleteLater();
    emit m_document->pageSetupChanged();
}
