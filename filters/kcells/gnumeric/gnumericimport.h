/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef GNUMERICIMPORT_H
#define GNUMERICIMPORT_H

#include <KoFilter.h>

#include <QDateTime>
#include <qdom.h>
#include <QByteArray>
#include <QVariantList>

class KCCell;
class KCSheet;

class GNUMERICFilter : public KoFilter
{
    Q_OBJECT
public:
    GNUMERICFilter(QObject* parent, const QVariantList&);
    virtual ~GNUMERICFilter() {}

    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);

    enum borderStyle { Left, Right, Top, Bottom, Diagonal, Revdiagonal};
private:
    class GnumericDate : public QDate
    {
    public:
        static uint greg2jul(int y, int m, int d);
        static void jul2greg(double num, int & y, int & m, int & d);
        static QTime getTime(double num);

    };

    void dateInit();
    QString convertVars(QString const & str, KCSheet * table) const;
    void ParsePrintInfo(QDomNode const & printInfo, KCSheet * table);
    void ParseFormat(QString const & formatString, const KCCell& kcells_cell);
    void setStyleInfo(QDomNode * sheet, KCSheet * table);
    bool setType(const KCCell& kcells_cell, QString const & formatString, QString & cell_content);
    void convertFormula(QString & formula) const;
    void importBorder(QDomElement  border, borderStyle _style, const KCCell&cell);
    void ParseBorder(QDomElement & gmr_styleborder, const KCCell& kcells_cell);
    double parseAttribute(const QDomElement &_element);

    void setText(KCSheet* sheet, int row, int column, const QString& text, bool asString = false);
};
#endif // GNUMERICIMPORT_H
