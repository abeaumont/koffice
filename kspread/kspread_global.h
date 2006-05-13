/* This file is part of the KDE project
   Copyright (C) 2005-2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
             (C) 2006 Fredrik Edemar <f_edemar@linux.se>
             (C) 2005-2006 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
             (C) 2004 Tomas Mecir <mecirt@gmail.com>
             (C) 2003 Norbert Andres <nandres@web.de>
             (C) 2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2000 David Faure <faure@kde.org>
             (C) 2000 Werner Trobin <trobin@kde.org>
             (C) 2000-2006 Laurent Montel <montel@kde.org>
             (C) 1999, 2000 Torben Weis <weis@kde.org>
             (C) 1999 Stephan Kulow <coolo@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_GLOBAL_H
#define KSPREAD_GLOBAL_H

#include <kiconloader.h>

#include "kspread_factory.h"

#include <QDateTime>
#include <kdebug.h>


#define KSBarIcon( x ) BarIcon( x, Factory::global() )
#define colWidth 60.0
#define heightOfRow 20.0

/* Definition of maximal supported rows - please check kspread_util (columnName) and kspread_cluster also */
#define KS_rowMax 0x7FFF
/* Definition of maximal supported columns - please check kspread_util (columnName) and kspread_cluster also */
#define KS_colMax 0x7FFF

/* Currently used by StyleCluster for the Quad tree - question is, if we can make the spreadsheet this big in general?
   We need this value to be the smallest number which is a power of 2 and more than both KS_rowMax and KS_colMax*/
#define KS_Max_Quad 0x8000

/* Definition of maximal supported columns/rows, which can be merged */
#define KS_spanMax 0xFFF


namespace KSpread
{

class ElapsedTime
{
 public:
  enum OutputMode { Default, PrintOnlyTime };
  ElapsedTime()
  {
    m_time.start();
  }

  ElapsedTime( QString const & name, OutputMode mode = Default )
    : m_name( name )
  {
    m_time.start();
    if ( mode != PrintOnlyTime )
      kDebug() << "*** (" << name << ")... Starting measuring... " << endl;
  }

  ~ElapsedTime()
  {
    uint milliSec = m_time.elapsed();
    uint min = (uint) ( milliSec / ( 1000 * 60 ) );
    milliSec -= ( min * 60 * 1000 );
    uint sec = (uint) ( milliSec / 1000 );
    milliSec -= sec * 1000;

    if ( m_name.isNull() )
      kDebug() << "*** Elapsed time: " << min << " min, " << sec << " sec, " << milliSec << " msec" << endl;
    else
      kDebug() << "*** (" << m_name << ") Elapsed time " << min << " min, " << sec << " sec, " << milliSec << " msec" << endl;
  }

 private:
  QTime   m_time;
  QString m_name;
};

/**
 * This namespace collects enumerations related to
 * pasting operations.
 */
namespace Paste
{
  /**
   * The pasted content
   */
  enum Mode
  {
    Normal /** Everything */,
    Text /** Text only */,
    Format /** Format only */,
    NoBorder /** not the borders */,
    Comment /** Comment only */,
    Result /** Result only, no formula */,
    NormalAndTranspose /** */,
    TextAndTranspose /** */,
    FormatAndTranspose /** */,
    NoBorderAndTranspose /** */
  };
  /**
   * The current cell value treatment.
   */
  enum Operation
  {
    OverWrite /** Overwrite */,
    Add /** Add */,
    Mul /** Multiply */,
    Sub /** Substract */,
    Div /** Divide */
  };
} // namespace Paste

/**
 * This namespace collects enumerations related to
 * cell content restrictions.
 */
namespace Restriction
{
  /**
   * The type of the restriction
   */
  enum Type
  {
    None /** No restriction */,
    Number /** Restrict to numbers */,
    Text /** Restrict to texts */,
    Time /** Restrict to times */,
    Date /** Restrict to dates */,
    Integer /** Restrict to integers  */,
    TextLength /** Restrict text length */,
    List /** Restrict to lists */
  };
} // namespace Restriction

namespace Action
{
  enum Type
  {
    Stop,
    Warning,
    Information
  };
}

enum Series { Column,Row,Linear,Geometric };
// necessary due to QDock* enums (Werner)
enum MoveTo { Bottom, Left, Top, Right, BottomFirst };
enum MethodOfCalc { SumOfNumber, Min, Max, Average, Count, NoneCalc, CountA };

enum FormatType {
    Generic_format=0,
    Number_format=1, Text_format=5, Money_format=10, Percentage_format=25,
    Scientific_format=30,
    ShortDate_format=35, TextDate_format=36, Time_format=50,
    SecondeTime_format=51,
    Time_format1=52, Time_format2=53, Time_format3=54, Time_format4=55,
    Time_format5=56, Time_format6=57, Time_format7=58, Time_format8=59,
    fraction_half=70,fraction_quarter=71,fraction_eighth=72,fraction_sixteenth=73,
    fraction_tenth=74,fraction_hundredth=75,fraction_one_digit=76,
    fraction_two_digits=77,fraction_three_digits=78,
    date_format1=200,date_format2=201,date_format3=202,date_format4=203,
    date_format5=204,date_format6=205,date_format7=206,date_format8=207,
    date_format9=208,date_format10=209,date_format11=210,date_format12=211,
    date_format13=212,date_format14=213,date_format15=214,date_format16=215,
    date_format17=216,date_format18=217,date_format19=218,date_format20=219,
    date_format21=220,date_format22=221,date_format23=222,date_format24=223,
    date_format25=224,date_format26=225,
    Custom_format = 300, No_format = 400
};

enum ModifyType {
  MT_NONE = 0,
  MT_MOVE,
  MT_RESIZE_UP,
  MT_RESIZE_DN,
  MT_RESIZE_LF,
  MT_RESIZE_RT,
  MT_RESIZE_LU,
  MT_RESIZE_LD,
  MT_RESIZE_RU,
  MT_RESIZE_RD
};

enum PropValue {
    STATE_ON = 0,
    STATE_OFF = 1,
    STATE_UNDEF = 2
};

//TODO Implement all objects :)
enum ToolEditMode {
    TEM_MOUSE = 0,
    //INS_RECT = 1,
    //INS_ELLIPSE = 2,
    //INS_TEXT = 3,
    //INS_PIE = 4,
    INS_OBJECT = 5,
    //INS_LINE = 6,
    INS_DIAGRAMM = 7,
    //INS_TABLE = 8,
    //INS_FORMULA = 9,
    //INS_AUTOFORM = 10,
    //INS_FREEHAND = 11,
    //INS_POLYLINE = 12,
    //INS_QUADRICBEZIERCURVE = 13,
    //INS_CUBICBEZIERCURVE = 14,
    //INS_POLYGON = 15,
    INS_PICTURE = 16
    //INS_CLIPART = 17,
    //TEM_ROTATE = 18,
    //TEM_ZOOM = 19,
    //INS_CLOSED_FREEHAND = 20,
    //INS_CLOSED_POLYLINE = 21,
    //INS_CLOSED_QUADRICBEZIERCURVE = 22,
    //INS_CLOSED_CUBICBEZIERCURVE = 23
};


} // namespace KSpread

#endif
