/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoCharacterStyle.h"

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextCursor>

#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoUnit.h>
#include <KoGenStyle.h>

#include <KDebug>

class KoCharacterStyle::Private {
public:
    Private() : stylesPrivate( new StylePrivate()) {}
    ~Private() {
        delete stylesPrivate;
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate->add(key, value);
    }
    double propertyDouble(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return 0.0;
        return variant.toDouble();
    }
    int propertyInt(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return 0;
        return variant.toInt();
    }
    QString propertyString(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return QString();
        return qvariant_cast<QString>(variant);
    }
    bool propertyBoolean(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return false;
        return variant.toBool();
    }
    QColor propertyColor(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return QColor();
        return variant.value<QColor>();
    }

    QString name;
    StylePrivate *stylesPrivate;
};

KoCharacterStyle::KoCharacterStyle(QObject *parent)
    : QObject(parent), d( new Private() )
{
    setFontPointSize(12.0);
    setFontWeight(QFont::Normal);
    setVerticalAlignment(QTextCharFormat::AlignNormal);
    setForeground(Qt::black);
    setStrikeOutColor(Qt::black);
}

KoCharacterStyle::KoCharacterStyle(const KoCharacterStyle &style)
    : QObject(0), d( new Private() )
{
    d->stylesPrivate->copyMissing(style.d->stylesPrivate);
    d->name = style.name();
}


KoCharacterStyle::KoCharacterStyle(const QTextCharFormat &format)
    : QObject(0), d( new Private() )
{
    QMapIterator<int, QVariant> iter(format.properties());
    while (iter.hasNext()) {
        iter.next();
        d->setProperty(iter.key(), iter.value());
    }
}

void KoCharacterStyle::copyProperties(const KoCharacterStyle *style) {
    d->stylesPrivate->clearAll();
    d->stylesPrivate->copyMissing(style->d->stylesPrivate);
    d->name = style->name();
}

KoCharacterStyle::~KoCharacterStyle() {
    delete d;
}

QPen KoCharacterStyle::textOutline () const {
    QVariant variant = d->stylesPrivate->value(QTextFormat::TextOutline);
    if(variant.isNull()) {
        QPen pen(Qt::NoPen);
        return pen;
    }
    return qvariant_cast<QPen>(variant);
}

