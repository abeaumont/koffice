/* This file is part of the KDE project
*  Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
*
*  Contact: Amit Aggarwal <amitcs06@gmail.com> 
*            <amit.5.aggarwal@nokia.com>
*
*  Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*/
#include "SCDeclarations.h"
#include <QDateTime>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KXmlReader.h>
#include <KoPALoadingContext.h>
#include <KoPASavingContext.h>
#include <KOdfStylesReader.h>
#include <KOdfLoadingContext.h>
#include <KOdf.h>

SCDeclarations::SCDeclarations()
{
}

SCDeclarations::~SCDeclarations()
{
}

bool SCDeclarations::loadOdf(const KXmlElement &body, KoPALoadingContext &context)
{
    Q_UNUSED(context);

    KXmlElement element;
    forEachElement(element, body) {
        if (element.namespaceURI() == KOdfXmlNS::presentation) {
            if (element.tagName() == "header-decl") {
                const QString name = element.attributeNS(KOdfXmlNS::presentation, "name", QString());
                m_declarations[Header].insert(name, element.text());
            }
            else if(element.tagName() == "footer-decl") {
                const QString name = element.attributeNS(KOdfXmlNS::presentation, "name", QString());
                m_declarations[Footer].insert(name, element.text());
            }
            else if(element.tagName() == "date-time-decl") {
                QMap<QString, QVariant> data;
                const QString name = element.attributeNS(KOdfXmlNS::presentation, "name", QString());
                data["fixed"] = element.attributeNS(KOdfXmlNS::presentation, "source", "fixed") == "fixed";

                QString styleName = element.attributeNS(KOdfXmlNS::style, "data-style-name", "");
                if (!styleName.isEmpty()) {
                    KOdfStylesReader::DataFormatsMap::const_iterator it = context.odfLoadingContext().stylesReader().dataFormats().constFind(styleName);
                    if (it != context.odfLoadingContext().stylesReader().dataFormats().constEnd()) {

                        QString formatString = (*it).first.prefix + (*it).first.formatStr + (*it).first.suffix;
                        data["format"] = formatString;
                    }
                }
                else {
                    data["format"] = QString("");
                    data["fixed value"] = element.text();
                }

                m_declarations[DateTime].insert(name, data);
            }
        }
        else if (element.tagName() == "page" && element.namespaceURI() == KOdfXmlNS::draw) {
            break;
        }
    }
    return true;
}


bool SCDeclarations::saveOdf(KoPASavingContext &paContext) const
{
    /*
       <presentation:header-decl presentation:name="hdr1">header</presentation:header-decl>
       <presentation:footer-decl presentation:name="ftr1">Footer for the slide</presentation:footer-decl>
       <presentation:footer-decl presentation:name="ftr2">footer</presentation:footer-decl>
       <presentation:date-time-decl presentation:name="dtd1" presentation:source="current-date" style:data-style-name="D3"/>
    */
    KXmlWriter &writer(paContext.xmlWriter());

    QHash<Type, QHash<QString, QVariant> >::const_iterator typeIt(m_declarations.constBegin());
    for (; typeIt != m_declarations.constEnd(); ++typeIt) {
        QHash<QString, QVariant>::const_iterator keyIt(typeIt.value().begin());
        for (; keyIt != typeIt.value().constEnd(); ++keyIt) {
            switch (typeIt.key()) {
            case Footer:
                writer.startElement("presentation:footer-decl");
                break;
            case Header:
                writer.startElement("presentation:header-decl");
                break;
            case DateTime:
                writer.startElement("presentation:date-time-decl");
                break;
            }

            writer.addAttribute("presentation:name", keyIt.key());
            if (typeIt.key() == DateTime) {
                //TODO
            }
            else {
                writer.addTextNode(keyIt.value().value<QString>());
            }
            writer.endElement();
        }
    }
    return true;
}

const QString SCDeclarations::declaration(Type type, const QString &key)
{
    QString retVal;
    if (type == DateTime) {
        QMap<QString, QVariant> dateTimeDefinition =
                m_declarations.value(type).value(key).value<QMap<QString, QVariant> >();

        // if there is no presenation declaration don't set a value
        if (!dateTimeDefinition.isEmpty()) {
            if (dateTimeDefinition["fixed"].toBool()) {
                retVal = dateTimeDefinition["fixed value"].toString();
            }
            else  {
                QDateTime target = QDateTime::currentDateTime();

                QString formatString = dateTimeDefinition["format"].toString();
                if (!formatString.isEmpty()) {
                    retVal = target.toString(formatString);
                }
                else {
                    // XXX: What do we do here?
                    retVal = target.date().toString(Qt::ISODate);
                }
            }
        }
    }
    else {
        retVal = m_declarations.value(type).value(key).toString();
    }
    return retVal;
}
