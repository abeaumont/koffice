/*
** A program to convert the XML rendered by KWord into LATEX.
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

#include <stdlib.h>

#include <kdebug.h>

#include "fileheader.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
FileHeader::FileHeader()
{
	_hasHeader    = false;
	_hasFooter    = false;
	_hasColor     = false;
	_hasUnderline = false;
	_hasEnumerate = false;
	_hasGraphics  = false;
	_hasTable     = false;
	_standardPage = 0;
	_processing   = TP_NORMAL;
	setFileHeader(this);		/* for xmlParser class. */
}

/*******************************************/
/* Destructor                              */
/*******************************************/
FileHeader::~FileHeader()
{
	kdDebug() << "FileHeader Destructor" << endl;
}

/*******************************************/
/* AnalysePaperParam                       */
/*******************************************/
void FileHeader::analysePaperParam(const QDomNode balise)
{
	setFormat(getAttr(balise, "format").toInt());
	_width = getAttr(balise, "width").toInt();
	_height = getAttr(balise, "height").toInt();
	setOrientation(getAttr(balise, "orientation").toInt());
	setColumns(getAttr(balise, "columns").toInt());
	_columnSpacing = getAttr(balise, "columnspacing").toInt();
	setHeadType(getAttr(balise, "hType").toInt());
	setFootType(getAttr(balise, "fType").toInt());
	_headBody = getAttr(balise, "spHeadBody").toInt();
	_footBody = getAttr(balise, "spFootBody").toInt();
	//getAttr(balise, "zoom").toInt();
}

/*******************************************/
/* AnalysePaper                            */
/*******************************************/
void FileHeader::analysePaper(const QDomNode balise)
{
	QDomNode fils;
	
	analysePaperParam(balise);

	//setTokenCurrent(balise_initiale->pContent);
	// Analyse children markups --> PAPERBORDERS
	fils = getChild(balise, "PAPERSBORDERS");
	_leftBorder = getAttr(fils, "left").toInt();
	_rightBorder = getAttr(fils, "right").toInt();
	_bottomBorder = getAttr(fils, "bottom").toInt();
	_topBorder = getAttr(fils, "top").toInt();
}

/*******************************************/
/* AnalyseAttributs                        */
/*******************************************/
void FileHeader::analyseAttributs(const QDomNode balise)
{
	setProcessing(getAttr(balise, "processing").toInt());
	setStandardPge(getAttr(balise, "standardpage").toInt());
	_hasHeader = getAttr(balise, "hasHeader").toInt();
	_hasFooter = getAttr(balise, "hasFooter").toInt();
	setUnit(getAttr(balise, "unit").toInt());
}

/*******************************************/
/* Generate                                */
/*******************************************/
void FileHeader::generate(QTextStream &out)
{
	kdDebug() << "GENERATION OF THE FILE HEADER" << endl;
	generatePreambule(out);
	generatePackage(out);
	if(getFormat() == TF_CUSTOM)
		generatePaper(out);
	out << "%%%%%%%%%%%%%%%%%% END OF PREAMBULE %%%%%%%%%%%%%%%%%%" <<endl;
}

/*******************************************/
/* GeneratePaper                           */
/*******************************************/
void FileHeader::generatePaper(QTextStream &out)
{
	QString unit;

	out << "% Format of paper" << endl;
	kdDebug() << "Generate custom size paper" << endl;
	/* paper size */
	out << "\\setlength{\\paperwidth}{"  << _width  << "pt}" << endl;
	out << "\\setlength{\\paperheight}{" << _height << "pt}" << endl;
	/* FileHeader and footer */
	out << "\\setlength{\\headsep}{" << _headBody << "pt}" << endl;
	out << "\\setlength{\\footskip}{" << _footBody + _bottomBorder << "pt}" << endl;
	/* Margin */
	out << "\\setlength{\\topmargin}{" << _topBorder << "pt}" << endl;
	out << "\\setlength{\\textwidth}{" << _width - _rightBorder - _leftBorder << "pt}" << endl;
	out << endl;
}

/*******************************************/
/* GeneratePreambule                       */
/*******************************************/
void FileHeader::generatePreambule(QTextStream &out)
{
	out << "%% Generated by Kword. Don't modify this file but the file *.kwd." << endl;
	out << "%% Send an email to rjaolin@ifrance.com for bugs, whishes, .... Thanks you." << endl;
	out << "%% Compile this file with : latex filename.tex" << endl;
	out << "%% a dvi file will be generated." << endl;
	out << "%% The file uses the latex style (not the kword style). " << endl;
	out << "\\documentclass[";
	switch(getFormat())
	{
		case TF_A3:
			out << "";
			break;
		case TF_A4:
			out << "a4paper, ";
			break;
		case TF_A5:
			out << "a5paper, ";
			break;
		case TF_USLETTER:
			out << "letterpaper, ";
			break;
		case TF_USLEGAL:
			out << "legalpaper, ";
			break;
		case TF_SCREEN:
			out << "";
			break;
		case TF_CUSTOM:
			out << "";
			break;
		case TF_B3:
			out << "";
			break;
		case TF_USEXECUTIVE:
			out << "executivepaper, ";
			break;
	}
	if(getOrientation() == TO_LANDSCAPE)
		out << "landscape, ";
	/* To change : will use a special latexcommand to able to
	 * obtain more than one column :))
	 */
	switch(getColumns())
	{
		case TC_1:
			//out << "onecolumn, ";
			break;
		case TC_2:
			out << "twocolumn, ";
			break;
		case TC_MORE:
			out << "";
	}
	/* The font and the type of the doc. can not be changed, hmm ? */
	out << "11pt]{article}" << endl;
}

/*******************************************/
/* GeneratePackage                         */
/*******************************************/
void FileHeader::generatePackage(QTextStream &out)
{
	out << "% Package(s) to include" << endl;
	if(getFormat() == TF_A4)
		out << "\\usepackage[a4paper]{geometry}" << endl;
	if(hasFooter() || hasHeader())
		out << "\\usepackage{fancyhdr}" << endl;
	if(hasColor())
		out << "\\usepackage{color}" << endl;
	if(hasUnderline())
		out << "\\usepackage{ulem}" << endl;
	if(hasEnumerate())
		out << "\\usepackage{enumerate}" << endl;
	if(hasGraphics())
		out << "\\usepackage{graphics}" << endl;
	if(hasTable())
	{
		out << "\\usepackage{array}" << endl;
		out << "\\usepackage{multirow}" << endl;
	}
	out << "\\usepackage{textcomp}" << endl;
	out << endl;
			
}