QBrush KoCharacterStyle::background() const {
    QVariant variant = d->stylesPrivate->value(QTextFormat::BackgroundBrush);

    if(variant.isNull()) {
        QBrush brush;
        return brush;
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearBackground() {
    d->stylesPrivate->remove(QTextCharFormat::BackgroundBrush);
}

QBrush KoCharacterStyle::foreground() const {
    QVariant variant = d->stylesPrivate->value(QTextFormat::ForegroundBrush);
    if(variant.isNull()) {
        QBrush brush;
        return brush;
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearForeground() {
    d->stylesPrivate->remove(QTextCharFormat::ForegroundBrush);
}

void KoCharacterStyle::applyStyle(QTextCharFormat &format) const {
    // copy all relevant properties.
    static const int properties[] = {
        StyleId,
        QTextFormat::FontPointSize,
        QTextCharFormat::ForegroundBrush,
        QTextFormat::FontFamily,
        QTextFormat::FontWeight,
        QTextFormat::FontItalic,
        QTextFormat::FontOverline,
        QTextFormat::FontFixedPitch,
        QTextFormat::TextVerticalAlignment,
        QTextFormat::TextOutline,
        QTextFormat::BackgroundBrush,
        QTextFormat::ForegroundBrush,
        QTextFormat::TextUnderlineColor,
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
        QTextFormat::FontLetterSpacing,
        QTextFormat::FontWordSpacing,
#endif
#if QT_VERSION >= KDE_MAKE_VERSION(4,5,0)
        QTextFormat::FontStyleHint,
        QTextFormat::FontStyleStrategy,
        QTextFormat::FontKerning,
#endif
        KoCharacterStyle::StrikeOutStyle,
        KoCharacterStyle::StrikeOutType,
        KoCharacterStyle::StrikeOutColor,
        KoCharacterStyle::UnderlineStyle,
        KoCharacterStyle::UnderlineType,
        KoCharacterStyle::TransformText,
        KoCharacterStyle::HasHyphenation,
        -1
    };

    QTextCharFormat newFormat;

    int i=0;
    while(properties[i] != -1) {
        QVariant variant = d->stylesPrivate->value(properties[i]);
        if ( !variant.isNull() ) {
            newFormat.setProperty(properties[i], variant);
        }
        i++;
    }

#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
    QVariant variant = d->stylesPrivate->value(TransformText);
    if(!variant.isNull()) {
        switch(static_cast<Transform>(variant.toInt())) {
        case Capitalize:
            newFormat.setFontCapitalization(QFont::Capitalize);
            break;
        case MixedCase:
            newFormat.setFontCapitalization(QFont::MixedCase);
            break;
        case SmallCaps:
            newFormat.setFontCapitalization(QFont::SmallCaps);
            break;
        case AllUppercase:
            newFormat.setFontCapitalization(QFont::AllUppercase);
            break;
        case AllLowercase:
            newFormat.setFontCapitalization(QFont::AllLowercase);
            break;
        }
    }
#endif
    format = newFormat;
}

void KoCharacterStyle::applyStyle(QTextBlock &block) const {
    QTextCursor cursor(block);
    QTextCharFormat cf = cursor.charFormat();
/*
    TODO make replacement of the style be a lot smarter.
    QTextBlock::Iterator fragmentIter = block.begin();
 */
    cursor.setPosition(block.position() + block.length()-1, QTextCursor::KeepAnchor);
    applyStyle(cf);
    // if we want to merge the formats we need a much more clever way to do that
    //cursor.mergeCharFormat(cf);
    cursor.setCharFormat(cf);
    cursor.setBlockCharFormat(cf);
}

void KoCharacterStyle::applyStyle(QTextCursor *selection) const {
    QTextCharFormat cf = selection->charFormat();
    applyStyle(cf);
    // if we want to merge the formats we need a much more clever way to do that
    //selection->mergeCharFormat(cf);
    selection->setCharFormat(cf);
}

// OASIS 14.2.29
static void importOasisLine( const QString& type, const QString& style,
                             KoCharacterStyle::LineStyle& lineStyle, KoCharacterStyle::LineType& lineType )
{
    lineStyle = KoCharacterStyle::NoLineStyle;
    lineType = KoCharacterStyle::NoLineType;
    
    QString fixedType = type;
    QString fixedStyle = style;
    if (fixedType.isEmpty() && !fixedStyle.isEmpty())
        fixedType = "single";
    else if (!fixedType.isEmpty() && fixedStyle.isEmpty())
        fixedStyle = "solid";
    
    if (fixedType == "single")
        lineType = KoCharacterStyle::SingleLine;
    else if (fixedType == "double")
        lineType = KoCharacterStyle::DoubleLine;
    
    if ( fixedStyle == "solid" )
        lineStyle = KoCharacterStyle::SolidLine;
    else if ( fixedStyle == "dotted" )
        lineStyle = KoCharacterStyle::DottedLine;
    else if ( fixedStyle == "dash")
        lineStyle = KoCharacterStyle::DashLine;
    else if ( fixedStyle == "long-dash" )
        lineStyle = KoCharacterStyle::LongDashLine;
    else if ( fixedStyle == "dot-dash" )
        lineStyle = KoCharacterStyle::DotDashLine;
    else if ( fixedStyle == "dot-dot-dash" )
        lineStyle = KoCharacterStyle::DotDotDashLine;
    else if ( fixedStyle == "wave" )
        lineStyle = KoCharacterStyle::WaveLine;
    // TODO bold. But this is another attribute in OASIS (text-underline-width), which makes sense.
    // We should separate them in kotext...
}

static QString exportOasisLineType(KoCharacterStyle::LineType lineType) {
    switch (lineType) {
        case KoCharacterStyle::NoLineType:
            return "none";
        case KoCharacterStyle::SingleLine:
            return "single";
        case KoCharacterStyle::DoubleLine:
            return "double";
        default:
            return "";
    }
}

static QString exportOasisLineStyle(KoCharacterStyle::LineStyle lineStyle) {
    switch (lineStyle) {
        case KoCharacterStyle::NoLineStyle:
            return "none";
        case KoCharacterStyle::SolidLine:
            return "solid";
        case KoCharacterStyle::DottedLine:
            return "dotted";
        case KoCharacterStyle::DashLine:
            return "dash";
        case KoCharacterStyle::LongDashLine:
            return "long-dash";
        case KoCharacterStyle::DotDashLine:
            return "dot-dash";
        case KoCharacterStyle::DotDotDashLine:
            return "dot-dot-dash";
        case KoCharacterStyle::WaveLine:
            return "wave";
        default:
            return "";
    }
}

void KoCharacterStyle::setFontFamily (const QString &family) {
    d->setProperty(QTextFormat::FontFamily, family);
}
QString KoCharacterStyle::fontFamily () const {
    return d->propertyString(QTextFormat::FontFamily);
}
void KoCharacterStyle::setFontPointSize (qreal size) {
    d->setProperty(QTextFormat::FontPointSize, size);
}
double KoCharacterStyle::fontPointSize () const {
    return d->propertyDouble(QTextFormat::FontPointSize);
}
void KoCharacterStyle::setFontWeight (int weight) {
    d->setProperty(QTextFormat::FontWeight, weight);
}
int KoCharacterStyle::fontWeight () const {
    return d->propertyInt(QTextFormat::FontWeight);
}
void KoCharacterStyle::setFontItalic (bool italic) {
    d->setProperty(QTextFormat::FontItalic, italic);
}
bool KoCharacterStyle::fontItalic () const {
    return d->propertyBoolean(QTextFormat::FontItalic);
}
void KoCharacterStyle::setFontOverline (bool overline) {
    d->setProperty(QTextFormat::FontOverline, overline);
}
bool KoCharacterStyle::fontOverline () const {
    return d->propertyBoolean(QTextFormat::FontOverline);
}
void KoCharacterStyle::setFontFixedPitch (bool fixedPitch) {
    d->setProperty(QTextFormat::FontFixedPitch, fixedPitch);
}
bool KoCharacterStyle::fontFixedPitch () const {
    return d->propertyBoolean(QTextFormat::FontFixedPitch);
}
#if QT_VERSION >= KDE_MAKE_VERSION(4,5,0)
void KoCharacterStyle::setFontStyleHint(QFont::StyleHint styleHint) {
    d->setProperty(QTextFormat::FontStyleHint, styleHint);
}
QFont::StyleHint KoCharacterStyle::fontStyleHint() const {
    return static_cast<QFont::StyleHint>(d->propertyInt(QTextFormat::FontStyleHint));
}
void KoCharacterStyle::setFontKerning(bool enable) {
    d->setProperty(QTextFormat::FontKerning, enable);
}
bool KoCharacterStyle::fontKerning() const {
    return d->propertyBoolean(QTextFormat::FontKerning);
}
#endif
void KoCharacterStyle::setVerticalAlignment (QTextCharFormat::VerticalAlignment alignment) {
    d->setProperty(QTextFormat::TextVerticalAlignment, alignment);
}
QTextCharFormat::VerticalAlignment KoCharacterStyle::verticalAlignment () const {
    return static_cast<QTextCharFormat::VerticalAlignment> (d->propertyInt(QTextFormat::TextVerticalAlignment));
}
void KoCharacterStyle::setTextOutline (const QPen &pen) {
    d->setProperty(QTextFormat::TextOutline, pen);
}
void KoCharacterStyle::setBackground (const QBrush &brush) {
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}
void KoCharacterStyle::setForeground (const QBrush &brush) {
    d->setProperty(QTextFormat::ForegroundBrush, brush);
}
QString KoCharacterStyle::name() const {
    return d->name;
}
void KoCharacterStyle::setName(const QString &name) {
    d->name = name;
}
int KoCharacterStyle::styleId() const {
    return d->propertyInt(StyleId);
}
void KoCharacterStyle::setStyleId(int id) {
    d->setProperty(StyleId, id);
}
QFont KoCharacterStyle::font() const {
    QFont font;
    if(d->stylesPrivate->contains(QTextFormat::FontFamily))
        font.setFamily(fontFamily());
    if(d->stylesPrivate->contains(QTextFormat::FontPointSize))
        font.setPointSizeF(fontPointSize());
    if(d->stylesPrivate->contains(QTextFormat::FontWeight))
        font.setWeight(fontWeight());
    if(d->stylesPrivate->contains(QTextFormat::FontItalic))
        font.setItalic(fontItalic());
    return font;
}
void KoCharacterStyle::setHasHyphenation(bool on) {
    d->setProperty(HasHyphenation, on);
}
bool KoCharacterStyle::hasHyphenation() const {
    return d->propertyBoolean(HasHyphenation);
}

void KoCharacterStyle::setStrikeOutStyle (KoCharacterStyle::LineStyle strikeOut) {
    d->setProperty(StrikeOutStyle, strikeOut);
}

KoCharacterStyle::LineStyle KoCharacterStyle::strikeOutStyle () const {
    return (KoCharacterStyle::LineStyle) d->propertyInt(StrikeOutStyle);
}

void KoCharacterStyle::setStrikeOutType (LineType lineType) {
    d->setProperty(StrikeOutType, lineType);
}

KoCharacterStyle::LineType KoCharacterStyle::strikeOutType () const {
    return (KoCharacterStyle::LineType) d->propertyInt(StrikeOutType);
}

void KoCharacterStyle::setStrikeOutColor (const QColor &color) {
    d->setProperty(StrikeOutColor, color);
}

QColor KoCharacterStyle::strikeOutColor () const {
    return d->propertyColor(StrikeOutColor);
}


void KoCharacterStyle::setUnderlineStyle (KoCharacterStyle::LineStyle underline) {
    d->setProperty(UnderlineStyle, underline);
}

KoCharacterStyle::LineStyle KoCharacterStyle::underlineStyle () const {
    return (KoCharacterStyle::LineStyle) d->propertyInt(UnderlineStyle);
}

void KoCharacterStyle::setUnderlineType (LineType lineType) {
    d->setProperty(UnderlineType, lineType);
}

KoCharacterStyle::LineType KoCharacterStyle::underlineType () const {
    return (KoCharacterStyle::LineType) d->propertyInt(UnderlineType);
}

void KoCharacterStyle::setUnderlineColor (const QColor &color) {
    d->setProperty(QTextFormat::TextUnderlineColor, color);
}

QColor KoCharacterStyle::underlineColor () const {
    return d->propertyColor(QTextFormat::TextUnderlineColor);
}

void KoCharacterStyle::setFontLetterSpacing(qreal spacing) {
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
    d->setProperty(QTextCharFormat::FontLetterSpacing, spacing);
#else
    Q_UNUSED(spacing);
#endif
}

qreal KoCharacterStyle::fontLetterSpacing() const {
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
    return d->propertyDouble(QTextCharFormat::FontLetterSpacing);
#else
    return 0.0;
#endif
}

void KoCharacterStyle::setFontWordSpacing(qreal spacing) {
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
    d->setProperty(QTextCharFormat::FontWordSpacing, spacing);
#else
    Q_UNUSED(spacing);
#endif
}

qreal KoCharacterStyle::fontWordSpacing() const {
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
    return d->propertyDouble(QTextCharFormat::FontWordSpacing);
#else
    return 0.0;
#endif
}


void KoCharacterStyle::setTransform(KoCharacterStyle::Transform transformtext) {
    d->setProperty(KoCharacterStyle::TransformText, transformtext);
}

KoCharacterStyle::Transform KoCharacterStyle::transform() const {
    return (KoCharacterStyle::Transform) d->propertyInt(KoCharacterStyle::TransformText);
}

bool KoCharacterStyle::hasProperty(int key) const {
    return d->stylesPrivate->contains(key);
}

//in 1.6 this was defined in KoTextFormat::load(KoOasisContext& context)
void KoCharacterStyle::loadOasis(KoOdfLoadingContext& context) {
    KoStyleStack &styleStack = context.styleStack();

    // The fo:color attribute specifies the foreground color of text.
    if ( styleStack.hasProperty( KoXmlNS::fo, "color" ) ) { // 3.10.3
        if (styleStack.property( KoXmlNS::style, "use-window-font-color") != "true") {
            QColor color(styleStack.property( KoXmlNS::fo, "color" )); // #rrggbb format
            if ( color.isValid() ) {
                QBrush brush = foreground();
                brush.setColor(color);
                setForeground(brush);
            }
        }
    }

    QString fontName;
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-family" ) ) {
        fontName = styleStack.property( KoXmlNS::fo, "font-family" );

        // Specify whether a font has a fixed or variable width.
        // These attributes are ignored if there is no corresponding fo:font-family attribute attached to the same formatting properties element.
        if ( styleStack.hasProperty( KoXmlNS::style, "font-pitch" ) ) {
            if ( styleStack.property( KoXmlNS::style, "font-pitch" ) == "fixed" )
                setFontFixedPitch( true );
        }

#if QT_VERSION >= KDE_MAKE_VERSION(4,5,0)
        if (styleStack.hasProperty(KoXmlNS::style, "font-family-generic")) {
            QString genericFamily = styleStack.property(KoXmlNS::style, "font-family-generic");
            if (genericFamily == "roman")
                setFontStyleHint(QFont::Serif);
            else if (genericFamily == "swiss")
                setFontStyleHint(QFont::SansSerif);
            else if (genericFamily == "modern")
                setFontStyleHint(QFont::TypeWriter);
            else if (genericFamily == "decorative")
                setFontStyleHint(QFont::Decorative);
            else if (genericFamily == "system")
                setFontStyleHint(QFont::System);
            else if (genericFamily == "script")
                ; // TODO: no hint available in Qt yet
        }
#endif
    }
    if ( styleStack.hasProperty( KoXmlNS::style, "font-family" ) )
        fontName = styleStack.property( KoXmlNS::style, "font-family" );
    if ( styleStack.hasProperty( KoXmlNS::style, "font-name" ) ) {
        // This font name is a reference to a font face declaration.
        KoOdfStylesReader &stylesReader = context.stylesReader();
        const KoXmlElement *fontFace = stylesReader.findStyle(styleStack.property( KoXmlNS::style, "font-name" ));
        if (fontFace != 0)
            fontName = fontFace->attributeNS(KoXmlNS::svg, "font-family", "");
    }

    if ( ! fontName.isNull() ) {
        // Hmm, the remove "'" could break it's in the middle of the fontname...
        fontName = fontName.remove( "'" );

        // 'Thorndale' is not known outside OpenOffice so we substitute it
        // with 'Times New Roman' that looks nearly the same.
        if ( fontName == "Thorndale" )
            fontName = "Times New Roman";

        fontName.remove(QRegExp("\\sCE$")); // Arial CE -> Arial
        setFontFamily( fontName );
    }

    // Specify the size of a font. The value of these attribute is either an absolute length or a percentage
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-size" ) ) {
        double pointSize = styleStack.fontSize( fontPointSize() );
        if (pointSize > 0)
            setFontPointSize(pointSize);
    }

    // These attributes specify a relative font size change as a length such as +1pt, -3pt. It changes the font size based on the font size of the parent style.
    if ( styleStack.hasProperty( KoXmlNS::style, "font-size-rel" ) ) {
        double pointSize = fontPointSize() + KoUnit::parseValue( styleStack.property( KoXmlNS::style, "font-size-rel" ) );
        if (pointSize > 0)
            setFontPointSize(pointSize);
    }

    // Specify the weight of a font. The permitted values are normal, bold, and numeric values 100-900, in steps of 100. Unsupported numerical values are rounded off to the next supported value.
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-weight" ) ) { // 3.10.24
        QString fontWeight = styleStack.property( KoXmlNS::fo, "font-weight" );
        int boldness;
        if ( fontWeight == "normal" )
            boldness = 50;
        else if ( fontWeight == "bold" )
            boldness = 75;
        else
            // XSL/CSS has 100,200,300...900. Not the same scale as Qt!
            // See http://www.w3.org/TR/2001/REC-xsl-20011015/slice7.html#font-weight
            boldness = fontWeight.toInt() / 10;
        setFontWeight( boldness );
    }

    // Specify whether to use normal or italic font face.
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-style" ) ) { // 3.10.19
        if ( styleStack.property( KoXmlNS::fo, "font-style" ) == "italic" ||
             styleStack.property( KoXmlNS::fo, "font-style" ) == "oblique" ) { // no difference in kotext
            setFontItalic( true );
        }
    }

//TODO
#if 0
    d->m_bWordByWord = styleStack.property( KoXmlNS::style, "text-underline-mode" ) == "skip-white-space";
    // TODO style:text-line-through-mode

    /*
    // OO compat code, to move to OO import filter
    d->m_bWordByWord = (styleStack.hasProperty( KoXmlNS::fo, "score-spaces")) // 3.10.25
                      && (styleStack.property( KoXmlNS::fo, "score-spaces") == "false");
    if( styleStack.hasProperty( KoXmlNS::style, "text-crossing-out" )) { // 3.10.6
        QString strikeOutType = styleStack.property( KoXmlNS::style, "text-crossing-out" );
        if( strikeOutType =="double-line")
            m_strikeOutType = S_DOUBLE;
        else if( strikeOutType =="single-line")
            m_strikeOutType = S_SIMPLE;
        else if( strikeOutType =="thick-line")
            m_strikeOutType = S_SIMPLE_BOLD;
        // not supported by KWord: "slash" and "X"
        // not supported by OO: stylelines (solid, dash, dot, dashdot, dashdotdot)
    }
    */
#endif

    // Specifies whether text is underlined, and if so, whether a single or double line will be used for underlining.
    if ( styleStack.hasProperty( KoXmlNS::style, "text-underline-type" )
        || styleStack.hasProperty( KoXmlNS::style, "text-underline-style" ) ) { // OASIS 14.4.28
        LineStyle underlineStyle;
        LineType underlineType;
    
        importOasisLine(styleStack.property( KoXmlNS::style, "text-underline-type" ),
                        styleStack.property( KoXmlNS::style, "text-underline-style" ),
                        underlineStyle, underlineType);
        setUnderlineStyle(underlineStyle);
        setUnderlineType(underlineType);
    }

    // Specifies the color that is used to underline text. The value of this attribute is either font-color or a color. If the value is font-color, the current text color is used for underlining.
    QString underLineColor = styleStack.property( KoXmlNS::style, "text-underline-color" ); // OO 3.10.23, OASIS 14.4.31
    if ( !underLineColor.isEmpty() && underLineColor != "font-color" )
        setUnderlineColor( QColor(underLineColor) );
    
    
    if (( styleStack.hasProperty( KoXmlNS::style, "text-line-through-type" ) ) ||  ( styleStack.hasProperty( KoXmlNS::style, "text-line-through-style" ))) { // OASIS 14.4.7
        KoCharacterStyle::LineStyle throughStyle;
        LineType throughType;
        
        importOasisLine(styleStack.property( KoXmlNS::style, "text-line-through-type" ),
                        styleStack.property( KoXmlNS::style, "text-line-through-style" ),
                        throughStyle, throughType);
        
        setStrikeOutStyle(throughStyle);
        setStrikeOutType(throughType);
    }
    
    QString lineThroughColor = styleStack.property( KoXmlNS::style, "text-line-through-color" ); // OO 3.10.23, OASIS 14.4.31
    if ( !lineThroughColor.isEmpty() && lineThroughColor != "font-color" )
        setStrikeOutColor( QColor(lineThroughColor) );
//TODO
#if 0
    if ( styleStack.hasProperty( KoXmlNS::style, "text-line-through-type" ) ) { // OASIS 14.4.7
        // Reuse code for loading underlines, and convert to strikeout enum (if not wave)
        UnderlineType uType; UnderlineStyle uStyle;
        importOasisUnderline( styleStack.property( KoXmlNS::style, "text-line-through-type" ),
                              styleStack.property( KoXmlNS::style, "text-line-through-style" ),
                              uType, uStyle );
        m_strikeOutType = S_NONE;
        if ( uType != U_WAVE )
            m_strikeOutType = (StrikeOutType)uType;
        m_strikeOutStyle = (StrikeOutStyle)uStyle;
    }

    // Text position
    va = AlignNormal;
    d->m_relativeTextSize = 0.58;
    d->m_offsetFromBaseLine = 0;
    if( styleStack.hasProperty( KoXmlNS::style, "text-position")) { // OO 3.10.7
        importTextPosition( styleStack.property( KoXmlNS::style, "text-position"), fn.pointSizeFloat(),
                            va, d->m_relativeTextSize, d->m_offsetFromBaseLine, context );
    }
#endif
    if( styleStack.hasProperty( KoXmlNS::style, "text-position")) { // OO 3.10.7
        QString textPosition = styleStack.property( KoXmlNS::style, "text-position");
        if (textPosition.startsWith("super"))
            setVerticalAlignment(QTextCharFormat::AlignSuperScript);
        else if (textPosition.startsWith("sub"))
            setVerticalAlignment(QTextCharFormat::AlignSubScript);
    }
    // The fo:font-variant attribute provides the option to display text as small capitalized letters.
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-variant" ) ) {
        QString textVariant = styleStack.property(KoXmlNS::fo, "font-variant");
        if (textVariant == "small-caps")
            setTransform(SmallCaps);
        else if (textVariant == "normal")
            setTransform(MixedCase);
    }
    // The fo:text-transform attribute specifies text transformations to uppercase, lowercase, and capitalization.
    else if ( styleStack.hasProperty( KoXmlNS::fo, "text-transform" ) ) {
        QString textTransform = styleStack.property( KoXmlNS::fo, "text-transform" );
        if ( textTransform == "uppercase" )
            setTransform( AllUppercase );
        else if ( textTransform == "lowercase" )
            setTransform( AllLowercase );
        else if ( textTransform == "capitalize" )
            setTransform( Capitalize );
    }

