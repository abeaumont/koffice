/* This file is part of the KOffice project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "document.h"
#include "conversion.h"
#include "texthandler.h"

#include <koGlobal.h>
#include <kdebug.h>

#include <styles.h>
#include <ustring.h>
#include <word97_generated.h>
#include <parser.h>
#include <parserfactory.h>
#include <paragraphproperties.h>


Document::Document( const std::string& fileName, QDomDocument& mainDocument, QDomElement& framesetsElement )
    : m_mainDocument( mainDocument ), m_framesetsElement( framesetsElement ),
      m_replacementHandler( new KWordReplacementHandler ),
      m_parser( wvWare::ParserFactory::createParser( fileName ) ),
      m_headerFooters( 0 ), m_bodyFound( false )
{
    if ( m_parser ) // 0 in case of major error (e.g. unsupported format)
    {
        m_textHandler = new KWordTextHandler( m_parser );
        connect( m_textHandler, SIGNAL( subDocFound( const wvWare::HeaderFunctor& ) ),
                 this, SLOT( pushSubDocument( const wvWare::HeaderFunctor& ) ) );
        m_parser->setSubDocumentHandler( this );
        m_parser->setTextHandler( m_textHandler );
        m_parser->setInlineReplacementHandler( m_replacementHandler );
        processStyles();
    }
}

Document::~Document()
{
    delete m_textHandler;
    delete m_replacementHandler;
}

void Document::finishDocument()
{
    const wvWare::Word97::DOP& dop = m_parser->dop();

    QDomElement elementDoc = m_mainDocument.documentElement();

    QDomElement element;
    element = m_mainDocument.createElement("ATTRIBUTES");
    element.setAttribute("processing",0); // WP
    char allHeaders = ( wvWare::HeaderData::HeaderEven |
                        wvWare::HeaderData::HeaderOdd |
                        wvWare::HeaderData::HeaderFirst );
    element.setAttribute("hasHeader", m_headerFooters & allHeaders ? 1 : 0 );
    char allFooters = ( wvWare::HeaderData::FooterEven |
                        wvWare::HeaderData::FooterOdd |
                        wvWare::HeaderData::FooterFirst );
    element.setAttribute("hasFooter", m_headerFooters & allFooters ? 1 : 0 );
    element.setAttribute("unit","mm"); // How to figure out the unit to use?

    element.setAttribute("tabStopValue", (double)dop.dxaTab / 20.0 );
    elementDoc.appendChild(element);

    // FOOTNOTESETTING: use nfcFtnRef for the type of counter
    // Hmm there's nfcFtnRef2 too.

    // Done at the end: write the type of headers/footers,
    // depending on which kind of headers and footers we received.
    QDomElement paperElement = elementDoc.namedItem("PAPER").toElement();
    Q_ASSERT ( !paperElement.isNull() ); // slotFirstSectionFound should have been called!
    if ( !paperElement.isNull() )
    {
        kdDebug() << k_funcinfo << "m_headerFooters=" << m_headerFooters << endl;
        paperElement.setAttribute("hType", Conversion::headerMaskToHType( m_headerFooters ) );
        paperElement.setAttribute("fType", Conversion::headerMaskToFType( m_headerFooters ) );
    }
}

void Document::processStyles()
{
    QDomElement stylesElem = m_mainDocument.createElement( "STYLES" );
    m_mainDocument.documentElement().appendChild( stylesElem );

    m_textHandler->setFrameSetElement( stylesElem ); /// ### naming!
    const wvWare::StyleSheet& styles = m_parser->styleSheet();
    unsigned int count = styles.size();
    //kdDebug() << k_funcinfo << "styles count=" << count << endl;
    for ( unsigned int i = 0; i < count ; ++i )
    {
        const wvWare::Style* style = styles.styleByIndex( i );
        Q_ASSERT( style );
        //kdDebug() << k_funcinfo << "style " << i << " " << style << endl;
        if ( style && style->type() == wvWare::Style::sgcPara )
        {
            QDomElement styleElem = m_mainDocument.createElement("STYLE");
            stylesElem.appendChild( styleElem );

            QConstString name = Conversion::string( style->name() );
            QDomElement element = m_mainDocument.createElement("NAME");
            element.setAttribute( "value", name.string() );
            styleElem.appendChild( element );

            //kdDebug() << k_funcinfo << "Style " << i << ": " << name.string() << endl;

            const wvWare::Style* followingStyle = styles.styleByID( style->followingStyle() );
            if ( followingStyle && followingStyle != style )
            {
                QConstString followingName = Conversion::string( followingStyle->name() );
                element = m_mainDocument.createElement("FOLLOWING");
                element.setAttribute( "name", followingName.string() );
                styleElem.appendChild( element );
            }

            m_textHandler->paragLayoutBegin(); // new style, reset some vars

            // It's important to do that one first, for m_shadowTextFound
            m_textHandler->writeFormat( styleElem, &style->chp(), 0L /*all of it, no ref chp*/, 0, 0 );

            m_textHandler->writeLayout( styleElem, style->paragraphProperties(), style );
        }
        // KWord doesn't support character styles yet
    }
}

bool Document::parse()
{
    if ( m_parser )
        return m_parser->parse();
    return false;
}

