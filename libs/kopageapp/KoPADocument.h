/* This file is part of the KDE project
   Copyright (C) 2006-2010 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPADOCUMENT_H
#define KOPADOCUMENT_H

#include <QtCore/QObject>

#include <KOdf.h>			//krazy:exclude=includes
#include <KoDocument.h>			//krazy:exclude=includes
#include <KShapeControllerBase.h>	//krazy:exclude=includes
#include "KoPageApp.h"			//krazy:exclude=includes
#include "kopageapp_export.h"

class KShapeSavingContext;
class KoPAPage;
class KoPAPage;
class KoPAMasterPage;
class KoPALoadingContext;
class KoPASavingContext;
class KXmlWriter;

class KInlineTextObjectManager;

/// Document class that stores KoPAPage and KoPAMasterPage objects
class KOPAGEAPP_EXPORT KoPADocument : public KoDocument, public KShapeControllerBase
{
    Q_OBJECT
public:

    explicit KoPADocument(QWidget* parentWidget, QObject* parent, bool singleViewMode = false);
    virtual ~KoPADocument();

    void paintContent(QPainter &painter, const QRect &rect);

    bool loadXML(const KXmlDocument &doc, KOdfStore *store);
    bool loadOdf(KOdfStoreReader &odfStore);

    bool saveOdf(SavingContext &documentContext);

    /**
     * The tag the body is saved in
     */
    virtual const char *odfTagName(bool withNamespace) = 0;

    /**
     * Load master pages
     *
     * @param masterStyles
     * @param context
     */
    QList<KoPAPage *> loadOdfMasterPages(const QHash<QString, KXmlElement*> masterStyles, KoPALoadingContext &context);

    /**
     * Save pages
     *
     * This is used by saveOdf and for copy and paste of pages.
     *
     * For all pages that are specified also the master slide has to be specified.
     */
    bool saveOdfPages(KoPASavingContext &paContext, QList<KoPAPage *> &pages, QList<KoPAPage *> &masterPages);

    /**
     * Save document styles
     */
    virtual void saveOdfDocumentStyles(KoPASavingContext &context);

    /**
     * Load document styles
     */
    virtual bool loadOdfDocumentStyles(KoPALoadingContext &context);

    /**
     * Get page by index.
     *
     * @param index of the page
     * @param masterPage if true return a masterPage, if false a normal page
     */
    KoPAPage* pageByIndex(int index, bool masterPage) const;

    /// reimplemnted
    virtual int pageCount() const;

    /**
     * Get the index of the page
     *
     * @param page The page you want to get the index for
     *
     * @return The index of the page or -1 if the page is not found
     */
    int pageIndex(KoPAPage *page) const;

    /**
     * Get page by navigation
     *
     * @param currentPage the current page
     * @param pageNavigation how to navigate from the current page
     *
     * @return the page which is reached by pageNavigation
     */
    KoPAPage* pageByNavigation(KoPAPage * currentPage, KoPageApp::PageNavigation pageNavigation) const;

    /**
     * Insert page to the document at index
     *
     * The function checks if it is a normal or a master page and puts it in
     * the correct list.
     *
     * @param page to insert to document
     * @param index where the page will be inserted.
     */
    void insertPage(KoPAPage* page, int index);

    /**
     * Insert @p page to the document after page @p before
     *
     * The function checks if it is a normal or a master page and puts it in
     * the correct list.
     *
     * @param page to insert to document
     * @param after the page which the inserted page should come after. Set after to 0 to add at the beginning
     */
    void insertPage(KoPAPage* page, KoPAPage* after);

    /**
     * Take @page from the page
     *
     * @param page taken from the document
     * @return the position of the page was taken from the document, or -1 if the page was not found
     */
    int takePage(KoPAPage *page);

    /**
     * Remove the page from the document
     *
     * This generates the command and adds the command that deletes the page
     *
     * @param page The page that gets removed
     */
    virtual void removePage(KoPAPage * page);

    void addShape(KShape *shape);
    void removeShape(KShape* shape);

    QList<KoPAPage*> pages(bool masterPages = false) const;

    /**
     * Get a new page for inserting into the document
     *
     * The page is created with new.
     *
     * Reimplement when you need a derivered class in your kopageapplication
     */
    virtual KoPAPage *newPage(KoPAMasterPage *masterPage);

    /**
     * Get a new master page for inserting into the document
     *
     * The page is created with new.
     *
     * Reimplement when you need a derivered class in your kopageapplication
     */
    virtual KoPAMasterPage * newMasterPage();

    /**
     * Get the type of the document
     */
    virtual KOdf::DocumentType documentType() const = 0;


    /// return the inlineTextObjectManager for this document.
    KInlineTextObjectManager *inlineTextObjectManager() const;

    void setRulersVisible(bool visible);
    bool rulersVisible() const;

    /**
     * Get the page on which the shape is located
     *
     * @param shape The shape for which the page should be found
     * @return The page on which the shape is located
     */
    KoPAPage * pageByShape(KShape * shape) const;

    /**
     * Update all views this document is displayed on
     *
     * @param page specify a page to be updated, all views with this page as active page will be updated.
     */
    void updateViews(KoPAPage *page);

    /**
     * Get the page type used in the document
     *
     * The default page type KoPageApp::Page is returned
     */
    virtual KoPageApp::PageType pageType() const;

    /**
     * Get the thumbnail for the page.
     *
     * Use this method instead the one in the pages directly
     */
    QPixmap pageThumbnail(KoPAPage* page, const QSize &size);

