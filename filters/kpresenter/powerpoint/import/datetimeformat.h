 /*
* This file is part of the KDE project
*
* Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
*
* Contact: Amit Aggarwal <amit.5.aggarwal@nokia.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 or (at your option) any later version as published by
* the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
*/

#ifndef DATE_TIME_FORMAT_H
#define DATE_TIME_FORMAT_H

#include <KoGenStyles.h>

#include "libppt.h"

using namespace Libppt;

class DateTimeFormat
{
public:
    /**
    * @brief Constructor
    * @param Slide* master-  Master Slide
    */
     DateTimeFormat(Slide *master);

     /**
     * @brief Destructor
     * @param 
     */
    ~DateTimeFormat();

     /**
     * @brief addMasterDateTimeSection(KoXmlWriter& xmlWriter) - Add DateTime section Master styles in styles.xml
     * @param KoXmlWriter - Content Writer
     */
     void addMasterDateTimeSection(KoXmlWriter& xmlWriter,QString tStyle);

     /**
     * @brief addDateTimeAutoStyles - Add DateTime styles in styles.xml
     * @param KoGenStyles - Styles generator handle
     */
     void addDateTimeAutoStyles(KoGenStyles& styles);

     /**
     * @brief eHeaderFooterAtom - HeaderFooter enum as per spec of formatId
     * @param 
     */
     enum{
        fHasDate = 0x01,       //(1 bit): A bit that specifies whether the date is displayed in the footer.
        fHasTodayDate = 0x02,  //(1 bit): A bit that specifies whether the current datetime is used for displaying the datetime.
        fHasUserDate = 0x04,    //(1 bit): A bit that specifies whether the date specified in UserDateAtom
        fHasSlideNumber = 0x08,  //(1 bit): A bit that specifies whether the slide number is displayed in the footer.
        fHasHeader = 0x10,      //(1 bit): A bit that specifies whether the header text specified by HeaderAtom record is displayed.
        fHasFooter = 0x20,    //(1 bit): A bit that specifies whether the footer text specified by FooterAtom
    }eHeaderFooterAtom;
     
private:

    /**
     * @brief addDateStyle - Add DateTime styles in as per the formatId in styles.xml.
     * @param KoGenStyles - Styles generator handle.
     * @param bool dayofweek - Long format is dayof week is true in formatId.
     * @param bool longMonth - Long Month as per the formatId spec.
     * @param bool textualmonth - Month in textual format also.
     * @param bool longyear  - Long year format
     * @param QString seperator="/" - Seperator b/w date.
     */ 
    void addDateStyle(KoGenStyles& styles, 
                      bool dayofweek, bool longMonth, bool textualmonth, bool longyear,
                      QString seperator="/");

    /**
     * @brief addTimeStyle - Add Time styles in as per the formatId in styles.xml.
     * @param KoGenStyles - Styles generator handle.
     * @param bool hr12Format - Time in 12 hr format.
     * @param bool second - Second is required in time or not.
     * @param QString seperator=":" - Seperator b/w time.
     */ 
    void addTimeStyle(KoGenStyles& styles, bool hr12Format,bool second,QString seperator=":");

    /**
     * @brief setDateStyleName - set the date style name for further usage.
     * @param QString name - name of the style
     */ 
    void setDateStyleName(const QString &name);

    /**
     * @brief dateStyleName - Retrun the date style name .
     * @parma QString name - name of the style
     */ 
    QString getDateStyleName() const;

    /**
     * @brief setTimeStyleName - set the time style name for further usage.
     * @parma QString name - name of the style
     */ 
    void setTimeStyleName(const QString &name);

    /**
     * @brief timeStyleName - Return the time style name.
     * @parma QString name - name of the style
     */ 
    QString getTimeStyleName() const;

    /**
     * @brief eDateTimeFormat - DateTime different format enum
     * @param 
     */
    enum{
       ShortDate,                         //0
       LongDate,                          //1
       LongDateWithoutWeekday,            //2
       AltShortDate,                      //3
       ShortDateWithAbbrMonth,             //4
       ShortDateWithSlashes,               //5
       DateAnd12HrTime,                    //6
       Hr24Time,                           //7
       Hr24TimeWithSec,                    //8
       Hr12Time,                           //9
       Hr12TimeWithSec,                    //10
       FixedUserDateFormat,                //11
    }eDateTimeFormat;
       
    Slide *master;
    /**
     * @brief formatId - Date represtation format
     * @param 
     */
    int formatId;
    QString dateStyleName;
    QString timeStyleName;
   
};

#endif //datetimeformat.h
