/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1998                   */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* needs c++ library Qt (http://www.troll.no)                     */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* written for KDE (http://www.kde.org)                           */
/* KPresenter is under GNU GPL                                    */
/******************************************************************/
/* Module: KPresenter Utilities (header)                          */
/******************************************************************/

#ifndef _kpresenter_utils_h__
#define _kpresenter_utils_h__

#include <qpixmap.h>
#include <qstring.h>
#include <qregexp.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpntarry.h>
#include <qcolor.h>
#include <qsize.h>

#include "global.h"

QString load_pixmap(const char *_file);
QString load_pixmap_native_format(const char *_file);
QPixmap string_to_pixmap(const char *_pixmap);
void drawFigure(LineEnd figure,QPainter* painter,QPoint coord,QColor color,int _w,float angle);
QSize getBoundingSize(LineEnd figure,int _w);

#endif
