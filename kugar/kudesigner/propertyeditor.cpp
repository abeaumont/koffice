/***************************************************************************
                          propertyeditor.cpp  -  description
                             -------------------
    begin                : 08.12.2002
    copyright            : (C) 2002 by Alexander Dymo
    email                : cloudtemple@mksat.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
 
#include <qdockwindow.h>
#include <qtable.h>
#include <qlayout.h>
 
#include "propertyeditor.h"
#include "property.h"

PropertyEditor::PropertyEditor( Place p, QWidget * parent, const char * name, WFlags f):
    QDockWindow(p, parent, name, f)
{
    setCloseMode(QDockWindow::Always);

    QWidget *container = new QWidget(this);

    table = new QTable(container);
    table->setNumRows(0);
    table->setNumCols(2);
    table->horizontalHeader()->setLabel(0, i18n("Property"));
    table->horizontalHeader()->setLabel(1, i18n("Value"));
    table->setLeftMargin(0);
    table->setColumnReadOnly(0, TRUE);

    QGridLayout *gl = new QGridLayout(container);
    gl->addWidget(table, 0, 0);
    setWidget(container);

    props = 0;
}


PropertyEditor::~PropertyEditor(){
}

void PropertyEditor::populateProperties(std::map<QString, Property> *v_props)
{
    props = v_props;

    for (std::map<QString, Property>::const_iterator it = props->begin(); it != props->end(); ++it)
    {
        int row = table->numRows() + 1;
        table->setNumRows(row);

        table->setText(row, 0, (*it).first);
        table->setCellWidget(row, 1, Property::editorOfType((*it).second.type(), this));
    }
}

void PropertyEditor::clearProperties()
{
    props->clear();
    delete props;
    props = 0;
}

#include "propertyeditor.moc"
