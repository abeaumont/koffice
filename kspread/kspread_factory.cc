#include "kspread_factory.h"
#include "kspread_doc.h"
#include <kaboutdata.h>
#include "KSpreadAppIface.h"

#include <klocale.h>
#include <kinstance.h>
#include <kstddirs.h>
#include <kdebug.h>

static const char* description=I18N_NOOP("KOffice Spreadsheet Application");
static const char* version="0.1";

extern "C"
{
    void* init_libkspread()
    {
	return new KSpreadFactory;
    }
};

KInstance* KSpreadFactory::s_global = 0;

KSpreadFactory::KSpreadFactory( QObject* parent, const char* name )
    : KoFactory( parent, name )
{
    kdDebug(36001) << "KSpreadFactory::KSpreadFactory()" << endl;
    (void)global();
    (void)new KSpreadAppIface;
}

KSpreadFactory::~KSpreadFactory()
{
  kdDebug(36001) << "KSpreadFactory::~KSpreadFactory()" << endl;
  if ( s_global )
  {
    delete s_global->aboutData();
    delete s_global;
  }
}

QObject* KSpreadFactory::create( QObject* parent, const char* name, const char* classname, const QStringList & )
{
/*    if ( parent && !parent->inherits("KoDocument") )
    {
	kdDebug(36001) << "KSpreadFactory: parent does not inherit KoDocument" << endl;
	return 0;
    }
*/
//    return new KSpreadDoc( (KoDocument*)parent, name );

  bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

  KSpreadDoc *doc = new KSpreadDoc( parent, name, !bWantKoDocument );

  if ( !bWantKoDocument )
    doc->setReadWrite( false );

  emit objectCreated( doc );

  return doc;
}

KAboutData* KSpreadFactory::aboutData()
{
  KAboutData * aboutData = new KAboutData( "kspread", I18N_NOOP("KSpread"),
        version, description, KAboutData::License_GPL,
        "(c) 1998-2000, Torben Weis");
  aboutData->addAuthor("Torben Weis",0, "weis@kde.org");
  aboutData->addAuthor("Laurent Montel",0, "montell@club-internet.fr");
  aboutData->addAuthor("David Faure",0, "faure@kde.org");
  return aboutData;
}

KInstance* KSpreadFactory::global()
{
    if ( !s_global )
    {
      s_global = new KInstance(aboutData());
      s_global->dirs()->addResourceType( "toolbar",
				         KStandardDirs::kde_default("data") + "koffice/toolbar/");
      s_global->dirs()->addResourceType( "extensions", KStandardDirs::kde_default("data") + "koffice/extensions/");
      s_global->dirs()->addResourceType( "extensions", KStandardDirs::kde_default("data") + "kspread/extensions/");
      s_global->dirs()->addResourceType( "table-styles", KStandardDirs::kde_default("data") + "kspread/tablestyles/");
      s_global->dirs()->addResourceType( "scripts", KStandardDirs::kde_default("data") + "koffice/scripts/");
      s_global->dirs()->addResourceType( "scripts", KStandardDirs::kde_default("data") + "kspread/scripts/");
    }
    return s_global;
}

#include "kspread_factory.moc"
