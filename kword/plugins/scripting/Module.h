/*
 * This file is part of KWord
 *
 * Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
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

#ifndef SCRIPTING_MODULE_H
#define SCRIPTING_MODULE_H

#include <QString>
#include <QStringList>
#include <QObject>

class KWDocument;
class KWView;

namespace Scripting {

    /**
    * The ScriptingModule class enables access to the KWord
    * functionality from within the scripting backends.
    */
    class Module : public QObject
    {
            Q_OBJECT
        public:
            explicit Module(QObject* parent = 0);
            virtual ~Module();

            KWDocument* doc();
            void setView(KWView* view = 0);

        public Q_SLOTS:

            /** Return the \a KoApplicationAdaptor object. */
            QObject* application();

            /** Return the \a KoDocumentAdaptor object. */
            QObject* document();

            /** Return total number of pages the document has. */
            int pageCount();
            /** Return the \a Page of the specific page number. */
            QObject* page(int pageNumber);

            /** Insert a new page and returns the new \a Page object.
            The new page is inserted after the page which has the pagenumber
            \p afterPageNum . If \p afterPageNum is 0, the new page is
            inserted before page 1.  In all cases, the new page will have
            the number afterPageNum+1. To append a new page use
            insertPage( pageCount() ). */
            QObject* insertPage( int afterPageNum );
            /** Remove the page with the pagenumber \p pageNumber . */
            void removePage( int pageNumber );

            /** Return the first pagenumber of the document. */
            int startPage();
            /** Set a new startpage for this document.  */
            void setStartPage(int pageNumber);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif
