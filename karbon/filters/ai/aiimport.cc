/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers
*/

#include <qdom.h>
#include <qcstring.h>
#include <qfile.h>
#include <qstring.h>

#include <kgenericfactory.h>
#include <koFilter.h>
#include <koFilterChain.h>
#include <koStore.h>

#include <kdebug.h>

#include "aiimport.h"


class AiImportFactory : KGenericFactory<AiImport, KoFilter>
{
public:
	AiImportFactory( void )
		: KGenericFactory<AiImport, KoFilter>( "karbonaiimport" )
	{}

protected:
	virtual void setupTranslations( void )
	{
		KGlobal::locale()->insertCatalogue( "karbonepsfilter" );
	}
};

K_EXPORT_COMPONENT_FACTORY( libkarbonaiimport, AiImportFactory() );

AiImport::AiImport( KoFilter*, const char*, const QStringList& )
	: KoFilter()
{
}

AiImport::~AiImport()
{
}

KoFilter::ConversionStatus
AiImport::convert( const QCString& from, const QCString& to )
{
	if ( from != "application/illustrator" || to != "application/x-karbon" )
	{
		return KoFilter::NotImplemented;
	}

	QFile fileIn( m_chain->inputFile() );
	if( !fileIn.open( IO_ReadOnly ) )
	{
		fileIn.close();
		return KoFilter::FileNotFound;
	}

	KoStoreDevice* storeOut = m_chain->storageFile( "root", KoStore::Write );
	if( !storeOut )
	{
		fileIn.close();
		return KoFilter::StorageCreationError;
	}

	QByteArray byteArrayIn( fileIn.size() );
	fileIn.readBlock( byteArrayIn.data(), fileIn.size() );
	fileIn.close();

	QString outStr;
	QTextStream s( &outStr, IO_WriteOnly );

	m_aiDocument.parse( s, byteArrayIn.data() );
kdDebug() << outStr << endl;

	QCString cStr = outStr.latin1();
	storeOut->writeBlock( cStr, cStr.length() );

	return KoFilter::OK;
}

#include "aiimport.moc"

