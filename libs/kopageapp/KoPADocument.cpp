/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>

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

#include "KoPADocument.h"

#include <KOdfStore.h>
#include <KXmlWriter.h>
#include <KOdfStoreReader.h>
#include <KOdfWriteStore.h>
#include <KOdfLoadingContext.h>
#include <KOdfSettings.h>
#include <KOdfStorageDevice.h>
#include <KTextShapeData.h>
#include <KTextSharedLoadingData.h>
#include <KTextDocumentLayout.h>
#include <KInlineTextObjectManager.h>
#include <KStyleManager.h>
#include <KOdfXmlNS.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KToolRegistry.h>
#include "tools/backgroundTool/KoPABackgroundToolFactory.h"

#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAViewMode.h"
#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"
#include "commands/KoPAPageDeleteCommand.h"

#include <kdebug.h>
#include <kconfiggroup.h>

#include <QPainter>

class KoPADocument::Private
{
public:
    QList<KoPAPage*> pages;
    QList<KoPAPage*> masterPages;
    KInlineTextObjectManager *inlineTextObjectManager;
    bool rulersVisible;
};

KoPADocument::KoPADocument(QWidget* parentWidget, QObject* parent, bool singleViewMode)
: KoDocument(parentWidget, parent, singleViewMode),
    d(new Private())
{
    d->inlineTextObjectManager = new KInlineTextObjectManager(this);
    d->rulersVisible = false;

    resourceManager()->setUndoStack(undoStack());
    resourceManager()->setOdfDocument(this);
    QVariant variant2;
    variant2.setValue<KInlineTextObjectManager*>(d->inlineTextObjectManager);
    resourceManager()->setResource(KOdfText::InlineTextObjectManager, variant2);
    loadConfig();

    if (!KToolRegistry::instance()->contains("KoPABackgroundTool")) {
        KoPABackgroundToolFactory *f = new KoPABackgroundToolFactory(KToolRegistry::instance());
        KToolRegistry::instance()->add(f);
    }
}

KoPADocument::~KoPADocument()
{
    saveConfig();
    qDeleteAll(d->pages);
    qDeleteAll(d->masterPages);
    delete d;
}

void KoPADocument::paintContent(QPainter &painter, const QRect &rect)
{
    KoPAPage * page = pageByIndex(0, false);
    Q_ASSERT(page);
    QPixmap thumbnail(pageThumbnail(page, rect.size()));
    painter.drawPixmap(rect, thumbnail);
}

bool KoPADocument::loadXML(const KXmlDocument &doc, KOdfStore *)
{
    Q_UNUSED(doc);

    //Perhaps not necessary if we use filter import/export for old file format
    //only needed as it is in the base class will be removed.
    return true;
}

bool KoPADocument::loadOdf(KOdfStoreReader &odfStore)
{
    QPointer<KoUpdater> updater;
    if (progressUpdater()) {
        updater = progressUpdater()->startSubtask(1, "KoPADocument::loadOdf");
        updater->setProgress(0);
    }
    KOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store(), componentData());
    KoPALoadingContext paContext(loadingContext, resourceManager());

    KXmlElement content = odfStore.contentDoc().documentElement();
    KXmlElement realBody (KoXml::namedItemNS(content, KOdfXmlNS::office, "body"));

    if (realBody.isNull()) {
        kError(30010) << "No body tag found!" << endl;
        return false;
    }

    KXmlElement body = KoXml::namedItemNS(realBody, KOdfXmlNS::office, odfTagName(false));

    if (body.isNull()) {
        kError(30010) << "No office:" << odfTagName(false) << " tag found!" << endl;
        return false;
    }

    // Load text styles before the corresponding text shapes try to use them!
    KTextSharedLoadingData * sharedData = new KTextSharedLoadingData();
    paContext.addSharedData(KODFTEXT_SHARED_LOADING_ID, sharedData);
    KStyleManager *styleManager = resourceManager()->resource(KOdfText::StyleManager).value<KStyleManager*>();

    sharedData->loadOdfStyles(paContext, styleManager);

    d->masterPages = loadOdfMasterPages(odfStore.styles().masterPages(), paContext);
    if (!loadOdfProlog(body, paContext)) {
        return false;
    }
    d->pages = loadOdfPages(body, paContext);

    // create pages if there are none
    if (d->masterPages.empty()) {
        d->masterPages.append(newMasterPage());
    }
    if (d->pages.empty()) {
        d->pages.append(newPage(static_cast<KoPAMasterPage*>(d->masterPages.first())));
    }

    if (!loadOdfEpilogue(body, paContext)) {
        return false;
    }

    loadOdfDocumentStyles(paContext);

    if (d->pages.size() > 1) {
        setActionEnabled(KoPAView::ActionDeletePage, false);
    }

    updatePageCount();

    if (updater) updater->setProgress(100);
    return true;
}

