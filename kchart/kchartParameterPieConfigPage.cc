/*
 * Copyright 2001 by Laurent Montel, released under Artistic License.
 */

#include "kchartParameterPieConfigPage.h"

#include "kchartParameterPieConfigPage.moc"

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include "kchart_params.h"

KChartParameterPieConfigPage::KChartParameterPieConfigPage( KChartParams* params,
                                                            QWidget* parent ) :
    QWidget( parent ),_params( params )
{
    QGridLayout *grid1 = new QGridLayout(this,2,2,15,7);
    QGroupBox* gb3 = new QGroupBox( i18n( "Parameter" ), this );
    QGridLayout *grid3 = new QGridLayout(gb3,4,2,15,7);

    pie3d=new QCheckBox(i18n("pie 3D"),gb3);
    grid3->addWidget(pie3d,2,0);

    drawShadowColor=new QCheckBox(i18n("Draw Shadow color"),gb3);
    grid3->addWidget(drawShadowColor,3,0);

    QLabel *label = new QLabel( i18n( "Start" ), gb3 );
    label->resize( label->sizeHint() );
    label->setAlignment(Qt::AlignCenter);
    grid3->addWidget( label,4,0);

    angle = new QSpinBox(0, 90, 1, gb3);
    angle->resize(100, angle->sizeHint().height() );
    grid3->addWidget( angle,5,0);

    label = new QLabel( i18n( "3D-Depth" ), gb3 );
    label->resize( label->sizeHint() );
    label->setAlignment(Qt::AlignCenter);
    grid3->addWidget( label,6,0);

    depth = new QSpinBox(0, 40, 1, gb3);
    depth->resize(100, depth->sizeHint().height() );
    grid3->addWidget( depth,7,0);

    grid1->addWidget(gb3,0,0);
    connect(pie3d,SIGNAL(toggled ( bool )),this, SLOT(active3DPie(bool)));
}

void KChartParameterPieConfigPage::active3DPie(bool b)
{
    depth->setEnabled(b);
    drawShadowColor->setEnabled(b);
}

void KChartParameterPieConfigPage::init()
{
    pie3d->setChecked(_params->threeDPies());
    bool state=_params->threeDPies();
    depth->setEnabled(state);
    active3DPie(state);
    if( state )	{
        depth->setValue( _params->threeDPieHeight() );
    }
    drawShadowColor->setChecked(_params->threeDShadowColors());
    angle->setValue( _params->pieStart() );

}


void KChartParameterPieConfigPage::apply()
{
     _params->setThreeDPies( pie3d->isChecked() );
     if( _params->threeDPies() )	{
        _params->setThreeDPieHeight( depth->value() );
    }
    _params->setThreeDShadowColors( drawShadowColor->isChecked());
    _params->setPieStart( angle->value() );
}
