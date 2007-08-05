/*
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2005-2007 Thomas Zander <zander@kde.org>
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
#ifndef _KO_TOOLBOX_H_
#define _KO_TOOLBOX_H_

#include <KoCanvasObserver.h>

#include <QList>
#include <QMap>
#include <QHash>
#include <QDockWidget>

class QButtonGroup;
class QBoxLayout;
class QAbstractButton;
class ToolArea;
class KoCanvasController;
class KoCanvasBase;

/**
 * KoToolBox is a kind of super-specialized toolbar that can order
 * tools according to type and priority.
 *
 * The ToolBox is a container for tool buttons which are themselves
 * divided into sections.
 *
 * Adding buttons using addButton() will allow you to show those buttons.  You should connect
 * the button to your handling method yourself.
 *
 * The unique property of this toolbox is that it can be shown horizontal as well as vertical,
 * rotating in a smart way to show the buttons optimally.
 * @see KoToolManager
 */
class KoToolBox : public QDockWidget, public KoCanvasObserver {
    Q_OBJECT
public:
    /// constructor
    explicit KoToolBox(KoCanvasController *canvas, const QString &title);
    ~KoToolBox();

    /**
     * Add a button to the toolbox.
     * The buttons should all be added before the first showing since adding will not really add
     * them to the UI until setup() is called.
     *
     * @param button the new button.  Please make sure you connect to the button yourself.
     * @param section the section in which this button will be shown.  Each section will be its own
     *        widget.
     * @param priority the priority in the section. Lowest value means it will be shown first.
     * @param buttonGroupId if passed this will allow you to use setActiveTool() to trigger
     *      this button
     * @see setup()
     */
    void addButton(QAbstractButton *button, const QString &section, int priority, int buttonGroupId=-1);

public slots:
    /**
     * Using the buttongroup id passed in addButton() you can set the new active button.
     * If the id does not resolve to a visible button, this call is ignored.
     * @param canvas the currently active canvas.
     * @param id an id to identify the button to activate.
     */
    void setActiveTool(const KoCanvasController *canvas, int id);

    /**
     * Show only the dynamic buttons that have a code from parameter codes.
     * The toolbox allows buttons to be optionally registered with a visibilityCode. This code
     * can be passed here and all buttons that have that code are shown. All buttons that
     * have another visibility code registered are hidden.
     * @param canvas the currently active canvas.
     * @param codes a list of all the codes to show.
     */
    void setButtonsVisible(const KoCanvasController *canvas, const QList<QString> &codes);

    /**
     * Enables or disables all the tools that this toolbox shows.
     * @param enable if true, then the tools will be clickable.
     */
    void enableTools(bool enable);

    /// reimplemented from KoCanvasObserver
    virtual void setCanvas(KoCanvasBase *canvas);

private:
    /**
     * Setup the toolbox by adding the buttons in the right configuration to the ui.
     * You should only call this method one time, and you should call it prior to showing.
     */
    void setup();
    void setButtonsVisible(const KoCanvasBase *canvas, const QList<QString> &codes);


private:
    class ToolArea : public QWidget {
    public:
        ToolArea(QWidget *parent);
        ~ToolArea();

        void setOrientation (Qt::Orientation orientation);
        void add(QWidget *button);

        QWidget* getNextParent();

    private:
        QList<QWidget *> m_children;
        QBoxLayout *m_layout;

        QWidget *m_leftRow;
        QWidget *m_rightRow;
        QBoxLayout *m_leftLayout;
        QBoxLayout *m_rightLayout;

        bool m_left;
    };
    QButtonGroup *m_buttonGroup;
    QBoxLayout* m_layout;
    QList<ToolArea *> m_toolBoxes;
    QHash<QAbstractButton*, QString> m_visibilityCodes;

    // Section,  [prio, button]
    QMap<QString, QMultiMap<int, QAbstractButton*> > m_buttons;
    QMap<QString, ToolArea*> m_toolAreas;

    void showEvent(QShowEvent *event);
    const KoCanvasBase *m_canvas;
};

#endif // _KO_TOOLBOX_H_
