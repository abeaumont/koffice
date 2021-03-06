/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2003 Norbert Andres <nandres@web.de>

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

#ifndef KC_STYLE_H
#define KC_STYLE_H

#include <QtGui/QFont>
#include <QtCore/QSharedDataPointer>
#include <QtGui/QTextCharFormat>

#include <KXmlReader.h>

#include "KCCurrency.h"
#include "KCFormat.h"
#include "Global.h"

class KOdfGenericStyle;
class KOdfGenericStyles;
class KOdfStylesReader;
class KOdfStyleStack;

class KCConditions;
class KCCustomStyle;
class KCSharedSubStyle;
class KCStyle;
class KCStyleManager;
class StyleManipulator;
class KCSubStyle;
class KCValueParser;

// used for preloading OASIS auto styles
typedef QHash<QString, KCStyle>       Styles;
// needs to be ordered (QMap) for the style dialog
typedef QMap<QString, KCCustomStyle*> CustomStyles;

KCELLS_EXPORT uint qHash(const KCStyle& style);

/**
 * \ingroup KCStyle
 * A cell style.
 */
class KCELLS_EXPORT KCStyle
{
public:
    enum HAlign {
        Left = 1,
        Center = 2,
        Right = 3,
        Justified = 4,
        HAlignUndefined = 0
    };

    enum VAlign {
        Top = 1,
        Middle = 2,
        Bottom = 3,
        VDistributed = 4,
        VJustified = 5,
        VAlignUndefined = 0
    };

    enum FloatFormat {
        DefaultFloatFormat = 0,
        AlwaysSigned = 1,
        AlwaysUnsigned = 2,
        OnlyNegSigned = DefaultFloatFormat
    };

    enum FloatColor {
        DefaultFloatColor = 0,
        NegRed = 1,
        AllBlack = DefaultFloatColor,
        NegBrackets = 3,
        NegRedBrackets = 4
    };

    /// The style type
    enum StyleType {
        BUILTIN,   ///< built-in style (the default style)
        CUSTOM,    ///< custom style (defined in the KCStyleManager dialog)
        AUTO,      ///< automatically generated on cell format changes
        TENTATIVE  ///< @internal temporary state
    };

    enum Key {
        // special cases
        DefaultStyleKey,
        /**
         * The NamedStyleKey has two functions:
         * \li In KCStyle: On lookup the KCStyle is composed (\see KCStyleStorage::composeStyle()).
         *               For named styles just the style's name is stored. On lookup, these styles
         *               are looked up in the KCStyleManager and their attributes, including all
         *               attributes of the parents, are filled in the composed style. Additionally,
         *               the name of the named style is stored in the NamedStyleKey attribute.
         *               This is especially useful while saving the styles.
         * \li In KCCustomStyle: It simply refers to the parent style.
         */
        NamedStyleKey,
        // borders
        LeftPen,
        RightPen,
        TopPen,
        BottomPen,
        FallDiagonalPen,
        GoUpDiagonalPen,
        // layout
        HorizontalAlignment,
        VerticalAlignment,
        MultiRow,
        VerticalText,
        Angle,
        ShrinkToFit,
        Indentation,
        // content format
        Prefix,
        Postfix,
        Precision,
        FormatTypeKey,
        FloatFormatKey,
        FloatColorKey,
        CurrencyFormat,
        CustomFormat,
        // background
        BackgroundBrush,
        BackgroundColor,
        // font
        FontColor,
        FontFamily,
        FontSize,
        FontBold,
        FontItalic,
        FontStrike,
        FontUnderline,
        //misc
        DontPrintText,
        NotProtected,
        HideAll,
        HideFormula
    };

    KCStyle();
    KCStyle(const KCStyle& style);
    virtual ~KCStyle();

    virtual StyleType type() const;

    QString parentName() const;
    void setParentName(const QString& name);


    bool loadXML(KXmlElement& format, Paste::Mode pm = Paste::Normal);
    void saveXML(QDomDocument& doc, QDomElement& format, const KCStyleManager* styleManager) const;
    void loadOdfStyle(KOdfStylesReader& stylesReader, const KXmlElement& element,
                      KCConditions& conditions, const KCStyleManager* styleManager,
                      const KCValueParser *parser);
    void loadOdfDataStyle(KOdfStylesReader& stylesReader, const QString& dataStyleName,
                          KCConditions& conditions, const KCStyleManager* styleManager,
                          const KCValueParser *parser);

