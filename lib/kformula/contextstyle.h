/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#ifndef CONTEXTSTYLE_H
#define CONTEXTSTYLE_H

//Qt Include
#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluestack.h>

//KDE Include
#include <kconfig.h>
#include <KoTextZoomHandler.h>

//Formula include
#include "kformuladefs.h"


KFORMULA_NAMESPACE_BEGIN

class FontStyle;
class SymbolTable;


/**
 * Contains all the style information for the formela. The idea
 * is to change the values here (user configurable) and have
 * the elements paint themselves with this information.
 *
 * All distances are stored in point. Most methods return pixel
 * values.
 */
class ContextStyle : public KoTextZoomHandler
{
public:

    enum Alignment { left, center, right };

    /**
     * Textstyles like in TeX. In the remaining documentation, the
     * styles are abbreviated like this:
     *
     * displayStyle: D
     *
     * textStyle: T
     *
     * scriptStyle: S
     *
     * scriptScriptStyle: SS
     **/
    enum TextStyle {
        displayStyle = 0,
        textStyle = 1,
        scriptStyle = 2,
        scriptScriptStyle = 3
    };

    enum IndexStyle {normal, cramped};

    /**
     * Build a default context style
     */
    ContextStyle();
    ~ContextStyle();

    /**
     * @param init if true fonts may be installed if needed.
     */
    void init( bool init = true );

    /**
     * @param init true if initialization may take place. This may cause font
     * installation. Mark as false when this is not intended (i. e. creating
     * configuration dialog from another component)
     */
    void readConfig( KConfig* config, bool init = true );

    bool edit() const { return m_edit; }
    void setEdit( bool e ) { m_edit = e; }

    /**
     * @returns our symbol table.
     */
    const SymbolTable& symbolTable() const;

    const FontStyle& fontStyle() const { return *m_fontStyle; }


    void setZoomAndResolution( int zoom, int dpiX, int dpiY );

    /**
     * Sets the zoom by hand. This is to be used in <code>paintContent</code>.
     * @returns whether there was any change.
     */
    bool setZoomAndResolution( int zoom, double zoomX, double zoomY, bool updateViews, bool forPrint );

    bool syntaxHighlighting() const { return m_syntaxHighlighting; }
    void setSyntaxHighlighting( bool highlight ) { m_syntaxHighlighting = highlight; }

    QColor getDefaultColor()  const { return defaultColor; }
    QColor getNumberColorPlain()   const { return numberColor; }
    QColor getOperatorColorPlain() const { return operatorColor; }
    QColor getErrorColorPlain()    const { return errorColor; }
    QColor getEmptyColorPlain()    const { return emptyColor; }
    QColor getHelpColorPlain()     const { return helpColor; }
    QColor getNumberColor()   const;
    QColor getOperatorColor() const;
    QColor getErrorColor()    const;
    QColor getEmptyColor()    const;
    QColor getHelpColor()     const;

    void setDefaultColor( const QColor& );
    void setNumberColor( const QColor& );
    void setOperatorColor( const QColor& );
    void setErrorColor( const QColor& );
    void setEmptyColor( const QColor& );
    void setHelpColor( const QColor& );

    QString getFontStyle() const { return m_fontStyleName; }
    void setFontStyle( const QString& fontStyle, bool init = true );

    QFont getDefaultFont()    const { return defaultFont; }
    QFont getNameFont()       const { return nameFont; }
    QFont getNumberFont()     const { return numberFont; }
    QFont getOperatorFont()   const { return operatorFont; }
    QFont getSymbolFont()     const { return symbolFont; }

    void setDefaultFont( QFont f )  { defaultFont = f; }
    void setNameFont( QFont f )     { nameFont = f; }
    void setNumberFont( QFont f )   { numberFont = f; }
    void setOperatorFont( QFont f ) { operatorFont = f; }

    //const QStringList& requestedFonts() const;
    //void setRequestedFonts( const QStringList& list );

    double getReductionFactor( TextStyle tstyle ) const;

    luPt getBaseSize() const;
    int baseSize() const { return m_baseSize; }
    void setBaseSize( int pointSize );
    void setSizeFactor( double factor );

    TextStyle getBaseTextStyle() const { return m_baseTextStyle; }
    bool isScript( TextStyle tstyle ) const { return ( tstyle == scriptStyle ) ||
                                                     ( tstyle == scriptScriptStyle ); }

    /**
     * TeX like spacings.
     */
    luPixel getSpace( TextStyle tstyle, SpaceWidth space, double factor ) const;
    luPixel getThinSpace( TextStyle tstyle, double factor ) const;
    luPixel getMediumSpace( TextStyle tstyle, double factor ) const;
    luPixel getThickSpace( TextStyle tstyle, double factor ) const;
    luPixel getQuadSpace( TextStyle tstyle, double factor ) const;

    luPixel axisHeight( TextStyle tstyle, double factor ) const;

    /**
     * Calculates the font size corresponding to the given TextStyle.
     */
    luPt getAdjustedSize( TextStyle tstyle, double factor ) const;

    /**
     * All simple lines like the one that makes up a fraction.
     */
    luPixel getLineWidth( double factor ) const;

    luPixel getEmptyRectWidth( double factor ) const;
    luPixel getEmptyRectHeight( double factor ) const;

    Alignment getMatrixAlignment() const { return center; }

    bool getCenterSymbol() const { return centerSymbol; }

