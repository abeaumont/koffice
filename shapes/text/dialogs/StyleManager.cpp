/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#define PARAGRAPH_STYLE 1000
#define CHARACTER_STYLE 1001

#include "StyleManager.h"

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

StyleManager::StyleManager(QWidget *parent)
    :QWidget(parent)
{
    widget.setupUi(this);
    layout()->setMargin(0);

    connect (widget.styles, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
                this, SLOT(setStyle(QListWidgetItem*, QListWidgetItem*)));
}

void StyleManager::setStyleManager(KoStyleManager *sm) {
    Q_ASSERT(sm);
    m_styleManager = sm;
    widget.styles->clear();
    bool defaultOne = true;
    foreach(KoParagraphStyle *style, m_styleManager->paragraphStyles()) {
        if(defaultOne) {
            defaultOne = false;
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(style->name(), widget.styles, PARAGRAPH_STYLE);
        item->setData(PARAGRAPH_STYLE, style->styleId());
        widget.styles->addItem(item);
        m_paragraphStyles.append(style);
    }
    QListWidgetItem *separator = new QListWidgetItem(widget.styles);
    separator->setBackgroundColor(QColor(Qt::black)); // TODO use theme
    separator->setSizeHint(QSize(20, 2));
    defaultOne = true;
    foreach(KoCharacterStyle *style, m_styleManager->characterStyles()) {
        if(defaultOne) {
            defaultOne = false;
            continue;
        }
        if(separator)
            widget.styles->addItem(separator);
        separator = 0;

        QListWidgetItem *item = new QListWidgetItem(style->name(), widget.styles, CHARACTER_STYLE);
        item->setData(CHARACTER_STYLE, style->styleId());
        widget.styles->addItem(item);
        m_characterStyles.append(style);
    }
    delete separator;

    widget.paragraphStylePage->setParagraphStyles(m_paragraphStyles);
    widget.styles->setCurrentRow(0);
}

void StyleManager::setStyle(QListWidgetItem *item, QListWidgetItem *previous) {
    int styleId = item->data(PARAGRAPH_STYLE).toInt();
    if(styleId > 0) {
        // TODO copy so we can press cancel
        KoParagraphStyle *style = m_styleManager->paragraphStyle(styleId);
        widget.paragraphStylePage->setStyle(style);
        widget.stackedWidget->setCurrentWidget(widget.paragraphStylePage);
    }
    else {
        styleId = item->data(CHARACTER_STYLE).toInt();
        if(styleId > 0) {
            KoCharacterStyle *style = m_styleManager->characterStyle(styleId);
            //widget.characterStylePage->setStyle(style);
            widget.stackedWidget->setCurrentWidget(widget.characterStylePage);
        }
        else {
            // separator clicked.
            const int row = widget.styles->row(item);
            if(widget.styles->row(previous) == row + 1) // moving up.
                widget.styles->setCurrentRow(row -1);
            else if(widget.styles->row(previous) == row - 1) // moving down.
                widget.styles->setCurrentRow(row +1);
            else
                widget.styles->setCurrentItem(previous);
        }
    }
}

#include <StyleManager.moc>