#if 0
    if ( styleStack.hasProperty( KoXmlNS::fo, "language") ) { // 3.10.17
        m_language = styleStack.property( KoXmlNS::fo, "language");
        const QString country = styleStack.property( KoXmlNS::fo, "country" );
        if ( !country.isEmpty() ) {
            m_language += '_';
            m_language += country;
        }
    }
#endif

    // The fo:background-color attribute specifies the background color of a paragraph.
    if ( styleStack.hasProperty( KoXmlNS::fo, "background-color") ) {
        const QString bgcolor = styleStack.property( KoXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
            brush.setStyle(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
        }
        setBackground(brush);
    }

    // The style:use-window-font-color attribute specifies whether or not the window foreground color should be as used as the foreground color for a light background color and white for a dark background color.
    if ( styleStack.hasProperty( KoXmlNS::style, "use-window-font-color" ) ) {
        if (styleStack.property( KoXmlNS::style, "use-window-font-color") == "true") {
            // Do like OpenOffice.org : change the foreground font if its color is too close to the background color...
            QColor back = background().color();
            QColor front = foreground().color();
            if ((abs(qGray(back.rgb()) - qGray(front.rgb())) < 10) && (background().style() != Qt::NoBrush) && (foreground().style() != Qt::NoBrush)) {
                front.setRed(255 - front.red());
                front.setGreen(255 - front.green());
                front.setBlue(255 - front.blue());
                QBrush frontBrush = foreground();
                frontBrush.setColor(front);
                setForeground(frontBrush);
            }
        }
    }

#if QT_VERSION >= KDE_MAKE_VERSION(4,5,0)
    if (styleStack.hasProperty(KoXmlNS::style, "letter-kerning")) {
        if (styleStack.property(KoXmlNS::style, "letter-kerning") == "true") {
            setFontKerning(true);
        } else {
            setFontKerning(false);
        }
    }
#endif
//TODO
#if 0
    if ( styleStack.hasProperty( KoXmlNS::fo, "text-shadow") ) { // 3.10.21
        parseShadowFromCss( styleStack.property( KoXmlNS::fo, "text-shadow") );
    }

    d->m_bHyphenation = true;
    if ( styleStack.hasProperty( KoXmlNS::fo, "hyphenate" ) ) // it's a character property in OASIS (but not in OO-1.1)
        d->m_bHyphenation = styleStack.property( KoXmlNS::fo, "hyphenate" ) == "true";

    /*
      Missing properties:
      style:text-outline, 3.10.5 - not implemented in kotext
      style:font-style-name, 3.10.11 - can be ignored, says DV, the other ways to specify a font are more precise
      style:font-charset, 3.10.14 - not necessary with Qt
      fo:letter-spacing, 3.10.16 - not implemented in kotext
      style:text-relief, 3.10.20 - not implemented in kotext
      style:text-blinking, 3.10.27 - not implemented in kotext IIRC
      style:text-combine, 3.10.29/30 - not implemented, see http://www.w3.org/TR/WD-i18n-format/
      style:text-emphasis, 3.10.31 - not implemented in kotext
      style:text-scale, 3.10.33 - not implemented in kotext
      style:text-rotation-angle, 3.10.34 - not implemented in kotext (kpr rotates whole objects)
      style:text-rotation-scale, 3.10.35 - not implemented in kotext (kpr rotates whole objects)
      style:punctuation-wrap, 3.10.36 - not implemented in kotext
    */

    d->m_underLineWidth = 1.0;

    generateKey();
    addRef();
#endif

}

