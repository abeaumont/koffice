/*
 * Copyright (c) 2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PDFIMPORT_H
#define PDFIMPORT_H

#include <KoFilter.h>

#include "pdfdocument.h"
#include "dialog.h"
//Added by qt3to4:
#include <Q3CString>


class PdfImport : public KoFilter
{
    Q_OBJECT
public:
    PdfImport(QObject* parent, const QStringList&);

    KoFilter::ConversionStatus
    convert(const QByteArray& from, const QByteArray& to);

private:
    void treatInfoDocument();

private:
    PDFImport::Document _doc;
    PDFImport::Options  _options;
};

#endif