    /**
     * Font-conversions a la TeX.
     *
     * For fractions (and also matrices), we have the following conversions:
     * D->T, T->S, S,SS->SS
     */
    TextStyle convertTextStyleFraction( TextStyle tstyle ) const;

    /**
     * Font-conversions a la TeX.
     *
     * For indices, we have the following conversions:
     * D->S, T->S, S,SS->SS
     */
    TextStyle convertTextStyleIndex( TextStyle tstyle ) const;

    /**
     * Index-style-conversions a la TeX.
     *
     * The function convertIndexStyleUpper is responsible for everything
     * that ends 'up', like nominators of fractions, or upper indices.
     *
     * We have the following rule:
     * normal->normal, cramped->cramped
     */
    IndexStyle convertIndexStyleUpper( IndexStyle istyle ) const {
	return istyle; }


    /**
     * Index-style-conversions a la TeX.
     *
     * The function convertIndexStyleLower is responsible for everything
     * that ends 'down', like nominators of fractions, or upper indices.
     *
     * We have the following rule:
     * normal->cramped, cramped->cramped
     */
    IndexStyle convertIndexStyleLower( IndexStyle /*istyle*/ ) const {
	return cramped; }

private:

    void setup();

    struct TextStyleValues {

        void setup( double reduction ) { reductionFactor = reduction; }

        luPt thinSpace( luPt quad ) const   { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )/6. ); }
        luPt mediumSpace( luPt quad ) const { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )*2./9. ); }
        luPt thickSpace( luPt quad ) const  { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )*5./18. ); }
        luPt quadSpace( luPt quad ) const   { return quad; }

        luPixel axisHeight( luPixel height ) const { return static_cast<luPixel>( reductionFactor*height ); }
        double reductionFactor;
    };

    TextStyleValues textStyleValues[ 4 ];

    QFont defaultFont;
    QFont nameFont;
    QFont numberFont;
    QFont operatorFont;
    QFont symbolFont;

    //QStringList m_requestedFonts;

    QColor defaultColor;
    QColor numberColor;
    QColor operatorColor;
    QColor errorColor;
    QColor emptyColor;
    QColor helpColor;

    /**
     * The cursors movement style. You need to notify each cursor
     * if you change this.
     */
    bool linearMovement;

    /**
     * The (font) size of the formula's main sequence.
     */
    int m_baseSize;

    /**
     * Hack! Each formula might set this to a value not too far from one
     * to get a size different from the default one.
     */
    double m_sizeFactor;

    /**
     * The base text style of the formula.
     **/
    TextStyle m_baseTextStyle;

    /**
     * The thickness of our lines.
     */
    pt lineWidth;

    /**
     * Size of one quad.
     */
    luPt quad;

    /**
     * Distance between base line and axis.
     */
    luPixel m_axisHeight;

    /**
     * true means to center the symbol between its indexes.
     * false means alignment to the right.
     */
    bool centerSymbol;

    /**
     * Whether we want coloured formulae.
     */
    bool m_syntaxHighlighting;

    /**
     * Whether we are in edit mode.
     */
    bool m_edit;

    /**
     * The symbols/names that are "known" to the system.
     */
    //SymbolTable table;

    FontStyle* m_fontStyle;
    QString m_fontStyleName;
};


class StyleAttributes {
 public:
    double sizeFactor() const ;
    bool customMathVariant() const ;
    CharStyle charStyle() const ;
    CharFamily charFamily() const ;
    QColor color() const ;
    QColor background() const ;
    QFont font() const ;
    bool fontWeight() const ;
    bool customFontWeight() const ;
    bool customFont() const ;

    void setSizeFactor( double s ) { m_size.push( s ); }
    void setCustomMathVariant( bool cmv ) { m_customMathVariant.push( cmv ); }
    void setCharStyle( CharStyle cs ) { m_charStyle.push( cs ); }
    void setCharFamily( CharFamily cf ) { m_charFamily.push( cf ); }
    void setColor( const QColor& c ) { m_color.push( c ); }
    void setBackground( const QColor& bg ) { m_background.push( bg ); }
    void setFont( const QFont& f ) { m_font.push( f ); }
    void setCustomFont( bool cf ) { m_customFontFamily.push ( cf ); }
    void setCustomFontWeight( bool cfw ) { m_customFontWeight.push( cfw ); }
    void setFontWeight( bool fw ) { m_fontWeight.push( fw ); }

    void reset();

 private:
    // Size of the font in points (mathsize / fontsize)
    QValueStack<double> m_size;

    // Whether a custom mathvariant attribute is in use
    QValueStack<bool> m_customMathVariant;

    // Font style (mathvariant, fontweight, fontstyle)
    QValueStack<CharStyle> m_charStyle;

    // Font family (mathvariant)
    QValueStack<CharFamily> m_charFamily;

    // Foreground color (mathcolor, color)
    QValueStack<QColor> m_color;

    // Background color (mathbackground)
    QValueStack<QColor> m_background;

    // Font family (fontfamily)
    QValueStack<QFont> m_font;

    // Whether a custom fontfamily attribute is in use (instead of CharFamily)
    QValueStack<bool> m_customFontFamily;

    // Font Weight (fontweight)
    QValueStack<bool> m_fontWeight;

    // Whether a custom fontweight attribute is in use
    QValueStack<bool> m_customFontWeight;
};

KFORMULA_NAMESPACE_END

#endif // CONTEXTSTYLE_H