bool KoPADocument::saveOdf(SavingContext &documentContext)
{
    KXmlWriter* contentWriter = documentContext.odfStore.contentWriter();
    if (!contentWriter)
        return false;

    KOdfGenericStyles mainStyles;
    KXmlWriter * bodyWriter = documentContext.odfStore.bodyWriter();

    KoPASavingContext paContext(*bodyWriter, mainStyles, documentContext.embeddedSaver, 1);

    saveOdfDocumentStyles(paContext);

    bodyWriter->startElement("office:body");
    bodyWriter->startElement(odfTagName(true));

    if (!saveOdfProlog(paContext)) {
        return false;
    }

    if (!saveOdfPages(paContext, d->pages, d->masterPages)) {
        return false;
    }

    if (! saveOdfEpilogue(paContext)) {
        return false;
    }

    paContext.writeConnectors();

    bodyWriter->endElement(); // office:odfTagName()
    bodyWriter->endElement(); // office:body

    mainStyles.saveOdfStyles(KOdfGenericStyles::DocumentAutomaticStyles, contentWriter);

    documentContext.odfStore.closeContentWriter();

    //add manifest line for content.xml
    documentContext.odfStore.manifestWriter()->addManifestEntry("content.xml", "text/xml");

    if (! mainStyles.saveOdfStylesDotXml(documentContext.odfStore.store(), documentContext.odfStore.manifestWriter())) {
        return false;
    }

    KOdfStore * store = documentContext.odfStore.store();
    if (! store->open("settings.xml")) {
        return false;
    }

    saveOdfSettings(store);

    if (! store->close()) {
        return false;
    }

    documentContext.odfStore.manifestWriter()->addManifestEntry("settings.xml", "text/xml");

    //setModified(false);

    return paContext.saveDataCenter(documentContext.odfStore.store(), documentContext.odfStore.manifestWriter());
}

QList<KoPAPage *> KoPADocument::loadOdfMasterPages(const QHash<QString, KXmlElement*> masterStyles, KoPALoadingContext &context)
{
    context.odfLoadingContext().setUseStylesAutoStyles(true);
    QList<KoPAPage *> masterPages;

    QHash<QString, KXmlElement*>::const_iterator it(masterStyles.constBegin());
    for (; it != masterStyles.constEnd(); ++it)
    {
        kDebug(30010) << "Master:" << it.key();
        KoPAMasterPage * masterPage = newMasterPage();
        masterPage->loadOdf(*(it.value()), context);
        masterPages.append(masterPage);
        context.addMasterPage(it.key(), masterPage);
    }
    context.odfLoadingContext().setUseStylesAutoStyles(false);
    return masterPages;
}

