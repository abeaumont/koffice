// $Header$

/* This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <nicog@snafu.de>

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

#include <kdebug.h>
#include <qptrstack.h>
#include <qdom.h>

#include "ImportTags.h"
#include "htmlimportsax.h"

// *Note for the reader of this code*
// Tags in lower case (e.g. <p>) are HTML's ones.
// Tags in upper case (e.g. <TEXT>) are KWord's ones.

bool CSS2StylesMap::setProperty(QString newName, QString newValue)
{
    replace(newName,CSS2Styles(newValue));
}

// Treat the "style" attribute of tags and split it in separates names and values
void TreatCSS2Styles(QString strProps, CSS2StylesMap &css2StylesMap)
{
    if (strProps.isEmpty())
        return;

    QString name,value;
    bool notFinished=true;
    int position=0;
    int result;
    while (notFinished)
    {
        //Find next name and its value
        result=strProps.find(':',position);
        if (result==-1)
        {
            name=strProps.mid(position).stripWhiteSpace();
            value="";
            notFinished=false;
        }
        else
        {
            name=strProps.mid(position,result-position).stripWhiteSpace();
            position=result+1;
            result=strProps.find(';',position);
            if (result==-1)
            {
                value=strProps.mid(position).stripWhiteSpace();
                notFinished=false;
            }
            else
            {
                value=strProps.mid(position,result-position).stripWhiteSpace();
                position=result+1;
            }
        }
        kdDebug(30503) << "========== (Property :" << name << "=" << value <<":)"<<endl;

        // Now set the property
        css2StylesMap.setProperty(name,value);
    }
}

void PopulateProperties(StackItem* stackItem, QString strStyle,
                        CSS2StylesMap& css2StylesMap, const bool allowInit)
// TODO: find a better name for this function
{
    if (allowInit)
    {
        // Initialize the QStrings with the previous values of the properties
        if (stackItem->italic)
        {
            css2StylesMap.setProperty("font-style","italic");
        }
        if (stackItem->bold)
        {
            css2StylesMap.setProperty("font-weight","bold");
        }

        if (stackItem->underline)
        {
            css2StylesMap.setProperty("text-decoration","underline");
        }
        else if (stackItem->strikeout)
        {
            css2StylesMap.setProperty("text-decoration","line-through");
        }
    }

    kdDebug(30503)<< "========== style=\"" << strStyle << "\"" << endl;
    TreatCSS2Styles(strStyle,css2StylesMap);

    stackItem->italic=(css2StylesMap["font-style"].getValue()=="italic");
    stackItem->bold=(css2StylesMap["font-weight"].getValue()=="bold");

    QString strDecoration=css2StylesMap["text-decoration"].getValue();
    stackItem->underline=(strDecoration=="underline");
    stackItem->strikeout=(strDecoration=="line-through");

    QString strTextPosition=css2StylesMap["text-position"].getValue();
    if (strTextPosition=="subscript")
    {
        stackItem->textPosition=1;
    }
    else if (strTextPosition=="superscript")
    {
        stackItem->textPosition=2;
    }
    else if (!strTextPosition.isEmpty())
    {
        // we have any other new value, assume it means normal!
        stackItem->textPosition=0;
    }

    QString strColour=css2StylesMap["color"].getValue();
    if (!strColour.isEmpty())
    {
        // we have a new colour, so decode it!
        long int colour=strColour.mid(1).toLong(NULL,16);
        // Note: mid(1) is for skiping the #
        // TODO: other colour descriptions of CSS2
        stackItem->red  =(colour&0xFF0000)>>16;
        stackItem->green=(colour&0x00FF00)>>8;
        stackItem->blue =(colour&0x0000FF);
    }

    QString strFontSize=css2StylesMap["font-size"].getValue();
    if (!strFontSize.isEmpty())
    {
        int size=0;
        int ch; // digit value of the character
        for (int pos=0;;pos++)
        {
            ch=strFontSize.at(pos).digitValue();
            if (ch==-1)
            {
                // Not a digit
                break;
            }
            else
            {
                size*=10;
                size+=ch;
            }
        }
        // TODO: verify that the unit of the font size is really "pt"
        if (size>0)
        {
            stackItem->fontSize=size;
        }
    }

    QString strFontFamily=css2StylesMap["font-family"].getValue();
    if (!strFontFamily.isEmpty())
    {
        // TODO: transform the font-family in a font we have on the system on which KWord runs.
        stackItem->fontName=strFontFamily;
    }
}

// Element <span>

bool StartElementSpan(StackItem* stackItem, StackItem* stackCurrent, const QString& strStyleLocal, const QString& strStyleAttribute)
{
    QString strStyle=strStyleLocal;
    strStyle+=strStyleAttribute;

    // <span> elements can be nested in <p> elements or in other <span> elements
    if ((stackCurrent->elementType==ElementTypeParagraph)||(stackCurrent->elementType==ElementTypeSpan))
    {
        CSS2StylesMap css2StylesMap;
        PopulateProperties(stackItem,strStyle,css2StylesMap,true);

        stackItem->stackElementParagraph=stackCurrent->stackElementParagraph;   // <PARAGRAPH>
        stackItem->stackElementText=stackCurrent->stackElementText;   // <TEXT>
        stackItem->stackElementFormatsPlural=stackCurrent->stackElementFormatsPlural; // <FORMATS>
        stackItem->pos=stackCurrent->pos; //Propagate the position
        stackItem->elementType=ElementTypeSpan;
    }
    else
    {   // We are nested in an unsupported element, so we cannot do much!
        kdError(30503) << "<span> tag not nested in neither a <p> nor a <span> tag (or a similar tag)" << endl;
        stackItem->elementType=ElementTypeUnknown; // Eat the content of the element!
    }
    return true;
}

bool charactersElementSpan (StackItem* stackItem, QDomDocument& mainDocument, const QString & ch)
{
    QDomElement elementText=stackItem->stackElementText;
    QDomElement elementFormatsPlural=stackItem->stackElementFormatsPlural;
    elementText.appendChild(mainDocument.createTextNode(ch));

    QDomElement formatElementOut=mainDocument.createElement("FORMAT");
    formatElementOut.setAttribute("id",1); // Normal text!
    formatElementOut.setAttribute("pos",stackItem->pos); // Start position
    formatElementOut.setAttribute("len",ch.length()); // Start position
    elementFormatsPlural.appendChild(formatElementOut); //Append to <FORMATS>
    stackItem->pos+=ch.length(); // Adapt new starting position

    if (!stackItem->fontName.isEmpty())
    {
        QDomElement fontElementOut=mainDocument.createElement("FONT");
        fontElementOut.setAttribute("name",stackItem->fontName); // Font name
        formatElementOut.appendChild(fontElementOut); //Append to <FORMAT>
    }

    if (stackItem->fontSize)
    {
        QDomElement fontElementOut=mainDocument.createElement("SIZE");
        fontElementOut.setAttribute("value",stackItem->fontSize);
        formatElementOut.appendChild(fontElementOut); //Append to <FORMAT>
    }

    if (stackItem->italic)
    {
        QDomElement fontElementOut=mainDocument.createElement("ITALIC");
        fontElementOut.setAttribute("value",1);
        formatElementOut.appendChild(fontElementOut); //Append to <FORMAT>
    }

    if (stackItem->bold)
    {
        QDomElement fontElementOut=mainDocument.createElement("WEIGHT");
        fontElementOut.setAttribute("value",75);
        formatElementOut.appendChild(fontElementOut); //Append to <FORMAT>
    }

    if (stackItem->underline)
    {
        QDomElement fontElementOut=mainDocument.createElement("UNDERLINE");
        fontElementOut.setAttribute("value",1);
        formatElementOut.appendChild(fontElementOut); //Append to <FORMAT>
    }

    if (stackItem->strikeout)
    {
        QDomElement fontElementOut=mainDocument.createElement("STRIKEOUT");
        fontElementOut.setAttribute("value",1);
        formatElementOut.appendChild(fontElementOut); //Append to <FORMAT>
    }

    if (stackItem->textPosition)
    {
        QDomElement fontElementOut=mainDocument.createElement("VERTALIGN");
        fontElementOut.setAttribute("value",stackItem->textPosition);
        formatElementOut.appendChild(fontElementOut); //Append to <FORMAT>
    }

    if (stackItem->red || stackItem->green || stackItem->blue)
    {
        QDomElement fontElementOut=mainDocument.createElement("COLOR");
        fontElementOut.setAttribute("red",stackItem->red);
        fontElementOut.setAttribute("green",stackItem->green);
        fontElementOut.setAttribute("blue",stackItem->blue);
        formatElementOut.appendChild(fontElementOut); //Append to <FORMAT>
    }
	return true;
}

bool EndElementSpan (StackItem* stackItem, StackItem* stackCurrent)
{
    if (!stackItem->elementType==ElementTypeSpan)
    {
        kdError(30503) << "Wrong element type!! Aborting! (</span> in StructureParser::endElement)" << endl;
        return false;
    }
    stackItem->stackElementText.normalize();
    stackCurrent->pos=stackItem->pos; //Propagate the position back to the parent element
    return true;
}

// Element <p>

bool StartElementP(StackItem* stackItem, StackItem* stackCurrent,
    QDomDocument& mainDocument, QDomElement& mainFramesetElement,
    const QString& strStyleLocal, const QString& strStyleAttribute, const QString& strAlign)
{
    QString strStyle=strStyleLocal;
    if (!strAlign.isEmpty())
    {
        strStyle+="text-align:";
        strStyle+=strAlign; //FIXME: improve security (check of strAlign needed!)
        strStyle+=";";
    }
    strStyle+=strStyleAttribute;

    QDomElement elementText=stackCurrent->stackElementText;
    //We use mainFramesetElement here not to be dependant that <section> has happened before
    QDomElement paragraphElementOut=mainDocument.createElement("PARAGRAPH");
    mainFramesetElement.appendChild(paragraphElementOut);
    QDomElement textElementOut=mainDocument.createElement("TEXT");
    paragraphElementOut.appendChild(textElementOut);
    QDomElement formatsPluralElementOut=mainDocument.createElement("FORMATS");
    paragraphElementOut.appendChild(formatsPluralElementOut);

    CSS2StylesMap css2StylesMap;
    PopulateProperties(stackItem,strStyle,css2StylesMap,false);

    stackItem->elementType=ElementTypeParagraph;
    stackItem->stackElementParagraph=paragraphElementOut; // <PARAGRAPH>
    stackItem->stackElementText=textElementOut; // <TEXT>
    stackItem->stackElementFormatsPlural=formatsPluralElementOut; // <FORMATS>
    stackItem->pos=0; // No text characters yet

    // Now we populate the layout
    QDomElement layoutElement=mainDocument.createElement("LAYOUT");
    paragraphElementOut.appendChild(layoutElement);

    QDomElement element;
    element=mainDocument.createElement("NAME");
    element.setAttribute("value","Standard");
    layoutElement.appendChild(element);

    element=mainDocument.createElement("FOLLOWING");
    element.setAttribute("value","Standard");
    layoutElement.appendChild(element);

    QString strFlow=css2StylesMap["text-align"].getValue();
    element=mainDocument.createElement("FLOW");
    if ((strFlow=="left") || (strFlow=="center") || (strFlow=="right") || (strFlow=="justify"))
    {
        element.setAttribute("align",strFlow);
    }
    else
    {
        element.setAttribute("align","left");
    }
    layoutElement.appendChild(element);

    QDomElement formatElementOut=mainDocument.createElement("FORMAT");
    layoutElement.appendChild(formatElementOut);

    if (!stackItem->fontName.isEmpty())
    {
        element=mainDocument.createElement("FONT");
        element.setAttribute("name",stackItem->fontName); // Font name
        formatElementOut.appendChild(element); //Append to <FORMAT>
    }

    element=mainDocument.createElement("SIZE");
    element.setAttribute("value",(stackItem->fontSize>0)?(stackItem->fontSize):12);
    formatElementOut.appendChild(element); //Append to <FORMAT>

    element=mainDocument.createElement("ITALIC");
    element.setAttribute("value",(stackItem->italic)?1:0);
    formatElementOut.appendChild(element); //Append to <FORMAT>

    element=mainDocument.createElement("WEIGHT");
    element.setAttribute("value",(stackItem->bold)?75:50);
    formatElementOut.appendChild(element); //Append to <FORMAT>

    element=mainDocument.createElement("UNDERLINE");
    element.setAttribute("value",(stackItem->underline)?1:0);
    formatElementOut.appendChild(element); //Append to <FORMAT>

    element=mainDocument.createElement("STRIKEOUT");
    element.setAttribute("value",(stackItem->strikeout)?1:0);
    formatElementOut.appendChild(element); //Append to <FORMAT>

    element=mainDocument.createElement("VERTALIGN");
    element.setAttribute("value",stackItem->textPosition);
    formatElementOut.appendChild(element); //Append to <FORMAT>

    element=mainDocument.createElement("COLOR");
    element.setAttribute("red",stackItem->red);
    element.setAttribute("green",stackItem->green);
    element.setAttribute("blue",stackItem->blue);
    formatElementOut.appendChild(element); //Append to <FORMAT>

    return true;
}

bool charactersElementP (StackItem* stackItem, QDomDocument& mainDocument, const QString & ch)
{
    QDomElement elementText=stackItem->stackElementText;

    elementText.appendChild(mainDocument.createTextNode(ch));

    stackItem->pos+=ch.length(); // Adapt new starting position

    return true;
}

bool EndElementP (StackItem* stackItem)
{
    if (!stackItem->elementType==ElementTypeParagraph)
    {
        kdError(30503) << "Wrong element type!! Aborting! (</p> in StructureParser::endElement)" << endl;
        return false;
    }
    stackItem->stackElementText.normalize();
    return true;
}

// <br> (forced line break)
bool StartElementBR(StackItem* stackItem, StackItem* stackCurrent,
                    QDomDocument& mainDocument,
                    QDomElement& mainFramesetElement)
{
    // We are simulating a line break by starting a new paragraph!
    // TODO: when KWord has learnt what line breaks are, change to them.

    if (stackCurrent->elementType==ElementTypeSpan)
    {
        // TODO: this can probabily happen , so we must support it!
        kdWarning(30503) << "Forced line break in <span> element! Ignoring! (Not supported)" <<endl;
        return true; // It is only a warning, so continue parsing!
    }
    if (stackCurrent->elementType!=ElementTypeParagraph)
    {
        kdError(30503) << "Forced line break found out of turn! Aborting! (in StartElementBR)" <<endl;
        return false;
    }
    // Now we are sure to be the child of a <p> element

    // The following code is similar to StartElementP
    // We use mainFramesetElement here not to be dependant that <section> has happened before
    QDomElement paragraphElementOut=mainDocument.createElement("PARAGRAPH");
    mainFramesetElement.appendChild(paragraphElementOut);
    QDomElement textElementOut=mainDocument.createElement("TEXT");
    paragraphElementOut.appendChild(textElementOut);
    QDomElement formatsPluralElementOut=mainDocument.createElement("FORMATS");
    paragraphElementOut.appendChild(formatsPluralElementOut);

    // We must now copy/clone the layout of elementText.

    QDomNodeList nodeList=stackCurrent->stackElementParagraph.elementsByTagName("LAYOUT");

    if (!nodeList.count())
    {
        kdError(30503) << "Unable to find <LAYOUT> element! Aborting! (in StartElementBR)" <<endl;
        kdDebug(30503) << mainDocument.toString() << endl;
        return false;
    }

    // Now clone it
    QDomNode newNode=nodeList.item(0).cloneNode(true); // We make a deep cloning of the first element/node
    if (newNode.isNull())
    {
        kdError(30503) << "Unable to clone <LAYOUT> element! Aborting! (in StartElementBR)" <<endl;
        return false;
    }
    paragraphElementOut.appendChild(newNode);

    // NOTE: The following code is similar to StartElementP but we are working on stackCurrent!
    // Now that we have done with the old paragraph,
    //  we can write stackCurrent with the data of the new one!
    stackCurrent->elementType=ElementTypeParagraph;
    stackCurrent->stackElementParagraph=paragraphElementOut; // <PARAGRAPH>
    stackCurrent->stackElementText=textElementOut; // <TEXT>
    stackCurrent->stackElementFormatsPlural=formatsPluralElementOut; // <FORMATS>
    stackCurrent->pos=0; // No text characters yet

    return true;
}

