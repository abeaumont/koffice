/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
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

#include "KTextDebug_p.h"

#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFrame>
#include <QTextBlock>
#include <QTextTable>
#include <QTextFragment>
#include <QTextList>
#include <QTextStream>

#include "styles/KParagraphStyle.h"
#include "styles/KCharacterStyle.h"
#include "styles/KListStyle.h"
#include "styles/KTableStyle.h"
#include "styles/KTableCellStyle.h"
#include "styles/KStyleManager.h"
#include "KTextDocument.h"
#include "KTextBlockData.h"
#include <KTextDocumentLayout.h>
#include <KInlineTextObjectManager.h>
#include <KoBookmark.h>
#include <KInlineNote.h>

#define PARAGRAPH_BORDER_DEBUG

int KTextDebug::depth = 0;
const int KTextDebug::INDENT = 2;
const QTextDocument *KTextDebug::document = 0;

#define dumpIndent(T) { for (int i=0; i<T; ++i) out << ' '; }
#define dumpList(T) { foreach (const QString &x, T) out << x << ' '; }

Q_DECLARE_METATYPE(QList<KOdfText::Tab>)

static QString fontProperties(const QTextCharFormat &textFormat)
{
    QMap<int, QVariant> properties = textFormat.properties();
    QStringList fontProps;
    // add only font properties here
    for (QMap<int, QVariant>::const_iterator prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
        int id = prop.key();
	QVariant value = prop.value();
        switch (id) {
        case QTextFormat::FontFamily:
            fontProps.append(value.toString());
            break;
        case QTextFormat::FontPointSize:
            fontProps.append(QString("%1pt").arg(value.toDouble()));
            break;
        case QTextFormat::FontSizeAdjustment:
            fontProps.append(QString("%1adj").arg(value.toDouble()));
            break;
        case QTextFormat::FontWeight:
            fontProps.append(QString("weight %1").arg(value.toInt()));
            break;
        case QTextFormat::FontItalic:
            fontProps.append(value.toBool() ? "italic" : "non-italic");
            break;
        case QTextFormat::FontPixelSize:
            fontProps.append(QString("%1px").arg(value.toDouble()));
            break;
        case QTextFormat::FontFixedPitch:
            fontProps.append(value.toBool() ? "fixedpitch" : "varpitch");
            break;
        case QTextFormat::FontCapitalization:
            fontProps.append(QString("caps %1").arg(value.toInt()));
            break;
        case KCharacterStyle::FontCharset:
            fontProps.append(value.toString());
            break;
        case QTextFormat::FontStyleHint:
            fontProps.append(QString::number(value.toInt()));
            break;
        case QTextFormat::FontKerning:
            fontProps.append(QString("kerning %1").arg(value.toInt()));
            break;
        default:
            break;
        }
    }
    return fontProps.join(",");
}

void KTextDebug::dumpDocument(const QTextDocument *doc, QTextStream &out)
{
    Q_ASSERT(doc);
    document = doc;
    out << QString("<document defaultfont=\"%1\">").arg(doc->defaultFont().toString());
    dumpFrame(document->rootFrame(), out);
    out << "</document>";
    document = 0;
}

QString KTextDebug::textAttributes(const KCharacterStyle &style)
{
    QTextCharFormat format;
    style.applyStyle(format);
    return textAttributes(format);
}

QString KTextDebug::inlineObjectAttributes(const QTextCharFormat &textFormat)
{
    QString attrs;

    if (textFormat.objectType() == QTextFormat::UserObject + 1) {
        KTextDocumentLayout *lay = document ? qobject_cast<KTextDocumentLayout *>(document->documentLayout()) : 0;
        KInlineTextObjectManager *inlineObjectManager = lay ? lay->inlineTextObjectManager() : 0;
        KInlineObject *inlineObject = inlineObjectManager->inlineTextObject(textFormat);
        if (KoBookmark *bookmark = dynamic_cast<KoBookmark *>(inlineObject)) {
            if (bookmark->type() == KoBookmark::SinglePosition) {
                attrs.append(" type=\"bookmark\"");
            } else if (bookmark->type() == KoBookmark::StartBookmark) {
                attrs.append(" type=\"bookmark-start\"");
            } else if (bookmark->type() == KoBookmark::EndBookmark) {
                attrs.append(" type=\"bookmark-end\"");
            } else {
                attrs.append(" type=\"bookmark-unknown\"");
            }
            attrs.append(QString(" name=\"%1\"").arg(bookmark->name()));
        } else if (KInlineNote *note = dynamic_cast<KInlineNote *>(inlineObject)) {
            attrs.append(QString(" id=\"%1\"").arg(note->id()));
            if (note->type() == KInlineNote::Footnote) {
                attrs.append(" type=\"footnote\"");
            } else if (note->type() == KInlineNote::Endnote) {
                attrs.append(" type=\"endnote\"");
            }
            attrs.append(QString(" label=\"%1\"").arg(note->label()));
            attrs.append(QString(" text=\"%1\"").arg(note->text().toPlainText()));
        } else {
            attrs.append(" type=\"inlineobject\">");
        }
    }

    return attrs;
}