bool KoCharacterStyle::operator==( const KoCharacterStyle &other ) const {
    return ((*(other.d->stylesPrivate)) == (*(this->d->stylesPrivate)));
}

void KoCharacterStyle::removeDuplicates ( const KoCharacterStyle &other ) {
    this->d->stylesPrivate->removeDuplicates(other.d->stylesPrivate);
}

void KoCharacterStyle::saveOdf( KoGenStyle &style )
{
    QList<int> keys = d->stylesPrivate->keys();
    foreach (int key, keys) {
        if (key == QTextFormat::FontWeight) {
            bool ok = false;
            int boldness = d->stylesPrivate->value(key).toInt(&ok);
            if (ok) {
                if (boldness == 50) {
                    style.addProperty("fo:font-weight", "normal", KoGenStyle::TextType);
                } else if (boldness == 75) {
                    style.addProperty("fo:font-weight", "bold", KoGenStyle::TextType);
                } else {
                    // Remember : Qt and CSS/XSL doesn't have the same scale...
                    style.addProperty("fo:font-weight", boldness*10, KoGenStyle::TextType);
                }
            }
        } else if (key == QTextFormat::FontItalic) {
            if (d->stylesPrivate->value(key).toBool()) {
                style.addProperty("fo:font-style", "italic", KoGenStyle::TextType);
            } else {
                style.addProperty("fo:font-style", "", KoGenStyle::TextType);
            }
        } else if (key == UnderlineStyle) {
            bool ok = false;
            int styleId = d->stylesPrivate->value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-underline-style", exportOasisLineStyle((KoCharacterStyle::LineStyle) styleId), KoGenStyle::TextType);
        } else if (key == UnderlineType) {
            bool ok = false;
            int type = d->stylesPrivate->value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-underline-type", exportOasisLineType((KoCharacterStyle::LineType) type), KoGenStyle::TextType);
        } else if (key == QTextFormat::TextUnderlineColor) {
            QColor color = d->stylesPrivate->value(key).value<QColor>();
            if (color.isValid())
                style.addProperty("style:text-underline-color", color.name(), KoGenStyle::TextType);
        } else if (key == StrikeOutStyle) {
            bool ok = false;
            int styleId = d->stylesPrivate->value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-line-through-style", exportOasisLineStyle((KoCharacterStyle::LineStyle) styleId), KoGenStyle::TextType);
        } else if (key == StrikeOutType) {
            bool ok = false;
            int type = d->stylesPrivate->value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-line-through-type", exportOasisLineType((KoCharacterStyle::LineType) type), KoGenStyle::TextType);
        } else if (key == StrikeOutColor) {
            QColor color = d->stylesPrivate->value(key).value<QColor>();
            if (color.isValid())
                style.addProperty("style:text-line-through-color", color.name(), KoGenStyle::TextType);
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush brush = d->stylesPrivate->value(key).value<QBrush>();
            if (brush.style() == Qt::NoBrush)
                style.addProperty("fo:background-color", "transparent", KoGenStyle::TextType);
            else
                style.addProperty("fo:background-color", brush.color().name(), KoGenStyle::TextType);
        } else if (key == QTextFormat::ForegroundBrush) {
            QBrush brush = d->stylesPrivate->value(key).value<QBrush>();
            if (brush.style() == Qt::NoBrush)
                style.addProperty("fo:color", "transparent", KoGenStyle::TextType);
            else
                style.addProperty("fo:color", brush.color().name(), KoGenStyle::TextType);
        } else if (key == QTextFormat::TextVerticalAlignment) {
            if (verticalAlignment() == QTextCharFormat::AlignSuperScript)
                style.addProperty("style:text-position", "super", KoGenStyle::TextType);
            else if (verticalAlignment() == QTextCharFormat::AlignSubScript)
                style.addProperty("style:text-position", "sub", KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TransformText) {
            Transform transform = (Transform) d->stylesPrivate->value(key).value<int>();
            switch (transform) {
                case SmallCaps:
                    style.addProperty("fo:font-variant", "small-caps", KoGenStyle::TextType);
                    break;
                case AllUppercase:
                    style.addProperty("fo:text-transform", "uppercase", KoGenStyle::TextType);
                    break;
                case AllLowercase:
                    style.addProperty("fo:text-transform", "lowercase", KoGenStyle::TextType);
                    break;
                case Capitalize:
                    style.addProperty("fo:text-transform", "capitalize", KoGenStyle::TextType);
                    break;
                case MixedCase:
                    style.addProperty("fo:text-transform", "normal", KoGenStyle::TextType);
                    break;
            }
        } else if (key == QTextFormat::FontPointSize) {
            style.addPropertyPt("fo:font-size", fontPointSize(), KoGenStyle::TextType);
        }
    }
    //TODO: font name and family
}

#include "KoCharacterStyle.moc"
