/*
 * This file is part of KWord
 *
 * Copyright (c) 2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCRIPTING_TOOL_H
#define SCRIPTING_TOOL_H

//#include <QString>
//#include <QStringList>
#include <QObject>
#include <QAction>
#include <QPointer>
#include <QSignalMapper>

#include <KoToolProxy.h>
#include <KoTextSelectionHandler.h>
#include <KoTextSelectionHandler.h>
//#include <KWord.h>
#include <KWView.h>
#include <KWCanvas.h>

#include "Module.h"
#include "TextCursor.h"

class KWDocument;

namespace Scripting {

    /**
    * The Tool class provides access to functionality like handling for
    * example current/active selections.
    */
    class Tool : public QObject
    {
            Q_OBJECT
        public:
            explicit Tool(Module* module) : QObject(module), m_module(module) {
                KWView* v = dynamic_cast< KWView* >( m_module->view() );
                KWCanvas* c = v ? v->kwcanvas() : 0;
                m_toolproxy = c ? c->toolProxy() : 0;

                m_signalMapper = new QSignalMapper(this);
                QHash<QString, QAction*> actionhash = actions();
                for(QHash<QString, QAction*>::const_iterator it = actionhash.constBegin(); it != actionhash.constEnd(); ++it) {
                    connect(it.value(), SIGNAL(triggered()), m_signalMapper, SLOT(map()));
                    m_signalMapper->setMapping( it.value() , it.key() );
                }
                connect(m_signalMapper, SIGNAL(mapped(const QString&)), this, SIGNAL(actionTriggered(const QString&)));
            }
            virtual ~Tool() {}

            KoToolSelection* toolSelection() const {
                return m_toolproxy ? m_toolproxy->selection() : 0;
            }
            KoTextSelectionHandler* textSelection() const {
                return dynamic_cast< KoTextSelectionHandler* >( toolSelection() );
            }
            QHash<QString, QAction*> actions() const {
                return m_toolproxy ? m_toolproxy->actions() : QHash<QString, QAction*>();
            }

        public Q_SLOTS:

            /** Return true if there is actualy a selection. */
            bool hasSelection() const { return toolSelection() != 0; }

            /** Return true if the selected object is a text object. */
            bool hasTextSelection() const { return textSelection() != 0; }

            /** Return the selected text. */
            QString selectedText() const {
                KoTextSelectionHandler* h = textSelection();
                return h ? h->selectedText() : QString();
            }

            /** Return the active/current \a TextCursor object. */
            QObject* cursor() {
                KoTextSelectionHandler* h = textSelection();
                return h ? new TextCursor(this, h->caret()) : 0;
            }

            /** Set the active/current \a TextCursor object. */
            bool setCursor(QObject* cursor) {
                kDebug() << "Scripting::Selection::setCursor" << endl;
                TextCursor* textcursor = dynamic_cast< TextCursor* >(cursor);
                if( ! textcursor ) return false;
                KWView* v = dynamic_cast< KWView* >( m_module->view() );
                KWCanvas* c = v ? v->kwcanvas() : 0;
                KoCanvasResourceProvider* r = c ? c->resourceProvider() : 0;
                if( ! r ) return false;
                QVariant variant;
                variant.setValue( (QObject*) &textcursor->cursor() );
                r->setResource(KWord::CurrentTextCursor, variant);
                return true;
            }

            /** Return a list of the action names. */
            QStringList actionNames() {
                return QStringList( actions().keys() );
            }
            /** Return the text the action with \p name has. */
            QString actionText(const QString& actionname) {
                QAction* a = actions()[ actionname ];
                return a ? a->text() : QString();
            }
            /** Trigger the action with \p name . */
            void triggerAction(const QString& actionname) {
                QAction* a = actions()[ actionname ];
                if( a ) a->trigger();
            }

        Q_SIGNALS:

            /** This signal got emitted if an action was triggered. */
            void actionTriggered(const QString& actionname);

        private:
            Module* m_module;
            KoToolProxy* m_toolproxy;
            QSignalMapper* m_signalMapper;
    };

}

#endif
