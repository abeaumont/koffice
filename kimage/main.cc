/* This file is part of the KDE project
   Copyright (C) 1999 Michael Koch <koch@kde.org>

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

#include <dcopclient.h>

#include <koQueryTypes.h>
#include <koApplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include "kimage_doc.h"
#include "kimage_shell.h"
#include "kimage_factory.h"

extern "C"
{
  void* init_libkimage();
}



static const KCmdLineOptions options[]=
{
	{"+[file]", I18N_NOOP("File To Open"),0},
	{0,0,0}
};

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, KImageFactory::global()->aboutData());
  KCmdLineArgs::addCmdLineOptions( options );
  
  KoApplication app;
    
  app.dcopClient()->attach();
  app.dcopClient()->registerAs( "kimage" );

  app.start();
  app.exec();

  return 0;
}
