/* This file is part of the KOffice project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 David Faure <faure@kde.org>
   Copyright (C) 2008 Benjamin Cail <cricketc@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "document.h"
#include "conversion.h"
#include "texthandler.h"
#include "graphicshandler.h"
#include "versionmagic.h"

#include <KoUnit.h>
#include <KoPageLayout.h>
#include <kdebug.h>

#include <wv2/styles.h>
#include <wv2/ustring.h>
#include <wv2/word97_generated.h>
#include <wv2/parser.h>
#include <wv2/parserfactory.h>
#include <wv2/paragraphproperties.h>
#include <wv2/associatedstrings.h>
#include <klocale.h>
#include <KoStore.h>
#include <KoFilterChain.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QBuffer>

Document::Document( const std::string& fileName, KoFilterChain* chain, KoXmlWriter* bodyWriter, KoGenStyles* mainStyles, KoXmlWriter* metaWriter )
    : m_replacementHandler( new KWordReplacementHandler ), m_tableHandler( new KWordTableHandler ),
      m_pictureHandler( new KWordPictureHandler( this ) ), m_textHandler( 0 ), m_headerCount(0),
      m_chain( chain ), m_currentListDepth( -1 ), m_evenOpen( false ), m_oddOpen( false ),
      m_parser( wvWare::ParserFactory::createParser( fileName ) )/*, m_headerFooters( 0 ), m_bodyFound( false ),
      m_footNoteNumber( 0 ), m_endNoteNumber( 0 )*/
{
    kDebug(30513) ;
    if ( m_parser ) // 0 in case of major error (e.g. unsupported format)
    {
	m_bodyWriter = bodyWriter; //pointer for writing to the body
	m_mainStyles = mainStyles; //KoGenStyles object for collecting styles
	m_metaWriter = metaWriter; //pointer for writing to meta.xml
	m_buffer = 0; //set pointers to 0
	m_bufferEven = 0;
	m_writer = 0;
	m_masterStyle = new KoGenStyle(KoGenStyle::StyleMaster); //for header/footer stuff
        m_textHandler = new KWordTextHandler( m_parser, bodyWriter, mainStyles );
        connect( m_textHandler, SIGNAL( subDocFound( const wvWare::FunctorBase*, int ) ),
                 this, SLOT( slotSubDocFound( const wvWare::FunctorBase*, int ) ) );
        connect( m_textHandler, SIGNAL( footnoteFound( const wvWare::FunctorBase*, int ) ),
                 this, SLOT( slotFootnoteFound( const wvWare::FunctorBase*, int ) ) );
        connect( m_textHandler, SIGNAL( tableFound( const KWord::Table& ) ),
                 this, SLOT( slotTableFound( const KWord::Table& ) ) );
        connect( m_textHandler, SIGNAL( pictureFound( const QString&, const QString&, const wvWare::FunctorBase* ) ),
                 this, SLOT( slotPictureFound( const QString&, const QString&, const wvWare::FunctorBase* ) ) );
        m_parser->setSubDocumentHandler( this );
        m_parser->setTextHandler( m_textHandler );
        m_parser->setTableHandler( m_tableHandler );
#ifdef IMAGE_IMPORT
        m_parser->setPictureHandler( m_pictureHandler );
#endif
        m_parser->setInlineReplacementHandler( m_replacementHandler );
        processStyles();
        processAssociatedStrings();
        connect( m_tableHandler, SIGNAL( sigTableCellStart( int, int, int, int, const QRectF&, const QString&, const wvWare::Word97::BRC&, const wvWare::Word97::BRC&, const wvWare::Word97::BRC&, const wvWare::Word97::BRC&, const wvWare::Word97::SHD& ) ),
                 this, SLOT( slotTableCellStart( int, int, int, int, const QRectF&, const QString&, const wvWare::Word97::BRC&, const wvWare::Word97::BRC&, const wvWare::Word97::BRC&, const wvWare::Word97::BRC&, const wvWare::Word97::SHD& ) ) );
        connect( m_tableHandler, SIGNAL( sigTableCellEnd() ),
                 this, SLOT( slotTableCellEnd() ) );
    }
}