QList<KoPAPage *> KoPADocument::loadOdfPages(const KXmlElement &body, KoPALoadingContext &context)
{
    if (d->masterPages.isEmpty()) { // we require at least one master page. Auto create one if the doc was faulty.
        d->masterPages << newMasterPage();
    }

    QList<KoPAPage *> pages;
    KXmlElement element;
    forEachElement(element, body) {
        if (element.tagName() == "page" && element.namespaceURI() == KOdfXmlNS::draw) {
            KoPAPage *page = newPage(static_cast<KoPAMasterPage*>(d->masterPages.first()));
            page->loadOdf(element, context);
            pages.append(page);
        }
    }
    return pages;
}

bool KoPADocument::loadOdfEpilogue(const KXmlElement &body, KoPALoadingContext &context)
{
    Q_UNUSED(body);
    Q_UNUSED(context);
    return true;
}

bool KoPADocument::loadOdfProlog(const KXmlElement &body, KoPALoadingContext &context)
{
    Q_UNUSED(body);
    Q_UNUSED(context);
    return true;
}

bool KoPADocument::saveOdfPages(KoPASavingContext &paContext, QList<KoPAPage *> &pages, QList<KoPAPage *> &masterPages)
{
    paContext.addOption(KoPASavingContext::DrawId);
    paContext.addOption(KoPASavingContext::AutoStyleInStyleXml);

    // save master pages
    foreach(KoPAPage *page, masterPages) {
        if (paContext.isSetClearDrawIds()) {
            paContext.clearDrawIds();
        }
        page->saveOdf(paContext);
    }

    paContext.removeOption(KoPASavingContext::AutoStyleInStyleXml);

    // save pages
    foreach (KoPAPage *page, pages) {
        page->saveOdf(paContext);
        paContext.incrementPage();
    }

    return true;
}

bool KoPADocument::saveOdfProlog(KoPASavingContext &paContext)
{
    Q_UNUSED(paContext);
    KVariableManager* variable_manager = inlineTextObjectManager()->variableManager();
    KXmlWriter& writer = paContext.xmlWriter();
    variable_manager->saveOdf(&writer);
    return true;
}

bool KoPADocument::saveOdfEpilogue(KoPASavingContext &paContext)
{
    Q_UNUSED(paContext);
    return true;
}

bool KoPADocument::saveOdfSettings(KOdfStore * store)
{
    KOdfStorageDevice settingsDev(store);
    KXmlWriter * settingsWriter = KOdfWriteStore::createOasisXmlWriter(&settingsDev, "office:document-settings");

    // add this so that OOo reads guides lines and grid data from ooo:view-settings
    settingsWriter->addAttribute("xmlns:ooo", "http://openoffice.org/2004/office");

    settingsWriter->startElement("office:settings");
    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "view-settings");

    saveUnitOdf(settingsWriter);

    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "ooo:view-settings");
    settingsWriter->startElement("config:config-item-map-indexed");
    settingsWriter->addAttribute("config:name", "Views");
    settingsWriter->startElement("config:config-item-map-entry");

    guidesData().saveOdfSettings(*settingsWriter);
    gridData().saveOdfSettings(*settingsWriter);

    settingsWriter->endElement(); // config:config-item-map-entry
    settingsWriter->endElement(); // config:config-item-map-indexed
    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->endElement(); // office:settings
    settingsWriter->endElement(); // office:document-settings

    settingsWriter->endDocument();

    delete settingsWriter;

    return true;
}

void KoPADocument::loadOdfSettings( const KXmlDocument &settingsDoc)
{
    if (settingsDoc.isNull()) {
        return ; // not an error if some file doesn't have settings.xml
    }

    KOdfSettings settings(settingsDoc);
    KOdfSettings::Items viewSettings = settings.itemSet("view-settings");
    if (!viewSettings.isNull()) {
        setUnit(KUnit::unit(viewSettings.parseConfigItemString("unit")));
        // FIXME: add other config here.
    }

    guidesData().loadOdfSettings(settingsDoc);
    gridData().loadOdfSettings(settingsDoc);
}

