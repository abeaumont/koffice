/* This file is part of the KDE project
   Copyright (C) 2000 Robert JACOLIN <rjacolin@ifrance.com>

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

#ifndef __XSLTIMPORTDIA_H__
#define __XSLTIMPORTDIA_H__

#include <kfiledialog.h>
#include <koStore.h>
#include "xsltdialog.h"

class XSLTImportDia : public XSLTDialog
{ 
    Q_OBJECT

	QString _fileIn;
	QString _fileOut;
	QByteArray _arrayIn;
	KoStore* _out;
	KURL _currentFile;
	QCString _format;

public:
    XSLTImportDia( const KoStore&, const QCString &format, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~XSLTImportDia();

	void setInputFile(QString file)  { _fileIn = file; }
	void setOutputFile(QString file) { _fileOut = file; }

public slots:
    virtual void cancelSlot();
    virtual void chooseSlot();
    virtual void okSlot();

};

#endif /* __XSLTIMPORTDIA_H__ */
