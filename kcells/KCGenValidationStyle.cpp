/* This file is part of the KDE project

   Copyright 2004 Laurent Montel <montel@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "KCGenValidationStyle.h"

// KOffice
#include <KXmlWriter.h>

// KCells
#include "KCValidity.h"
#include "KCValueConverter.h"

KCGenValidationStyles::KCGenValidationStyles()
{

}

KCGenValidationStyles::~KCGenValidationStyles()
{

}

QString KCGenValidationStyles::insert(const KCGenValidationStyle& style)
{
    StyleMap::iterator it = m_styles.find(style);
    if (it == m_styles.end()) {

        QString styleName("val");
        styleName = makeUniqueName(styleName);
        m_names.insert(styleName, true);
        it = m_styles.insert(style, styleName);
    }
    return it.value();
}

QString KCGenValidationStyles::makeUniqueName(const QString& base) const
{
    int num = 1;
    QString name;
    do {
        name = base;
        name += QString::number(num++);
    } while (m_names.find(name) != m_names.end());
    return name;
}

void KCGenValidationStyles::writeStyle(KXmlWriter& writer)
{
    if (m_styles.count() > 0) {
        writer.startElement("table:content-validations");
        StyleMap::Iterator it;
        for (it = m_styles.begin(); it != m_styles.end(); ++it) {
            writer.startElement("table:content-validation");
            writer.addAttribute("table:name", it.value());
            writer.addAttribute("table:allow-empty-cell", it.key().allowEmptyCell);
            writer.addAttribute("table:condition", it.key().condition);

            writer.startElement("table:help-message");
            writer.addAttribute("table:title", it.key().title);
            writer.addAttribute("table:display", it.key().displayValidationInformation);

            QStringList text = it.key().messageInfo.split('\n', QString::SkipEmptyParts);
            for (QStringList::Iterator it2 = text.begin(); it2 != text.end(); ++it2) {
                writer.startElement("text:p");
                writer.addTextNode(*it2);
                writer.endElement();
            }
            writer.endElement();

            writer.startElement("table:error-message");
            writer.addAttribute("table:message-type", it.key().messageType);

            writer.addAttribute("table:title", it.key().titleInfo);
            writer.addAttribute("table:display", it.key().displayMessage);
            text = it.key().message.split('\n', QString::SkipEmptyParts);
            for (QStringList::Iterator it3 = text.begin(); it3 != text.end(); ++it3) {
                writer.startElement("text:p");
                writer.addTextNode(*it3);
                writer.endElement();
            }
            writer.endElement();

            writer.endElement();
        }
        writer.endElement();//close sheet:content-validation
    }
}

void KCGenValidationStyle::initVal(KCValidity *validity, const KCValueConverter *converter)
{
    if (validity) {
        allowEmptyCell = (validity->allowEmptyCell() ? "true" : "false");
        condition = createValidationCondition(validity, converter);
        title = validity->title();
        displayValidationInformation = (validity->displayValidationInformation() ? "true" : "false");
        messageInfo = validity->messageInfo();

        switch (validity->action()) {
        case KCValidity::Warning:
            messageType = "warning";
            break;
        case KCValidity::Information:
            messageType = "information";
            break;
        case KCValidity::Stop:
            messageType = "stop";
            break;
        }

        titleInfo = validity->titleInfo();
        displayMessage = (validity->displayMessage() ? "true" : "false");
        message = validity->message();
    }
}

QString KCGenValidationStyle::createValidationCondition(KCValidity* validity, const KCValueConverter *converter)
{
    QString result;
    switch (validity->restriction()) {
    case KCValidity::None:
        //nothing
        break;
    case KCValidity::Text:
        //doesn't exist into oo spec
        result = "cell-content-is-text()";
        break;
    case KCValidity::Time:
        result = createTimeValidationCondition(validity, converter);
        break;
    case KCValidity::Date:
        result = createDateValidationCondition(validity, converter);
        break;
    case KCValidity::Integer:
    case KCValidity::KCNumber:
        result = createNumberValidationCondition(validity);
        break;
    case KCValidity::TextLength:
        result = createTextValidationCondition(validity);
        break;
    case KCValidity::List:
        result = createListValidationCondition(validity);
        break;
    }
    return result;
}

QString KCGenValidationStyle::createListValidationCondition(KCValidity* validity)
{
    QString result = "oooc:cell-content-is-in-list(";
    result = validity->validityList().join(";");
    result += ')';
    return result;
}

QString KCGenValidationStyle::createNumberValidationCondition(KCValidity* validity)
{
    QString result;
    if (validity->restriction() == KCValidity::KCNumber)
        result = "oooc:cell-content-is-whole-number() and ";
    else if (validity->restriction() == KCValidity::Integer)
        result = "oooc:cell-content-is-decimal-number() and ";
    switch (validity->condition()) {
    case KCConditional::None:
    case KCConditional::IsTrueFormula:
        //nothing
        break;
    case KCConditional::Equal:
        result += "cell-content()";
        result += '=';
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::Superior:
        result += "cell-content()";
        result += '>';
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::Inferior:
        result += "cell-content()";
        result += '<';
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::SuperiorEqual:
        result += "cell-content()";
        result += ">=";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::InferiorEqual:
        result += "cell-content()";
        result += "<=";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::Different:
        result += "cell-content()";
        result += "!=";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::Between:
        result += "cell-content-is-between(";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        result += ',';
        result += QString::number((double)numToDouble(validity->maximumValue().asFloat()));
        result += ')';
        break;
    case KCConditional::DifferentTo:
        result += "cell-content-is-not-between(";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        result += ',';
        result += QString::number((double)numToDouble(validity->maximumValue().asFloat()));
        result += ')';
        break;
    }
    return result;
}


QString KCGenValidationStyle::createTimeValidationCondition(KCValidity* validity, const KCValueConverter *converter)
{
    QString result("oooc:cell-content-is-time() and ");
    switch (validity->condition()) {
    case KCConditional::None:
    case KCConditional::IsTrueFormula:
        //nothing
        break;
    case KCConditional::Equal:
        result += "cell-content()";
        result += '=';
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::Superior:
        result += "cell-content()";
        result += '>';
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::Inferior:
        result += "cell-content()";
        result += '<';
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::SuperiorEqual:
        result += "cell-content()";
        result += ">=";
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::InferiorEqual:
        result += "cell-content()";
        result += "<=";
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::Different:
        result += "cell-content()";
        result += "!=";
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::Between:
        result += "cell-content-is-between(";
        result += converter->asString(validity->minimumValue()).asString();
        result += ',';
        result += converter->asString(validity->maximumValue()).asString();
        result += ')';
        break;
    case KCConditional::DifferentTo:
        result += "cell-content-is-not-between(";
        result += converter->asString(validity->minimumValue()).asString();
        result += ',';
        result += converter->asString(validity->maximumValue()).asString();
        result += ')';
        break;
    }
    return result;
}

QString KCGenValidationStyle::createDateValidationCondition(KCValidity* validity, const KCValueConverter *converter)
{
    QString result("oooc:cell-content-is-date() and ");
    switch (validity->condition()) {
    case KCConditional::None:
    case KCConditional::IsTrueFormula:
        //nothing
        break;
    case KCConditional::Equal:
        result += "cell-content()";
        result += '=';
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::Superior:
        result += "cell-content()";
        result += '>';
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::Inferior:
        result += "cell-content()";
        result += '<';
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::SuperiorEqual:
        result += "cell-content()";
        result += ">=";
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::InferiorEqual:
        result += "cell-content()";
        result += "<=";
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::Different:
        result += "cell-content()";
        result += "!=";
        result += converter->asString(validity->minimumValue()).asString();
        break;
    case KCConditional::Between:
        result += "cell-content-is-between(";
        result += converter->asString(validity->minimumValue()).asString();
        result += ',';
        result += converter->asString(validity->maximumValue()).asString();
        result += ')';
        break;
    case KCConditional::DifferentTo:
        result += "cell-content-is-not-between(";
        result += converter->asString(validity->minimumValue()).asString();
        result += ',';
        result += converter->asString(validity->maximumValue()).asString();
        result += ')';
        break;
    }
    return result;
}

QString KCGenValidationStyle::createTextValidationCondition(KCValidity* validity)
{
    QString result;
    switch (validity->condition()) {
    case KCConditional::None:
    case KCConditional::IsTrueFormula:
        //nothing
        break;
    case KCConditional::Equal:
        result += "oooc:cell-content-text-length()";
        result += '=';
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::Superior:
        result += "oooc:cell-content-text-length()";
        result += '>';
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::Inferior:
        result += "oooc:cell-content-text-length()";
        result += '<';
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::SuperiorEqual:
        result += "oooc:cell-content-text-length()";
        result += ">=";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::InferiorEqual:
        result += "oooc:cell-content-text-length()";
        result += "<=";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::Different:
        result += "oooc:cell-content-text-length()";
        result += "!=";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        break;
    case KCConditional::Between:
        result += "oooc:cell-content-text-length-is-between(";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        result += ',';
        result += QString::number((double)numToDouble(validity->maximumValue().asFloat()));
        result += ')';
        break;
    case KCConditional::DifferentTo:
        result += "oooc:cell-content-text-length-is-not-between(";
        result += QString::number((double)numToDouble(validity->minimumValue().asFloat()));
        result += ',';
        result += QString::number((double)numToDouble(validity->maximumValue().asFloat()));
        result += ')';
        break;
    }
    return result;
}