Document::~Document()
{
    delete m_textHandler;
    delete m_pictureHandler;
    delete m_tableHandler;
    delete m_replacementHandler;
}

//set whether or not document has header or footer
//set tabstop value
//add footnote settings & endnote settings
//write out header & footer type
//write out picture information
void Document::finishDocument()
{
    kDebug(30513);

    //finish a header if we need to - this should only be necessary if there's an even header w/o an odd header
    if(m_oddOpen) {
        QString contents = QString::fromUtf8( m_buffer->buffer(), m_buffer->buffer().size() );
	m_masterStyle->addChildElement( QString::number( m_headerCount ), contents );
	m_oddOpen = false;
	m_textHandler->m_headerWriter = 0;
	delete m_writer;
	m_writer = 0;
	delete m_buffer;
	m_buffer = 0;
	//we're done with this header, so reset to false
	m_textHandler->m_writingHeader = false;
    }

    const wvWare::Word97::DOP& dop = m_parser->dop();
    //"tabStopValue", (double)dop.dxaTab / 20.0
    //dop.nFtn = initial footnote number for document, starts at 1
    //Conversion::numberFormatCode(dop.nfcFtnRef2)
    //dop.nEdn = initial endnote number for document, starts at 1
    //Conversion::numberFormatCode(dop.nfcEdnRef2)
/*
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
    //element.setAttribute("unit","mm"); // How to figure out the unit to use?

    element.setAttribute("tabStopValue", (double)dop.dxaTab / 20.0 );
    elementDoc.appendChild(element);

    element = m_mainDocument.createElement("FOOTNOTESETTING");
    elementDoc.appendChild(element);
    element.setAttribute( "start", dop.nFtn ); // initial footnote number for document. Starts at 1.
    element.setAttribute( "type", Conversion::numberFormatCode( dop.nfcFtnRef2 ) );

    element = m_mainDocument.createElement("ENDNOTESETTING");
    elementDoc.appendChild(element);
    element.setAttribute( "start", dop.nEdn ); // initial endnote number for document. Starts at 1.
    element.setAttribute( "type", Conversion::numberFormatCode( dop.nfcEdnRef2 ) );

    // Done at the end: write the type of headers/footers,
    // depending on which kind of headers and footers we received.
    QDomElement paperElement = elementDoc.namedItem("PAPER").toElement();
    Q_ASSERT ( !paperElement.isNull() ); // slotFirstSectionFound should have been called!
    if ( !paperElement.isNull() )
    {
        kDebug(30513) <<"m_headerFooters=" << m_headerFooters;
        paperElement.setAttribute("hType", Conversion::headerMaskToHType( m_headerFooters ) );
        paperElement.setAttribute("fType", Conversion::headerMaskToFType( m_headerFooters ) );
    }

    // Write out <PICTURES> tag
    QDomElement picturesElem = m_mainDocument.createElement("PICTURES");
    elementDoc.appendChild( picturesElem );
    for( QStringList::Iterator it = m_pictureList.begin(); it != m_pictureList.end(); ++it ) {
        QDomElement keyElem = m_mainDocument.createElement("KEY");
        picturesElem.appendChild( keyElem );
        keyElem.setAttribute( "filename", *it );
        keyElem.setAttribute( "name", *it );
    }*/
}

//write document info, author, fullname, title, about
void Document::processAssociatedStrings() 
{
    kDebug(30513) ;
    wvWare::AssociatedStrings strings( m_parser->associatedStrings() );
    if(!strings.author().isNull()) {
	m_metaWriter->startElement("meta:initial-creator");
	m_metaWriter->addTextSpan(Conversion::string(strings.author()).string());
	m_metaWriter->endElement();
    }
    if(!strings.title().isNull()) {
	m_metaWriter->startElement("dc:title");
	m_metaWriter->addTextSpan(Conversion::string(strings.title()).string());
	m_metaWriter->endElement();
    }
    //any other information here??
}