void KoPADocument::saveOdfDocumentStyles(KoPASavingContext &context)
{
    KStyleManager *styleManager = resourceManager()->resource(KOdfText::StyleManager).value<KStyleManager*>();
    Q_ASSERT(styleManager);
    styleManager->saveOdf(context.mainStyles());
}

bool KoPADocument::loadOdfDocumentStyles(KoPALoadingContext &context)
{
    Q_UNUSED(context);
    return true;
}

KoPAPage* KoPADocument::pageByIndex(int index, bool masterPage) const
{
    if (masterPage)
    {
        return d->masterPages.at(index);
    }
    else
    {
        return d->pages.at(index);
    }
}

int KoPADocument::pageIndex(KoPAPage *page) const
{
    const QList<KoPAPage*> &pages = dynamic_cast<KoPAMasterPage *>(page) ? d->masterPages : d->pages;
    return pages.indexOf(page);
}

KoPAPage* KoPADocument::pageByNavigation(KoPAPage * currentPage, KoPageApp::PageNavigation pageNavigation) const
{
    const QList<KoPAPage*> &pages = dynamic_cast<KoPAMasterPage *>(currentPage) ? d->masterPages : d->pages;

    Q_ASSERT(! pages.isEmpty());

    KoPAPage * newPage = currentPage;

    switch (pageNavigation)
    {
        case KoPageApp::PageFirst:
            newPage = pages.first();
            break;
        case KoPageApp::PageLast:
            newPage = pages.last();
            break;
        case KoPageApp::PagePrevious:
        {
            int index = pages.indexOf(currentPage) - 1;
            if (index >= 0)
            {
                newPage = pages.at(index);
            }
        }   break;
        case KoPageApp::PageNext:
            // fall through
        default:
        {
            int index = pages.indexOf(currentPage) + 1;
            if (index < pages.size())
            {
                newPage = pages.at(index);
            }
            break;
        }
    }

    return newPage;
}

void KoPADocument::addShape(KShape * shape)
{
    Q_ASSERT(shape);
    // the KShapeController sets the active layer as parent
    KoPAPage * page(pageByShape(shape));

    foreach(KoView *view, views()) {
        KoPAView *kopaView = static_cast<KoPAView*>(view);
        kopaView->viewMode()->addShape(shape);
    }

    emit shapeAdded(shape);

    // it can happen in showcase notes view that there is no page
    if (page)
        page->shapeAdded(shape);
    postAddShape(shape);
}

void KoPADocument::postAddShape(KShape * shape)
{
    Q_UNUSED(shape);
}

void KoPADocument::removeShape(KShape *shape)
{
    if (!shape)
        return;

    KoPAPage * page(pageByShape(shape));

    foreach(KoView *view, views())
    {
        KoPAView * kopaView = static_cast<KoPAView*>(view);
        kopaView->viewMode()->removeShape(shape);
    }

    emit shapeRemoved(shape);

    page->shapeRemoved(shape);
    postRemoveShape(page, shape);
}

void KoPADocument::postRemoveShape(KoPAPage * page, KShape * shape)
{
    Q_UNUSED(page);
    Q_UNUSED(shape);
}

void KoPADocument::removePage(KoPAPage * page)
{
    KoPAPageDeleteCommand * command = new KoPAPageDeleteCommand(this, page);
    pageRemoved(page, command);
    addCommand(command);
}

void KoPADocument::pageRemoved(KoPAPage * page, QUndoCommand * parent)
{
    Q_UNUSED(page);
    Q_UNUSED(parent);
}

KoPAPage * KoPADocument::pageByShape(KShape * shape) const
{
    KShape * parent = shape;
    KoPAPage * page = 0;
    while (!page && (parent = parent->parent()))
    {
        page = dynamic_cast<KoPAPage*>(parent);
    }
    return page;
}

