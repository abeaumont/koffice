/*
** Header file for inclusion with kword_xml2latex.c
**
** Copyright (C) 2000 Robert JACOLIN
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** To receive a copy of the GNU Library General Public License, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
*/

#ifndef __KSPREAD_LATEX_SPREADSHEET_H__
#define __KSPREAD_LATEX_SPREADSHEET_H__

#include <qptrlist.h>
#include <qtextstream.h>

#include "map.h"
#include "config.h"

enum EGenerate
{
	E_LATEX,
	E_KWORD,
	E_CONFIG
};

/***********************************************************************/
/* Class: Spreadsheet                                                  */
/***********************************************************************/

/**
 * This class hold a whole document with its headers, footers, footnotes, endnotes,
 * content, ... It can generate a latex file.
 */
class Spreadsheet: public XmlParser, Config
{

	//Paper _paper;
	Map _map;
	//Locale _locale;
	//AreaName _areaname;

	public:
		/**
		 * Constructor
		 *
		 * Creates a new instance of Spreadsheet.
		 */
		Spreadsheet();

		/**
		 * Destructor
		 *
		 * Remove the list of headers, footers and the body.
		 */
		virtual ~Spreadsheet();

		/**
		 * Accessors
		 */

		void analyse(const QDomNode);
		void analyse_attr(const QDomNode);

		void generate(QTextStream&, bool);

	private:
		/**
		 * Generate the second part of the preambule
		 */
		void generatePreambule(QTextStream&);

		/**
		 * Generate the header
		 */
		void  generateTypeHeader(QTextStream&);

		/**
		 * Generate the footer
		 */
		void  generateTypeFooter(QTextStream&);
};

#endif /* __KSPREAD_LATEX_SPREADSHEET_H__ */