void Document::processStyles()
{
    kDebug(30513) ;

    //need to add master style to m_mainStyle
    kDebug(30513) << "adding master style to main styles collection";
    m_masterStyleName = m_mainStyles->lookup(*m_masterStyle, "Standard");
    delete m_masterStyle; //delete the object since we've added it to the collection
    //set master style name in m_textHandler because that's where we'll write it
    m_textHandler->m_masterStyleName = m_masterStyleName;
    //get a pointer to the object in the collection
    m_masterStyle = m_mainStyles->styleForModification(m_masterStyleName);
    m_textHandler->m_writeMasterStyleName = true;

    //m_textHandler->setFrameSetElement( stylesElem ); /// ### naming!
    const wvWare::StyleSheet& styles = m_parser->styleSheet();
    unsigned int count = styles.size();
    kDebug(30513) <<"styles count=" << count;

    //create page layout style here
    KoGenStyle style(KoGenStyle::StylePageLayout);

    //just dummy hard-coded values
    style.addProperty("fo:page-width", "8.5in");
    style.addProperty("fo:page-height", "11in");
    style.addProperty("style:footnote-max-height", "0in");
    style.addProperty("style:writing-mode", "lr-tb");
    style.addProperty("style:print-orientation", "portrait");
    style.addProperty("style:num-format", "1");
    style.addProperty("fo:margin-top", "1in");
    style.addProperty("fo:margin-bottom", "1in");
    style.addProperty("fo:margin-left", "1in");
    style.addProperty("fo:margin-right", "1in");
    QString header("<style:header-style>");
    header.append("<style:header-footer-properties fo:min-height=\"0.5in\" fo:margin-bottom=\"0.461in\" style:dynamic-spacing=\"true\" />");
    header.append("</style:header-style>");
    QString footer("<style:footer-style>");
    footer.append("<style:header-footer-properties fo:min-height=\"0.5in\" fo:margin-top=\"0.461in\" style:dynamic-spacing=\"true\" />");
    footer.append("</style:footer-style>");
    style.addProperty("1header-style", header, KoGenStyle::StyleChildElement);
    style.addProperty("2footer-style", footer, KoGenStyle::StyleChildElement);
    style.setAutoStyleInStylesDotXml(true);

    //insert the style into the collection, and get the name it's assigned
    QString pageLayoutName = m_mainStyles->lookup(style, QString("pm"));
    //set the page-layout-name in the master style
    m_masterStyle->addAttribute("style:page-layout-name", pageLayoutName);

    //loop through each style
    for ( unsigned int i = 0; i < count ; ++i )
    {
	//grab style
        const wvWare::Style* style = styles.styleByIndex( i );
        Q_ASSERT( style );
        QConstString name = Conversion::string( style->name() );
        kDebug(30513) << "Style" << i << ":" << name.string();
	kDebug(30513) << "style->type() = " << style->type();
	kDebug(30513) << "style->sti() = " << style->sti();

	//construct a KoGenStyle object
	//use chp() & paragraphProperties()
	
	//process paragraph styles
        if ( style && style->type() == wvWare::Style::sgcPara )
        {
            //QDomElement styleElem = m_mainDocument.createElement("STYLE");
            //stylesElem.appendChild( styleElem );

            //QDomElement element = m_mainDocument.createElement("NAME");
            //element.setAttribute( "value", name.string() );
            //styleElem.appendChild( element );

            const wvWare::Style* followingStyle = styles.styleByID( style->followingStyle() );
            if ( followingStyle && followingStyle != style )
            {
                QConstString followingName = Conversion::string( followingStyle->name() );
                //element = m_mainDocument.createElement("FOLLOWING");
                //element.setAttribute( "name", followingName.string() );
                //styleElem.appendChild( element );
            }

	    //create this style & add formatting info to it
	    kDebug(30513) << "creating ODT style" << name.string();
	    KoGenStyle userStyle(KoGenStyle::StyleUser, "paragraph"); 
	    userStyle.addAttribute("style:display-name", name);
	    m_textHandler->writeFormattedText(&userStyle, &style->chp(), 0L, QString(""), false, QString(""));
            m_textHandler->writeLayout(style->paragraphProperties(), &userStyle, style, false, QString(""));
	    //add style to main collection, using the name that it had in the .doc
	    QString actualName = m_mainStyles->lookup(userStyle, name, KoGenStyles::DontForceNumbering);
	    kDebug(30513) << "added style " << actualName << "\n";
        }
	else if(style && style->type()==wvWare::Style::sgcChp) {
	    // KWord doesn't support character styles yet

	}
    }
}

