/* This file is part of the KDE project
   Copyright (C) 2005 Thorsten Zachmann <zachmann@kde.org>

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
#include "brushproperty.h"

#include "brushpropertyui.h"
#include "gradientpropertyui.h"
#include "pbpreview.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qslider.h>
#include <qwidgetstack.h>

#include <klocale.h>
#include <kcolorbutton.h>
#include <kcombobox.h>

BrushProperty::BrushProperty( QWidget *parent, const char *name, const BrushCmd::Brush &brush )
    : QWidget( parent, name )
    , m_brush( brush )
{
    QGridLayout *layout = new QGridLayout( this, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );

    m_typeCombo = new KComboBox( this );
    layout->addWidget( m_typeCombo, 0, 1 );
    m_typeCombo->insertItem( i18n( "Single Color" ) );
    m_typeCombo->insertItem( i18n( "Gradient" ) );
    m_typeCombo->insertItem( i18n( "Transparant" ) );

    QLabel *typeLabel = new QLabel( i18n( "&Type:" ), this );
    layout->addWidget( typeLabel, 0, 0 );
    typeLabel->setBuddy( m_typeCombo );

    m_stack = new QWidgetStack( this );
    layout->addMultiCellWidget( m_stack, 1, 1, 0, 1 );
    connect( m_typeCombo, SIGNAL( activated( int ) ),
             this, SLOT( slotTypeChanged( int ) ) );

    m_brushUI = new BrushPropertyUI( m_stack );

    m_brushUI->styleCombo->insertItem( i18n( "%1% Fill Pattern" ).arg( 100 ) );
    m_brushUI->styleCombo->insertItem( i18n( "%1% Fill Pattern" ).arg( 94 ) );
    m_brushUI->styleCombo->insertItem( i18n( "%1% Fill Pattern" ).arg( 88 ) );
    m_brushUI->styleCombo->insertItem( i18n( "%1% Fill Pattern" ).arg( 63 ) );
    m_brushUI->styleCombo->insertItem( i18n( "%1% Fill Pattern" ).arg( 50 ) );
    m_brushUI->styleCombo->insertItem( i18n( "%1% Fill Pattern" ).arg( 37 ) );
    m_brushUI->styleCombo->insertItem( i18n( "%1% Fill Pattern" ).arg( 12 ) );
    m_brushUI->styleCombo->insertItem( i18n( "%1% Fill Pattern" ).arg( 6 ) );
    m_brushUI->styleCombo->insertItem( i18n( "Horizontal Lines" ) );
    m_brushUI->styleCombo->insertItem( i18n( "Vertical Lines" ) );
    m_brushUI->styleCombo->insertItem( i18n( "Crossing Lines" ) );
    m_brushUI->styleCombo->insertItem( i18n( "Diagonal Lines ( / )" ) );
    m_brushUI->styleCombo->insertItem( i18n( "Diagonal Lines ( \\ )" ) );
    m_brushUI->styleCombo->insertItem( i18n( "Diagonal Crossing Lines" ) );

    m_preview_color = new PBPreview( m_brushUI->previewPanel, 0, PBPreview::Brush );
    QHBoxLayout *hbox = new QHBoxLayout( m_brushUI->previewPanel );
    hbox->addWidget(m_preview_color);

    connect( m_brushUI->styleCombo, SIGNAL( activated( int ) ),
             this, SLOT( slotBrushChanged() ) );
    connect( m_brushUI->colorChooser, SIGNAL( changed( const QColor& ) ),
             this, SLOT( slotBrushChanged() ) );

    m_stack->addWidget( m_brushUI, 0 );

    m_gradientUI = new GradientPropertyUI( m_stack );
    m_gradientUI->styleCombo->insertItem( i18n( "Horizontal" ) );
    m_gradientUI->styleCombo->insertItem( i18n( "Vertical" ) );
    m_gradientUI->styleCombo->insertItem( i18n( "Diagonal 1" ) );
    m_gradientUI->styleCombo->insertItem( i18n( "Diagonal 2" ) );
    m_gradientUI->styleCombo->insertItem( i18n( "Circle" ) );
    m_gradientUI->styleCombo->insertItem( i18n( "Rectangle" ) );
    m_gradientUI->styleCombo->insertItem( i18n( "PipeCross" ) );
    m_gradientUI->styleCombo->insertItem( i18n( "Pyramid" ) );

    m_preview_gradient = new PBPreview( m_gradientUI->previewPanel, 0, PBPreview::Gradient );
    hbox = new QHBoxLayout( m_gradientUI->previewPanel );
    hbox->addWidget(m_preview_gradient);

    connect( m_gradientUI->styleCombo, SIGNAL( activated( int ) ),
             this, SLOT( slotBackColorTypeChanged() ) );
    connect( m_gradientUI->color1Chooser, SIGNAL( changed( const QColor& ) ),
             this, SLOT( slotColor1Changed() ) );
    connect( m_gradientUI->color2Chooser, SIGNAL( changed( const QColor& ) ),
             this, SLOT( slotColor2Changed() ) );
    connect( m_gradientUI->unbalancedCheckBox, SIGNAL( clicked() ),
             this, SLOT( slotUnbalancedChanged() ) );
    connect( m_gradientUI->xSlider, SIGNAL( valueChanged( int ) ),
             this, SLOT( slotXFactorChanged() ) );
    connect( m_gradientUI->ySlider, SIGNAL( valueChanged( int ) ),
             this, SLOT( slotYFactorChanged() ) );

    m_stack->addWidget( m_gradientUI, 1 );
    m_stack->addWidget( new QFrame(), 2 ); // the transparant case

    slotReset();
}


BrushProperty::~BrushProperty()
{
}


FillType BrushProperty::getFillType() const
{
    int selected = m_typeCombo->currentItem();
    if(selected == 2)
        selected = 0;
    return (FillType) selected;
}


QBrush BrushProperty::getQBrush() const
{
    QBrush brush;

    switch ( m_brushUI->styleCombo->currentItem() )
    {
        case 0:
            brush.setStyle( SolidPattern );
            break;
        case 1:
            brush.setStyle( Dense1Pattern );
            break;
        case 2:
            brush.setStyle( Dense2Pattern );
            break;
        case 3:
            brush.setStyle( Dense3Pattern );
            break;
        case 4:
            brush.setStyle( Dense4Pattern );
            break;
        case 5:
            brush.setStyle( Dense5Pattern );
            break;
        case 6:
            brush.setStyle( Dense6Pattern );
            break;
        case 7:
            brush.setStyle( Dense7Pattern );
            break;
        case 8:
            brush.setStyle( HorPattern );
            break;
        case 9:
            brush.setStyle( VerPattern );
            break;
        case 10:
            brush.setStyle( CrossPattern );
            break;
        case 11:
            brush.setStyle( BDiagPattern );
            break;
        case 12:
            brush.setStyle( FDiagPattern );
            break;
        case 13:
            brush.setStyle( DiagCrossPattern );
            break;
    }
    if( m_typeCombo->currentItem() == 2)
        brush.setStyle( QBrush::NoBrush );

    brush.setColor( m_brushUI->colorChooser->color() );

    return brush;
}


QColor BrushProperty::getGColor1()const
{
    return m_gradientUI->color1Chooser->color();
}


QColor BrushProperty::getGColor2()const
{
    return m_gradientUI->color2Chooser->color();
}


BCType BrushProperty::getGType()const
{
    return (BCType)( m_gradientUI->styleCombo->currentItem() +1 );
}


bool BrushProperty::getGUnbalanced()const
{
    return m_gradientUI->unbalancedCheckBox->isChecked();
}


int BrushProperty::getGXFactor() const
{
    return m_gradientUI->xSlider->value();
}


int BrushProperty::getGYFactor() const
{
    return m_gradientUI->ySlider->value();
}


int BrushProperty::getBrushPropertyChange() const
{
    int flags = 0;
    bool fillTypeChanged = getFillType() != m_brush.fillType;

    if ( getFillType() == FT_BRUSH )
    {
        QBrush brush = getQBrush();
        if ( fillTypeChanged || brush.color() != m_brush.brush.color() )
        {
            flags |= BrushCmd::BrushColor;
        }
        if ( fillTypeChanged || brush.style() != m_brush.brush.style() )
        {
            flags |= BrushCmd::BrushStyle;
        }
        if ( fillTypeChanged )
        {
            flags |= BrushCmd::BrushGradientSelect;
        }
    }
    else
    {
        if ( fillTypeChanged || getGColor1() != m_brush.gColor1 )
        {
            flags |= BrushCmd::GradientColor1;
        }
        if ( fillTypeChanged || getGColor2() != m_brush.gColor2 )
        {
            flags |= BrushCmd::GradientColor2;
        }
        if ( fillTypeChanged || getGType() != m_brush.gType )
        {
            flags |= BrushCmd::GradientType;
        }
        if ( fillTypeChanged || getGUnbalanced() != m_brush.unbalanced )
        {
            flags |= BrushCmd::GradientBalanced;
        }
        if ( fillTypeChanged || getGXFactor() != m_brush.xfactor )
        {
            flags |= BrushCmd::GradientXFactor;
        }
        if ( fillTypeChanged || getGYFactor() != m_brush.yfactor )
        {
            flags |= BrushCmd::GradientYFactor;
        }
        if ( fillTypeChanged )
        {
            flags |= BrushCmd::BrushGradientSelect;
        }
    }
    return flags;
}


BrushCmd::Brush BrushProperty::getBrush() const
{
    BrushCmd::Brush brush( getQBrush(),
                           getGColor1(),
                           getGColor2(),
                           getGType(),
                           getFillType(),
                           getGUnbalanced(),
                           getGXFactor(),
                           getGYFactor() );
    return brush;
}


void BrushProperty::setBrush( BrushCmd::Brush &brush )
{
    m_brush = brush;
    slotReset();
}


void BrushProperty::apply()
{
    int flags = getBrushPropertyChange();

    if ( flags & BrushCmd::BrushGradientSelect )
        m_brush.fillType = getFillType();

    if ( flags & BrushCmd::BrushColor )
        m_brush.brush.setColor( getQBrush().color() );

    if ( flags & BrushCmd::BrushStyle )
        m_brush.brush.setStyle( getQBrush().style() );

    if ( flags & BrushCmd::GradientColor1 )
        m_brush.gColor1 = getGColor1();

    if ( flags & BrushCmd::GradientColor2 )
        m_brush.gColor2 = getGColor2();

    if ( flags & BrushCmd::GradientType )
        m_brush.gType = getGType() ;

    if ( flags & BrushCmd::GradientBalanced )
        m_brush.unbalanced = getGUnbalanced() ;

    if ( flags & BrushCmd::GradientXFactor )
        m_brush.xfactor = getGXFactor() ;

    if ( flags & BrushCmd::GradientYFactor )
        m_brush.yfactor = getGYFactor() ;
}


void BrushProperty::setQBrush( const QBrush &brush )
{
    switch ( brush.style() )
    {
        case NoBrush:
            // TODO
            break;
        case SolidPattern:
            m_brushUI->styleCombo->setCurrentItem( 0 );
            break;
        case Dense1Pattern:
            m_brushUI->styleCombo->setCurrentItem( 1 );
            break;
        case Dense2Pattern:
            m_brushUI->styleCombo->setCurrentItem( 2 );
            break;
        case Dense3Pattern:
            m_brushUI->styleCombo->setCurrentItem( 3 );
            break;
        case Dense4Pattern:
            m_brushUI->styleCombo->setCurrentItem( 4 );
            break;
        case Dense5Pattern:
            m_brushUI->styleCombo->setCurrentItem( 5 );
            break;
        case Dense6Pattern:
            m_brushUI->styleCombo->setCurrentItem( 6 );
            break;
        case Dense7Pattern:
            m_brushUI->styleCombo->setCurrentItem( 7 );
            break;
        case HorPattern:
            m_brushUI->styleCombo->setCurrentItem( 8 );
            break;
        case VerPattern:
            m_brushUI->styleCombo->setCurrentItem( 9 );
            break;
        case CrossPattern:
            m_brushUI->styleCombo->setCurrentItem( 10 );
            break;
        case BDiagPattern:
            m_brushUI->styleCombo->setCurrentItem( 11 );
            break;
        case FDiagPattern:
            m_brushUI->styleCombo->setCurrentItem( 12 );
            break;
        case DiagCrossPattern:
            m_brushUI->styleCombo->setCurrentItem( 13 );
            break;
        case CustomPattern:
            break;
    }

    m_brushUI->colorChooser->setColor( brush.color() );
    m_preview_color->setBrush( brush );
}


void BrushProperty::setGradient( const QColor &_c1, const QColor &_c2, BCType _t,
                                    bool _unbalanced, int _xfactor, int _yfactor )
{
    m_gradientUI->styleCombo->setCurrentItem( (int) _t - 1 );
    m_gradientUI->color1Chooser->setColor( _c1 );
    m_gradientUI->color2Chooser->setColor( _c2 );
    m_gradientUI->unbalancedCheckBox->setChecked( _unbalanced );
    setUnbalancedEnabled( _unbalanced );
    m_gradientUI->xSlider->setValue( _xfactor );
    m_gradientUI->ySlider->setValue( _yfactor );
    m_preview_gradient->setGradient( _c1, _c2, _t, _unbalanced, _xfactor, _yfactor );
}


void BrushProperty::setUnbalancedEnabled( bool state )
{
    m_gradientUI->xFactorLabel->setEnabled( state );
    m_gradientUI->xSlider->setEnabled( state );
    m_gradientUI->yFactorLabel->setEnabled( state );
    m_gradientUI->ySlider->setEnabled( state );
}


void BrushProperty::slotReset()
{
    if ( m_brush.gType == BCT_PLAIN )
        m_brush.gType = BCT_GHORZ;
    setQBrush( m_brush.brush );
    setGradient( m_brush.gColor1,
                 m_brush.gColor2,
                 m_brush.gType,
                 m_brush.unbalanced,
                 m_brush.xfactor,
                 m_brush.yfactor );

    int panelIndex;
    if( m_brush.fillType == FT_BRUSH && m_brush.brush.style() == QBrush::NoBrush )
        panelIndex = 2;
    else
        panelIndex = (int)m_brush.fillType;

    m_typeCombo->setCurrentItem( panelIndex );
    slotTypeChanged( panelIndex );
}


void BrushProperty::slotTypeChanged( int pos )
{
    m_stack->raiseWidget( pos );
    slotBrushChanged();
}


void BrushProperty::slotBrushChanged()
{
    m_preview_color->setBrush( getQBrush() );
}


void BrushProperty::slotColor1Changed()
{
    m_preview_gradient->setColor1( getGColor1() );
}


void BrushProperty::slotColor2Changed()
{
    m_preview_gradient->setColor2( getGColor2() );
}


void BrushProperty::slotBackColorTypeChanged()
{
    BCType type = getGType();
    m_preview_gradient->setBackColorType( type );
    m_gradientUI->xSlider->setEnabled( type != 1 );
    m_gradientUI->ySlider->setEnabled( type != 2 );
}


void BrushProperty::slotUnbalancedChanged()
{
    bool state = getGUnbalanced();
    setUnbalancedEnabled( state );
    m_preview_gradient->setUnbalanced( state );

    slotBackColorTypeChanged(); // make sure the sliders enabled-ness is up-to-date
}


void BrushProperty::slotXFactorChanged()
{
    m_preview_gradient->setXFactor( getGXFactor() );
}


void BrushProperty::slotYFactorChanged()
{
    m_preview_gradient->setYFactor( getGYFactor() );
}

#include "brushproperty.moc"
