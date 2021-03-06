/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#ifndef KTEXTSHAREDLOADINGDATA_H
#define KTEXTSHAREDLOADINGDATA_H

#include <KSharedLoadingData.h>

#include <QList>
#include "styles/KListLevelProperties.h"
#include "kodftext_export.h"

class QString;
class KOdfLoadingContext;
class KParagraphStyle;
class KCharacterStyle;
class KListStyle;
class KTableStyle;
class KTableColumnStyle;
class KTableRowStyle;
class KTableCellStyle;
class KSectionStyle;
class KStyleManager;
class KShape;
class KShapeLoadingContext;

#define KODFTEXT_SHARED_LOADING_ID "KoTextSharedLoadingId"

/**
 * This class is used to cache the loaded styles so that they have to be loaded only once
 * and can be used by all text shapes.
 * When a text shape is loaded it checks if the KTextSharedLoadingData is already there.
 * If not it is created.
 */
class KODFTEXT_EXPORT KTextSharedLoadingData : public KSharedLoadingData
{
    friend class KTextLoader;
public:
    KTextSharedLoadingData();
    virtual ~KTextSharedLoadingData();

    /**
     * Load the styles
     *
     * If your application uses a style manager call this function from your application
     * to load the custom styles into the style manager before the rest of the loading is started.
     *
     * @param scontext The shape loading context.
     * @param styleManager The style manager too use or 0 if you don't have a style manager.
     */
    void loadOdfStyles(KShapeLoadingContext &scontext, KStyleManager *styleManager);

    /**
     * Get the paragraph style for the given name
     *
     * The name is the style:name given in the file
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The paragraph style for the given name or 0 if not found
     */
    KParagraphStyle *paragraphStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the character style for the given name
     *
     * The name is the style:name given in the file
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The character style for the given name or 0 if not found
     */
    KCharacterStyle *characterStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Return all character styles.
     *
     * @param stylesDotXml If set the styles from styles.xml are used if unset styles from content.xml are used.
     * @return All character styles from the givin file
     */
    QList<KCharacterStyle*> characterStyles(bool stylesDotXml) const;

    /**
     * Get the list style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The list style for the given name or 0 if not found
     */
    KListStyle *listStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the table style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The table style for the given name or 0 if not found
     */
    KTableStyle *tableStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the table column style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The table column style for the given name or 0 if not found
     */
    KTableColumnStyle *tableColumnStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the table row style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The table row style for the given name or 0 if not found
     */
    KTableRowStyle *tableRowStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the table cell style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The table cell style for the given name or 0 if not found
     */
    KTableCellStyle *tableCellStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the section style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The section style for the given name or 0 if not found
     */
    KSectionStyle *sectionStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Set the appication default style
     *
     * This is done so the application default style needs to be loaded only once.
     * The ownership of the style is transfered to this class.
     */
    void setApplicationDefaultStyle(KCharacterStyle *applicationDefaultStyle);

    /**
     * Get the application default style
     */
    KCharacterStyle *applicationDefaultStyle() const;

protected:
    /**
     * This method got called by kotext once a \a KShape got inserted and an
     * application can implement this to do additional things with shapes once
     * they got inserted.
     * @param shape a shape that has finished loading.
     * @param element the xml element that represents the shape being inserted.
     */
    virtual void shapeInserted(KShape *shape, const KXmlElement &element, KShapeLoadingContext &context);

private:
    enum StyleType {
        ContentDotXml = 1,
        StylesDotXml = 2
    };
    // helper functions for loading of paragraph styles
    void addParagraphStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                            KStyleManager *styleManager = 0);
    QList<QPair<QString, KParagraphStyle *> > loadParagraphStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements,
            int styleTypes, KStyleManager *manager = 0);

    void addDefaultParagraphStyle(KShapeLoadingContext &context, const KXmlElement *styleElem, const KXmlElement *appDefault, KStyleManager *styleManager);

    // helper functions for loading of character styles
    void addCharacterStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                            KStyleManager *styleManager = 0);
    struct OdfCharStyle {
        QString odfName;
        QString parentStyle;
        KCharacterStyle *style;
    };
    QList<OdfCharStyle> loadCharacterStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements, KStyleManager *sm);

    // helper functions for loading of list styles
    void addListStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KStyleManager *styleManager = 0);
    QList<QPair<QString, KListStyle *> > loadListStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements, KStyleManager *styleManager);

    // helper functions for loading of table styles
    void addTableStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KStyleManager *styleManager = 0);
    QList<QPair<QString, KTableStyle *> > loadTableStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    // helper functions for loading of table column styles
    void addTableColumnStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KStyleManager *styleManager = 0);
    QList<QPair<QString, KTableColumnStyle *> > loadTableColumnStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    // helper functions for loading of table row styles
    void addTableRowStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KStyleManager *styleManager = 0);
    QList<QPair<QString, KTableRowStyle *> > loadTableRowStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    // helper functions for loading of table cell styles
    void addTableCellStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KStyleManager *styleManager = 0);
    QList<QPair<QString, KTableCellStyle *> > loadTableCellStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    // helper functions for loading of section styles
    void addSectionStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KStyleManager *styleManager = 0);
    QList<QPair<QString, KSectionStyle *> > loadSectionStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    void addOutlineStyle(KShapeLoadingContext & context, KStyleManager *styleManager);

    class Private;
    Private * const d;
};

#endif /* KTEXTSHAREDLOADINGDATA_H */
