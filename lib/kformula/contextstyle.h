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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef CONTEXTSTYLE_H
#define CONTEXTSTYLE_H

//Qt Include
#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>

//KDE Include
#include <kconfig.h>
#include <kozoomhandler.h>

//Formula include
#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN

/**
 * Contains all the style information for the formela. The idea
 * is to change the values here (user configurable) and have
 * the elements paint themselves with this information.
 *
 * All distances are stored in point. Most methods return pixel
 * values.
 */
class ContextStyle : public KoZoomHandler
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

    void readConfig( KConfig* config );

    bool edit() const { return m_edit; }
    void setEdit( bool e ) { m_edit = e; }

    /**
     * Sets the zoom by hand. This is to be used in <code>paintContent</code>.
     * @returns whether there was any change.
     */
    bool setZoom( double zoomX, double zoomY, bool updateViews, bool forPrint );

    bool syntaxHighlighting() const { return m_syntaxHighlighting; }
    void setSyntaxHighlighting( bool highlight ) { m_syntaxHighlighting = highlight; }

    QColor getDefaultColor()  const { return defaultColor; }
    QColor getNumberColor()   const;
    QColor getOperatorColor() const;
    QColor getErrorColor()    const;
    QColor getEmptyColor()    const;

    void setDefaultColor( const QColor& );
    void setNumberColor( const QColor& );
    void setOperatorColor( const QColor& );
    void setErrorColor( const QColor& );
    void setEmptyColor( const QColor& );

    QFont getDefaultFont()    const { return defaultFont; }
    QFont getNameFont()       const { return nameFont; }
    QFont getNumberFont()     const { return numberFont; }
    QFont getOperatorFont()   const { return operatorFont; }
    QFont getSymbolFont()     const { return symbolFont; }


    double getReductionFactor( TextStyle tstyle ) const;

    luPt getBaseSize() const;

    // a hack! needs fix.
    void setBaseSize( pt pointSize );

    TextStyle getBaseTextStyle() const { return m_baseTextStyle; }
    bool isScript( TextStyle tstyle ) const { return ( tstyle == scriptStyle ) ||
                                                     ( tstyle == scriptScriptStyle ); }

    /**
     * TeX like spacings.
     */
    luPt getSpace( TextStyle tstyle, SpaceWidth space ) const;
    luPt getThinSpace( TextStyle tstyle ) const;
    luPt getMediumSpace( TextStyle tstyle ) const;
    luPt getThickSpace( TextStyle tstyle ) const;
    luPt getQuadSpace( TextStyle tstyle ) const;

    /**
     * Calculates the font size corresponding to the given TextStyle.
     */
    luPt getAdjustedSize( TextStyle tstyle ) const;

    /**
     * All simple lines like the one that makes up a fraction.
     */
    luPixel getLineWidth() const;

    luPixel getEmptyRectWidth() const;
    luPixel getEmptyRectHeight() const;

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

        void setup( QFont font, luPt baseSize, double reduction );

        luPt thinSpace() const   { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )/6. ); }
        luPt mediumSpace() const { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )*2./9. ); }
        luPt thickSpace() const  { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )*5./18. ); }
        luPt quadSpace() const   { return quad; }

        double reductionFactor;
        luPt quad;
    };

    TextStyleValues textStyleValues[ 4 ];

    QFont defaultFont;
    QFont nameFont;
    QFont numberFont;
    QFont operatorFont;
    QFont symbolFont;

    QColor defaultColor;
    QColor numberColor;
    QColor operatorColor;
    QColor errorColor;
    QColor emptyColor;

    /**
     * The cursors movement style. You need to notify each cursor
     * if you change this.
     */
    bool linearMovement;

    /**
     * The (font) size of the formula's main sequence.
     */
    pt baseSize;

    /**
     * The base text style of the formula.
     **/
    TextStyle m_baseTextStyle;

    /**
     * The thickness of our lines.
     */
    pt lineWidth;

    /**
     * The little rect (square in most cases) that marks the
     * empty place where elements might be inserted.
     */
    pt emptyRectWidth;
    pt emptyRectHeight;

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
};

KFORMULA_NAMESPACE_END

#endif // CONTEXTSTYLE_H
