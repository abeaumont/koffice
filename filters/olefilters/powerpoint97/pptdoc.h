/*
    Copyright (C) 2000, S.R.Haque <shaheedhaque@hotmail.com>.
    This file is part of the KDE project

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

DESCRIPTION

    This file implements a simplified abstraction of Microsoft Powerpoint documents.
*/

#ifndef PPTDOC_H
#define PPTDOC_H

#include <kdebug.h>
#include <myfile.h>
#include <powerpoint.h>
#include <qstring.h>
#include <qstringlist.h>

#define TITLE_TEXT 		0	//title
#define BODY_TEXT 		1	//body
#define NOTES_TEXT 		2	//notes
#define NOTUSED_TEXT 		3	//not used
#define OTHER_TEXT 		4	//other(test in shape)
#define CENTER_BODY_TEXT 	5	//center body(subtitle in title slide)
#define CENTER_TITLE_TEXT 	6	//center title(title in title slide)
#define HALF_BODY_TEXT 		7	//half body(body in two-column slide)
#define QUARTER_BODY_TEXT 	8	//quarter body(body in four-body slide)

class myFile;

class PptDoc:
    protected Powerpoint
{
protected:

    // Construction. Invoke with the OLE streams that comprise the Powerpoint document.

    PptDoc(
        const myFile &mainStream,
        const myFile &currentUser);
    virtual ~PptDoc();

    // Metadata.

    QString getAuthor(void) const;

    // Call the parse() function to process the document. The callbacks return
    // the text along with any relevant attributes.

    bool parse();

    typedef struct
    {
        QString title;
        QStringList body;
        QString notes;
	QString styleRun;
    } Slide;

    virtual void gotDrawing(
        unsigned id,
        QString type,
        unsigned length,
        const char *data) = 0;
    virtual void gotSlide(
        const Slide &slide) = 0;

private:
    PptDoc(const PptDoc &);
    const PptDoc &operator=(const PptDoc &);

    // Error handling and reporting support.

    static const int s_area = 30512;

    myFile m_mainStream;
    myFile m_currentUser;

    // Override the base class functions.

    void gotSlide(
        Powerpoint::Slide &slide);
};
#endif
