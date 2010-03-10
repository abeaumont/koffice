/*
 *  Copyright (c) 2010 Sebastian Sauer <sebsauer@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CHARTING_H
#define CHARTING_H

#include <QtCore/QString>
//#include <QtCore/QStringList>
#include <QtCore/QRect>
#include <QtCore/QMap>
//#include <QtGui/QColor>
#include <QtCore/QDebug>

namespace Charting
{

    class Value
    {
    public:
        enum DataId {
            SeriesLegendOrTrendlineName = 0x00, ///< Referenced data specifies the series, legend entry, or trendline name. Error bars name MUST be empty.
            HorizontalValues = 0x01, ///< Referenced data specifies the values or horizontal values on bubble and scatter chart groups of the series and error bars.
            VerticalValues = 0x02, ///< Referenced data specifies the categories or vertical values on bubble and scatter chart groups of the series and error bars.
            BubbleSizeValues = 0x03 ///< Referenced data specifies the bubble size values of the series.
        };
        DataId m_dataId;

        enum DataType {
            AutoGeneratedName = 0x00, ///< The data source is a category (3) name, series name or bubble size that was automatically generated.
            TextOrValue = 0x01, ///< The data source is the text or value as specified by the formula field.
            CellRange = 0x02 ///< The data source is the value from a range of cells in a sheet specified by the formula field.
        };
        DataType m_type;

        bool m_isUnlinkedFormat; ///< false=data uses the number formatting of the referenced data, true=data uses the custom formatting specified via m_numberFormat.
        unsigned m_numberFormat; ///< specifies the numnber format to use for the data.
        QString m_formula; ///< the optional formula. could be for example "[Sheet1.$D$2:$F$2]"

        Value(DataId dataId, DataType type, bool isUnlinkedFormat, unsigned numberFormat, const QString& formula) : m_dataId(dataId), m_type(type), m_isUnlinkedFormat(isUnlinkedFormat), m_numberFormat(numberFormat), m_formula(formula) {}
        virtual ~Value() {}
    };

    class Format
    {
    public:
        Format() {}
        virtual ~Format() {}
    };

    class PieFormat : public Format
    {
    public:
        int m_pcExplode; // from PieFormat
        PieFormat(int pcExplode = 0) : Format(), m_pcExplode(pcExplode) {}
    };

    class ChartImpl
    {
    public:
        ChartImpl() {}
        virtual ~ChartImpl() {}
        virtual const char* name() const = 0;
    };

    class PieImpl : public ChartImpl
    {
    public:
        int m_anStart;
        unsigned int m_pcDonut;
        PieImpl(int anStart, unsigned int pcDonut = 0) : ChartImpl(), m_anStart(anStart), m_pcDonut(pcDonut) {}
        virtual const char* name() const { return "circle"; }
    };

    class Obj
    {
    public:
        unsigned int m_mdTopLt, m_mdBotRt, m_x1, m_y1, m_x2, m_y2;
        explicit Obj() : m_mdTopLt(0), m_mdBotRt(0), m_x1(0), m_y1(0), m_x2(0), m_y2(0) {}
        virtual ~Obj() {}
    };

    class Text : public Obj
    {
    public:
        QString m_text;
        explicit Text() : Obj() {}
        virtual ~Text() {}
    };

    class Series
    {
    public:
        /// the type of data in categories, or horizontal values on bubble and scatter chart groups, in the series. MUST be either 0x0001=numeric or 0x0003=text.
        uint m_dataTypeX;
        /// the count of categories (3), or horizontal values on bubble and scatter chart groups, in the series.
        uint m_countXValues;
        /// the count of values, or vertical values on bubble and scatter chart groups, in the series.
        uint m_countYValues;
        /// the count of bubble size values in the series.
        uint m_countBubbleSizeValues;
        /// Range that contains the values that should be visualized by the dataSeries.
        QString m_valuesCellRangeAddress;
        /// The referenced values used in the chart
        QMap<Value::DataId, Value*> m_datasetValue;
        /// The formatting for the referenced values
        QList<Format*> m_datasetFormat;
        /// List of text records attached to the series.
        //QHash<QList<Text*> > texts;

        explicit Series() {}
        ~Series() { qDeleteAll(m_datasetValue); qDeleteAll(m_datasetFormat); }
    };

    /// The main charting class that represents a single chart.
    class Chart
    {
    public:
#if 0
        /// anchored to sheet
        QString m_sheetName;
        /// anchored to cell
        //unsigned long m_colL, m_rwT;
        
        QString m_href;
        QString m_endCellAddress;
        QString m_x, m_y, m_width, m_height;
        QString m_notifyOnUpdateOfRanges;
#endif
        /// Optional total positioning. The need to be ignored if the chart is embedded into a sheet.
        int m_total_x, m_total_y, m_total_width, m_total_height;

        //int m_x, m_y, m_width, m_height;

        /// If true then the chart is a 3d chart else teh chart is 2d.
        bool m_is3d;
        //int anRot, anElv, pcDist;
        /// Margins around the chart object
        int m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin;
        /// List of series
        QList<Series*> m_series;
        /// List of text records attached to the chart.
        QList<Text*> m_texts;
        /// Range of all referenced cells.
        QRect m_cellRangeAddress;
        /// Range that contains the vertical values (the categories) for the plot-area.
        QString m_verticalCellRangeAddress;
        /// The more concrete chart implementation like e.g. a PieImpl for a pie chart.
        ChartImpl *m_impl;

        explicit Chart() : /*m_colL(0), m_rwT(0),*/ m_total_x(-1), m_total_y(-1), m_total_width(-1), m_total_height(-1), m_is3d(false), m_leftMargin(0), m_topMargin(0), m_rightMargin(0), m_bottomMargin(0), m_impl(0) {}
        virtual ~Chart() { qDeleteAll(m_series); qDeleteAll(m_texts); delete m_impl; }
    };

} // namespace Charting

#endif