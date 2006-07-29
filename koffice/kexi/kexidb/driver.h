/* This file is part of the KDE project
   Copyright (C) 2003-2006 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIDB_DRIVER_H
#define KEXIDB_DRIVER_H

#include <qobject.h>
#include <qdatetime.h>
#include <qdict.h>

#include <kexidb/object.h>
#include <kexidb/field.h>

class KService;

namespace KexiDB {

class AdminTools;
class Connection;
class ConnectionData;
class ConnectionInternal;
class DriverManager;
class DriverBehaviour;
class DriverPrivate;

//! Generic database abstraction.
/*! This class is a prototype of the database driver for implementations.
 Driver allows new connections to be created, and groups
 these as a parent. 
 Before destruction, all connections are destructed.

 Notes:
  - driver must be provided within KDE module file named with "kexidb_" prefix
  - following line should be placed in driver's implementation:
    \code
    KEXIDB_DRIVER_INFO( CLASS_NAME, INTERNAL_NAME );
    \endcode
    where:
    - CLASS_NAME is actual driver's class name, e.g. MySqlDriver
    - INTERNAL_NAME is driver name's most significant part (without quotation marks), e.g. mysql
    Above information uses K_EXPORT_COMPONENT_FACTORY macro for KTrader to find the module's entry point.
    For example, this line declares kexidb_mysqldriver.so module's entry point:
    \code
    KEXIDB_DRIVER_INFO( MySqlDriver, mysql );
    \endcode

 \sa SQLiteDriver MySqlDriver, pqxxSqlDriver
*/
class KEXI_DB_EXPORT Driver : public QObject, public KexiDB::Object
{
	Q_OBJECT
	public:
		/*! Helpful for retrieving info about driver from using 
		 KexiDB::DriverManager::driversInfo() without loading driver libraries. */
		class Info {
			public:
			Info();
			QString name, caption, comment, fileDBMimeType;
			//! true is the driver is for file-based database backend
			bool fileBased : 1;
			/*! true is the driver is for a backend that allows importing.
			 Defined by X-Kexi-DoNotAllowProjectImportingTo in "kexidb_driver" service type.
			 Used for migration. */
			bool allowImportingTo : 1;
		};
		typedef QMap<QString,Info> InfoMap;
		
		/*! Features supported by driver (sum of few Features enum items). */
		enum Features {
			NoFeatures = 0,
			//! single trasactions are only supported
			SingleTransactions = 1,   
			//! multiple concurent trasactions are supported
			//! (this implies !SingleTransactions)
			MultipleTransactions = 2, 
//(js) NOT YET IN USE:
			/*! nested trasactions are supported
			 (this should imply !SingleTransactions and MultipleTransactions) */
			NestedTransactions = 4,
			/*! forward moving is supported for cursors
			 (if not available, no cursors available at all) */
			CursorForward = 8, 
			/*! backward moving is supported for cursors (this implies CursorForward) */
			CursorBackward = (CursorForward+16),
			/*! compacting database supported (aka VACUUM) */
			CompactingDatabaseSupported = 32,
			//-- temporary options: can be removed later, use at your own risk --
			/*! If set, actions related to transactions will be silently bypassed
			 with success. Set this if your driver does not support transactions at all
			 Currently, this is only way to get it working with KexiDB.
			 Keep in mind that this hack do not provide data integrity!
			 This flag is currently used for MySQL driver. */
			IgnoreTransactions = 1024
		};

		//! Options used for createConnection()
		enum CreateConnectionOptions {
			ReadOnlyConnection = 1 //!< set to perform read only connection
		};
		
		virtual ~Driver();

		/*! Creates connection using \a conn_data as parameters. 
		 \return 0 and sets error message on error.
		 driverName member of \a conn_data will be updated with this driver name.
		 \a options can be a combination of CreateConnectionOptions enum values.
		 */
		Connection *createConnection( ConnectionData &conn_data, int options = 0 );

		/*! \return List of created connections. */
		const QPtrList<Connection> connectionsList() const;

//		/*! \return a name equal to the service name (X-Kexi-DriverName) 
//		 stored in given service .desktop file. */
//		QString driverName() { return m_driverName; }

		/*! \return a name of MIME type of files handled by this driver 
		 if it is a file-based database's driver 
		 (equal X-Kexi-FileDBDriverMime service property)
		 otherwise returns null string. \sa isFileDriver()
		*/
		QString fileDBDriverMimeType() const;

		/*! \return default file-based driver mime type 
		 (typically something like "application/x-kexiproject-sqlite") */
		static QString defaultFileBasedDriverMimeType();

		/*! \return default file-based driver name (currently, "sqlite3"). */
		static QString defaultFileBasedDriverName();

		/*! Info about the driver as a service. */
		const KService* service() const;

		/*! \return true if this driver is file-based */
		bool isFileDriver() const;

