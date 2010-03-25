/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
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

#include "XlsxXmlChartReader.h"
#include "XlsxXmlDrawingReader.h"
#include "XlsxXmlWorksheetReader.h"
#include "XlsxImport.h"

#include "Charting.h"
#include "ChartExport.h"

#define MSOOXML_CURRENT_NS "c"
#define MSOOXML_CURRENT_CLASS XlsxXmlChartReader
#define BIND_READ_CLASS MSOOXML_CURRENT_CLASS

#include <MsooXmlReader_p.h>
#include <MsooXmlUtils.h>

#include <QFontMetricsF>

// calculates the column width in pixels
int columnWidth(unsigned long col, unsigned long dx = 0, qreal defaultColumnWidth = 8.43) {
    QFont font("Arial", 10);
    QFontMetricsF fm(font);
    const qreal characterWidth = fm.width("h");
    defaultColumnWidth *= characterWidth;
    return (defaultColumnWidth * col) + (dx / 1024.0 * defaultColumnWidth);
}

// calculates the row height in pixels
int rowHeight(unsigned long row, unsigned long dy = 0, qreal defaultRowHeight = 12.75)
{
    return defaultRowHeight * row + dy;
}

// Returns A for 1, B for 2, C for 3, etc.
QString columnName(uint column)
{
    QString s;
    unsigned digits = 1;
    unsigned offset = 0;
    for (unsigned limit = 26; column >= limit + offset; limit *= 26, digits++)
        offset += limit;
    for (unsigned col = column - offset; digits; --digits, col /= 26)
        s.prepend(QChar('A' + (col % 26)));
    return s;
}

XlsxXmlChartReaderContext::XlsxXmlChartReaderContext(XlsxXmlDrawingReaderContext* _drawingReaderContext)
    : MSOOXML::MsooXmlReaderContext()
    , drawingReaderContext(_drawingReaderContext)
    , m_chart(0)
    , m_chartExport(0)
{
}

XlsxXmlChartReaderContext::~XlsxXmlChartReaderContext()
{
    delete m_chart;
}

XlsxXmlChartReader::XlsxXmlChartReader(KoOdfWriters *writers)
    : MSOOXML::MsooXmlCommonReader(writers)
    , m_context(0)
    , m_currentSeries(0)
{
}

XlsxXmlChartReader::~XlsxXmlChartReader()
{
}

