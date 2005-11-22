/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
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
#ifndef KIVIO_SML_STENCIL_SPAWNER_H
#define KIVIO_SML_STENCIL_SPAWNER_H

#include <qdom.h>
#include <qptrlist.h>
#include <qstring.h>

#include "kivio_stencil_spawner.h"

class KivioStencilSpawnerSet;
class KivioStencil;
class KivioSMLStencil;
class KivioConnectorTarget;
class QPainter;
class QPixmap;
class QRect;

class KivioSMLStencilSpawner : public KivioStencilSpawner
{
protected:
    KivioSMLStencil *m_pStencil;
    QString m_filename;

    // Target list
    QPtrList<KivioConnectorTarget>*m_pTargets;

protected:
    void loadShape( QDomNode & );
    QString readDesc( const QString & );

public:
    KivioSMLStencilSpawner( KivioStencilSpawnerSet * );
    virtual ~KivioSMLStencilSpawner();
    
    virtual bool load( const QString & );
    virtual bool loadXML( const QString &, QDomDocument & );

    virtual QDomElement saveXML( QDomDocument & );

    virtual QString &filename() { return m_filename; }

    virtual KivioStencil *newStencil();

    QPtrList <KivioConnectorTarget> *targets() { return m_pTargets; }
};

#endif