		/*! \return true if \a n is a system object's name, 
		 eg. name of build-in system table that cannot be used or created by a user,
		 and in most cases user even shouldn't see this. The list is specific for 
		 a given driver implementation. 
		 By default calls Driver::isKexiDBSystemObjectName() static method.
		 Note for driver developers: Also call Driver::isSystemObjectName()
		 from your reimplementation.
		 \sa isSystemFieldName().
		*/
		virtual bool isSystemObjectName( const QString& n ) const;

		/*! \return true if \a n is a kexibd-related 'system' object's
		 name, i.e. when \a n starts with "kexi__" prefix.
		*/
		static bool isKexiDBSystemObjectName( const QString& n );

		/*! \return true if \a n is a system database's name, 
		 eg. name of build-in, system database that cannot be used or created by a user,
		 and in most cases user even shouldn't see this. The list is specific for 
		 a given driver implementation. For implemenation.
		 \sa isSystemObjectName().
		*/
		virtual bool isSystemDatabaseName( const QString& n ) const = 0;

		/*! \return true if \a n is a system field's name, build-in system 
		 field that cannot be used or created by a user,
		 and in most cases user even shouldn't see this. The list is specific for 
		 a given driver implementation. 
		 \sa isSystemObjectName().
		*/
		bool isSystemFieldName( const QString& n ) const;

		/*! \return Driver's features that are combination of Driver::Features
		enum. */
		int features() const;

		/*! \return true if transaction are supported (single or 
		 multiple). */
		bool transactionsSupported() const;

		/*! \return admin tools object providing a number of database administration 
		 tools for the driver. Tools availablility varies from driver to driver. 
		 You can check it using features().  */
		AdminTools& adminTools() const;

		/*! SQL-implementation-dependent name of given type */
		virtual QString sqlTypeName(int id_t, int p=0) const;

		/*! used when we do not have Driver instance yet */
		static QString defaultSQLTypeName(int id_t);

		/*! \return true if this driver's implementation is valid.
		 Just few constriants are checked to ensure that driver 
		 developer didn't forget about something. 
		 This method is called automatically on createConnection(),
		 and proper error message is set properly on any error. */
		virtual bool isValid();
	
		/*! Driver's static version information, it is automatically defined
		 in implementation using KEXIDB_DRIVER macro (see driver_p.h) */
		virtual int versionMajor() const = 0;

		virtual int versionMinor() const = 0;

		/*! Escapes and converts value \a v (for type \a ftype) 
		 to string representation required by SQL commands.
		 Reimplement this if you need other behaviour (eg. for 'date' type handling)
		 This implementation return date, datetime and time values in ISO format,
		 what seems to be accepted by SQL servers. 
		 @see Qt::DateFormat */
		virtual QString valueToSQL( uint ftype, const QVariant& v ) const;
		
		//! Like above but with the fildtype as string.
		inline QString valueToSQL( const QString& ftype, const QVariant& v ) const {
			return valueToSQL(Field::typeForString(ftype), v);
		}

		//! Like above method, for \a field.
		inline QString valueToSQL( const Field *field, const QVariant& v ) const {
			return valueToSQL( (field ? field->type() : Field::InvalidType), v );
		}

		/*! not compatible with all drivers - reimplement */
		inline virtual QString dateTimeToSQL(const QDateTime& v) const {

		/*! (was compatible with SQLite: http://www.sqlite.org/cvstrac/wiki?p=DateAndTimeFunctions)
			Now it's ISO 8601 DateTime format - with "T" delimiter:
			http://www.w3.org/TR/NOTE-datetime
			(e.g. "1994-11-05T13:15:30" not "1994-11-05 13:15:30")
			@todo add support for time zones?
		*/
//old			const QDateTime dt( v.toDateTime() );
//old			return QString("\'")+dt.date().toString(Qt::ISODate)+" "+dt.time().toString(Qt::ISODate)+"\'";
			return QString("\'")+v.toString(Qt::ISODate)+"\'";
		}

		/*! Driver-specific SQL string escaping.
		 Implement escaping for any character like " or ' as your 
		 database engine requires. Prepend and append quotation marks.
		*/
		virtual QString escapeString( const QString& str ) const = 0;
		
		/*! This is overloaded version of escapeString( const QString& str )
		 to be implemented in the same way.
		*/
		virtual QCString escapeString( const QCString& str ) const = 0;
		
		/*! Driver-specific SQL BLOB value escaping.
		 Implement escaping for any character like " or ' and \\0 as your 
		 database engine requires. Prepend and append quotation marks.
		*/
		virtual QString escapeBLOB(const QByteArray& array) const = 0;

//todo enum EscapeType { EscapeDriver = 0x00, EscapeKexi = 0x01};
//todo enum EscapePolicy { EscapeAsNecessary = 0x00, EscapeAlways = 0x02 };

		enum EscapeType { EscapeDriver = 0x01, EscapeKexi = 0x02};
		
