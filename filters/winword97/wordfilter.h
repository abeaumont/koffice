/******************************************************************/
/* WordFilter - (c) by Werner Trobin 1999                         */
/* Version: 0.0.1                                                 */
/* Author: Werner Trobin                                          */
/* E-Mail: wtrobin@carinthia.com                                  */
/* Homepage: http://members.carinthia.com/wtrobin/                */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Filter (header)                                        */
/******************************************************************/

#ifndef WORDFILTER_H
#define WORDFILTER_H

#include <filterbase.h>
#include <winworddoc.h>
#include <kworddoc.h>
#include <myfile.h>


class WordFilter : public FilterBase {

    Q_OBJECT

public:
    WordFilter(const myFile &mainStream, const myFile &table0Stream,
               const myFile &table1Stream, const myFile &dataStream);
    virtual ~WordFilter();
    virtual const QString part();

private:
    WordFilter(const WordFilter &);
    const WordFilter &operator=(const WordFilter &);

    WinWordDoc *myDoc;
    KWordDoc *myKwd;
};
#endif // WORDFILTER_H
