/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <qiodevice.h>
#include <qstring.h>
#include <qvaluestack.h>

/**
 * A class for writing out XML (to any QIODevice), with a special attention on performance.
 * The XML is being written out along the way, which avoids requiring the entire
 * document in memory (like QDom does), and avoids using QTextStream at all
 * (which in Qt3 has major performance issues when converting to utf8).
 */
class KoXmlWriter
{
public:
    /**
     * Create a KoXmlWriter instance to write out an XML document into
     * the given QIODevice.
     */
    KoXmlWriter( QIODevice* dev );
    /**
     *  Return an XML writer for saving Oasis XML into the device @p dev,
     *  including the XML processing instruction,
     *  the complete DOCTYPE tag (with systemId and publicId),
     *  and the root element with all its namespaces.
     *  You can add more namespaces afterwards with addAttribute.
     *
     *  @param rootElementName the tag name of the root element.
     *     This is either office:document, office:document-content,
     *     office:document-styles, office:document-meta or office:document-settings
     *  @return the KoXmlWriter instance. It becomes owned by the caller, which
     *  must delete it at some point.
     *
     * Once done with writing the contents of the root element, you
     * will need to call endElement(); endDocument(); before destroying the KoXmlWriter.
     */
    KoXmlWriter( QIODevice* dev, const char* rootElementName );

    /// Destructor
    ~KoXmlWriter();

    /**
     * Start the XML document.
     * This writes out the \<?xml?\> tag with utf8 encoding, and the DOCTYPE.
     * @param rootElemName the name of the root element, used in the DOCTYPE tag.
     * @param publicId the public identifier, e.g. "-//OpenOffice.org//DTD OfficeDocument 1.0//EN"
     * @param systemId the system identifier, e.g. "office.dtd" or a full URL to it.
     */
    void startDocument( const char* rootElemName, const char* publicId = 0, const char* systemId = 0 );

    /// Call this to terminate an XML document.
    void endDocument();

    /**
     * Start a new element, as a child of the current element.
     * @param tagName the name of the tag. Warning: this string must
     * remain alive until endElement, no copy is internally made.
     * Usually tagName is a string constant so this is no problem anyway.
     */
    void startElement( const char* tagName );

    /**
     * Overloaded version of addAttribute( const char*, const char* ),
     * which is a bit slower because it needs to convert @p value to utf8 first.
     */
    inline void addAttribute( const char* attrName, const QString& value ) {
        addAttribute( attrName, value.utf8() );
    }
    /**
     * Add an attribute whose value is an integer
     */
    inline void addAttribute( const char* attrName, int value ) {
        QCString str;
        str.setNum( value );
        addAttribute( attrName, str.data() );
    }
    /**
     * Add an attribute whose value is a floating point number
     * The number is written out with the highest possible precision
     * (unlike QString::number and setNum, which default to 6 digits)
     */
    void addAttribute( const char* attrName, double value );
    /**
     * Add an attribute which represents a distance, measured in pt
     * The number is written out with the highest possible precision
     * (unlike QString::number and setNum, which default to 6 digits),
     * and the unit name ("pt") is appended to it.
     */
    void addAttributePt( const char* attrName, double value );

    /// Overloaded version of the one taking a const char* argument, for convenience
    inline void addAttribute( const char* attrName, const QCString& value ) {
        addAttribute( attrName, value.data() );
    }
    /**
     * Add an attribute to the current element.
     */
    void addAttribute( const char* attrName, const char* value );
    /**
     * Terminate the current element. After this you should start a new one (sibling),
     * add a sibling text node, or close another one (end of siblings).
     */
    void endElement();
    /**
     * Overloaded version of addTextNode( const char* ),
     * which is a bit slower because it needs to convert @p str to utf8 first.
     */
    inline void addTextNode( const QString& str ) {
        addTextNode( str.utf8() );
    }
    /// Overloaded version of the one taking a const char* argument
    inline void addTextNode( const QCString& cstr ) {
        addTextNode( cstr.data() );
    }
    /**
     * Adds a text node as a child of the current element.
     * This is appends the litteral content of @p str to the contents of the element.
     * E.g. addTextNode( "foo" ) inside a \<p\> element gives \<p\>foo\</p\>,
     * and startElement( "b" ); endElement( "b" ); addTextNode( "foo" ) gives \<p\>\<b/\>foo\</p\>
     */
    void addTextNode( const char* cstr );

    // #### Maybe we want to subclass KoXmlWriter for manifest files.
    /**
     * Special helper for writing "manifest" files
     * This is equivalent to startElement/2*addAttribute/endElement
     * This API will probably have to change (or not be used anymore)
     * when we add support for encrypting/signing.
     */
    void addManifestEntry( const QString& fullPath, const QString& mediaType );

private:
    struct Tag {
        Tag( const char* t = 0 ) : tagName( t ), hasChildren( false ),
                                   openingTagClosed( false ), lastChildIsText( false ) {}
        const char* tagName;
        bool hasChildren; ///< element or text children
        bool openingTagClosed; ///< true once the '\>' in \<tag a="b"\> is written out
        bool lastChildIsText;
    };

    /// Write out \n followed by the number of spaces required.
    void writeIndent();

    // writeCString is much faster than writeString.
    // Try to use it as much as possible, especially with constants.
    void writeString( const QString& str );

    // unused and possibly incorrect if length != size
    //inline void writeCString( const QCString& cstr ) {
    //    m_dev->writeBlock( cstr.data(), cstr.size() - 1 );
    //}

    inline void writeCString( const char* cstr ) {
        m_dev->writeBlock( cstr, qstrlen( cstr ) );
    }
    inline void writeChar( char c ) {
        m_dev->putch( c );
    }
    inline void closeStartElement( Tag& tag ) {
        if ( !tag.openingTagClosed ) {
            tag.openingTagClosed = true;
            writeChar( '>' );
        }
    }
    char* escapeForXML( const char* source ) const;
    void init();

    QIODevice* m_dev;
    QValueStack<Tag> m_tags;

    char* m_indentBuffer; // TODO make it static, but then it needs a KStaticDeleter
    char* m_escapeBuffer; // can't really be static if we want to be thread-safe
    static const int s_escapeBufferLen = 10000;

    KoXmlWriter( const KoXmlWriter & ); // forbidden
    KoXmlWriter& operator=( const KoXmlWriter & ); // forbidden
};

#endif /* XMLWRITER_H */

