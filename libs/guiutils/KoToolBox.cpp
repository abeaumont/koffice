/*
 * Copyright (c) 2005-2008 Thomas Zander <zander@kde.org>
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

#include "KoToolBox.h"

#include <KoCanvasController.h>
#include <KoToolManager.h>

#include <KDebug>
#include <QLayout>
#include <QMap>
#include <QButtonGroup>
#include <QToolButton>
#include <QHash>

#include "math.h"

class SectionLayout : public QLayout
{
public:
    SectionLayout(QWidget *parent)
        : QLayout(parent)
    {
    }

    void addButton(QAbstractButton *button, int priority)
    {
        addChildWidget(button);
        m_priorities.insert(button, priority);
        int index = 1;
        foreach(QWidgetItem *item, m_items) {
            if (m_priorities.value(static_cast<QAbstractButton*>(item->widget())) > priority)
                break;
            index++;
        }
        m_items.insert(index-1, new QWidgetItem(button));
    }

    QSize sizeHint() const
    {
        Q_ASSERT(0);
        return QSize();
    }

    void addItem(QLayoutItem*) { Q_ASSERT(0); }
    QLayoutItem* itemAt(int i) const
    {
        if (m_items.count() <= i)
            return 0;
        return m_items.at(i);
    }
    QLayoutItem* takeAt(int i) { return m_items.takeAt(i); }
    int count() const { return m_items.count(); }

    void setGeometry (const QRect &rect)
    {
        int x = 0;
        int y = 0;
        const QSize &size = buttonSize();
        foreach (QWidgetItem* w, m_items) {
            if (w->isEmpty())
                continue;
            w->widget()->setGeometry(QRect(x, y, size.width(), size.height()));
            x += size.width();
            if (x + size.width() > rect.width()) {
                x = 0;
                y += size.height();
            }
        }
    }

    const QSize &buttonSize() const
    {
        if (!m_items.isEmpty() && ! m_buttonSize.isValid())
            const_cast<SectionLayout*> (this)->m_buttonSize = m_items[0]->widget()->sizeHint();
        return m_buttonSize;
    }

private:
    QSize m_buttonSize;
    QMap<QAbstractButton*, int> m_priorities;
    QList<QWidgetItem*> m_items;
};

class Section : public QWidget
{
public:
    Section(QWidget *parent = 0)
        : QWidget(parent),
        m_layout(new SectionLayout(this))
    {
        setLayout(m_layout);
    }

    void addButton(QAbstractButton *button, int priority)
    {
        m_layout->addButton(button, priority);
    }

    void setName(const QString &name)
    {
        m_name = name;
    }

    QString name() const
    {
        return m_name;
    }

    QSize iconSize() const
    {
        return m_layout->buttonSize();
    }

    int visibleButtonCount() const
    {
        int count = 0;
        for(int i = m_layout->count()-1; i >= 0; --i) {
            if (! static_cast<QWidgetItem*> (m_layout->itemAt(i))->isEmpty())
                ++count;
        }
        return count;
    }

private:
    SectionLayout *m_layout;
    QString m_name;
};

// Priorities for a specific columnwidth of a child depending on the width of the parent.
static const int rowPriorities[][5] = {
    { 1, 1, 1, 1, 1 },
    { 2, 1, 1, 1, 1 },
    { 3, 2, 1, 1, 1 },
    { 2, 4, 1, 1, 1 },
    { 3, 2, 4, 5, 1 }
};

class ToolBoxLayout : public QLayout
{
public:
    ToolBoxLayout(QWidget *parent)
        : QLayout(parent)
    {
        setSpacing(6);
    }

    QSize sizeHint() const
    {
        if (m_sections.isEmpty())
            return QSize();
        QSize oneIcon = static_cast<Section*> (m_sections[0]->widget())->iconSize();
        return QSize(oneIcon.width() * 2, oneIcon.height() * 2);
    }
    QSize minimumSize() const
    {
        return sizeHint();
    }

    void addSection(Section *section)
    {
        addChildWidget(section);
        m_sections.append(new QWidgetItem(section));
    }

    void addItem(QLayoutItem*)
    {
        Q_ASSERT(0); // don't let anything else be added. (code depends on this!)
    }

    QLayoutItem* itemAt(int i) const
    {
        if (m_sections.count() >= i)
            return 0;
        return m_sections.at(i);
    }
    QLayoutItem* takeAt(int i) { return m_sections.takeAt(i); }
    int count() const { return m_sections.count(); }
    bool hasHeightForWidth () const { return true; }

    void setGeometry (const QRect &rect)
    {
        tryPlaceItems(rect.width(), true);
    }

    int heightForWidth (int width) const
    {
        return tryPlaceItems(width, false);
    }

    /// returns height
    int tryPlaceItems(int width, bool actuallyPlace) const
    {
        if (m_sections.isEmpty())
            return 0;
        QSize iconSize = static_cast<Section*> (m_sections[0]->widget())->iconSize();
        const int maxColumns = qMax(1, width / iconSize.width());
        const int prioIndex = qMin(5, maxColumns) - 1;
        // kDebug() << "tryPlaceItems w:" << width << "prioIndex:" << prioIndex << "max:" << maxColumns;

        int x = 0;
        int y = 0;
        int rowHeight = 0; // height of the current row, in icons.
        int colsLeft = maxColumns;
        foreach (QWidgetItem *wi, m_sections) {
            Section *section = static_cast<Section*> (wi->widget());
            const int buttonCount = section->visibleButtonCount();
            if (buttonCount == 0)
                continue;
            // kDebug() << " + section" << buttonCount;
            int rows = 0;
            int preferredColumnWidth = 0; // in buttons
            for (int i = 0; i < maxColumns; ++i) {
                const int suggestedColumnWidth = rowPriorities[prioIndex][ qMin (4, i) ];
                // kDebug() << "   + " << i << suggestedColumnWidth;
                rows = (int) ceilf(buttonCount / (float) suggestedColumnWidth);
                if (suggestedColumnWidth > buttonCount) // would leave empty space.
                    continue;
                if (suggestedColumnWidth > colsLeft)
                    continue; // won't fit.
                if (x > 0) {
                    // check if it would be better to go to the next line;
                    const int wastedSpace = rows - rowHeight;
                    if (wastedSpace > ceilf(buttonCount / (float) maxColumns)) { // this would be a bad solution.
                        if (colsLeft <= buttonCount / rowHeight) { // no better place expected, go to next row
                            colsLeft = maxColumns;
                            y += rowHeight * iconSize.height() + spacing();
                            x = 0;
                            i = -1;
                            rowHeight = 0;
                            continue;
                        }
                    }
                }

                preferredColumnWidth = suggestedColumnWidth;
                if (rows > 2 && rows < colsLeft) // then its pretty wide, wait for something better...
                    continue;
                break;
            }
            // kDebug() << " + preferredColumnWidth" << preferredColumnWidth;
            Q_ASSERT(preferredColumnWidth > 0);
            if (rows > rowHeight)
                rowHeight = rows;
            if (actuallyPlace)
                section->setGeometry(QRect(x, y, preferredColumnWidth * iconSize.width(),
                            rowHeight * iconSize.height()));
            x += preferredColumnWidth * iconSize.width() + spacing();
            colsLeft = (width - x) / iconSize.width();
            if (colsLeft == 0) {
                colsLeft = maxColumns;
                y += rowHeight * iconSize.height() + spacing();
                x = 0;
                rowHeight = 0;
            }
        }
        return y;
    }

    int minimumHeightForWidth (int width) const
    {
        return heightForWidth(width);
    }

private:
    QList <QWidgetItem*> m_sections;
};

class KoToolBox::Private
{
public:
    Private(KoCanvasController *c) : layout(0), buttonGroup(0), canvas(c->canvas()) { }

    QMap<QString, Section*> sections;
    ToolBoxLayout *layout;
    QButtonGroup *buttonGroup;
    KoCanvasBase *canvas;
    QHash<QToolButton*, QString> visibilityCodes;
};

KoToolBox::KoToolBox(KoCanvasController *canvas)
    : d( new Private(canvas))
{
    d->layout = new ToolBoxLayout(this);
    d->buttonGroup = new QButtonGroup(this);
    setLayout(d->layout);
    foreach(KoToolManager::Button button, KoToolManager::instance()->createToolList()) {
        addButton(button.button, button.section, button.priority, button.buttonGroupId);
        d->visibilityCodes.insert(button.button, button.visibilityCode);
    }

    connect(KoToolManager::instance(), SIGNAL(changedTool(const KoCanvasController*, int)),
            this, SLOT(setActiveTool(const KoCanvasController*, int)));
    connect(KoToolManager::instance(), SIGNAL(toolCodesSelected(const KoCanvasController*, QList<QString>)),
            this, SLOT(setButtonsVisible(const KoCanvasController*, QList<QString>)));
}

KoToolBox::~KoToolBox()
{
    delete d;
}

void KoToolBox::addButton(QToolButton *button, const QString &section, int priority, int buttonGroupId)
{
    // ensure same L&F
    button->setCheckable(true);
    Section *sectionWidget = d->sections.value(section);
    if (sectionWidget == 0) {
        sectionWidget = new Section(this);
        sectionWidget->setName(section);
        d->layout->addSection(sectionWidget);
        d->sections.insert(section, sectionWidget);
    }
    sectionWidget->addButton(button, priority);

    if(buttonGroupId < 0)
        d->buttonGroup->addButton(button);
    else
        d->buttonGroup->addButton(button, buttonGroupId);
}

void KoToolBox::setActiveTool(const KoCanvasController *canvas, int id)
{
    if(canvas->canvas() != d->canvas)
        return;
    QAbstractButton *button = d->buttonGroup->button(id);
    if(button)
        button->setChecked(true);
    else
        kWarning(30004) << "KoToolBox::setActiveTool(" << id << "): no such button found\n";
}

void KoToolBox::setButtonsVisible(const KoCanvasController *canvas, const QList<QString> &codes)
{
    if(canvas->canvas() != d->canvas)
        return;
    foreach(QToolButton *button, d->visibilityCodes.keys()) {
        QString code = d->visibilityCodes.value(button);
        if(code == "flake/always")
            continue;
        if(code.isEmpty()) {
            button->setVisible(true);
            button->setEnabled( codes.count() != 0 );
        }
        else
            button->setVisible( codes.contains(code) );
    }
}

void KoToolBox::setCanvas(KoCanvasBase *canvas) {
    d->canvas = canvas;
}

#include "KoToolBox.moc"
