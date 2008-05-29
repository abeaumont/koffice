/* This file is part of the KDE project
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

#include "KoTextDebug.h"

#include <QTextDocument>
#include <QTextFrame>
#include <QTextBlock>
#include <QTextTable>
#include <QTextFragment>
#include <QTextList>
#include <QDebug>
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextBlockData.h"
#include <KoTextDocumentLayout.h>
#include <kdeversion.h>

int KoTextDebug::depth = 0;
const int KoTextDebug::INDENT = 2;
const QTextDocument *KoTextDebug::document = 0;

Q_DECLARE_METATYPE(QList<KoText::Tab>)

void KoTextDebug::dumpDocument(const QTextDocument *doc)
{
    document = doc;
    qDebug() << "<document>";
    dumpFrame(document->rootFrame());
    qDebug() << "</document>";
    document = 0;
}

QString KoTextDebug::attributes(const QMap<int, QVariant> &properties)
{
    QString attrs;
    foreach(int id, properties.keys()) {
        QString key, value;
        switch (id) {
        case QTextFormat::TextOutline: {
            key = "outline";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen)
                value = "false";
            else
                value = pen.color().name();
            break;
                                       }
        case KoCharacterStyle::UnderlineStyle:
            key = "underlinestyle";
            value = QString::number(properties[id].toInt());
            break;
        case QTextFormat::TextUnderlineColor:
            key = "underlinecolor";
            value = qvariant_cast<QColor>(properties[id]).name();
            break;
        case KoCharacterStyle::UnderlineType:
            key = "underlinetype";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::UnderlineMode:
            key = "underlinemode";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::UnderlineWeight:
            key = "underlineweight";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::UnderlineWidth:
            key = "underlinewidth";
            value = QString::number(properties[id].toDouble());
            break;
        case KoCharacterStyle::StrikeOutStyle:
            key = "strikeoutstyle";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::StrikeOutColor:
            key = "strikeoutcolor";
            value = qvariant_cast<QColor>(properties[id]).name();
            break;
        case KoCharacterStyle::StrikeOutType:
            key = "strikeouttype";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::StrikeOutMode:
            key = "strikeoutmode";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::StrikeOutWeight:
            key = "strikeoutweight";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::StrikeOutWidth:
            key = "strikeoutwidth";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFormat::ForegroundBrush:
            key = "foreground";
            value = qvariant_cast<QBrush>(properties[id]).color().name(); // beware!
            break;
        case QTextFormat::BackgroundBrush:
            key = "background";
            value = qvariant_cast<QBrush>(properties[id]).color().name(); // beware!
            break;
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
        case QTextFormat::FontCapitalization:
            key = "font-caps";
            value = QString::number(properties[id].toInt());
            break;
#endif
        case QTextFormat::BlockAlignment:
            key = "align";
            value = QString::number(properties[id].toInt());
            break;
        case QTextFormat::TextIndent:
            key = "textindent";
            value = QString::number(properties[id].toInt());
            break;
        case KoParagraphStyle::AutoTextIndent:
            key = "autotextindent";
            value = properties[id].toBool()? "true" : "false" ;
            break;
        case QTextFormat::BlockIndent:
            key = "indent";
            value = QString::number(properties[id].toInt());
            break;
#if QT_VERSION >= KDE_MAKE_VERSION(4,5,0)
        case QTextFormat::FontStyleHint:
            key = "font-family-generic";
            value = QString::number(properties[id].toInt());
            break;
        case QTextFormat::FontKerning:
            key = "font-kerning";
            value = QString::number(properties[id].toInt());
            break;
#endif
        case KoCharacterStyle::Country:
            key = "country";
            value = properties[id].toString();
            break;
        case KoCharacterStyle::Language:
            key = "language";
            value = properties[id].toString();
            break;
#ifdef PARAGRAPH_BORDER_DEBUG // because it tends to get annoyingly long :)
        case KoParagraphStyle::LeftBorderWidth:
            key = "border-width-left";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::TopBorderWidth:
            key = "border-width-top";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::RightBorderWidth:
            key = "border-width-right";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::BottomBorderWidth:
            key = "border-width-bottom";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::LeftBorderStyle:
            key = "border-style-left";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::LeftBorderSpacing:
            key = "inner-border-spacing-left";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::LeftInnerBorderWidth:
            key = "inner-border-width-left";
            value = QString::number(properties[id].toDouble()) ;
            break;
#endif
        case KoCharacterStyle::FontCharset:
            key = "font-charset";
            value = properties[id].toString();
            break;
        case KoParagraphStyle::TabPositions:
            key = "tab-stops";
            value = "";
            foreach(QVariant qvtab, qvariant_cast<QList<QVariant> >(properties[id])) {
                KoText::Tab tab = qvtab.value<KoText::Tab>();
                value.append("{");
                value.append(" pos:").append(QString::number(tab.position));
                value.append(" type:").append(QString::number(tab.type));
                if (! tab.delimiter.isNull())
                    value.append(" delim:").append(QString(tab.delimiter));
                value.append(" leadertype:").append(QString::number(tab.leaderType));
                value.append(" leaderstyle:").append(QString::number(tab.leaderStyle));
                value.append(" leaderweight:").append(QString::number(tab.leaderWeight));
                value.append(" leaderwidth:").append(QString().setNum(tab.leaderWidth));
                value.append(" leadercolor:").append(tab.leaderColor.name());
                if (! tab.leaderText.isNull())
                    value.append(" leadertext:").append(QString(tab.leaderText));
                value.append("}, ");
            }
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

void KoTextDebug::dumpFrame(const QTextFrame *frame)
{
    depth += INDENT;
    qDebug("%*s%s", depth, " ", "<frame>");

    QTextFrame::iterator iterator = frame->begin();

    for (; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFrame *childFrame = iterator.currentFrame();
        QTextBlock textBlock = iterator.currentBlock();

        if (childFrame) {
            QTextTable *table = qobject_cast<QTextTable *>(childFrame);
            if (table) {
                dumpTable(table);
            } else {
                dumpFrame(frame);
            }
        } else if (textBlock.isValid()) {
            dumpBlock(textBlock);
        }
    }

    qDebug("%*s%s", depth, " ", "</frame>");
    depth -= INDENT;
}

void KoTextDebug::dumpBlock(const QTextBlock &block)
{
    depth += INDENT;

    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout *>(document->documentLayout());
    QString attrs;
    if (lay && lay->styleManager()) {
        int id = block.blockFormat().intProperty(KoParagraphStyle::StyleId);
        KoParagraphStyle *paragraphStyle = lay->styleManager()->paragraphStyle(id);
        attrs.append(" paragraphStyle=\"id:").append(QString::number(id));
        if (paragraphStyle)
             attrs.append(" name:").append(paragraphStyle->name());
        attrs.append("\"");
    }

    QTextList *list = block.textList();
    if (list) {
        attrs.append(" list=\"item:").append(QString::number(list->itemNumber(block)+1)).append('/')
              .append(QString::number(list->count()));
        attrs.append(" listindent:").append(QString::number(list->format().indent()));
        attrs.append(" liststyle:").append(QString::number(list->format().style()));
        attrs.append("\"");
    }

    attrs.append(attributes(block.blockFormat().properties()));

    qDebug("%*s<block%s>", depth, " ", qPrintable(attrs));

    QTextBlock::Iterator iterator = block.begin();
    for(; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFragment fragment = iterator.fragment();
        if (fragment.isValid()) {
            dumpFragment(fragment);
        }
    }
    qDebug("%*s%s", depth, " ", "</block>");
    depth -= INDENT;
    if (block.next().isValid())
        qDebug(" ");
}

void KoTextDebug::dumpTable(const QTextTable *)
{
    depth += INDENT;
    qDebug("%*s%s", depth, " ", "<table>");
    // FIXME
    qDebug("%*s%s", depth, " ", "</table>");
    depth -= INDENT;
}

void KoTextDebug::dumpFragment(const QTextFragment &fragment)
{
    depth += INDENT;

    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout *>(document->documentLayout());
    QString attrs;
    if (lay && lay->styleManager()) {
        int id = fragment.charFormat().intProperty(KoCharacterStyle::StyleId);
        KoCharacterStyle *characterStyle = lay->styleManager()->characterStyle(id);
        attrs.append(" characterStyle=\"id:").append(QString::number(id));
        if (characterStyle)
             attrs.append(" name:").append(characterStyle->name());
        attrs.append("\"");
    }

    QTextCharFormat textFormat = fragment.charFormat();
    QTextImageFormat imageFormat = textFormat.toImageFormat();
 
    if (imageFormat.isValid()) {
        attrs.append(" type=\"image\">");
    } else {
        attrs.append(" type=\"char\"");
        attrs.append(" font=\"").append(textFormat.font().toString()).append("\"");
        if (textFormat.isAnchor()) {
            attrs.append(QString(" achorHref=\"%1\"").arg(textFormat.anchorHref()));
            attrs.append(QString(" achorName=\"%1\"").arg(textFormat.anchorName()));
        }
        attrs.append(attributes(textFormat.properties()));
    }

    qDebug("%*s<fragment%s>", depth, " ", qPrintable(attrs));

    qDebug("%*s|%s|", depth+INDENT, " ", qPrintable(fragment.text()));

    qDebug("%*s%s", depth, " ", "</fragment>");
    depth -= INDENT;
}

