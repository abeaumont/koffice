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

    The code in this file is mostly derived from the Microsoft specifications,
    but some of the workarounds for the broken specifications etc. come from
    the pioneering work on the "wvWare" library by Caolan McNamara (see
    http://www.wvWare.com).
*/

#include <errno.h>
#ifdef __linux__
#include <iconv.h>
#endif
#include <kdebug.h>
#include <msword.h>
#include <paragraph.h>
//#include <string.h>

short MsWord::char2unicode(unsigned char c)
{
    static const short CP2UNI[] =
    {
        0x20ac, 0x0000, 0x201a, 0x0192,
        0x201e, 0x2026, 0x2020, 0x2021,
        0x02c6, 0x2030, 0x0160, 0x2039,
        0x0152, 0x0000, 0x017d, 0x0000,
        0x0000, 0x2018, 0x2019, 0x201c,
        0x201d, 0x2022, 0x2013, 0x2014,
        0x02dc, 0x2122, 0x0161, 0x203a,
        0x0153, 0x0000, 0x017e, 0x0178
    };

    if (c <= 0x7f || c >= 0xa0)
        return static_cast<short>(c);
    else
        return CP2UNI[c-0x80];
/*
    char f_code[33];            // From CCSID
    char t_code[33];            // To CCSID
    iconv_t iconv_handle;       // Conversion Descriptor returned
                                // from iconv_open() function
    char *obuf;                 // Buffer for converted characters
    char *p;
    size_t ibuflen;             // Length of input buffer
    size_t obuflen;             // Length of output buffer
    const char *ibuf;
    char *codepage;
    char buffer[1];
    char buffer2[2];
    U16 eachchar = c;
    buffer[0]=eachchar;
    ibuf = buffer;
    obuf = buffer2;

    // All reserved positions of from code (last 12 characters) and to code
    // (last 19 characters) must be set to hexadecimal zeros.

    memset(f_code,'\0',33);
    memset(t_code,'\0',33);

    strcpy(f_code,"CP1252");
    strcpy(t_code,"UCS-2");

    iconv_handle = iconv_open(t_code,f_code);
    if (iconv_handle == (iconv_t)-1)
    {
        kdError(s_area) << "iconv_open fail: " << errno << " cannot convertto unicode" << endl;
        return('?');
    }

    ibuflen = 1;
    obuflen = 2;
    p = obuf;
    iconv(iconv_handle, &ibuf, &ibuflen, &obuf, &obuflen);
    eachchar = (U8)*p++;
    eachchar = (eachchar << 8)&0xFF00;
    eachchar += (U8)*p;

    iconv_close(iconv_handle);
    return(eachchar);
*/
}

void MsWord::constructionError(unsigned line, const char *reason)
{
    m_constructionError="[" + QString(__FILE__ ) + ":" + QString::number(line) + "]" + reason;
    kdError(s_area) << m_constructionError << endl;
}

void MsWord::decodeParagraph(const QString &text, MsWord::PHE &layout, MsWord::PAPXFKP &style)
{
    Paragraph paragraph = Paragraph(*this);

    // Work out the paragraph details.

    paragraph.apply(style);
    paragraph.apply(layout);

    // We treat table paragraphs somewhat differently...so deal with
    // them first.

    if (paragraph.m_pap.fInTable)
    {
        if (!m_wasInTable)
        {
            gotTableBegin();
            m_tableColumn = 0;
        }
        m_wasInTable = true;

        // When we get to the end of the row, output the whole lot.

        if (paragraph.m_pap.fTtp)
        {
            MsWord::TAP tap;

            // A TAP describes the row.

            memset(&tap, 0, sizeof(tap));
            paragraph.apply(style.grpprl, style.grpprlBytes, &tap);
            gotTableRow(m_tableText, m_tableStyle, tap);
            m_tableColumn = 0;
        }
        else
        {
            m_tableText[m_tableColumn] = text;
            m_tableStyle[m_tableColumn] = paragraph.m_pap;
            m_tableColumn++;
        }
        return;
    }
    if (m_wasInTable)
        gotTableEnd();
    m_wasInTable = false;

    // What kind of paragraph was this?

    if ((paragraph.m_pap.istd >= 1) && (paragraph.m_pap.istd <= 9))
    {
        gotHeadingParagraph(text, paragraph.m_pap);
    }
    else
    if (paragraph.m_pap.ilfo)
    {
        const U8 *ptr = m_tableStream + m_fib.fcPlfLfo; //lcbPlfLfo.
        const U8 *ptr2;
        const U8 *ptr3;
        U32 lfoCount;
        int i;

        // Find the number of LFOs.

        ptr += MsWordGenerated::read(ptr, &lfoCount);
        ptr2 = ptr + lfoCount * sizeof(LFO);
        if (lfoCount < paragraph.m_pap.ilfo)
            kdError(s_area) << "MsWord::error finding LFO[" << paragraph.m_pap.ilfo << "]" << endl;

        // Skip all the LFOs before our one, so that we can traverse the variable
        // length LFOLVL arrays.

        for (i = 1; i < paragraph.m_pap.ilfo; i++)
        {
            LFO data;
            LFOLVL levelOverride;
            LVLF level;
            U16 numberTextLength;
            QString numberText;

            // Read the LFO, and then skip any LFOLVLs.

            ptr += MsWordGenerated::read(ptr, &data);
            for (unsigned j = 0; j < data.clfolvl; j++)
            {
                ptr2 += MsWordGenerated::read(ptr2, &levelOverride);
                if (levelOverride.fFormatting)
                {
                    ptr2 += MsWordGenerated::read(ptr2, &level);
                    ptr3 = ptr2;
                    ptr2 += level.cbGrpprlPapx;
                    ptr2 += level.cbGrpprlChpx;
                    ptr2 += MsWordGenerated::read(ptr2, &numberTextLength);
                    ptr2 += read(ptr2, &numberText, numberTextLength, true);
                }
            }
        }

        // We have found the LFO from its 1-based array. Check to see if there are any overrides for this particular level.

        LFO data;
        LFOLVL levelOverride;
        LVLF level;
        U16 numberTextLength;
        QString numberText;

        // Read our LFO, and then search any LFOLVLs for a matching level.

        ptr += MsWordGenerated::read(ptr, &data);
        for (i = 0; i < data.clfolvl; i++)
        {
            ptr2 += MsWordGenerated::read(ptr2, &levelOverride);
            if (levelOverride.fFormatting)
            {
                ptr2 += MsWordGenerated::read(ptr2, &level);
                ptr3 = ptr2;
                ptr2 += level.cbGrpprlPapx;
                ptr2 += level.cbGrpprlChpx;
                ptr2 += MsWordGenerated::read(ptr2, &numberTextLength);
                ptr2 += read(ptr2, &numberText, numberTextLength, true);
            }

            // If this LFOLVL is ours, we are done!

            if (paragraph.m_pap.ilvl == levelOverride.ilvl)
            {
                break;
            };
        }
        if (i == data.clfolvl)
        {
            // No overriding LFOLVL was found.

            levelOverride.fFormatting = false;
            levelOverride.fStartAt = false;
        }

        // If the LFOLVL was not a complete override, resort to the LSTs for whatever
        // is missing.

        paragraph.apply(data, !levelOverride.fFormatting, !levelOverride.fStartAt);
        if (levelOverride.fStartAt)
        {
            // Apply the startAt.

            paragraph.m_pap.anld.iStartAt = levelOverride.iStartAt;
            kdDebug(s_area) << "got startAt " << paragraph.m_pap.anld.iStartAt << " from LFOLVL" << endl;
        }
        if (levelOverride.fFormatting)
        {
            // Apply the grpprl.

            kdDebug(s_area) << "getting formatting from LFO" << endl;
            paragraph.apply(ptr3, level.cbGrpprlPapx);

            // Apply the startAt.

            paragraph.m_pap.anld.iStartAt = level.iStartAt;
            kdDebug(s_area) << "got startAt " << paragraph.m_pap.anld.iStartAt << " from LVLF" << endl;
        }
        gotListParagraph(text, paragraph.m_pap);
    }
    else
    {
        gotParagraph(text, paragraph.m_pap);
    }
}

template <class T1, class T2>
MsWord::Fkp<T1, T2>::Fkp(FIB &fib) :
    m_fib(fib)
{
};

//
// Get the next entry in an FKP.
//
template <class T1, class T2>
bool MsWord::Fkp<T1, T2>::getNext(
    U32 *startFc,
    U32 *endFc,
    U8 *rgb,
    T1 *data1,
    T2 *data2)
{
    // Sanity check accesses beyond end of Fkp.

    if (m_i >= m_crun)
    {
        return false;
    }

    // Get fc range.

    m_fcNext += MsWordGenerated::read(m_fcNext, startFc);
    MsWordGenerated::read(m_fcNext, endFc);

    // Get word offset to the second piece of data, and the first piece of data
    // if required.

    m_dataNext += MsWordGenerated::read(m_dataNext, rgb);
    if (data1)
        m_dataNext += read(m_fib.nFib, m_dataNext, data1);

    if (!(*rgb))
    {
        // If the word offset is zero, then the second piece of data is
        // not explicitly stored.
    }
    else
    {
        // Get the second piece of data.
        MsWord::read(m_fib.nFib, m_fkp + (2 * (*rgb)), data2);
    }
    return (m_i++ < m_crun);
}

template <class T1, class T2>
void MsWord::Fkp<T1, T2>::startIteration(const U8 *fkp)
{
    U32 startFc;

    // Get the number of entries in the FKP.

    m_fkp = fkp;
    MsWordGenerated::read(m_fkp + 511, &m_crun);
    m_fcNext = m_fkp;
    m_dataNext = m_fkp + ((m_crun + 1) * sizeof(startFc));
    m_i = 0;
}

void MsWord::getCHPXFKP()
{
    // A bin table is a plex of BTEs.

    Plex<BTE, 2> btes = Plex<BTE, 2>(m_fib);
    U32 startFc;
    U32 endFc;
    BTE data;

    // Walk the BTEs.

    btes.startIteration(m_tableStream + m_fib.fcPlcfbteChpx, m_fib.lcbPlcfbteChpx);
    while (btes.getNext(&startFc, &endFc, &data))
    {
        getCHPX(m_mainStream + (data.pn * 512));
    }
}

void MsWord::getCHPX(const U8 *fkp)
{
    // A CHPX FKP contains no extra data, specify a dummy PHE for the template.

    Fkp<PHE, CHPXFKP> chpx = Fkp<PHE, CHPXFKP>(m_fib);

    U32 startFc;
    U32 endFc;
    U8 rgb;
    CHPXFKP style;

    chpx.startIteration(fkp);
    while (chpx.getNext(&startFc, &endFc, &rgb, NULL, &style))
    {
        //kdDebug(s_area) << "chp from: " << startFc << ".." << endFc << ": rgb: " << rgb << endl;
    }
}

void MsWord::getPAPXFKP(const U8 *textStartFc, U32 textLength, bool unicode)
{
    // A bin table is a plex of BTEs.

    Plex<BTE, 2> btes = Plex<BTE, 2>(m_fib);
    U32 startFc;
    U32 endFc;
    BTE data;

    // Walk the BTEs.

    btes.startIteration(m_tableStream + m_fib.fcPlcfbtePapx, m_fib.lcbPlcfbtePapx);
    while (btes.getNext(&startFc, &endFc, &data))
    {
        getPAPX(
            m_mainStream + (data.pn * 512),
            textStartFc,
            textLength,
            unicode);
    }
}

void MsWord::getPAPX(
    const U8 *fkp,
    const U8 *textStartFc,
    U32 textLength,
    bool unicode)
{
    // A PAPX FKP contains PHEs.

    Fkp<PHE, PAPXFKP> papx = Fkp<PHE, PAPXFKP>(m_fib);

    U32 startFc;
    U32 endFc;
    U8 rgb;
    PHE layout;
    PAPXFKP style;

    papx.startIteration(fkp);
    while (papx.getNext(&startFc, &endFc, &rgb, &layout, &style))
    {
        QString text;

        //kdDebug(s_area) << "pap from: " << startFc << ".." << endFc << ": rgb: " << rgb << endl;
        read(m_mainStream + startFc, &text, endFc - startFc, unicode);
        decodeParagraph(text, layout, style);
    }
}

// Create a cache of information about lists.
//
//    m_listLevels: an array of arrays of pointers to LVLFs for each list style in the
//    LST array. The entries must be looked up using the list id and list level.

void MsWord::getListStyles()
{
    const U8 *ptr = m_tableStream + m_fib.fcPlcfLst; //lcbPlcfLst.
    const U8 *ptr2;
    U16 lstfCount;

    // Failsafe for simple documents.

    m_listStyles = NULL;
    if (!m_fib.lcbPlcfLst)
    {
        kdDebug(s_area) << "MsWord::getListStyles: no data " << endl;
        return;
    }

    // Find the number of LSTFs.

    ptr += MsWordGenerated::read(ptr, &lstfCount);
    ptr2 = ptr + lstfCount * sizeof(LSTF);

    // Construct the array of styles, and then walk the array reading in the style definitions.

    m_listStyles = new LVLF **[lstfCount];
    for (unsigned i = 0; i < lstfCount; i++)
    {
        LSTF data;
        unsigned levelCount;

        ptr += MsWordGenerated::read(ptr, &data);
        if (data.fSimpleList)
            levelCount = 1;
        else
            levelCount = 9;

        // Create an array of LVLF pointers, one for each level in the list.

        m_listStyles[i] = new LVLF *[levelCount];
        for (unsigned j = 0; j < levelCount; j++)
        {
            m_listStyles[i][j] = (LVLF *)ptr2;

            // Skip the variable length parts.

            LVLF level;
            U16 numberTextLength;
            QString numberText;

            ptr2 += MsWordGenerated::read(ptr2, &level);
            ptr2 += level.cbGrpprlPapx;
            ptr2 += level.cbGrpprlChpx;
            ptr2 += MsWordGenerated::read(ptr2, &numberTextLength);
            ptr2 += read(ptr2, &numberText, numberTextLength, true);
        }
    }
}

// Create a cache of information about built-in styles.
//
// The cache consists of:
//
//    m_styles: an array of fully-decoded PAPs for each built in style
//    indexed by istd.

void MsWord::getStyles()
{
    const U8 *ptr = m_tableStream + m_fib.fcStshf;
    U16 cbStshi;
    STSHI stshi;

    // Failsafe for simple documents.

    m_styles = NULL;
    if (!m_fib.lcbStshf)
    {
        kdError(s_area) << "MsWord::getListStyles: no data " << endl;
        return;
    }

    // Fetch the STSHI.

    ptr += MsWordGenerated::read(ptr, &cbStshi);
    if (cbStshi > sizeof(stshi))
    {
        kdError(s_area) << "MsWord::getStyles: unsupported STSHI size " << cbStshi << endl;
        return;
    }

    // We know that older/smaller STSHIs can simply be zero extended into our STSHI.
    // So, we overwrite anything that is not valid with zeros.

    ptr += MsWordGenerated::read(ptr, &stshi);
    memset(((char *)&stshi) + cbStshi, 0, sizeof(stshi) - cbStshi);
    ptr -= sizeof(stshi) - cbStshi;

    // Construct the array of styles, and then walk the array reading in the style definitions.

    m_styles = new Paragraph *[stshi.cstd];
    for (unsigned i = 0; i < stshi.cstd; i++)
    {
        U16 cbStd;
        STD std;

        ptr += MsWordGenerated::read(ptr, &cbStd);
        if (cbStd)
        {
            read(ptr, stshi.cbSTDBaseInFile, &std);
            kdDebug(s_area) << "MsWord::getStyles: style: " << std.xstzName <<
                ", types: " << std.cupx <<
                endl;

            // If this is a paragraph style, fill it.

            if (std.sgc == 1)
            {
                m_styles[i] = new Paragraph(*this);
                m_styles[i]->apply(std);
            }
        }
        else
        {
            // Set the style to be the same as stiNormal. This is a purely
            // defensive thing...and relies on a viable 0th entry.

            m_styles[i] = m_styles[0];
        }
        ptr += cbStd;
    }
}

void MsWord::gotParagraph(const QString &text, PAP &style)
{
    kdDebug(s_area) << "MsWord::gotParagraph: normal" << endl;
}

void MsWord::gotHeadingParagraph(const QString &text, PAP &style)
{
    kdDebug(s_area) << "MsWord::gotParagraph: heading level: " << style.istd << ": " << text << endl;
}

void MsWord::gotListParagraph(const QString &text, PAP &style)
{
    kdDebug(s_area) << "MsWord::gotParagraph: list level: " << style.ilvl << endl;
}

void MsWord::gotTableBegin()
{
    kdDebug(s_area) << "MsWord::gotParagraph: table begin" << endl;
}

void MsWord::gotTableEnd()
{
    kdDebug(s_area) << "MsWord::gotParagraph: table end" << endl;
}

void MsWord::gotTableRow(const QString texts[], const PAP styles[], TAP &row)
{
    kdDebug(s_area) << "MsWord::gotParagraph: table row: cells: " << row.itcMac << endl;
}

MsWord::MsWord(
        const U8 *mainStream,
        const U8 *table0Stream,
        const U8 *table1Stream,
        const U8 *dataStream)
{
    m_constructionError = QString("");
    m_fib.nFib = s_minWordVersion;
    read(mainStream, &m_fib);
    if (m_fib.nFib <= s_minWordVersion)
    {
        constructionError(__LINE__, "the document was created using an unsupported version of Word");
        return;
    }
    if (m_fib.fEncrypted)
    {
        constructionError(__LINE__, "the document is encrypted");
        return;
    }
    kdDebug(s_area) << "MsWord::MsWord: nFib: " << m_fib.nFib << endl;
    kdDebug(s_area) << "MsWord::MsWord: lid: " << m_fib.lid << " lidFE: " << m_fib.lidFE << endl;

    // Store away the streams for future use. Note that we do not
    // copy the contents of the streams, and that we rely on the storage
    // being present until we are destroyed.

    m_mainStream = mainStream;
    m_tableStream = m_fib.fWhichTblStm ? table1Stream : table0Stream;
    m_dataStream = dataStream;
    if (!m_tableStream)
    {
        // Older versions of Word had no separate table stream.

        kdDebug(s_area) << "MsWord::MsWord: no table stream" << endl;
        m_tableStream = m_mainStream;
    }
    if (!m_dataStream)
    {
        // Older versions of Word had no separate data stream.

        kdDebug(s_area) << "MsWord::MsWord: no data stream" << endl;
        m_dataStream = m_mainStream;
    }
    getStyles();
    getListStyles();
    getCHPXFKP();
}

MsWord::~MsWord()
{
}

void MsWord::parse()
{
    if (m_constructionError.length())
    {
       gotError(m_constructionError);
       return;
    }

    // Fill the style cache.

    m_wasInTable = false;

    // Note that we test for the presence of complex structure, rather than
    // m_fib.fComplex. This allows us to treat newer files which always seem
    // to have piece tables in a consistent manner.
    //
    // There is also the implication that without the complex structures, the
    // text cannot be in unicode form.

    if (m_fib.lcbClx)
    {
        // Start with the grpprl and PCD.
        //
        // For the grpprl array, we store the offset to the
        // byte count preceeding the first entry, and the number of entries.
        //
        // For the text plex, we store the start and size of the plex in the table

        typedef enum
        {
            clxtGrpprl = 1,
            clxtPlcfpcd = 2
        };

        struct
        {
            U32 byteCountOffset;
            U32 count;
        } grpprls;

        struct
        {
            const U8 *ptr;
            U32 byteCount;
        } textPlex;

        unsigned count = 0;
        const U8 *ptr;
        const U8 *end;
        U8 clxt = 0;
        U16 cb;
        U32 lcb;

        // First skip the grpprls.

        ptr = m_tableStream + m_fib.fcClx;
        end = ptr + m_fib.lcbClx;
        grpprls.byteCountOffset = (ptr + 1) - m_tableStream;
        grpprls.count = 0;
        while (ptr < end)
        {
            ptr += MsWordGenerated::read(ptr, &clxt);
            if (clxt != clxtGrpprl)
            {
                ptr--;
                break;
            }
            grpprls.count++;
            ptr += MsWordGenerated::read(ptr, &cb);
            ptr += cb;
        }

        // Now locate the piece table.

        while (ptr < end)
        {
            ptr += MsWordGenerated::read(ptr, &clxt);
            if (clxt != clxtPlcfpcd)
            {
                ptr--;
                break;
            }
            count++;
            ptr += MsWordGenerated::read(ptr, &lcb);
            textPlex.byteCount = lcb;
            textPlex.ptr = ptr;
            ptr += lcb;
        }
        if ((clxt != clxtPlcfpcd) ||
            (count != 1))
        {
            constructionError(__LINE__, "cannot locate the piece table");
            return;
        };

        // Locate the piece table in a complex document.

        Plex<PCD, 8> *pieceTable = new Plex<PCD, 8>(m_fib);

        U32 startFc;
        U32 endFc;
        PCD data;
        const U32 codepage1252mask = 0x40000000;
        bool unicode;

        pieceTable->startIteration(textPlex.ptr, textPlex.byteCount);
        while (pieceTable->getNext(&startFc, &endFc, &data))
        {
            unicode = ((data.fc & codepage1252mask) != codepage1252mask);
            if (!unicode)
            {
                data.fc &= ~ codepage1252mask;
                data.fc /= 2;
            }
            getPAPXFKP(m_mainStream + data.fc, endFc - startFc, unicode);
        }
    }
    else
    {
        getPAPXFKP(
            m_mainStream + m_fib.fcMin,
            m_fib.fcMac - m_fib.fcMin,
            false);
    }
}

template <class T, int word6Size>
MsWord::Plex<T, word6Size>::Plex(FIB &fib) :
    m_fib(fib)
{
};

template <class T, int word6Size>
bool MsWord::Plex<T, word6Size>::getNext(U32 *startFc, U32 *endFc, T *data)
{
    // Sanity check accesses beyond end of Plex.

    if (m_i >= m_crun)
    {
        return false;
    }
    m_fcNext += MsWordGenerated::read(m_fcNext, startFc);
    MsWordGenerated::read(m_fcNext, endFc);
    m_dataNext += MsWord::read(m_fib.nFib, m_dataNext, data);
    m_i++;
    return true;
}

template <class T, int word6Size>
void MsWord::Plex<T, word6Size>::startIteration(const U8 *plex, const U32 byteCount)
{
    U32 startFc;

    m_plex = plex;
    m_byteCount = byteCount;

    // Calculate the number of entries in the plex.

    if (m_fib.nFib > s_maxWord6Version)
        m_crun = (m_byteCount - sizeof(startFc)) / (sizeof(T) + sizeof(startFc));
    else
        m_crun = (m_byteCount - sizeof(startFc)) / (word6Size + sizeof(startFc));
    kdDebug(s_area) << "MsWord::Plex::startIteration: " << m_crun << endl;
    m_fcNext = m_plex;
    m_dataNext = m_plex + ((m_crun + 1) * sizeof(startFc));
    m_i = 0;
}

// Read a string, converting to unicode if needed.
unsigned MsWord::read(const U8 *in, QString *out, unsigned count, bool unicode)
{
    U16 char16;
    U8 char8;
    unsigned bytes = 0;

    *out = QString("");
    if (unicode)
    {
        for (unsigned i = 0; i < count; i++)
        {
            bytes += MsWordGenerated::read(in + bytes, &char16);
            *out += QChar(char16);
        }
    }
    else
    {
        for (unsigned i = 0; i < count; i++)
        {
            bytes += MsWordGenerated::read(in + bytes, &char8);
            *out += QChar(char2unicode(char8));
        }
    }
    return bytes;
}

//
// Read a CHPX as stored in a FKP.
//
unsigned MsWord::read(unsigned nFib, const U8 *in, CHPXFKP *out)
{
    unsigned bytes = 0;

    bytes += MsWordGenerated::read(in + bytes, &out->grpprlBytes);
    out->grpprl = (U8 *)(in + bytes);
    bytes += out->grpprlBytes;
    return bytes;
}

//
// Read a PAPX as stored in a FKP.
//
unsigned MsWord::read(unsigned nFib, const U8 *in, PAPXFKP *out)
{
    unsigned bytes = 0;
    U8 cw;

    bytes += MsWordGenerated::read(in + bytes, &cw);
    if (nFib > s_maxWord6Version)
    {
        if (!cw)
        {
            bytes += MsWordGenerated::read(in + bytes, &cw);
            out->grpprlBytes = 2 * (cw - 1);
        }
        else
        {
            out->grpprlBytes = 2 * (cw - 1) - 1;
        }
        bytes += MsWordGenerated::read(in + bytes, &out->istd);
        out->grpprl = (U8 *)(in + bytes);
        bytes += out->grpprlBytes;
    }
    else
    {
        U8 tmp;

        out->grpprlBytes = 2 * (cw - 1);
        // The spec says that the Word6 istd is a byte, but that seems to be wrong.
        bytes += MsWordGenerated::read(in + bytes, &out->istd);
        out->grpprl = (U8 *)(in + bytes);
        bytes += out->grpprlBytes;
    }
    return bytes;
}

unsigned MsWord::read(const U8 *in, unsigned baseInFile, STD *out, unsigned count)
{
    U8 *ptr = (U8 *)out;
    unsigned bytes = 0;

    for (unsigned i = 0; i < count; i++)
    {
        U8 offset;

        offset = 0;
        offset += MsWordGenerated::read(in + offset, (U16 *)(ptr + bytes), 5);
        memset((ptr + bytes) + baseInFile, 0, 10 - baseInFile);
        offset -= 10 - baseInFile;

        // If the baseInFile is less than 10, then the style name is not stored in unicode!

        if (baseInFile < 10)
        {
            U8 nameLength;
            U8 terminator;

            offset += MsWordGenerated::read(in + offset, &nameLength);
            offset += read(in + offset, &out->xstzName, nameLength, false);
            offset += MsWordGenerated::read(in + offset, &terminator);
        }
        else
        {
            U16 nameLength;
            U16 terminator;

            offset += MsWordGenerated::read(in + offset, &nameLength);
            offset += read(in + offset, &out->xstzName, nameLength, true);
            offset += MsWordGenerated::read(in + offset, &terminator);
        }
        out->grupx = in + offset;
        if ((int)out->grupx & 1)
            out->grupx++;
        bytes += out->bchUpe;
        out++;
    }
    return bytes;
} // STD

unsigned MsWord::read(const U8 *in, FIB *out, unsigned count)
{
    U8 *ptr = (U8 *)out;
    unsigned bytes = 0;

    for (unsigned i = 0; i < count; i++)
    {
        // What version of Word are we dealing with?
        // Word 6 for the PC writes files with nFib = 101-102.
        // Word 6 for the Mac writes files with nFib = 103-104.
        // Word 8 (a.k.a. Winword 97) and later products write files with nFib > 105.

        // Bytes 0 to 31 are common.

        bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes), 7);
        bytes += MsWordGenerated::read(in + bytes, (U32 *)(ptr + bytes), 1);
        bytes += MsWordGenerated::read(in + bytes, (U8 *)(ptr + bytes), 2);
        bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes), 2);
        bytes += MsWordGenerated::read(in + bytes, (U32 *)(ptr + bytes), 2);
        if (out->nFib > s_maxWord6Version)
        {
            bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes), 16);
            bytes += MsWordGenerated::read(in + bytes, (U32 *)(ptr + bytes), 22);
            bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes), 1);
            bytes += MsWordGenerated::read(in + bytes, (U32 *)(ptr + bytes), 186);
        }
        else
        if (out->nFib > s_minWordVersion)
        {
            // We will convert the FIB into the same form as for Winword

            out->csw = 14;
            out->wMagicCreated = 0;
            out->wMagicRevised = 0;
            out->wMagicCreatedPrivate = 0;
            out->wMagicRevisedPrivate = 0;
            out->pnFbpChpFirst_W6 = 0;
            out->pnChpFirst_W6 = 0;
            out->cpnBteChp_W6 = 0;
            out->pnFbpPapFirst_W6 = 0;
            out->pnPapFirst_W6 = 0;
            out->cpnBtePap_W6 = 0;
            out->pnFbpLvcFirst_W6 = 0;
            out->pnLvcFirst_W6 = 0;
            out->cpnBteLvc_W6 = 0;
            out->lidFE = out->lid;
            out->clw = 22;
            bytes += MsWordGenerated::read(in + bytes, &out->cbMac);
            bytes += 16;
            out->lProductCreated = 0;
            out->lProductRevised = 0;

            // ccpText through ccpHdrTxbx.

            bytes += MsWordGenerated::read(in + bytes, &out->ccpText, 8);

            // ccpSpare2.

            bytes += 4;
            out->cfclcb = 93;

            // fcStshfOrig through lcbSttbfAtnbkmk.

            bytes += MsWordGenerated::read(in + bytes, &out->fcStshfOrig, 76);

            // wSpare4Fib.

            bytes += 2;

            // pnChpFirst through cpnBtePap.

            U16 tmp;
            bytes += MsWordGenerated::read(in + bytes, &tmp);
            out->pnChpFirst = tmp;
            bytes += MsWordGenerated::read(in + bytes, &tmp);
            out->pnPapFirst = tmp;
            bytes += MsWordGenerated::read(in + bytes, &tmp);
            out->cpnBteChp = tmp;
            bytes += MsWordGenerated::read(in + bytes, &tmp);
            out->cpnBtePap = tmp;

            // fcPlcdoaMom through lcbSttbFnm.

            bytes += MsWordGenerated::read(in + bytes, &out->fcPlcdoaMom, 70);
        }
        else
        {
            // We don't support this.

            kdError(s_area) << "unsupported version of Word (nFib" << out->nFib << ")";
            break;
        }
        out++;
    }
    return bytes;
} // FIB

unsigned MsWord::read(unsigned nFib, const U8 *in, BTE *out)
{
    U8 *ptr = (U8 *)out;
    unsigned bytes = 0;
    U16 tmp;

    if (nFib > s_maxWord6Version)
    {
        bytes = MsWordGenerated::read(in, out);
    }
    else
    {
        bytes += MsWordGenerated::read(in + bytes, &tmp);
        out->pn = tmp;
    }
    return bytes;
} // BTE

unsigned MsWord::read(unsigned nFib, const U8 *in, PCD *out)
{
    U8 *ptr = (U8 *)out;
    unsigned bytes = 0;

    bytes = MsWordGenerated::read(in, out);
    return bytes;
} // PCD

unsigned MsWord::read(unsigned nFib, const U8 *in, PHE *out)
{
    U8 *ptr = (U8 *)out;
    unsigned bytes = 0;
    U16 tmp;

    if (nFib > s_maxWord6Version)
    {
        bytes = MsWordGenerated::read(in, out);
    }
    else
    {
        bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes));
        bytes += MsWordGenerated::read(in + bytes, &tmp);
        out->dxaCol = tmp;
        bytes += MsWordGenerated::read(in + bytes, &tmp);
        out->dym = tmp;
    }
    return bytes;
} // PHE