		enum EscapePolicy { EscapeAsNecessary = 0x04, EscapeAlways = 0x08 };

		//! Driver-specific identifier escaping (e.g. for a table name, db name, etc.)
		/*! Escape database identifier (\a str) in order that keywords
		   can be used as table names, column names, etc.
		   \a options is the union of the EscapeType and EscapePolicy types.
		   If no escaping options are given, defaults to driver escaping as
		   necessary. */
		QString escapeIdentifier( const QString& str, 
			int options = EscapeDriver|EscapeAsNecessary) const;

		QCString escapeIdentifier( const QCString& str, 
			int options = EscapeDriver|EscapeAsNecessary) const;

		//! \return property value for \a propeName available for this driver. 
		//! If there's no such property defined for driver, Null QVariant value is returned.
		QVariant propertyValue( const QCString& propName ) const;

		//! \return translated property caption for \a propeName. 
		//! If there's no such property defined for driver, empty string value is returned.
		QString propertyCaption( const QCString& propName ) const;

		//! \return a list of property names available for this driver.
		QValueList<QCString> propertyNames() const;

	protected:
		/*! Used by DriverManager. 
		 Note for driver developers: Reimplement this.
		 In your reimplementation you should initialize:
		 - d->typeNames - to types accepted by your engine
		 - d->isFileDriver - to true or false depending if your driver is file-based
		 - d->features - to combination of selected values from Features enum
		 
		 You may also want to change options in DriverBehaviour *beh member.
		 See drivers/mySQL/mysqldriver.cpp for usage example.
		 */
		Driver( QObject *parent, const char *name, const QStringList &args = QStringList() );

		/*! For reimplemenation: creates and returns connection object 
		 with additional structures specific for a given driver.
		 Connection object should inherit Connection and have a destructor 
		 that descructs all allocated driver-dependent connection structures. */
		virtual Connection *drv_createConnection( ConnectionData &conn_data ) = 0;
//virtual ConnectionInternal* createConnectionInternalObject( Connection& conn ) = 0;

		/*! Driver-specific SQL string escaping.
		 This method is used by escapeIdentifier().
		 Implement escaping for any character like " or ' as your 
		 database engine requires. Do not append or prepend any quotation 
		 marks characters - it is automatically done by escapeIdentifier() using
		 DriverBehaviour::QUOTATION_MARKS_FOR_IDENTIFIER.
		*/
		virtual QString drv_escapeIdentifier( const QString& str ) const = 0;
		
		/*! This is overloaded version of drv_escapeIdentifier( const QString& str )
		 to be implemented in the same way.
		*/
		virtual QCString drv_escapeIdentifier( const QCString& str ) const = 0;
		
		/*! \return true if \a n is a system field's name, build-in system 
		 field that cannot be used or created by a user,
		 and in most cases user even shouldn't see this. The list is specific for 
		 a given driver implementation. For implementation.*/
		virtual bool drv_isSystemFieldName( const QString& n ) const = 0;
		
		/* Creates admin tools object providing a number of database administration 
		 tools for the driver. This is called once per driver.

		 Note for driver developers: Reimplement this method by returning 
		 KexiDB::AdminTools-derived object. Default implementation creates 
		 empty admin tools. 
		 @see adminTools() */
		virtual AdminTools* drv_createAdminTools() const;

		/*! \return connection \a conn , do not deletes it nor affect.
		 Returns 0 if \a conn is not owned by this driver.
		 After this, you are owner of \a conn object, so you should
		 eventually delete it. Better use Connection destructor. */
		Connection* removeConnection( Connection *conn );

		/*! Helper, used in escapeBLOB(). 
		 * use \a type == BLOB_ESCAPING_TYPE_USE_X to get escaping like X'ABCD0' (used by sqlite)
		 * use \a type == BLOB_ESCAPING_TYPE_USE_0x to get escaping like 0xABCD0 (used by mysql)
		 * use \a type == BLOB_ESCAPING_TYPE_USE_OCTAL to get escaping like '\\253\\315\\000' 
		     (used by pgsql)
		*/
		QString escapeBLOBInternal(const QByteArray& array, int type) const;

	friend class Connection;
	friend class Cursor;
	friend class DriverManagerInternal;


	/*! Used to initialise the dictionary of driver-specific keywords.
	    Should be called by the Driver's constructor.
	    \a hashSize is the number of buckets to use in the dictionary.
	    \sa DriverPrivate::SQL_KEYWORDS. */
	void initSQLKeywords(int hashSize = 17);

	DriverBehaviour *beh;
	DriverPrivate *d;
};

} //namespace KexiDB

/*! Driver's static version information, automatically impemented for KexiDB drivers.
 Put this into driver class declaration just like Q_OBJECT macro. */
#define KEXIDB_DRIVER \
	public: \
	virtual int versionMajor() const; \
	virtual int versionMinor() const;

#endif

