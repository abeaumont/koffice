/* This file is part of the KDE project
   Copyright (C) 2001-2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2001-2005,2007 Rob Buis <buis@kde.org>
   Copyright (C) 2002,2004-2005 Laurent Montel <montel@kde.org>
   Copyright (C) 2002 Benoit Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2004-2005,2007 David Faure <faure@kde.org>
   Copyright (C) 2004,2006 Peter Simonsson <psn@linux.se>
   Copyright (C) 2004-2005 Fredrik Edemar <f_edemar@linux.se>
   Copyright (C) 2005-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006-2007 Thorsten Zachmann <t.zachmann@zagge.de>

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

#ifndef ARTWORK_PART_H
#define ARTWORK_PART_H

#include <QMap>

#include <KShapeControllerBase.h>

#include <KoDocument.h>
#include <KUnit.h>
#include <ArtworkDocument.h>

#include <artworkui_export.h>

class QRect;
class KDataCenterBase;

/**
 * Keeps track of visual per document properties.
 * It loads initial settings and applies them to the document and its views.
 */
class ARTWORKUI_EXPORT ArtworkPart : public KoDocument, public KShapeControllerBase
{
    Q_OBJECT
public:
    explicit ArtworkPart(QWidget* parentWidget = 0L, const char* widgetName = 0L,
                        QObject* parent = 0L, const char* name = 0L, bool singleViewMode = false);
    virtual ~ArtworkPart();

    /// reimplemented form KoDocument
    virtual void paintContent(QPainter& painter, const QRect& rect);
    /// reimplemented form KoDocument
    virtual bool loadXML(const KXmlDocument& document, KOdfStore *store);
    /// reimplemented form KoDocument
    virtual bool loadOdf(KOdfStoreReader & odfStore);
    /// reimplemented form KoDocument
    virtual bool completeLoading(KOdfStore* store);
    /// reimplemented form KoDocument
    virtual bool saveOdf(SavingContext &documentContext);

    /// implemented from KShapeController
    virtual void addShape(KShape* shape);
    /// implemented from KShapeController
    virtual void removeShape(KShape* shape);
    /// implemented from KShapeController
    virtual QMap<QString, KDataCenterBase*> dataCenterMap() const;

    /// Gives access to document content
    ArtworkDocument& document();

    /// Returns if status bar is shown
    bool showStatusBar() const;
    /// Shows/hides status bar
    void setShowStatusBar(bool b);
    /// update attached view(s) on the current doc settings
    /// at this time only the status bar is handled
    void reorganizeGUI();

    /// Returns maximum number of recent files
    uint maxRecentFiles() const;

    /// Sets page layout of the document
    virtual void setPageLayout(const KOdfPageLayoutData& layout);

    bool mergeNativeFormat(const QString & file);

public slots:
    void slotDocumentRestored();

protected:
    /// reimplemented form KoDocument
    virtual KoView* createViewInstance(QWidget* parent);
    /// reimplemented form KoDocument
    virtual void removeView(KoView *view);

    /// Loads settings like grid and guide lines from given xml document
    void loadOasisSettings(const KXmlDocument & settingsDoc);
    /// Saves settings like grid and guide lines to store
    void saveOasisSettings(KOdfStore * store);

    /// Sets given page size to all attached views/canvases
    void setPageSize(const QSizeF &pageSize);

    /// Reads settings from config file
    void initConfig();

protected slots:
    /// reimplemented from KoDocument
    virtual void openTemplate(const KUrl& url);

private:
    class Private;
    Private * const d;
};

#endif // ARTWORK_PART_H