    /**
     * Saves an OASIS automatic style.
     * Reimplemented by KCCustomStyle for OASIS user styles.
     * \return the OASIS style's name
     */
    virtual QString saveOdf(KOdfGenericStyle& style, KOdfGenericStyles& mainStyles,
                            const KCStyleManager* manager) const;


    void clearAttribute(Key key);
    bool hasAttribute(Key key) const;
    void loadAttributes(const QList<KCSharedSubStyle>& subStyles);


    uint bottomPenValue() const;
    uint rightPenValue() const;
    uint leftPenValue() const;
    uint topPenValue() const;

    QColor  fontColor()       const;
    QColor  backgroundColor() const;
    QPen    rightBorderPen()  const;
    QPen    bottomBorderPen() const;
    QPen    leftBorderPen()   const;
    QPen    topBorderPen()    const;
    QPen    fallDiagonalPen() const;
    QPen    goUpDiagonalPen() const;
    QBrush  backgroundBrush() const;
    QString customFormat()    const;
    QString prefix()          const;
    QString postfix()         const;
    QString fontFamily()      const;

    HAlign      halign()      const;
    VAlign      valign()      const;
    FloatFormat floatFormat() const;
    FloatColor  floatColor()  const;
    KCFormat::Type  formatType()  const;

    KCCurrency currency() const;

    QFont  font()         const;
    bool   bold()         const;
    bool   italic()       const;
    bool   underline()    const;
    bool   strikeOut()    const;
    uint   fontFlags()    const;
    int    fontSize()     const;
    int    precision()    const;
    int    angle()        const;
    double indentation()  const;
    bool   shrinkToFit()  const;
    bool   verticalText() const;
    bool   wrapText()     const;
    bool   printText()    const;
    bool   hideAll()      const;
    bool   hideFormula()  const;
    bool   notProtected() const;
    bool   isDefault()    const;
    bool   isEmpty()      const;

protected:
    /**
     * Helper function for saveOdf
     * Does the real work by determining the used attributes.
     */
    void saveOdfStyle(const QSet<Key>& subStyles, KOdfGenericStyle &style,
                      KOdfGenericStyles &mainStyles, const KCStyleManager* manager) const;

    void loadOdfDataStyle(KOdfStylesReader& stylesReader, const KXmlElement& element,
                          KCConditions& conditions, const KCStyleManager* styleManager,
                          const KCValueParser *parser);
    void loadOdfParagraphProperties(KOdfStylesReader& stylesReader, const KOdfStyleStack& element);
    void loadOdfTableCellProperties(KOdfStylesReader& stylesReader, const KOdfStyleStack& element);
    void loadOdfTextProperties(KOdfStylesReader& stylesReader, const KOdfStyleStack& element);

public:
    void setHAlign(HAlign align);
    void setVAlign(VAlign align);
    void setFont(QFont const & font);
    void setFontFamily(QString const & fam);
    void setFontBold(bool enable);
    void setFontItalic(bool enable);
    void setFontUnderline(bool enable);
    void setFontStrikeOut(bool enable);
    void setFontSize(int size);
    void setFontColor(QColor const & color);
    void setRightBorderPen(QPen const & pen);
    void setBottomBorderPen(QPen const & pen);
    void setLeftBorderPen(QPen const & pen);
    void setTopBorderPen(QPen const & pen);
    void setFallDiagonalPen(QPen const & pen);
    void setGoUpDiagonalPen(QPen const & pen);
    void setAngle(int angle);
    void setIndentation(double indent);
    void setBackgroundBrush(QBrush const & brush);
    void setFloatFormat(FloatFormat format);
    void setFloatColor(FloatColor color);
    void setFormatType(KCFormat::Type format);
    void setCustomFormat(QString const & strFormat);
    void setPrecision(int precision);
    void setPrefix(QString const & prefix);
    void setPostfix(QString const & postfix);
    void setCurrency(KCCurrency const & currency);
    void setWrapText(bool enable);
    void setHideAll(bool enable);
    void setHideFormula(bool enable);
    void setNotProtected(bool enable);
    void setDontPrintText(bool enable);
    void setVerticalText(bool enable);
    void setShrinkToFit(bool enable);
    void setBackgroundColor(QColor const & color);
    void setDefault();
    void clear();


    // static functions
    //
    static KCFormat::Type dateType(const QString&);
    static KCFormat::Type timeType(const QString&);
    static KCFormat::Type fractionType(const QString&);
    static KCFormat::Type numberType(const QString&);
    static KCCurrency numberCurrency(const QString&);

