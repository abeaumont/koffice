/* This file is part of the KDE project

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

#include <qstring.h>
#include <kdebug.h>

#include <xmltree.h>
#include <records.h>
#include <excelfilter.h>

ExcelFilter::ExcelFilter(const myFile &mainStream):FilterBase(), length(mainStream.length)
{
    length *= .85; // we reduce to 85% so that the progress-bar reachs 100%
    QByteArray a;
    a.setRawData((char*) mainStream.data, (int) mainStream.length);

    s = new QDataStream(a, IO_ReadOnly);
    s->setByteOrder(QDataStream::LittleEndian);

    tree = new XMLTree();
}

ExcelFilter::~ExcelFilter()
{
    delete s;
    s=0L;
    delete tree;
    tree=0L;
}

const bool ExcelFilter::filter()
{
    unsigned int i, cont = 0;
    double count = 0;

    Q_UINT8 byte;
    Q_UINT16 opcode, size, readAhead;
    Q_UINT32 contSize = 0;

    QByteArray record(MAX_RECORD_SIZE);
    QDataStream *body;

    *s >> opcode;
    *s >> size;
    count += size;

    s->readRawBytes(record.data(), size);
    *s >> readAhead;

    while (!s->atEnd() && m_success == true)
    {
        if (readAhead != 0x003c) // any other record, lets handle the current
        {
            body = new QDataStream(record, IO_ReadOnly);
            body->setByteOrder(QDataStream::LittleEndian);

            for (i = 0; biff[i].opcode != opcode && biff[i].opcode != 0; i++);

            if (biff[i].opcode == opcode)
            {
                if (cont)
                    m_success = (tree->*(biff[i].func))(contSize, *body);
                else
                    m_success = (tree->*(biff[i].func))(size, *body);
            }
            delete body;

            opcode = readAhead;
            *s >> size;
            count += size;

            if (size > MAX_RECORD_SIZE)
                kdError(30511) << "Record larger than MAX_RECORD_SIZE!" << endl;

            s->readRawBytes(record.data(), size);

            if (cont)
            {
                cont = 0;
                contSize = 0;
            }
        }
        else // a CONTINUE record, lets add it
        {
            cont = 1;
            *s >> size;
            *s >> byte; // we do a look-ahead
            record.resize(contSize + size);

            if (byte == 0) // skip the zero
            {
                --size;
                s->readRawBytes(record.data() + contSize, size);
            }
            else
            {
                *(record.data() + contSize) = byte;
                s->readRawBytes(record.data() + contSize + 1, size - 1);
            }
        }
        count += size;
        *s >> readAhead;

        if (readAhead == 0x003c)
            contSize += size;
        if (readAhead == 0) break; // we are at the end of the file

        emit sigProgress(count/length*100);
    }
    m_ready = true;

    return m_success;
}

const QDomDocument* const ExcelFilter::part()
{

    if(m_ready && m_success)
    {
        return tree->part();
    }
    else
    {
        m_part=QDomDocument("spreadsheet");
        m_part.setContent(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE spreadsheet > \n"
                                  "<spreadsheet author=\"Torben Weis\" email=\"weis@kde.org\" editor=\"KSpread\" mime=\"application/x-kspread\" >\n"
                                  "<paper format=\"A4\" orientation=\"Portrait\">\n"
                                  "<borders left=\"20\" top=\"20\" right=\"20\" bottom=\"20\"/>\n"
                                  "<head left=\"\" center=\"\" right=\"\"/>\n"
                                  "<foot left=\"\" center=\"\" right=\"\"/>\n"
                                  "</paper>\n"
                                  "<map>\n"
                                  "<table name=\"Table1\">\n"
                                  "<cell row=\"1\" column=\"1\">\n"
                                  "<format align=\"4\" precision=\"-1\" float=\"3\" floatcolor=\"2\" faktor=\"1\"/>\n"
                                  "Sorry :(\n"
                                  "</cell>\n"
                                  "</table>\n"
                                  "</map>\n"
                                  "</spreadsheet>"));
        return &m_part;
    }
}