void KoPADocument::updateViews(KoPAPage *page)
{
    if (!page) return;

    foreach (KoView *view, views()) {
        KoPAView *paView = static_cast<KoPAView *>(view);
        if (paView->activePage() == page) {
            paView->viewMode()->updateActivePage(page);
        }
        else if (dynamic_cast<KoPAMasterPage *>(page)) {
            // if the page changed is a master page, we need to check whether it is the current page's master page
            KoPAPage *activePage = dynamic_cast<KoPAPage *>(paView->activePage());
            if (activePage && activePage->masterPage() == page) {
                paView->viewMode()->updateActivePage(activePage);
            }
        }
    }
}

KoPageApp::PageType KoPADocument::pageType() const
{
    return KoPageApp::Page;
}

QPixmap KoPADocument::pageThumbnail(KoPAPage* page, const QSize &size)
{
    return page->thumbnail(size);
}

void KoPADocument::initEmpty()
{
    d->masterPages.clear();
    d->pages.clear();
    KoPAMasterPage * masterPage = newMasterPage();
    d->masterPages.append(masterPage);
    KoPAPage * page = newPage(masterPage);
    d->pages.append(page);
    KoDocument::initEmpty();
}

void KoPADocument::setActionEnabled(int actions, bool enable)
{
    foreach(KoView *view, views())
    {
        KoPAView * kopaView = static_cast<KoPAView*>(view);
        kopaView->setActionEnabled(actions, enable);
    }
}

void KoPADocument::insertPage(KoPAPage* page, int index)
{
    if (!page)
        return;

    QList<KoPAPage*> &pages = dynamic_cast<KoPAMasterPage *>(page) ? d->masterPages : d->pages;

    if (index > pages.size() || index < 0)
    {
        index = pages.size();
    }

    pages.insert(index, page);
    updatePageCount();

    setActionEnabled(KoPAView::ActionDeletePage, pages.size() > 1);

    emit pageAdded(page);
}

void KoPADocument::insertPage(KoPAPage* page, KoPAPage* after)
{
    if (!page)
        return;

    QList<KoPAPage*> &pages = dynamic_cast<KoPAMasterPage *>(page) ? d->masterPages : d->pages;

    int index = 0;

    if (after != 0)
    {
        index = pages.indexOf(after) + 1;

        // Append the page if after wasn't found in pages
        if (index == 0)
            index = pages.count();
    }

    pages.insert(index, page);
    updatePageCount();

    setActionEnabled(KoPAView::ActionDeletePage, pages.size() > 1);

    emit pageAdded(page);
}

int KoPADocument::takePage(KoPAPage *page)
{
    Q_ASSERT(page);

    QList<KoPAPage *> &pages = dynamic_cast<KoPAMasterPage *>(page) ? d->masterPages : d->pages;

    int index = pages.indexOf(page);

    // it should not be possible to delete the last page
    Q_ASSERT(pages.size() > 1);

    if (index != -1) {
        pages.removeAt(index);

        // change to previous page when the page is the active one if the first one is delete go to the next one
        int newIndex = index == 0 ? 0 : index - 1;
        KoPAPage * newActivePage = pages.at(newIndex);
        foreach(KoView *view, views())
        {
            KoPAView * kopaView = static_cast<KoPAView*>(view);
            if (page == kopaView->activePage()) {
                kopaView->viewMode()->updateActivePage(newActivePage);
            }
        }
        updatePageCount();
    }

    if (pages.size() == 1) {
        setActionEnabled(KoPAView::ActionDeletePage, false);
    }

    emit pageRemoved(page);

    return index;
}

QList<KoPAPage*> KoPADocument::pages(bool masterPages) const
{
    return masterPages ? d->masterPages : d->pages;
}

KoPAPage *KoPADocument::newPage(KoPAMasterPage *masterPage)
{
    KoPAPage *page = new KoPAPage(this);
    page->setMasterPage(masterPage);
    return page;
}

