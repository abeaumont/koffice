/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

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

#include <klocale.h>

#include "vzordercmd.h"
#include "vselection.h"

VZOrderCmd::VZOrderCmd( VDocument *doc, VOrder state )
	: VCommand( doc, i18n( "Order Selection" ) ), m_state( state )
{
	m_selection = m_doc->selection()->clone();
}

VZOrderCmd::~VZOrderCmd()
{
	delete( m_selection );
}

void
VZOrderCmd::execute()
{
	if( m_state == sendToBack )
	{
		VObjectListIterator itr( m_doc->selection()->objects() );
		for ( itr.toLast() ; itr.current() ; --itr )
		{
			// remove from old layer
			VObjectList objects;
			VLayerListIterator litr( m_doc->layers() );

			for ( ; litr.current(); ++litr )
			{
				objects = litr.current()->objects();
				VObjectListIterator itr2( objects );
				for ( ; itr2.current(); ++itr2 )
					if( itr2.current() == itr.current() )
						litr.current()->sendToBack( *itr2.current() );
			}
		}
	}
	else if( m_state == bringToFront )
	{
		VObjectListIterator itr( m_doc->selection()->objects() );
		for ( ; itr.current() ; ++itr )
		{
			// remove from old layer
			VObjectList objects;
			VLayerListIterator litr( m_doc->layers() );

			for ( ; litr.current(); ++litr )
			{
				objects = litr.current()->objects();
				VObjectListIterator itr2( objects );
				for ( ; itr2.current(); ++itr2 )
					if( itr2.current() == itr.current() )
						litr.current()->bringToFront( *itr2.current() );
			}
		}
	}
	else if( m_state == up || m_state == down )
	{
		VSelection selection = *m_doc->selection();

		VObjectList objects;

		VLayerListIterator litr( m_doc->layers() );
		while( !selection.objects().isEmpty() )
		{
			for ( ; litr.current(); ++litr )
			{
				VObjectList todo;
				VObjectListIterator itr( selection.objects() );
				for ( ; itr.current() ; ++itr )
				{
					objects = litr.current()->objects();
					VObjectListIterator itr2( objects );
					// find all selected VObjects that are in the current layer
					for ( ; itr2.current(); ++itr2 )
					{
						if( itr2.current() == itr.current() )
						{
							todo.append( itr.current() );
							// remove from selection
							selection.take( *itr.current() );
						}
					}
				}

				// we have found the affected vobjects in this vlayer
				VObjectListIterator itr3( todo );
				for ( ; itr3.current(); ++itr3 )
				{
					if( m_state == up )
						litr.current()->upwards( *itr3.current() );
					else
						litr.current()->downwards( *itr3.current() );
				}
			}
		}
	}
}

void
VZOrderCmd::unexecute()
{
}