public slots:
    /// reimplemented
    virtual void initEmpty();

signals:
    void shapeAdded(KShape* shape);
    void shapeRemoved(KShape* shape);
    void pageAdded(KoPAPage* page);
    void pageRemoved(KoPAPage* page);

protected:
    virtual KoView *createViewInstance(QWidget *parent) = 0;

    /**
     * Load the presentation declaration
     *
     * The default implementation is empty
     */
    virtual bool loadOdfProlog(const KXmlElement &body, KoPALoadingContext &context);


    /**
     * Load the epilouge
     *
     * The default implementation is empty
     */
    virtual bool loadOdfEpilogue(const KXmlElement &body, KoPALoadingContext &context);

    /**
     * Save the prolog
     *
     * The default implementation is empty
     */
    virtual bool saveOdfProlog(KoPASavingContext &paContext);

    /**
     * Save the epilouge
     *
     * The default implementation is empty
     */
    virtual bool saveOdfEpilogue(KoPASavingContext &paContext);

    /**
     * Save settings
     */
    bool saveOdfSettings(KOdfStore * store);

    /**
     * Load settings
     */
    void loadOdfSettings(const KXmlDocument &settingsDoc);

    /**
     * This function is called by at the end of addShape. This is used
     * e.g. for doing work on the application which is in the KoShapeAppData.
     *
     * The default impementation does nothing
     */
    virtual void postAddShape(KShape *shape);

    /**
     * This function is called by at the end of removeShape. This is used
     * e.g. for doing work on the application which is in the KoShapeAppData.
     *
     * The default impementation does nothing
     */
    virtual void postRemoveShape(KoPAPage * page, KShape * shape);

    /**
     * This function is called with the command that will remove the page
     * given.
     * The default implementation is empty.
     *
     * @param page The page that will be removed
     * @param parent The command that will be used to delete the page
     */
    virtual void pageRemoved(KoPAPage * page, QUndoCommand * parent);

    /**
     * @brief Enables/Disables the given actions in all views
     *
     * The actions are of Type KoPAAction
     *
     * @param actions which should be enabled/disabled
     * @param enable new state of the actions
     */
    void setActionEnabled(int actions, bool enable);

    /// Load the configuration
    void loadConfig();
    /// Save the configuration
    void saveConfig();

    /// set the page count so it gets shown correctly in variables
    void updatePageCount();

private:

    friend class KoPAPastePage;
    /**
     * Load pages
     *
     * @param body
     * @param context
     */
    QList<KoPAPage *> loadOdfPages(const KXmlElement &body, KoPALoadingContext &context);


private:
    class Private;
    Private * const d;
};

#endif /* KOPADOCUMENT_H */