KoPAMasterPage * KoPADocument::newMasterPage()
{
    return new KoPAMasterPage(this);
}

/// return the inlineTextObjectManager for this document.
KInlineTextObjectManager *KoPADocument::inlineTextObjectManager() const {
    return d->inlineTextObjectManager;
}

void KoPADocument::loadConfig()
{
    KSharedConfigPtr config = componentData().config();

    if (config->hasGroup("Grid"))
    {
        KoGridData defGrid;
        KConfigGroup configGroup = config->group("Grid");
        bool showGrid = configGroup.readEntry<bool>("ShowGrid", defGrid.showGrid());
        gridData().setShowGrid(showGrid);
        bool snapToGrid = configGroup.readEntry<bool>("SnapToGrid", defGrid.snapToGrid());
        gridData().setSnapToGrid(snapToGrid);
        qreal spacingX = configGroup.readEntry<qreal>("SpacingX", defGrid.gridX());
        qreal spacingY = configGroup.readEntry<qreal>("SpacingY", defGrid.gridY());
        gridData().setGrid(spacingX, spacingY);
        QColor color = configGroup.readEntry("Color", defGrid.gridColor());
        gridData().setGridColor(color);
    }

    if (config->hasGroup("Interface"))
    {
        KConfigGroup configGroup = config->group("Interface");
        bool showRulers = configGroup.readEntry<bool>("ShowRulers", true);
        setRulersVisible(showRulers);
    }
}

void KoPADocument::saveConfig()
{
    KSharedConfigPtr config = componentData().config();
    KConfigGroup configGroup = config->group("Grid");
    KoGridData defGrid;

    bool showGrid = gridData().showGrid();
    if ((showGrid == defGrid.showGrid()) && !configGroup.hasDefault("ShowGrid"))
        configGroup.revertToDefault("ShowGrid");
    else
        configGroup.writeEntry("ShowGrid", showGrid);

    bool snapToGrid = gridData().snapToGrid();
    if ((snapToGrid == defGrid.snapToGrid()) && !configGroup.hasDefault("SnapToGrid"))
        configGroup.revertToDefault("SnapToGrid");
    else
        configGroup.writeEntry("SnapToGrid", snapToGrid);

    qreal spacingX = gridData().gridX();
    if ((spacingX == defGrid.gridX()) && !configGroup.hasDefault("SpacingX"))
        configGroup.revertToDefault("SpacingX");
    else
        configGroup.writeEntry("SpacingX", spacingX);

    qreal spacingY = gridData().gridY();
    if ((spacingY == defGrid.gridY()) && !configGroup.hasDefault("SpacingY"))
        configGroup.revertToDefault("SpacingY");
    else
        configGroup.writeEntry("SpacingY", spacingY);

    QColor color = gridData().gridColor();
    if ((color == defGrid.gridColor()) && !configGroup.hasDefault("Color"))
        configGroup.revertToDefault("Color");
    else
        configGroup.writeEntry("Color", color);

    configGroup = config->group("Interface");

    bool showRulers = rulersVisible();
    if ((showRulers == true) && !configGroup.hasDefault("ShowRulers"))
        configGroup.revertToDefault("ShowRulers");
    else
        configGroup.writeEntry("ShowRulers", showRulers);
}

void KoPADocument::setRulersVisible(bool visible)
{
    d->rulersVisible = visible;
}

bool KoPADocument::rulersVisible() const
{
    return d->rulersVisible;
}

int KoPADocument::pageCount() const
{
    return d->pages.count();
}

void KoPADocument::updatePageCount()
{
    if (resourceManager()->hasResource(KOdfText::InlineTextObjectManager)) {
        QVariant var = resourceManager()->resource(KOdfText::InlineTextObjectManager);
        KInlineTextObjectManager *om = var.value<KInlineTextObjectManager*>();
        om->setProperty(KInlineObject::PageCount, pageCount());
    }
}

#include <KoPADocument.moc>
