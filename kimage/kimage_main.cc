/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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

#include <qprinter.h>

#include <opAutoLoader.h>

#include <koIMR.h>
#include <koFactory.h>
#include <koDocument.h>
#include <koQueryTypes.h>
#include <koMainWindow.h>

#include "kimage_shell.h"
#include "kimage_main.h"
#include "kimage_doc.h"
#include "kimage.h"

KOFFICE_DOCUMENT_FACTORY( KImageDoc, KImageFactory, KImage::DocumentFactory_skel )
typedef OPAutoLoader<KImageFactory> KImageAutoLoader;

KImageApp::KImageApp( int& argc, char** argv ) 
  : KoApplication( argc, argv, "kimage" )
{
}

KImageApp::~KImageApp()
{
}

KoMainWindow* KImageApp::createNewShell()
{
  return new KImageShell;
}

int main( int argc, char** argv )
{
  KImageAutoLoader loader( "IDL:KImage/DocumentFactory:1.0", "KImage" );
  KImageApp app( argc, argv );

  app.exec();

  return 0;
}

#include "kimage_main.moc"
