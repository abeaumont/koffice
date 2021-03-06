/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2004 Tomas Mecir <mecirt@gmail.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KC_VALUE_FORMATTER
#define KC_VALUE_FORMATTER

#include <QDateTime>

#include "Global.h"
#include "KCNumber.h"
#include "KCStyle.h"

class KCCalculationSettings;
class KCValue;
class KCValueConverter;

/**
 * \ingroup KCValue
 * Generates a textual representation of a KCValue with a given formatting.
 */
class KCELLS_EXPORT KCValueFormatter
{
public:
    /**
     * Constructor.
     */
    explicit KCValueFormatter(const KCValueConverter* converter);

    /**
     * Returns the calculation settings this KCValueFormatter uses.
     */
    const KCCalculationSettings* settings() const;

    /**
     * Creates a textual representation of \p value with the explicit given
     * formattings.
     * \param formatType the value format, e.g. number, date
     * \param precision the number of decimals
     * \param floatFormat the number format, i.e. signed/unsigned information
     * \param prefix the preceding text
     * \param postfix the subsequent text
     * \param currencySymbol the currency symbol
     */
    KCValue formatText(const KCValue& value,
                     KCFormat::Type formatType, int precision = -1,
                     KCStyle::FloatFormat floatFormat = KCStyle::OnlyNegSigned,
                     const QString& prefix = QString(),
                     const QString& postfix = QString(),
                     const QString& currencySymbol = QString(), const QString& formatString = QString());

    /**
     * Creates a date format.
     * \param formatType the value format, e.g. number, date
     */
    QString dateFormat(const QDate& date, KCFormat::Type formatType, const QString& formatString = QString() );

    /**
     * Creates a time format.
     * \param formatType the value format, e.g. number, date
     */
    QString timeFormat(const QDateTime& time, KCFormat::Type formatType, const QString& formatString = QString() );

    /**
     * Creates a date and time format.
     * \param formatType the value format, e.g. number, date
     */
    QString dateTimeFormat(const QDateTime& time, KCFormat::Type formatType, const QString& formatString = QString() );

protected:
    /**
     * Determines the formatting type that should be used to format this value
     * in a cell with a given format type
     * \param formatType the value format, e.g. number, date
     */
    KCFormat::Type determineFormatting(const KCValue& value, KCFormat::Type formatType);

    /**
     * Creates a number format.
     * \param precision the number of decimals
     * \param formatType the value format, e.g. number, date
     * \param floatFormat the number format, i.e. signed/unsigned information
     * \param currencySymbol the currency symbol
     */
    QString createNumberFormat(KCNumber value, int precision,
                               KCFormat::Type formatType,
                               KCStyle::FloatFormat floatFormat,
                               const QString& currencySymbol,
                               const QString& formatString);

    /**
     * Creates a fraction format.
     * \param formatType the value format, e.g. number, date
     */
    QString fractionFormat(KCNumber value, KCFormat::Type formatType);

    /**
     * Creates a complex number format.
     * \param precision the number of decimals
     * \param formatType the value format, e.g. number, date
     * \param floatFormat the number format, i.e. signed/unsigned information
     * \param currencySymbol the currency symbol
     */
    QString complexFormat(const KCValue& value, int precision,
                          KCFormat::Type formatType,
                          KCStyle::FloatFormat floatFormat,
                          const QString& currencySymbol);

    /**
     * Removes the trailing zeros and the decimal symbol \p decimalSymbol in
     * \p string , if necessary.
     * \return the truncated string
     */
    QString removeTrailingZeros(const QString& string, const QString& decimalSymbol);

private:
    const KCValueConverter* m_converter;
};

#endif  //KC_VALUE_FORMATTER