    /**
     * @return the name of the data style (number, currency, percentage, date,
     * boolean, text)
     */
    static QString saveOdfStyleNumeric(KOdfGenericStyle &style, KOdfGenericStyles &mainStyles, KCFormat::Type _style,
                                       const QString &_prefix, const QString &_postfix, int _precision, const QString& symbol);
    static QString saveOdfStyleNumericDate(KOdfGenericStyles &mainStyles, KCFormat::Type _style,
                                           const QString &_prefix, const QString &_suffix);
    static QString saveOdfStyleNumericFraction(KOdfGenericStyles &mainStyles, KCFormat::Type _style,
            const QString &_prefix, const QString &_suffix);
    static QString saveOdfStyleNumericTime(KOdfGenericStyles& mainStyles, KCFormat::Type _style,
                                           const QString &_prefix, const QString &_suffix);
    static QString saveOdfStyleNumericCustom(KOdfGenericStyles&mainStyles, KCFormat::Type _style,
            const QString &_prefix, const QString &_suffix);
    static QString saveOdfStyleNumericScientific(KOdfGenericStyles&mainStyles, KCFormat::Type _style,
            const QString &_prefix, const QString &_suffix, int _precision);
    static QString saveOdfStyleNumericPercentage(KOdfGenericStyles&mainStyles, KCFormat::Type _style, int _precision,
            const QString &_prefix, const QString &_suffix);
    static QString saveOdfStyleNumericMoney(KOdfGenericStyles&mainStyles, KCFormat::Type _style,
                                            const QString& symbol, int _precision,
                                            const QString &_prefix, const QString &_suffix);
    static QString saveOdfStyleNumericText(KOdfGenericStyles&mainStyles, KCFormat::Type _style, int _precision,
                                           const QString &_prefix, const QString &_suffix);
    static QString saveOdfStyleNumericNumber(KOdfGenericStyles&mainStyles, KCFormat::Type _style, int _precision,
            const QString &_prefix, const QString &_suffix);
    static QString saveOdfBackgroundStyle(KOdfGenericStyles &mainStyles, const QBrush &brush);

    /**
     * Returns the name of a color.  This is the same as returned by QColor::name, but an internal cache
     * is used to reduce the overhead when asking for the name of the same color.
     */
    static QString colorName(const QColor& color);

    static bool compare(const KCSubStyle* one, const KCSubStyle* two);


    /** Returns true if both styles have the same properties */
    bool operator== (const KCStyle& style) const;
    inline bool operator!=(const KCStyle& other) const {
        return !operator==(other);
    }
    friend uint qHash(const KCStyle& style);
    void operator=(const KCStyle& style);
    KCStyle operator-(const KCStyle& style) const;
    void merge(const KCStyle& style);

    /**
     * The keys, that are contained in this style, but not in \p other and
     * the keys, that are contained in both but differ in value.
     * \return a set of keys, in which this style and \p other differ.
     */
    QSet<Key> difference(const KCStyle& other) const;

    void dump() const;

    /**
     * Return the properties of this style that can be represented as a QTextCharFormat
     */
    QTextCharFormat asCharFormat() const;
protected:
    QList<KCSharedSubStyle> subStyles() const;

    KCSharedSubStyle createSubStyle(Key key, const QVariant& value);
    virtual void insertSubStyle(Key key, const QVariant& value);
    void insertSubStyle(const KCSharedSubStyle& subStyle);
    bool releaseSubStyle(Key key);

private:
    friend class KCStyleStorage;
    friend class KCBorderColorCommand;

    class Private;
    QSharedDataPointer<Private> d;
};



/**
 * \ingroup KCStyle
 * A named cell style.
 */
class KCELLS_EXPORT KCCustomStyle : public KCStyle
{
public:
    /**
     * Constructor.
     * Creates a custom style.
     * \param name The name of this style.
     * \param parent The style whose attributes are inherited - the parent style.
     */
    explicit KCCustomStyle(const QString& name, KCCustomStyle* parent = 0);
    virtual ~KCCustomStyle();

    virtual StyleType type() const;
    void setType(StyleType type);

    void setName(QString const & name);
    QString const & name() const;

    bool loadXML(KXmlElement const & style, QString const & name);
    void save(QDomDocument & doc, QDomElement & styles, const KCStyleManager* styleManager);

