/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIVIO_GRADIENT_H
#define KIVIO_GRADIENT_H

#include <qcolor.h>
#include <qdom.h>
#include <qlist.h>

class KivioPoint;

class KivioGradient
{
public:
    // Gradient type enumerations
    typedef enum {
        kgtNone=0,
        kgtLinear,
        kgtBiLinear,
        kgtRadial,
        kgtSquare,
        kgtConicalSymmetric,
        kgtConicalAsymmetric,
        kgtLast,
    } KivioGradientType;
    
protected:
    QList <QColor> *m_pColors;              // A list of the colors in the gradient
    QList <KivioPoint> *m_pPoints;         // A list of the points for each color
    KivioGradientType m_gradientType;       // The type of gradient
    
    
public:
    KivioGradient();
    KivioGradient( const KivioGradient & );
    virtual ~KivioGradient();
    
    void copyInto( KivioGradient *pTarget ) const;
    
    bool loadXML( const QDomElement & );
    QDomElement saveXML( QDomDocument & );
    
    
    QList<QColor> *colors() const { return m_pColors; }
    
    QList<KivioPoint> *points() const { return m_pPoints; }
    
    KivioGradientType gradientType() const { return m_gradientType; }
    void setGradientType( KivioGradientType t ) { m_gradientType=t; }
};

#endif


