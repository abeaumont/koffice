/***************************************************************************
                       main.cc - using the thesaurus KDataTool stand alone
                       -------------------
    begin            : 2001-12-22
    copyright        : (C) 2001 by Daniel Naber
    email            : daniel.naber@t-online.de
	$Id $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdatatool.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

// fixme?:
static KCmdLineOptions options[] =
{
  { "+[term]",   I18N_NOOP("Term to search for when starting up"), 0 },
  { 0, 0, 0 }
};

int main(int argc, char **argv)
{

	KAboutData aboutData("kthesaurus", I18N_NOOP("KThesaurus"), "1.0",
		I18N_NOOP( "KWrite - Leightweight Kate" ), KAboutData::License_GPL,
		I18N_NOOP( "(c) 2001 Daniel Naber" ), 0, "fixme");

	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);

	// TODO: take term from command line!

	KService::Ptr service = KService::serviceByDesktopName("thesaurustool");
	if( ! service ) {
		kdWarning() << "Could not find Service/KDataTool 'thesaurustool'!" << endl;
		return 1;
	}

	KDataToolInfo *info = new KDataToolInfo(service, 0);
	KDataTool *tool = info->createTool();
	if ( !tool ) {
		kdWarning() << "Could not create tool 'thesaurustool'!" << endl;
		return 2;
	}

/* TODO: get selection(), not only clipboard!
	QClipboard *cb = QApplication::clipboard();
	QString text = cb->text();
	if( text == QString::null || text.length() > 50 ) {
		// long texts are probably not supposed to be searched for
		text = "";
	}
*/
	QString text = "";

	QString command = "thesaurus_standalone";	// 'standalone' will give us different buttons
	QString mimetype = "text/plain";
	QString datatype = "QString";

	//kdDebug() << "KThesaurus command=" << command
	//		<< " dataType=" << info->dataType() << endl;

	tool->run(command, &text, datatype, mimetype);

	delete tool;
	
	return 0;
}
