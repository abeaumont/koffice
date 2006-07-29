/* This file is part of the KDE project
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KEXI_VALIDATOR_H
#define KEXI_VALIDATOR_H

#include "kexiutils_export.h"

#include <qvalidator.h>
#include <qvariant.h>
#include <qstring.h>

#include <klocale.h>

namespace KexiUtils {

/*! KexiUtils::Validator class extends QValidator with offline-checking 
 for value's validity (check() method).
 Thus, it groups two purposes into one container:
 -string validator for line editors (online checking, "on typing")
 -offline-checking for QVariant values, reimplementing validate().

 It also offers error and warning messages for check() method. 
 You may need to reimplement: 
 -  QValidator::State IdentifierValidator::validate( QString& input, int& pos ) const;
 -  Result check(const QString &valueName, QVariant v, QString &message, QString &details);
 */
class KEXIUTILS_EXPORT Validator : public QValidator
{
	public:
		typedef enum Result { Error = 0, Ok = 1, Warning = 2 };

		Validator(QObject * parent = 0, const char * name = 0);
		virtual ~Validator();

		/*! Sets accepting empty values on (true) or off (false). 
		 By default the validator does not accepts empty values. */
		void setAcceptsEmptyValue( bool set ) { m_acceptsEmptyValue = set; }

		/*! Return accepting empty values flag. \sa setAcceptsEmptyValue(). */
		bool acceptsEmptyValue() const { return m_acceptsEmptyValue; }

		/*! Checks if value \a v is ok and returns one of \a Result value:
		 \a Error is returned on error, \a Ok on success, \a Warning if there 
		 is something to warn. In any case except \a Ok, 
		 i18n'ed \a message is set and (optionally) \a details are set, 
		 e.g. for use in a message box.
		 \a valueName can be used to contruct \a message as well, for example:
		 "[valueName] is not a valid login name". 
		 Depending on acceptsEmptyValue(), immediately accepts empty values or not. */
		Result check(const QString &valueName, const QVariant& v, QString &message,
			QString &details);

		/*! This implementation always return value QValidator::Acceptable. */
		virtual QValidator::State validate ( QString & input, int & pos ) const;

		//!Generic error/warning messages:
		static const QString msgColumnNotEmpty() {
			return I18N_NOOP("\"%1\" value has to be entered.");
		}

		void addChildValidator( Validator* v );

	protected:
		/* Used by check(), for reimplementation, 
		 by default always returns \a Error.*/
		virtual Result internalCheck(const QString &valueName, const QVariant& v, 
			QString &message, QString &details);

		bool m_acceptsEmptyValue : 1;

	friend class MultiValidator;
};

/*! MultiValidator behaves like normal KexiUtils::Validator,
 but it allows to add define more than one different validator.
 Given validation is successful if every subvalidator accepted given value.

 - acceptsEmptyValue() is used globally here 
   (no matter what is defined in subvalidators).

 - result of calling check() depends on value of check() returned by subvalidators:
   - Error is returned if at least one subvalidator returned Error;
   - Warning is returned if at least one subvalidator returned Warning and 
     no validator returned error;
   - Ok is returned only if exactly all subvalidators returned Ok.
   - If there is no subvalidators defined, Error is always returned.

 - result of calling validate() (a method implemented for QValidator)
   depends on value of validate() returned by subvalidators:
   - Invalid is returned if at least one subvalidator returned Invalid
   - Intermediate is returned if at least one subvalidator returned Intermediate
   - Acceptable is returned if exactly all subvalidators returned Acceptable.
   - If there is no subvalidators defined, Invalid is always returned.

*/
class KEXIUTILS_EXPORT MultiValidator : public Validator
{
	public:
		/*! Constructs multivalidator with no subvalidators defined.
		 You can add more validators with addSubvalidator(). */
		MultiValidator(QObject * parent = 0, const char * name = 0);

		/*! Constructs multivalidator with one \a validator.
		 It will be owned if has no parent defined.
		 You can add more validators with addSubvalidator(). */
		MultiValidator(Validator *validator, QObject * parent = 0, const char * name = 0);

		/*! Adds validator \a validator as another subvalidator.
		 Subvalidator will be owned by this multivalidator if \a owned is true
		 and its parent is NULL. */
		void addSubvalidator( Validator* validator, bool owned = true );

		/*! Reimplemented to call validate() on subvalidators. */
		virtual QValidator::State validate ( QString & input, int & pos ) const;

		/*! Calls QValidator::fixup() on every subvalidator. 
		 This may be senseless to use this methog in certain cases 
		 (can return weir results), so think twice before.. */
		virtual void fixup ( QString & input ) const;

	private:
		virtual Validator::Result internalCheck(
			const QString &valueName, const QVariant& v, 
			QString &message, QString &details);


	protected:
		QPtrList<Validator> m_ownedSubValidators;
		QValueList<Validator*> m_subValidators;
};

}

#endif