void Document::startBody()
{
    kdDebug() << k_funcinfo << endl;

    QDomElement mainFramesetElement = m_mainDocument.createElement("FRAMESET");
    mainFramesetElement.setAttribute("frameType",1);
    mainFramesetElement.setAttribute("frameInfo",0);
    // TODO: "name" attribute (needs I18N)
    m_framesetsElement.appendChild(mainFramesetElement);

    createInitialFrame( mainFramesetElement, 42, 566 );

    m_textHandler->setFrameSetElement( mainFramesetElement );
    connect( m_textHandler, SIGNAL( firstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> ) ),
             this, SLOT( slotFirstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> ) ) );
    m_bodyFound = true;
}

void Document::endBody()
{
    kdDebug() << k_funcinfo << endl;
    disconnect( m_textHandler, SIGNAL( firstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> ) ),
             this, SLOT( slotFirstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> ) ) );
}


void Document::slotFirstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> sep )
{
    kdDebug() << k_funcinfo << endl;
    QDomElement elementDoc = m_mainDocument.documentElement();

    QDomElement elementPaper = m_mainDocument.createElement("PAPER");
    bool landscape = (sep->dmOrientPage == 2);
    double width = (double)sep->xaPage / 20.0;
    double height = (double)sep->yaPage / 20.0;
    elementPaper.setAttribute("width", width);
    elementPaper.setAttribute("height", height);

    // guessFormat takes millimeters
    width = POINT_TO_MM( width );
    height = POINT_TO_MM( height );
    KoFormat paperFormat = KoPageFormat::guessFormat( landscape ? height : width, landscape ? width : height );
    elementPaper.setAttribute("format",paperFormat);

    elementPaper.setAttribute("orientation", landscape ? PG_LANDSCAPE : PG_PORTRAIT );
    elementPaper.setAttribute("columns", sep->ccolM1 + 1 );
    elementPaper.setAttribute("columnspacing", (double)sep->dxaColumns / 20.0);
    elementPaper.setAttribute("spHeadBody", (double)sep->dyaHdrTop / 20.0);
    elementPaper.setAttribute("spFootBody", (double)sep->dyaHdrBottom / 20.0);
    // elementPaper.setAttribute("zoom",100); // not a doc property in kword
    elementDoc.appendChild(elementPaper);

    QDomElement element = m_mainDocument.createElement("PAPERBORDERS");
    element.setAttribute("left", (double)sep->dxaLeft / 20.0);
    element.setAttribute("top",(double)sep->dyaTop / 20.0);
    element.setAttribute("right", (double)sep->dxaRight / 20.0);
    element.setAttribute("bottom", (double)sep->dyaBottom / 20.0);
    elementPaper.appendChild(element);

    // TODO apply brcTop/brcLeft etc. to the main FRAME
    // TODO use sep->fEndNote to set the 'use endnotes or footnotes' flag
}

void Document::startHeader( unsigned char type )
{
    kdDebug() << k_funcinfo << type << endl;
    // Werner says the headers are always emitted in the order of the Type enum.
    // So when HeaderOdd arrives, there are two cases.
    // * If HeaderEven was sent before, we know we're in the even/odd case.
    // * If not, the HeaderOdd header will be used for odd and even pages
    // ..... in which case it needs to be saved as the "Even" header for KWord !!!
    if ( type == wvWare::HeaderData::HeaderOdd &&
         ( ( m_headerFooters & wvWare::HeaderData::HeaderEven ) == 0 ) )
        type = wvWare::HeaderData::HeaderEven;
    else if ( type == wvWare::HeaderData::FooterOdd &&
              ( ( m_headerFooters & wvWare::HeaderData::FooterEven ) == 0 ) )
        type = wvWare::HeaderData::FooterEven;

    QDomElement framesetElement = m_mainDocument.createElement("FRAMESET");
    framesetElement.setAttribute("frameType",1);
    framesetElement.setAttribute("frameInfo",Conversion::headerTypeToFrameInfo(type));
    // TODO: "name" attribute (needs I18N)
    m_framesetsElement.appendChild(framesetElement);

    bool isHeader = Conversion::isHeader( type );

    createInitialFrame( framesetElement, isHeader?0:567, isHeader?41:567+41 );

    m_textHandler->setFrameSetElement( framesetElement );

    m_headerFooters |= type;

    /*if ( Conversion::isHeader( type ) )
        m_hasHeader = true;
    else
        m_hasFooter = true;*/
}

void Document::endHeader()
{
    kdDebug() << k_funcinfo << endl;

}

void Document::createInitialFrame( QDomElement& parentFramesetElem, int top, int bottom )
{
    QDomElement frameElementOut = parentFramesetElem.ownerDocument().createElement("FRAME");
    // Those values are unused. The paper margins make recalcFrames() resize this frame.
    frameElementOut.setAttribute( "left", 28 );
    frameElementOut.setAttribute( "right", 798 );
    frameElementOut.setAttribute( "top", top );
    frameElementOut.setAttribute( "bottom", bottom );
    frameElementOut.setAttribute( "runaround", 1 );
    frameElementOut.setAttribute( "autoCreateNewFrame", 1 );
    parentFramesetElem.appendChild( frameElementOut );
}

void Document::pushSubDocument( const wvWare::HeaderFunctor& functor /*const SubDocument& subdoc*/ )
{
    m_subdocQueue.push( functor );
}

bool Document::hasSubDocument() const
{
    return !m_subdocQueue.empty();
}

Document::SubDocument Document::popSubDocument()
{
    SubDocument subdoc = m_subdocQueue.front();
    m_subdocQueue.pop();
    return subdoc;
}

void Document::processSubDocQueue()
{
    while ( hasSubDocument() )
    {
        SubDocument subdoc = popSubDocument();
        subdoc(); // call it
    }
}

#include "document.moc"
