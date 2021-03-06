/* This file is part of the KDE project
   Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>

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

#ifndef KODFGENERICCHANGE_H
#define KODFGENERICCHANGE_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include "flake_export.h"

#include <QtCore/QByteArray>

#include <kdebug.h>

class KOdfGenericChanges;
class KXmlWriter;

/**
 * A generic change, i.e. basically a collection of properties and a name.
 * Instances of KOdfGenericChange can either be held in the KOdfGenericChanges collection,
 * or created (e.g. on the stack) and given to KOdfGenericChanges::insert.
 *
 * Derived from code from KOdfGenericStyle
 */
class FLAKE_EXPORT KOdfGenericChange
{
public:
    /**
     * Possible values for the "type" of the KOdfGenericChange.
     * If there is a still missing add it here so that it is possible to use the same
     * saving code in all applications.
     */
    enum Type {
        InsertChange,
        FormatChange,
        DeleteChange
    };

    enum ChangeFormat {
        ODF_1_2,
        DELTAXML
    };

    /**
     * Start the definition of a new change. Its name will be set later by KOdfGenericChanges::insert(),
     * but first you must define its properties and attributes.
     *
     */
    explicit KOdfGenericChange(KOdfGenericChange::ChangeFormat changeFormat = KOdfGenericChange::ODF_1_2);
    ~KOdfGenericChange();

    /// Set the type of this change
    void setType(KOdfGenericChange::Type type) {
        m_type = type;
    }

    /// set the format to be used to save changes
    void setChangeFormat(KOdfGenericChange::ChangeFormat changeFormat) {
        m_changeFormat = changeFormat;
    }

    /// Return the type of this style
    Type type() const {
        return m_type;
    }

    /// Return the format to be used to save changes
    KOdfGenericChange::ChangeFormat changeFormat() const {
        return m_changeFormat;
    }

    /// Add a property to the style
    void addChangeMetaData(const QString &propName, const QString &propValue) {
        m_changeMetaData.insert(propName, propValue);
    }

    /// Overloaded version of addProperty that takes a char*, usually for "..."
    void addChangeMetaData(const QString &propName, const char *propValue) {
        m_changeMetaData.insert(propName, propValue);
    }
    /// Overloaded version of addProperty that converts an int to a string
    void addChangeMetaData(const QString &propName, int propValue) {
        m_changeMetaData.insert(propName, QString::number(propValue));
    }
    /// Overloaded version of addProperty that converts a bool to a string (false/true)
    void addChangeMetaData(const QString &propName, bool propValue) {
        m_changeMetaData.insert(propName, propValue ? "true" : "false");
    }

    /**
     * @brief Add a child element to the properties.
     *
     * What is meant here is that the contents of the QString
     * will be written out literally. This means you should use
     * KXmlWriter to generate it:
     * @code
     * QBuffer buffer;
     * buffer.open( QIODevice::WriteOnly );
     * KXmlWriter elementWriter( &buffer );  // TODO pass indentation level
     * elementWriter.startElement( "..." );
     * ...
     * elementWriter.endElement();
     * QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
     * gs.addChildElement( "...", elementContents );
     * @endcode
     *
     * The value of @p elementName isn't used, except that it must be unique.
     */
    void addChildElement(const QString &elementName, const QString &elementContents) {
        m_literalData.insert(elementName, elementContents);
    }

    /**
     *  Write the definition of this change to @p writer, using the OASIS format.
     *  @param writer the KXmlWriter in which the element will be created and filled in
     *  @param name must come from the collection.
     */
    void writeChange(KXmlWriter *writer, const QString &name) const;

    /**
     *  QMap requires a complete sorting order.
     *  Another solution would have been a qdict and a key() here, a la KoTextFormat,
     *  but the key was difficult to generate.
     *  Solutions with only a hash value (not representative of the whole data)
     *  require us to write a hashtable by hand....
     */
    bool operator<(const KOdfGenericChange &other) const;

    /// Not needed for QMap, but can still be useful
    bool operator==(const KOdfGenericChange &other) const;

private:
    QString changeMetaData(const QString &propName) const {
        QMap<QString, QString>::const_iterator it = m_changeMetaData.find(propName);
        if (it != m_changeMetaData.end())
            return it.value();
        return QString();
    }

    void writeChangeMetaData(KXmlWriter *writer) const;

    void writeODF12Change(KXmlWriter *writer, const QString &name) const;

    void writeDeltaXmlChange(KXmlWriter *writer, const QString &name) const;

private:
    // Note that the copy constructor and assignment operator are allowed.
    // Better not use pointers below!
    Type m_type;
    ChangeFormat m_changeFormat;
    /// We use QMaps since they provide automatic sorting on the key (important for unicity!)
    QMap<QString, QString> m_changeMetaData;
    QMap<QString, QString> m_literalData;
};

#endif /* KODFGENERICCHANGE_H */