KoFilter::ConversionStatus XlsxXmlChartReader::read(MSOOXML::MsooXmlReaderContext* context)
{
    m_context = dynamic_cast<XlsxXmlChartReaderContext*>(context);
    Q_ASSERT(m_context);
    Q_ASSERT(m_context->drawingReaderContext);
    Q_ASSERT(m_context->drawingReaderContext->worksheetReaderContext);

    readNext();
    if (!isStartDocument()) {
        return KoFilter::WrongFormat;
    }

    readNext();
    if (!expectEl("c:chartSpace")) {
        return KoFilter::WrongFormat;
    }

    delete m_context->m_chart;
    m_context->m_chart = new Charting::Chart;

    delete m_context->m_chartExport;
    m_context->m_chartExport = new ChartExport(m_context->m_chart);

    while (!atEnd()) {
        QXmlStreamReader::TokenType tokenType = readNext();
        if(tokenType == QXmlStreamReader::Invalid || tokenType == QXmlStreamReader::EndDocument) break;
        if (isStartElement()) {
            TRY_READ_IF(plotArea)
            ELSE_TRY_READ_IF(legend)
        }
    }

    // static is fine here cause we only need to take care that that number is unique in the
    // exported ODS file and do not take if the number is continuous or whatever.
    static int chartNumber = 0;

    m_context->m_chartExport->m_href = QString("Chart%1").arg(++chartNumber);
    QMap<XlsxXmlDrawingReaderContext::AnchorType, XlsxXmlDrawingReaderContext::Position> positions = m_context->drawingReaderContext->m_positions;
    const QString sheetName = m_context->drawingReaderContext->worksheetReaderContext->worksheetName;
    if(! sheetName.isEmpty()) {
        m_context->m_chartExport->m_endCellAddress += sheetName + '.';
    }

    if(positions.contains(XlsxXmlDrawingReaderContext::FromAnchor)) {
        XlsxXmlDrawingReaderContext::Position f = positions[XlsxXmlDrawingReaderContext::FromAnchor];
        m_context->m_chartExport->m_endCellAddress += columnName(f.m_col) + QString::number(f.m_row);
        m_context->m_chartExport->m_x = QString::number(columnWidth(f.m_col-1, 0 /*f.m_colOff*/)) + "pt";
        m_context->m_chartExport->m_y = QString::number(rowHeight(f.m_row-1, 0 /*f.m_rowOff*/)) + "pt";
        if(positions.contains(XlsxXmlDrawingReaderContext::ToAnchor)) {
            XlsxXmlDrawingReaderContext::Position t = positions[XlsxXmlDrawingReaderContext::ToAnchor];
            m_context->m_chartExport->m_width = QString::number(columnWidth( t.m_col - f.m_col - 1, 0 /*t.m_colOff*/)) + "pt";
            m_context->m_chartExport->m_height = QString::number(rowHeight( t.m_row - f.m_row - 1, 0 /*t.m_rowOff*/)) + "pt";
        }
    }

    //m_context->m_chartExport->m_notifyOnUpdateOfRanges = m_cellRangeAddress;
    //m_context->m_chartExport->m_cellRangeAddress = m_cellRangeAddress;
    m_context->m_chartExport->m_notifyOnUpdateOfRanges = "Sheet1.D2:Sheet1.F2";
    m_context->m_chartExport->m_cellRangeAddress = "Sheet1.D2:Sheet1.F2";//string(cell->sheet()->name()) + "." + columnName(chart->m_chart->m_cellRangeAddress.left()) + QString::number(chart->m_chart->m_cellRangeAddress.top()) + ":" +

    // the index will by written by the XlsxXmlWorksheetReader
    //m_context->m_chartExport->saveIndex(body);

    // write the embedded object file
    KoStore* storeout = m_context->drawingReaderContext->worksheetReaderContext->import->outputStore();
    m_context->m_chartExport->saveContent(storeout, manifest);

    m_context = 0;
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL plotArea
KoFilter::ConversionStatus XlsxXmlChartReader::read_plotArea()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(ser)
            ELSE_TRY_READ_IF(pieChart)
            ELSE_TRY_READ_IF(firstSliceAng)
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL ser
KoFilter::ConversionStatus XlsxXmlChartReader::read_ser()
{
    READ_PROLOGUE
    
    m_currentSeries = new Charting::Series;
    m_context->m_chart->m_series << m_currentSeries;
    //m_currentSeries->m_dataTypeX = record->dataTypeX();
    //m_currentSeries->m_countXValues = record->countXValues();
    //m_currentSeries->m_countYValues = record->countYValues();
    //m_currentSeries->m_countBubbleSizeValues = record->countBubbleSizeValues();

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(val)
            //ELSE_TRY_READ_IF(idx)
            //ELSE_TRY_READ_IF(order)
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL val
KoFilter::ConversionStatus XlsxXmlChartReader::read_val()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(numCache)
            if (qualifiedName() == QLatin1String(QUALIFIED_NAME(f))) {
                //m_cellRangeAddress = readElementText();
                m_currentSeries->m_valuesCellRangeAddress = readElementText();
            }
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL numCache
KoFilter::ConversionStatus XlsxXmlChartReader::read_numCache()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (qualifiedName() == QLatin1String(QUALIFIED_NAME(ptCount))) {
                const QXmlStreamAttributes attrs(attributes());
                TRY_READ_ATTR_WITHOUT_NS(val)
                m_currentSeries->m_countYValues = val.toInt();
            }
            //else if (qualifiedName() == QLatin1String(QUALIFIED_NAME(pt)))
            //else if (qualifiedName() == QLatin1String(QUALIFIED_NAME(formatCode)))
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL legend
KoFilter::ConversionStatus XlsxXmlChartReader::read_legend()
{
    //TODO
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL pieChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_pieChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::PieImpl();
    }
    return KoFilter::OK;
}

KoFilter::ConversionStatus XlsxXmlChartReader::read_firstSliceAng()
{
    if(Charting::PieImpl* pie = dynamic_cast<Charting::PieImpl*>(m_context->m_chart->m_impl)) {
        const QXmlStreamAttributes attrs(attributes());
        TRY_READ_ATTR(val)
        pie->m_anStart = val.toInt(); // default value is zero
    }
    return KoFilter::OK;
}
