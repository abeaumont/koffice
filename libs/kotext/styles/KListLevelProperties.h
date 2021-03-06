/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Nandita Suri <suri.nandita@gmail.com>
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
#ifndef KLISTLEVELPROPERTIES_H
#define KLISTLEVELPROPERTIES_H

#include "KListStyle.h"

#include <QString>
#include <QChar>

#include <KXmlReader.h>

class KListStyle;
class KShapeLoadingContext;
class KXmlWriter;
class KImageData;

/**
 * Properties per list level.
 */
class KODFTEXT_EXPORT KListLevelProperties
{
public:
    /// Constructor
    explicit KListLevelProperties();
    /// Copy constructor
    KListLevelProperties(const KListLevelProperties &other);
    /// Destructor
    ~KListLevelProperties();

    /// each style has a unique ID (non persistent) given out by the styleManager
    int styleId() const;
    /// each style has a unique ID (non persistent) given out by the styleManager
    void setStyleId(int id);

    /// set the style to be used for this list-level.
    void setStyle(KListStyle::Style style);
    /// return the used style
    KListStyle::Style style() const;
    /// set the string that will be shown before the counter in the list label
    void setListItemPrefix(const QString &prefix);
    /// return the string that will be shown before the counter in the list label
    QString listItemPrefix() const;
    /// set the string that will be shown after the counter in the list label
    void setListItemSuffix(const QString &suffix);
    /// return the string that will be shown after the counter in the list label
    QString listItemSuffix() const;
    /// set the index of the first value of this whole list.
    void setStartValue(int value);
    /// return the index of the first value of this whole list.
    int startValue() const;
    /// set the list level which is how deep the counter is nested below other lists (should be >=1)
    void setLevel(int level);
    /// return the list level which is how deep the counter is nested below other lists
    int level() const;
    /// set the amount of levels that will be shown in list items of this list.
    void setDisplayLevel(int level);
    /// return the amount of levels that will be shown in list items of this list.
    int displayLevel() const;
    /// set the styleId of the KCharacterStyle to be used to layout the listitem
    void setCharacterStyleId(int id);
    /// return the styleId of the KCharacterStyle to be used to layout the listitem
    int characterStyleId() const;
    /// set the character to be used as the counter of the listitem
    void setBulletCharacter(QChar character);
    /// return the character to be used as the counter of the listitem
    QChar bulletCharacter() const;
    /// set the color to be used as the counter of the listitem
    void setBulletColor(QColor color);
    /// return the color to be used as the counter of the listitem
    QColor bulletColor() const;
    /// set the size, in percent, of the bullet counter relative to the fontsize of the counter
    void setRelativeBulletSize(int percent);
    /// return the size, in percent, of the bullet counter relative to the fontsize of the counter
    int relativeBulletSize() const;
    /// set how the list label should be aligned in the width this list reserves for the listitems
    void setAlignment(Qt::Alignment align);
    /// return how the list label should be aligned in the width this list reserves for the listitems
    Qt::Alignment alignment() const;
    /// set the minimum width (in pt) of the list label for all items in this list
    void setMinimumWidth(qreal width);
    /// return the minimum width (in pt) of the list label for all items in this list
    qreal minimumWidth() const;
    /// set the width (in pt) of the image bullet
    void setWidth(qreal width);
    /// return the width (in pt) of the image bullet
    qreal width() const;
    /// set the height (in pt) of the image bullet
    void setHeight(qreal height);
    /// return the height (in pt) of the image bullet
    qreal height() const;
    /// set the bullet image key (as from the KImageData)
    void setBulletImage(KImageData *imageData);
    /// set the listId used by all list-styles that together make 1 user defined list in an ODF file.
    void setListId(KListStyle::ListIdType listId);
    /// return the listId used by all list-styles that together make 1 user defined list in an ODF file.
    KListStyle::ListIdType listId() const;
    /**
     * For alpha-based lists numbers above the 'z' will increase the value of all characters at the same time.
     * If true; we get the sequence 'aa', 'bb', 'cc'. If false; 'aa', 'ab', 'ac'.
     * @return if letterSynchronization should be applied.
     */
    bool letterSynchronization() const;
    /**
     * For alpha-based lists numbers above the 'z' will increase the value of all characters at the same time.
     * If true; we get the sequence 'aa', 'bb', 'cc'. If false; 'aa', 'ab', 'ac'.
     * @param on if letterSynchronization should be applied.
     */
    void setLetterSynchronization(bool on);

    /// set to true to continue numbering from a previous list of the same style
    void setContinueNumbering(bool enable);
    /// returns whether this list continues numbering from a previous list of the same style
    bool continueNumbering() const;

    /// sets the indentation of paragraph
    void setIndent(qreal value);
    /// returns the indentation of paragraphs
    qreal indent() const;

    /// sets the minimum distance between the counter and the text
    void setMinimumDistance(qreal value);
    /// returns the minimum distance between the counter and text
    qreal minimumDistance() const;

    bool operator==(const KListLevelProperties &other) const;
    bool operator!=(const KListLevelProperties &other) const;
    KListLevelProperties & operator=(const KListLevelProperties &other);

    /**
     * Create a KListLevelProperties object from a QTextList instance.
     */
    static KListLevelProperties fromTextList(QTextList *list);

    /**
     * Apply this style to a QTextListFormat by copying all properties from this style
     * to the target list format.
     */
    void applyStyle(QTextListFormat &format) const;

    /**
     * Load the properties from the \p style using the OpenDocument format.
     */
    void loadOdf(KShapeLoadingContext& scontext, const KXmlElement& style);

    /**
     * Save the properties of the style using the OpenDocument format
     */
    void saveOdf(KXmlWriter *writer) const;

private:
    void setProperty(int key, const QVariant &value);
    int propertyInt(int key) const;
    uint propertyUInt(int key) const;
    qulonglong propertyULongLong(int key) const;
    bool propertyBoolean(int key) const;
    qreal propertyDouble(int key) const;
    QString propertyString(int key) const;
    QColor propertyColor(int key) const;

    class Private;
    Private * const d;
};

#endif