QString KTextDebug::textAttributes(const QTextCharFormat &textFormat)
{
    QString attrs;

    QTextImageFormat imageFormat = textFormat.toImageFormat();

    if (imageFormat.isValid()) {
        attrs.append(" type=\"image\">");
        return attrs;
    }

    KStyleManager *styleManager = document ? KTextDocument(document).styleManager() : 0;
    if (styleManager && textFormat.hasProperty(KCharacterStyle::StyleId)) {
        int id = textFormat.intProperty(KCharacterStyle::StyleId);
        KCharacterStyle *characterStyle = styleManager->characterStyle(id);
        attrs.append(" characterStyle=\"id:").append(QString::number(id));
        if (characterStyle)
            attrs.append(" name:").append(characterStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = textFormat.properties();
    attrs.append(" type=\"char\"");
    QString fontProps = fontProperties(textFormat);
    if (!fontProps.isEmpty())
        attrs.append(QString(" font=\"%1\"").arg(fontProps));

    if (textFormat.isAnchor()) {
        attrs.append(QString(" achorHref=\"%1\"").arg(textFormat.anchorHref()));
        attrs.append(QString(" achorName=\"%1\"").arg(textFormat.anchorName()));
    }
 
    for(QMap<int, QVariant>::const_iterator prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
        int id = prop.key();
	QString key, value;
	QVariant prop_value = prop.value();
        switch (id) {
        case QTextFormat::TextOutline: {
            key = "outline";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen)
                value = "false";
            else
                value = pen.color().name();
            break;
        }
        case KCharacterStyle::UnderlineStyle:
            key = "underlinestyle";
            value = QString::number(prop_value.toInt());
            break;
        case QTextFormat::TextUnderlineColor:
            key = "underlinecolor";
            value = qvariant_cast<QColor>(prop_value).name();
            break;
        case KCharacterStyle::UnderlineType:
            key = "underlinetype";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::UnderlineMode:
            key = "underlinemode";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::UnderlineWeight:
            key = "underlineweight";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::UnderlineWidth:
            key = "underlinewidth";
            value = QString::number(prop_value.toDouble());
            break;
        case KCharacterStyle::StrikeOutStyle:
            key = "strikeoutstyle";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::StrikeOutColor:
            key = "strikeoutcolor";
            value = qvariant_cast<QColor>(prop_value).name();
            break;
        case KCharacterStyle::StrikeOutType:
            key = "strikeouttype";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::StrikeOutMode:
            key = "strikeoutmode";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::StrikeOutWeight:
            key = "strikeoutweight";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::StrikeOutWidth:
            key = "strikeoutwidth";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFormat::ForegroundBrush:
            key = "foreground";
            value = qvariant_cast<QBrush>(prop_value).color().name(); // beware!
            break;
        case QTextFormat::BackgroundBrush:
            key = "background";
            value = qvariant_cast<QBrush>(prop_value).color().name(); // beware!
            break;
        case QTextFormat::BlockAlignment:
            key = "align";
            value = QString::number(prop_value.toInt());
            break;
        case QTextFormat::TextIndent:
            key = "textindent";
            value = QString::number(prop_value.toInt());
            break;
        case QTextFormat::BlockIndent:
            key = "indent";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::Country:
            key = "country";
            value = prop_value.toString();
            break;
        case KCharacterStyle::Language:
            key = "language";
            value = prop_value.toString();
            break;
        case KCharacterStyle::HasHyphenation:
            key = "hypenation";
            value = prop_value.toBool();
            break;
        case KCharacterStyle::StrikeOutText:
            key = "strikeout-text";
            value = prop_value.toString();
            break;
        case KCharacterStyle::FontCharset:
            key = "font-charset";
            value = prop_value.toString();
            break;
        case KCharacterStyle::TextRotationAngle:
            key = "rotation-angle";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::TextRotationScale:
            key = "text-rotation-scale";
            value = prop_value.toInt() == KCharacterStyle::Fixed ? "Fixed" : "LineHeight";
            break;
        case KCharacterStyle::TextScale:
            key = "text-scale";
            value = QString::number(prop_value.toInt());
            break;
        case KCharacterStyle::InlineRdf:
            key = "inline-rdf";
            value = QString::number(prop_value.toInt());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KTextDebug::paraAttributes(const KParagraphStyle &style)
{
    QTextBlockFormat format;
    style.applyStyle(format);
    return paraAttributes(format);
}

QString KTextDebug::paraAttributes(const QTextBlockFormat &blockFormat)
{
    QString attrs;
    KStyleManager *styleManager = document ? KTextDocument(document).styleManager() : 0;
    if (styleManager && blockFormat.hasProperty(KParagraphStyle::StyleId)) {
        int id = blockFormat.intProperty(KParagraphStyle::StyleId);
        KParagraphStyle *paragraphStyle = styleManager->paragraphStyle(id);
        attrs.append(" paragraphStyle=\"id:").append(QString::number(id));
        if (paragraphStyle)
            attrs.append(" name:").append(paragraphStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = blockFormat.properties();
    for (QMap<int, QVariant>::const_iterator prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
	int id = prop.key();
	QVariant prop_value = prop.value();
        QString key, value;
        switch (id) {
        // the following are 'todo'
        case KParagraphStyle::PercentLineHeight:
        case KParagraphStyle::FixedLineHeight:
        case KParagraphStyle::MinimumLineHeight:
        case KParagraphStyle::LineSpacing:
        case KParagraphStyle::LineSpacingFromFont:
        case KParagraphStyle::AlignLastLine:
        case KParagraphStyle::WidowThreshold:
        case KParagraphStyle::OrphanThreshold:
        case KParagraphStyle::DropCapsTextStyle:
        case KParagraphStyle::FollowDocBaseline:
        case KParagraphStyle::HasLeftBorder:
        case KParagraphStyle::HasTopBorder:
        case KParagraphStyle::HasRightBorder:
        case KParagraphStyle::HasBottomBorder:
        case KParagraphStyle::BorderLineWidth:
        case KParagraphStyle::SecondBorderLineWidth:
        case KParagraphStyle::DistanceToSecondBorder:
        case KParagraphStyle::LeftPadding:
        case KParagraphStyle::TopPadding:
        case KParagraphStyle::RightPadding:
        case KParagraphStyle::BottomPadding:
        case KParagraphStyle::LeftBorderColor:
        case KParagraphStyle::TopInnerBorderWidth:
        case KParagraphStyle::TopBorderSpacing:
        case KParagraphStyle::TopBorderStyle:
        case KParagraphStyle::TopBorderColor:
        case KParagraphStyle::RightInnerBorderWidth:
        case KParagraphStyle::RightBorderSpacing:
        case KParagraphStyle::RightBorderStyle:
        case KParagraphStyle::RightBorderColor:
        case KParagraphStyle::BottomInnerBorderWidth:
        case KParagraphStyle::BottomBorderSpacing:
        case KParagraphStyle::BottomBorderStyle:
        case KParagraphStyle::BottomBorderColor:
        case KParagraphStyle::ListStyleId:
        case KParagraphStyle::ListStartValue:
        case KParagraphStyle::RestartListNumbering:
        case KParagraphStyle::TextProgressionDirection:
        case KParagraphStyle::MasterPageName:
        case KParagraphStyle::OutlineLevel:
            break;
        case KParagraphStyle::AutoTextIndent:
            key = "autotextindent";
            value = prop_value.toBool() ? "true" : "false" ;
            break;
#ifdef PARAGRAPH_BORDER_DEBUG // because it tends to get annoyingly long :)
        case KParagraphStyle::LeftBorderWidth:
            key = "border-width-left";
            value = QString::number(prop_value.toDouble()) ;
            break;
        case KParagraphStyle::TopBorderWidth:
            key = "border-width-top";
            value = QString::number(prop_value.toDouble()) ;
            break;
        case KParagraphStyle::RightBorderWidth:
            key = "border-width-right";
            value = QString::number(prop_value.toDouble()) ;
            break;
        case KParagraphStyle::BottomBorderWidth:
            key = "border-width-bottom";
            value = QString::number(prop_value.toDouble()) ;
            break;
        case KParagraphStyle::LeftBorderStyle:
            key = "border-style-left";
            value = QString::number(prop_value.toDouble()) ;
            break;
        case KParagraphStyle::LeftBorderSpacing:
            key = "inner-border-spacing-left";
            value = QString::number(prop_value.toDouble()) ;
            break;
        case KParagraphStyle::LeftInnerBorderWidth:
            key = "inner-border-width-left";
            value = QString::number(prop_value.toDouble()) ;
            break;
#endif
        case KParagraphStyle::TabStopDistance:
            key = "tab-stop-distance";
            value = QString::number(prop_value.toDouble());
            break;
        case KParagraphStyle::TabPositions:
            key = "tab-stops";
            value = "";
            foreach(const QVariant & qvtab, qvariant_cast<QList<QVariant> >(prop_value)) {
                KOdfText::Tab tab = qvtab.value<KOdfText::Tab>();
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
                if (! tab.leaderText.isEmpty())
                    value.append(" leadertext:").append(QString(tab.leaderText));
                value.append("}, ");
            }
            break;
        case KParagraphStyle::DropCaps:
            key = "drop-caps";
            value = QString::number(prop_value.toBool());
            break;
        case KParagraphStyle::DropCapsLines:
            key = "drop-caps-lines";
            value = QString::number(prop_value.toInt());
            break;
        case KParagraphStyle::DropCapsLength:
            key = "drop-caps-length";
            value = QString::number(prop_value.toInt());
            break;
        case KParagraphStyle::DropCapsDistance:
            key = "drop-caps-distance";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFormat::BlockBottomMargin:
            value = QString::number(prop_value.toDouble());
            if (value != "0")
                key = "block-bottom-margin";
            break;
        case QTextFormat::BlockTopMargin:
            value = QString::number(prop_value.toDouble());
            if (value != "0")
                key = "block-top-margin";
            break;
        case QTextFormat::BlockLeftMargin:
            value = QString::number(prop_value.toDouble());
            if (value != "0")
                key = "block-left-margin";
            break;
        case QTextFormat::BlockRightMargin:
            value = QString::number(prop_value.toDouble());
            if (value != "0")
                key = "block-right-margin";
            break;
        case KParagraphStyle::UnnumberedListItem:
            key = "unnumbered-list-item";
            value = QString::number(prop_value.toBool());
            break;
        case KParagraphStyle::IsListHeader:
            key = "list-header";
            value = '1';
            break;
        case KParagraphStyle::ListLevel:
            key = "list-level";
            value = QString::number(prop_value.toInt());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KTextDebug::listAttributes(const QTextListFormat &listFormat)
{
    QString attrs;
    KStyleManager *styleManager = document ? KTextDocument(document).styleManager() : 0;
    if (styleManager && listFormat.hasProperty(KListStyle::StyleId)) {
        int id = listFormat.intProperty(KListStyle::StyleId);
        KListStyle *listStyle = styleManager->listStyle(id);
        attrs.append(" listStyle=\"id:").append(QString::number(id));
        if (listStyle)
            attrs.append(" name:").append(listStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = listFormat.properties();
    for(QMap<int, QVariant>::const_iterator prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
	const int &id = prop.key();
	const QVariant &prop_value = prop.value();
        QString key, value;
        switch (id) {
        case QTextListFormat::ListStyle:
            key = "type";
            value = QString::number(prop_value.toInt());
            break;
        case QTextListFormat::ListIndent:
            key = "indent";
            value = QString::number(prop_value.toDouble());
            break;
        case KListStyle::ListItemPrefix:
            key = "prefix";
            value = prop_value.toString();
            break;
        case KListStyle::ListItemSuffix:
            key = "suffix";
            value = prop_value.toString();
            break;
        case KListStyle::StartValue:
            key = "start-value";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::Level:
            key = "level";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::DisplayLevel:
            key = "display-level";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::CharacterStyleId:
            key = "charstyleid";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::Alignment:
            key = "alignment";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::BulletSize:
            key = "bullet-size";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::BulletCharacter:
            key = "bullet-char";
            value = prop_value.toString();
            break;
        case KListStyle::LetterSynchronization:
            key = "letter-sync";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::StyleId:
            key = "styleid";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::ContinueNumbering:
            key = "continue-numbering";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::MinimumWidth:
            key = "minimum-width";
            value = QString::number(prop_value.toDouble());
            break;
        case KListStyle::ListId:
            key = "list-id";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::IsOutline:
            key = "is-outline";
            value = prop_value.toBool();
            break;
        case KListStyle::Indent:
            key = "indent";
            value = QString::number(prop_value.toInt());
            break;
        case KListStyle::MinimumDistance:
            key = "minimum-distance";
            value = QString::number(prop_value.toDouble());
            break;
        case KListStyle::Width:
            key = "width";
            value = QString::number(prop_value.toDouble());
            break;
        case KListStyle::Height:
            key = "height";
            value = QString::number(prop_value.toDouble());
            break;
        case KListStyle::BulletImageKey:
            key = "bullet-image-key";
            value = QString::number(prop_value.toInt());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KTextDebug::tableAttributes(const KTableStyle &tableStyle)
{
    QTextTableFormat format;
    tableStyle.applyStyle(format);
    return tableAttributes(format);
}

QString KTextDebug::tableAttributes(const QTextTableFormat &tableFormat)
{
    QString attrs;
    KStyleManager *styleManager = document ? KTextDocument(document).styleManager() : 0;
    if (styleManager) {
        int id = tableFormat.intProperty(KTableStyle::StyleId);
        KTableStyle *tableStyle = styleManager->tableStyle(id);
        attrs.append(" tableStyle=\"id:").append(QString::number(id));
        if (tableStyle)
            attrs.append(" name:").append(tableStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = tableFormat.properties();
    for (QMap<int, QVariant>::const_iterator prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
	const int &id = prop.key();
	const QVariant &prop_value = prop.value();
        QString key, value;
        switch (id) {
        case QTextTableFormat::TableColumnWidthConstraints:
        case QTextFormat::BackgroundBrush:
            key = "background";
            value = qvariant_cast<QBrush>(prop_value).color().name(); // beware!
            break;
        case QTextFormat::BlockAlignment:
            key = "alignment";
            switch (prop_value.toInt()) {
                case Qt::AlignLeft:
                    value = "left";
                    break;
                case Qt::AlignRight:
                    value = "right";
                    break;
                case Qt::AlignHCenter:
                    value = "center";
                    break;
                case Qt::AlignJustify:
                    value = "justify";
                    break;
                default:
                    value = "";
                    break;
            }
            break;
        case KTableStyle::KeepWithNext:
            key = "keep-with-next";
            value = prop_value.toBool() ? "true" : "false";
            break;
        case KTableStyle::BreakBefore:
            key = "break-before";
            value = prop_value.toBool() ? "true" : "false";
            break;
        case KTableStyle::BreakAfter:
            key = "break-after";
            value = prop_value.toBool() ? "true" : "false";
            break;
        case KTableStyle::MayBreakBetweenRows:
            key = "may-break-between-rows";
            value = prop_value.toBool() ? "true" : "false";
            break;
        case KTableStyle::MasterPageName:
            key = "master-page-name";
            value = prop_value.toString();
            break;
        case QTextTableFormat::TableColumns:
            key = "columns";
            value = QString::number(prop_value.toInt());
            break;
        case QTextTableFormat::TableCellSpacing:
            key = "cell-spacing";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextTableFormat::TableHeaderRowCount:
            key = "header-row-count";
            value = QString::number(prop_value.toInt());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KTextDebug::frameAttributes(const QTextFrameFormat &frameFormat)
{
    QString attrs;

    QMap<int, QVariant> properties = frameFormat.properties();
    for (QMap<int, QVariant>::const_iterator prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
	int id = prop.key();
	QVariant prop_value = prop.value();
        QString key, value;
        switch (id) {
        case QTextFrameFormat::FrameBorderBrush:
            break;
        case QTextFrameFormat::FrameBorderStyle:
            key = "border-style";
            // determine border style.
            switch (prop_value.toInt()) {
            case QTextFrameFormat::BorderStyle_None:
                value = "None";
                break;
            case QTextFrameFormat::BorderStyle_Dotted:
                value = "Dotted";
                break;
            case QTextFrameFormat::BorderStyle_Dashed:
                value = "Dashed";
                break;
            case QTextFrameFormat::BorderStyle_Solid:
                value = "Solid";
                break;
            case QTextFrameFormat::BorderStyle_Double:
                value = "Double";
                break;
            case QTextFrameFormat::BorderStyle_DotDash:
                value = "DotDash";
                break;
            case QTextFrameFormat::BorderStyle_DotDotDash:
                value = "DotDotDash";
                break;
            case QTextFrameFormat::BorderStyle_Groove:
                value = "Groove";
                break;
            case QTextFrameFormat::BorderStyle_Ridge:
                value = "Ridge";
                break;
            case QTextFrameFormat::BorderStyle_Inset:
                value = "Inset";
                break;
            case QTextFrameFormat::BorderStyle_Outset:
                value = "Outset";
                break;
            default:
                value = "Unknown";
                break;
            }
            break;
        case QTextFrameFormat::FrameBorder:
            key = "border";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFrameFormat::FrameMargin:
            key = "margin";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFrameFormat::FramePadding:
            key = "padding";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFrameFormat::FrameWidth:
            key = "width";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFrameFormat::FrameHeight:
            key = "height";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFrameFormat::FrameTopMargin:
            key = "top-margin";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFrameFormat::FrameBottomMargin:
            key = "bottom-margin";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFrameFormat::FrameLeftMargin:
            key = "left-margin";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFrameFormat::FrameRightMargin:
            key = "right-margin";
            value = QString::number(prop_value.toDouble());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KTextDebug::tableCellAttributes(const KTableCellStyle &tableCellStyle)
{
    QTextTableCellFormat format;
    tableCellStyle.applyStyle(format);
    return tableCellAttributes(format);
}

QString KTextDebug::tableCellAttributes(const QTextTableCellFormat &tableCellFormat)
{
    QString attrs;
    KStyleManager *styleManager = document ? KTextDocument(document).styleManager() : 0;
    if (styleManager) {
        int id = tableCellFormat.intProperty(KTableCellStyle::StyleId);
        KTableCellStyle *tableCellStyle = styleManager->tableCellStyle(id);
        attrs.append(" tableCellStyle=\"id:").append(QString::number(id));
        if (tableCellStyle)
            attrs.append(" name:").append(tableCellStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = tableCellFormat.properties();
    for (QMap<int, QVariant>::const_iterator prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
	int id = prop.key();
	QVariant prop_value = prop.value();
        QString key, value;
        switch (id) {
        case QTextTableCellFormat::TableCellRowSpan:
            key = "row-span";
            value = QString::number(prop_value.toInt());
            break;
        case QTextTableCellFormat::TableCellColumnSpan:
            key = "column-span";
            value = QString::number(prop_value.toInt());
            break;
        case QTextFormat::TableCellTopPadding:
            key = "top-padding";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFormat::TableCellBottomPadding:
            key = "bottom-padding";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFormat::TableCellLeftPadding:
            key = "left-padding";
            value = QString::number(prop_value.toDouble());
            break;
        case QTextFormat::TableCellRightPadding:
            key = "right-padding";
            value = QString::number(prop_value.toDouble());
            break;
        case KTableCellStyle::TopBorderOuterPen: {
            key = "top-border-outer";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KTableCellStyle::TopBorderSpacing:
            key = "top-border-spacing";
            value = QString::number(prop_value.toDouble());
            break;
        case KTableCellStyle::TopBorderInnerPen: {
            key = "top-border-inner";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KTableCellStyle::LeftBorderOuterPen: {
            key = "left-border-outer";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KTableCellStyle::LeftBorderSpacing:
            key = "left-border-spacing";
            value = QString::number(prop_value.toDouble());
            break;
        case KTableCellStyle::LeftBorderInnerPen: {
            key = "left-border-inner";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KTableCellStyle::BottomBorderOuterPen: {
            key = "bottom-border-outer";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KTableCellStyle::BottomBorderSpacing:
            key = "bottom-border-spacing";
            value = QString::number(prop_value.toDouble());
            break;
        case KTableCellStyle::BottomBorderInnerPen: {
            key = "bottom-border-inner";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KTableCellStyle::RightBorderOuterPen: {
            key = "right-border-outer";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KTableCellStyle::RightBorderSpacing:
            key = "right-border-spacing";
            value = QString::number(prop_value.toDouble());
            break;
        case KTableCellStyle::RightBorderInnerPen: {
            key = "right-border-inner";
            QPen pen = qvariant_cast<QPen>(prop_value);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KTableCellStyle::MasterPageName:
            key = "master-page-name";
            value = prop_value.toString();
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

void KTextDebug::dumpFrame(const QTextFrame *frame, QTextStream &out)
{
    depth += INDENT;

    dumpIndent(depth);
    out << "<frame" << frameAttributes(frame->frameFormat()) << '>' << endl;

    QTextFrame::iterator iterator = frame->begin();

    for (; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFrame *childFrame = iterator.currentFrame();
        QTextBlock textBlock = iterator.currentBlock();

        if (childFrame) {
            QTextTable *table = qobject_cast<QTextTable *>(childFrame);
            if (table) {
                dumpTable(table, out);
            } else {
                dumpFrame(frame, out);
            }
        } else if (textBlock.isValid()) {
            dumpBlock(textBlock, out);
        }
    }

    dumpIndent(depth);
    out << "</frame>" << endl;
    depth -= INDENT;
}

void KTextDebug::dumpBlock(const QTextBlock &block, QTextStream &out)
{
    depth += INDENT;

    QString attrs;
    attrs.append(paraAttributes(block.blockFormat()));
    //attrs.append(" blockcharformat=\"").append(textAttributes(QTextCursor(block).blockCharFormat())).append('\"');
    attrs.append(textAttributes(QTextCursor(block).blockCharFormat()));

    QTextList *list = block.textList();
    if (list) {
        attrs.append(" list=\"item:").append(QString::number(list->itemNumber(block) + 1)).append('/')
        .append(QString::number(list->count()));
        attrs.append('"');
        attrs.append(listAttributes(list->format()));
    }

    dumpIndent(depth);
    out << "<block" << attrs << '>' << endl;

    QTextBlock::Iterator iterator = block.begin();
    for (; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFragment fragment = iterator.fragment();
        if (fragment.isValid()) {
            dumpFragment(fragment, out);
        }
    }
    dumpIndent(depth);
    out << "</block>" << endl;
    depth -= INDENT;
    if (block.next().isValid())
        out << ' ';
}

void KTextDebug::dumpTable(const QTextTable *table, QTextStream &out)
{
    depth += INDENT;

    QString attrs;
    attrs.append(tableAttributes(table->format()));
    attrs.append(frameAttributes(table->frameFormat())); // include frame attributes too.

    dumpIndent(depth);
    out << "<table" << attrs << '>' << endl;

    // loop through all the cells in the table and dump the cells.
    for (int row = 0; row < table->rows(); ++row) {
        for (int column = 0; column < table->columns(); ++column) {
            dumpTableCell(table->cellAt(row, column), out);
        }
    }

    dumpIndent(depth);
    out << "</table>" << endl;
    depth -= INDENT;
}

void KTextDebug::dumpTableCell(const QTextTableCell &cell, QTextStream &out)
{
    depth += INDENT;

    QString attrs;
    attrs.append(textAttributes(cell.format()));
    attrs.append(tableCellAttributes(cell.format().toTableCellFormat()));

    dumpIndent(depth);
    out << "<cell" << attrs << '>' << endl;

    // iterate through the cell content.
    QTextFrame::iterator cellIter = cell.begin();
    while (!cellIter.atEnd()) {
        if (cellIter.currentFrame() != 0) {
            // content is a frame or table.
            dumpFrame(cellIter.currentFrame(), out);
        } else {
            // content is a block.
            dumpBlock(cellIter.currentBlock(), out);
        }
        ++cellIter;
    }

    dumpIndent(depth);
    out << "</cell>\n";

    depth -= INDENT;
}

void KTextDebug::dumpFragment(const QTextFragment &fragment, QTextStream &out)
{
    depth += INDENT;

    KTextDocumentLayout *lay = document ? qobject_cast<KTextDocumentLayout *>(document->documentLayout()) : 0;
    QTextCharFormat charFormat = fragment.charFormat();
    KInlineObject *inlineObject = lay ? lay->inlineTextObjectManager()->inlineTextObject(charFormat) : 0;
    if (inlineObject) {
        QString cf = inlineObjectAttributes(charFormat);

        dumpIndent(depth);
        out << "<fragment" << cf << ">\n";
    } else {
        QString cf = textAttributes(charFormat);

        dumpIndent(depth);
        out << "<fragment" << cf << ">\n";
        dumpIndent(depth + INDENT);
        out << '|' << fragment.text() << "|\n";
        dumpIndent(depth);
        out << "</fragment>\n";
    }

    depth -= INDENT;
}