//just call parsing function
bool Document::parse()
{
    kDebug(30513) ;
    if ( m_parser )
        return m_parser->parse();
    return false;
}

//connects firstSectionFound signal & slot together; sets flag to true
void Document::bodyStart()
{

    //QDomElement mainFramesetElement = m_mainDocument.createElement("FRAMESET");
    //mainFramesetElement.setAttribute("frameType",1);
    //mainFramesetElement.setAttribute("frameInfo",0);
    // TODO: "name" attribute (needs I18N)
    //m_framesetsElement.appendChild(mainFramesetElement);

    // Those values are unused. The paper margins make recalcFrames() resize this frame.
    //createInitialFrame( mainFramesetElement, 29, 798, 42, 566, false, Reconnect );

    //m_textHandler->setFrameSetElement( mainFramesetElement );
    kDebug(30513) << "connecting firstSectionFound signal & updateListDepth signal";
    connect( m_textHandler, SIGNAL( firstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> ) ),
             this, SLOT( slotFirstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> ) ) );
    connect( m_textHandler, SIGNAL( updateListDepth( int ) ),
	     this, SLOT( slotUpdateListDepth( int ) ) );
    m_bodyFound = true;
}

//disconnects firstSectionFound signal & slot
void Document::bodyEnd()
{
    kDebug(30513) << "m_currentListDepth = " << m_currentListDepth;

    //close a list if we need to
    if ( m_currentListDepth >= 0 )
    {
	kDebug(30513) << "closing the final list in the document body";
	//m_listStylesWriter->endElement(); //text:list-style
	//reset listStyleName
	m_textHandler->m_listStyleName = "";
	m_textHandler->m_currentListDepth = -1;
	m_textHandler->m_currentListID = 0;
	//close any open list tags in the body writer
        for (int i = 0; i <= m_currentListDepth; i++)
        {
	    m_bodyWriter->endElement(); //close the text:list-item
	    m_bodyWriter->endElement(); //text:list
	}
	m_currentListDepth = -1;
    }

    disconnect( m_textHandler, SIGNAL( firstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> ) ),
             this, SLOT( slotFirstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> ) ) );
}

//sets paper size
//sets format & orientation
//sets column information
//sets up borders
void Document::slotFirstSectionFound( wvWare::SharedPtr<const wvWare::Word97::SEP> sep )
{
    kDebug(30513) ;
    //QDomElement elementDoc = m_mainDocument.documentElement();

    //set the paper size and orientation!
    bool landscape = (sep->dmOrientPage == 2);
    double width = (double)sep->xaPage / 20.0;
    double height = (double)sep->yaPage / 20.0;

    // guessFormat takes millimeters
    width = POINT_TO_MM( width );
    height = POINT_TO_MM( height );
    KoPageFormat::Format  paperFormat = KoPageFormat::guessFormat( landscape ? height : width, landscape ? width : height );
    //elementPaper.setAttribute("format",paperFormat);

    //set up columns
    //sep->dyaHdrTop/Bottom

    //elementPaper.setAttribute("orientation", landscape ? KoPageFormat::Landscape : KoPageFormat::Portrait );
    //elementPaper.setAttribute("columns", sep->ccolM1 + 1 );
    //elementPaper.setAttribute("columnspacing", (double)sep->dxaColumns / 20.0);
    //elementPaper.setAttribute("spHeadBody", (double)sep->dyaHdrTop / 20.0);
    //elementPaper.setAttribute("spFootBody", (double)sep->dyaHdrBottom / 20.0);
    // elementPaper.setAttribute("zoom",100); // not a doc property in kword
    //elementDoc.appendChild(elementPaper);

    //page margins

    //QDomElement element = m_mainDocument.createElement("PAPERBORDERS");
    //element.setAttribute("left", (double)sep->dxaLeft / 20.0);
    //element.setAttribute("top",(double)sep->dyaTop / 20.0);
    //element.setAttribute("right", (double)sep->dxaRight / 20.0);
    //element.setAttribute("bottom", (double)sep->dyaBottom / 20.0);
    //elementPaper.appendChild(element);

    // TODO apply brcTop/brcLeft etc. to the main FRAME
    // TODO use sep->fEndNote to set the 'use endnotes or footnotes' flag
}