    /**
     * Loads the style properties from @p style .
     * Determines also the parent's name.
     * @param stylesReader map of all styles
     * @param style the DOM element defining the style
     * @param name the style's new name
     */
    void loadOdf(KOdfStylesReader& stylesReader, const KXmlElement& style,
                 const QString& name, KCConditions& conditions,
                 const KCStyleManager* styleManager, const KCValueParser *parser);

    /**
     * @reimp
     * Stores an OASIS user style.
     * @return the OASIS style's name
     */
    virtual QString saveOdf(KOdfGenericStyle& style, KOdfGenericStyles &mainStyles,
                            const KCStyleManager* manager) const;


    bool operator==(const KCCustomStyle& other) const;
    inline bool operator!=(const KCCustomStyle& other) const {
        return !operator==(other);
    }

    /**
     * @return the number of references to this style.
     */
    int usage() const;

private:
    friend class KCStyleManager;

    /**
     * Constructor.
     * Constructs the default cell style.
     */
    KCCustomStyle();

    class Private;
    QSharedDataPointer<Private> d;
};


/**
 * \ingroup KCStyle
 * A single style attribute.
 */
class KCELLS_TEST_EXPORT KCSubStyle : public QSharedData
{
public:
    KCSubStyle() {}
    virtual ~KCSubStyle() {}
    virtual KCStyle::Key type() const {
        return KCStyle::DefaultStyleKey;
    }
    virtual void dump() const {
        kDebug() << debugData();
    }
    virtual QString debugData(bool withName = true) const {
        QString out; if (withName) out = name(KCStyle::DefaultStyleKey); return out;
    }
    virtual uint koHash() const { return uint(type()); }
    static QString name(KCStyle::Key key);
};

// Provides a default KCSubStyle for the tree.
// Otherwise, we would have QSharedDataPointer<KCSubStyle>() as default,
// which has a null pointer and crashes.
// Also, this makes the code more readable:
// QSharedDataPointer<KCSubStyle> vs. KCSharedSubStyle
class KCSharedSubStyle
{
public:
    inline KCSharedSubStyle() : d(s_defaultStyle.d){}
    inline KCSharedSubStyle(KCSubStyle* subStyle) : d(subStyle) {}
    inline const KCSubStyle *operator->() const {
        return d.data();
    }
    inline const KCSubStyle *data() const {
        return d.data();
    }
    inline bool operator<(const KCSharedSubStyle& o) const {
        return d.data() < o.d.data();
    }
    inline bool operator==(const KCSharedSubStyle& o) const {
        return d.data() == o.d.data();
    }
    inline bool operator!() const {
        return !d;
    }

private:
    QSharedDataPointer<KCSubStyle> d;
    static KCSharedSubStyle s_defaultStyle;
};

class KCNamedStyle : public KCSubStyle
{
public:
    KCNamedStyle(const QString& n) : KCSubStyle(), name(n) {}
    virtual KCStyle::Key type() const {
        return KCStyle::NamedStyleKey;
    }
    virtual void dump() const {
        kDebug() << debugData();
    }
    virtual QString debugData(bool withName = true) const {
        QString out; if (withName) out = KCSubStyle::name(KCStyle::NamedStyleKey) + ' '; out += name; return out;
    }
    virtual uint koHash() const { return uint(type()) ^ qHash(name); }
    QString name;
};


static inline uint qHash(const QColor& color)
{ return uint(color.rgb()); }

static inline uint qHash(const QPen& pen)
{ return qHash(pen.color()) ^ 37 * uint(pen.style()); }

static inline uint qHash(const QBrush& brush)
{ return qHash(brush.color()) ^ 91 * uint(brush.style()); }

template<KCStyle::Key key, class Value1>
class SubStyleOne : public KCSubStyle
{
public:
    SubStyleOne(const Value1& v = Value1()) : KCSubStyle(), value1(v) {}
    virtual KCStyle::Key type() const {
        return key;
    }
    virtual void dump() const {
        kDebug(36006) << debugData();
    }
    virtual QString debugData(bool withName = true) const {
        QString out; if (withName) out = name(key) + ' '; QDebug qdbg(&out); qdbg << value1; return out;
    }
    virtual uint koHash() const { return uint(type()) ^ qHash(value1); }
    Value1 value1;
};

Q_DECLARE_TYPEINFO(KCStyle, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KCCustomStyle, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KCSharedSubStyle, Q_MOVABLE_TYPE);

#endif // KC_STYLE_H