//creates a frameset element with the header info
void Document::headerStart( wvWare::HeaderData::Type type )
{
    kDebug(30513) << "startHeader type=" << type << " (" << Conversion::headerTypeToFramesetName( type ) << ")";
    // Werner says the headers are always emitted in the order of the Type enum.
    //	Header Even, Header Odd, Footer Even, Footer Odd, Header First, Footer First

    m_headerCount++;

    switch(type) {
    //TODO fix first header
    case wvWare::HeaderData::HeaderFirst:
	m_buffer = new QBuffer();
	m_buffer->open(QIODevice::WriteOnly);
	m_writer = new KoXmlWriter(m_buffer);
	break;
    case wvWare::HeaderData::HeaderOdd:
	//set up buffer & writer for odd header
	m_buffer = new QBuffer();
	m_buffer->open(QIODevice::WriteOnly);
	m_writer = new KoXmlWriter(m_buffer);
	m_oddOpen = true;
	m_writer->startElement("style:header");
	break;
    case wvWare::HeaderData::HeaderEven:
	//write to the buffer for even headers/footers
	m_bufferEven = new QBuffer();
	m_bufferEven->open(QIODevice::WriteOnly);
	m_writer = new KoXmlWriter(m_bufferEven);
	m_evenOpen = true;
	m_writer->startElement("style:header-left");
	break;
    //TODO fix first footer
    case wvWare::HeaderData::FooterFirst:
	m_buffer = new QBuffer();
	m_buffer->open(QIODevice::WriteOnly);
	m_writer = new KoXmlWriter(m_buffer);
	break;
    case wvWare::HeaderData::FooterOdd:
	//set up buffer & writer for odd header
	m_buffer = new QBuffer();
	m_buffer->open(QIODevice::WriteOnly);
	m_writer = new KoXmlWriter(m_buffer);
	m_oddOpen = true;
	m_writer->startElement("style:footer");
	break;
    case wvWare::HeaderData::FooterEven:
	//write to the buffer for even headers/footers
	m_bufferEven = new QBuffer();
	m_bufferEven->open(QIODevice::WriteOnly);
	m_writer = new KoXmlWriter(m_bufferEven);
	m_evenOpen = true;
	m_writer->startElement("style:footer-left");
	break;
    }

    //tell texthandler we're writing a header
    m_textHandler->m_writingHeader = true;
    //and set up the tmp writer so writeFormattedText() writes to styles.xml
    m_textHandler->m_headerWriter = m_writer;
}

//creates empty frameset element?
void Document::headerEnd()
{
    kDebug(30513) ;
    //close a list if we need to (you can have a list inside a header)
    if ( m_currentListDepth >= 0 )
    {
	kDebug(30513) << "closing a list in a header/footer";
	//reset listStyleName, m_currentListDepth, & m_currentListID in m_textHandler
	m_textHandler->m_currentListDepth = -1;
	m_textHandler->m_listStyleName = "";
	m_textHandler->m_currentListID = 0;
	//close any open list tags in the body writer
        for (int i = 0; i <= m_currentListDepth; i++)
        {
	    m_writer->endElement(); //close the text:list-item
	    m_writer->endElement(); //text:list
	}
	m_currentListDepth = -1;
    }

    //close writer & add to m_masterStyle
    //if it was a first header/footer, we wrote to this writer, but we won't do anything with it
    //handle the even flag first, because they'll both be open if the even one is, and
    //	we would want to handle the odd flag when we actually see the odd header/footer
    if(m_evenOpen) {
	m_writer->endElement(); //style:header/footer-left
	m_evenOpen = false;
	delete m_writer;
	m_writer = 0;
	return;
    }
    if(m_oddOpen) {
	m_writer->endElement();//style:header/footer
	//add the even header/footer stuff here
	if(m_bufferEven) {
	    m_writer->addCompleteElement(m_bufferEven);
	    delete m_bufferEven;
	    m_bufferEven = 0;
	}
        QString contents = QString::fromUtf8( m_buffer->buffer(), m_buffer->buffer().size() );
	m_masterStyle->addChildElement( QString::number( m_headerCount ), contents );
	m_oddOpen = false;
    }
    m_textHandler->m_headerWriter = 0;
    delete m_writer;
    m_writer = 0;
    delete m_buffer;
    m_buffer = 0;
    //we're done with this header, so reset to false
    m_textHandler->m_writingHeader = false;
}

//get text of the footnote
//set information about the footnote/endnote
//increment the number of footnotes/endnotes
void Document::footnoteStart()
{
    kDebug(30513) ;

    //this stuff should all be handled in texthandler.cpp, I think

    // Grab data that was stored with the functor, that triggered this parsing
    //SubDocument subdoc( m_subdocQueue.front() );
    //int type = subdoc.data;

    // Create footnote/endnote frameset
    //QDomElement framesetElement = m_mainDocument.createElement("FRAMESET");
    //framesetElement.setAttribute( "frameType", 1 /* text */ );
    //framesetElement.setAttribute( "frameInfo", 7 /* footnote/endnote */ );
    //if ( type == wvWare::FootnoteData::Endnote )
        // Keep name in sync with KWordTextHandler::footnoteFound
        //framesetElement.setAttribute("name", i18n("Endnote %1", ++m_endNoteNumber ) );
    //else
        // Keep name in sync with KWordTextHandler::footnoteFound
        //framesetElement.setAttribute("name", i18n("Footnote %1", ++m_footNoteNumber ) );
    //m_framesetsElement.appendChild(framesetElement);

    //createInitialFrame( framesetElement, 29, 798, 567, 567+41, true, NoFollowup );

    //m_textHandler->setFrameSetElement( framesetElement );
}

//add empty frameset element to end it?
void Document::footnoteEnd()
{
    kDebug(30513);
    //m_textHandler->setFrameSetElement( QDomElement() );
}

void Document::slotUpdateListDepth( int depth )
{
    kDebug(30513) << "setting m_currentListDepth = " << depth;
    m_currentListDepth = depth;
}

//create frame for the table cell?
void Document::slotTableCellStart( int row, int column, int rowSpan, int columnSpan, const QRectF& cellRect, const QString& tableName, const wvWare::Word97::BRC& brcTop, const wvWare::Word97::BRC& brcBottom, const wvWare::Word97::BRC& brcLeft, const wvWare::Word97::BRC& brcRight, const wvWare::Word97::SHD& shd )
{
    kDebug(30513) ;
    // Create footnote/endnote frameset
    //QDomElement framesetElement = m_mainDocument.createElement("FRAMESET");
    //framesetElement.setAttribute( "frameType", 1 /* text */ );
    //framesetElement.setAttribute( "frameInfo", 0 /* normal text */ );
    //framesetElement.setAttribute( "grpMgr", tableName );
    QString name = i18nc("Table_Name Cell row,column", "%1 Cell %2,%3",tableName,row,column);
    //framesetElement.setAttribute( "name", name );
    //framesetElement.setAttribute( "row", row );
    //framesetElement.setAttribute( "col", column );
    //framesetElement.setAttribute( "rows", rowSpan );
    //framesetElement.setAttribute( "cols", columnSpan );
    //m_framesetsElement.appendChild(framesetElement);

    //QDomElement frameElem = createInitialFrame( framesetElement, cellRect.left(), cellRect.right(), cellRect.top(), cellRect.bottom(), true, NoFollowup );
    //generateFrameBorder( frameElem, brcTop, brcBottom, brcLeft, brcRight, shd );

    //m_textHandler->setFrameSetElement( framesetElement );
}

//add empty element to end it?
void Document::slotTableCellEnd()
{
    kDebug(30513) ;
    //m_textHandler->setFrameSetElement( QDomElement() );
}

/*QDomElement Document::createInitialFrame( QDomElement& parentFramesetElem, double left, double right, double top, double bottom, bool autoExtend, NewFrameBehavior nfb )
{
    QDomElement frameElementOut = parentFramesetElem.ownerDocument().createElement("FRAME");
    frameElementOut.setAttribute( "left", left );
    frameElementOut.setAttribute( "right", right );
    frameElementOut.setAttribute( "top", top );
    frameElementOut.setAttribute( "bottom", bottom );
    frameElementOut.setAttribute( "runaround", 1 );
    // AutoExtendFrame for header/footer/footnote/endnote, AutoCreateNewFrame for body text
    frameElementOut.setAttribute( "autoCreateNewFrame", autoExtend ? 0 : 1 );
    frameElementOut.setAttribute( "newFrameBehavior", nfb );
    parentFramesetElem.appendChild( frameElementOut );
    return frameElementOut;
}*/

//set up frame borders (like for a table cell?)
//set the background fill
void Document::generateFrameBorder( QDomElement& frameElementOut, const wvWare::Word97::BRC& brcTop, const wvWare::Word97::BRC& brcBottom, const wvWare::Word97::BRC& brcLeft, const wvWare::Word97::BRC& brcRight, const wvWare::Word97::SHD& shd )
{
    kDebug(30513) ;
    // Frame borders

    //figure out what this is supposed to do!

    /*if ( brcTop.ico != 255 && brcTop.dptLineWidth != 255 ) // see tablehandler.cpp
        Conversion::setBorderAttributes( frameElementOut, brcTop, "t" );
    if ( brcBottom.ico != 255 && brcBottom.dptLineWidth != 255 ) // see tablehandler.cpp
        Conversion::setBorderAttributes( frameElementOut, brcBottom, "b" );
    if ( brcLeft.ico != 255 && brcLeft.dptLineWidth != 255 ) // could still be 255, for first column
        Conversion::setBorderAttributes( frameElementOut, brcLeft, "l" );
    if ( brcRight.ico != 255 && brcRight.dptLineWidth != 255 ) // could still be 255, for last column
        Conversion::setBorderAttributes( frameElementOut, brcRight, "r" );*/

    // Frame background brush (color and fill style)
    if ( shd.icoFore != 0 || shd.icoBack != 0 )
    {
        // If ipat = 0 (solid fill), icoBack is the background color.
        // But otherwise, icoFore is the one we need to set as bkColor
        // (and icoBack is usually white; it's the other color of the pattern,
        // something that we can't set in Qt apparently).
        int bkColor = shd.ipat ? shd.icoFore : shd.icoBack;
        kDebug(30513) <<"generateFrameBorder:" <<" icoFore=" << shd.icoFore <<" icoBack=" << shd.icoBack <<" ipat=" << shd.ipat <<" -> bkColor=" << bkColor;

        // Reverse-engineer MSWord's own hackery: it models various gray levels
        // using dithering. But this looks crappy with Qt. So we go back to a QColor.
        bool grayHack = ( shd.ipat && shd.icoFore == 1 && shd.icoBack == 8 );
        if ( grayHack )
        {
            bool ok;
            int grayLevel = Conversion::ditheringToGray( shd.ipat, &ok );
            if ( ok )
            {
                QColor color( 0, 0, grayLevel, QColor::Hsv );
                QString prefix = "bk";
                frameElementOut.setAttribute( "bkRed", color.red() );
                frameElementOut.setAttribute( "bkBlue", color.blue() );
                frameElementOut.setAttribute( "bkGreen", color.green() );
            }
            else grayHack = false;
        }
        if ( !grayHack )
        {
            //Conversion::setColorAttributes( frameElementOut, bkColor, "bk", true );
            // Fill style
            int brushStyle = Conversion::fillPatternStyle( shd.ipat );
            //frameElementOut.setAttribute( "bkStyle", brushStyle );
        }
    }
}

//create SubDocument object & add it to the queue
void Document::slotSubDocFound( const wvWare::FunctorBase* functor, int data )
{
    kDebug(30513) ;
    SubDocument subdoc( functor, data, QString(), QString() );
    m_subdocQueue.push( subdoc );
}

void Document::slotFootnoteFound(const wvWare::FunctorBase* functor, int data)
{
    kDebug(30513) ;
    SubDocument subdoc( functor, data, QString(), QString() );
    (*subdoc.functorPtr)();
    delete subdoc.functorPtr;
}

//add KWord::Table object to the table queue
void Document::slotTableFound( const KWord::Table& table )
{
    kDebug(30513) ;
    m_tableQueue.push( table );
}

//add the picture SubDocument to the queue
void Document::slotPictureFound( const QString& frameName, const QString& pictureName,
                                 const wvWare::FunctorBase* pictureFunctor )
{
    kDebug(30513) ;
    SubDocument subdoc( pictureFunctor, 0, frameName, pictureName );
    m_subdocQueue.push( subdoc );
}

//process through all the subDocs and the tables
void Document::processSubDocQueue()
{
    kDebug(30513) ;
    // Table cells can contain footnotes, and footnotes can contain tables [without footnotes though]
    // This is why we need to repeat until there's nothing more do to (#79024)
    while ( !m_subdocQueue.empty() || !m_tableQueue.empty() )
    {
        while ( !m_subdocQueue.empty() )
        {
            SubDocument subdoc( m_subdocQueue.front() );
            Q_ASSERT( subdoc.functorPtr );
            (*subdoc.functorPtr)(); // call it
            delete subdoc.functorPtr; // delete it
            m_subdocQueue.pop();
        }
        while ( !m_tableQueue.empty() )
        {
            KWord::Table& table = m_tableQueue.front();
            m_tableHandler->tableStart( &table );
            Q3ValueList<KWord::Row> &rows = table.rows;
            for( Q3ValueList<KWord::Row>::Iterator it = rows.begin(); it != rows.end(); ++it ) {
                KWord::TableRowFunctorPtr f = (*it).functorPtr;
                Q_ASSERT( f );
                (*f)(); // call it
                delete f; // delete it
            }
            m_tableHandler->tableEnd();
            m_tableQueue.pop();
        }
    }
}

//get the subdocument
//create the frame for it
//create the picture element
KoStoreDevice* Document::createPictureFrameSet( const QSizeF& size )
{
    kDebug(30513) ;
    // Grab data that was stored with the functor, that triggered this parsing
    SubDocument subdoc( m_subdocQueue.front() );

    //QDomElement framesetElement = m_mainDocument.createElement("FRAMESET");
    //framesetElement.setAttribute( "frameType", 2 /*picture*/ );
    //framesetElement.setAttribute( "frameInfo", 0 );
    //framesetElement.setAttribute( "name", subdoc.name );
    //m_framesetsElement.appendChild(framesetElement);

    // The position doesn't matter as long as the picture is inline
    // FIXME for non-inline pics ####
    // To determine the size, look at OOo's filter (WW8PicDesc in ww8graf2.cxx, version 1.50, line 406)
    // Hint: #i17200#, a bit of guesswork I'm afraid
    //        if (aPic.dxaGoal == 1000 && aPic.mx == 1)  //100% hack ? (from ww8graf2.cxx)
    //createInitialFrame( framesetElement, 0, size.width(), 0, size.height(), false, NoFollowup );

    //QDomElement pictureElem = m_mainDocument.createElement("PICTURE");
    //framesetElement.appendChild( pictureElem );

    //QDomElement keyElem = m_mainDocument.createElement("KEY");
    //pictureElem.appendChild( keyElem );
    //keyElem.setAttribute( "filename", subdoc.extraName );
    m_pictureList.append( subdoc.extraName );

    kDebug(30513) <<"Preparing to write picture for '" << subdoc.name <<"' into" << subdoc.extraName;
    return m_chain->storageFile( subdoc.extraName, KoStore::Write );
}


#include "document.moc"
