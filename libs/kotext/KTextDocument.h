/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
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

#ifndef KTEXTDOCUMENT_H
#define KTEXTDOCUMENT_H

#include <QtGui/QTextDocument>

#include <KDE/KUrl>

#include "KoList.h"

class KStyleManager;
class KInlineTextObjectManager;
class KUndoStack;
class KoTextEditor;
class KChangeTracker;

/**
 * KTextDocument provides an easy mechanism to set and access the
 * editing members of a QTextDocument. The meta data are stored as resources
 * in the QTextDocument using QTextDocument::addResource() and fetched
 * using QTextDocument::resource().
 *
 */
class KODFTEXT_EXPORT KTextDocument
{
public:
    /// Constructor
    KTextDocument(QTextDocument *document);
    /// Constructor
    KTextDocument(const QTextDocument *document);

    /// Destructor
    ~KTextDocument();

    /// Returns the document that was passed in the constructor
    QTextDocument *document() const;

    ///Returns the text editor for that document
    KoTextEditor *textEditor();

    ///Sets the text editor for the document
    void setTextEditor(KoTextEditor *textEditor);

    /// Sets the style manager that defines the named styles in the document
    void setStyleManager(KStyleManager *styleManager);

    /// Returns the style manager
    KStyleManager *styleManager() const;

    /// Sets the change tracker of the document
    void setChangeTracker(KChangeTracker *changeTracker);

    ///Returns the change tracker of the document
    KChangeTracker *changeTracker() const;

    ///Sets the global undo stack
    void setUndoStack(KUndoStack *undoStack);

    ///Returns the global undo stack
    KUndoStack *undoStack() const;

    ///Sets the global heading list
    void setHeadingList(KoList *list);

    ///Returns the global heading list
    KoList *headingList() const;

    /// Sets the lists of the document
    void setLists(const QList<KoList *> &lists);

    /// Returns the lists in the document
    QList<KoList *> lists() const;

    /// Adds a list to the document
    void addList(KoList *list);

    /// Removes a list from the document
    void removeList(KoList *list);

    /// Returns the KoList that holds \a block; 0 if block is not part of any list
    KoList *list(const QTextBlock &block) const;

    /// Returns the KoList that holds \a list
    KoList *list(QTextList *textList) const;

    /// Return the KoList that holds \a listId
    KoList *list(KListStyle::ListIdType listId) const;

    /// Returns the KInlineTextObjectManager
    KInlineTextObjectManager *inlineTextObjectManager() const;

    /// Set the KInlineTextObjectManager
    void setInlineTextObjectManager(KInlineTextObjectManager *manager);

    /**
     * Enum to describe the text document's automatic resizing behaviour
     */
    enum ResizeMethod {
        /// Makes sure that the text shape takes op only as much space as absolutely necessary
        /// to fit the entire text into its boundaries.
        AutoResize,
        /// Deactivates auto-resizing
        NoResize
    };

    /**
     * Specifies how the document should be resized upon a change in the document.
     *
     * If auto-resizing is turned on, text will not be wrapped unless enforced by e.g. a newline.
     *
     * By default, NoResize is set.
     */
    void setResizeMethod(ResizeMethod method);

    /**
     * Returns the auto-resizing mode. By default, this is NoResize.
     *
     * @see setResizeMethod
     */
    ResizeMethod resizeMethod() const;

    /**
     * Clears the text in the document. Unlike QTextDocument::clear(), this
     * function does not clear the resources of the QTextDocument.
     */
    void clearText();

    /// Enum (type) used to add resources using QTextDocument::addResource()
    enum ResourceType {
        StyleManager = QTextDocument::UserResource,
        Lists,
        InlineTextManager,
        ChangeTrackerResource,
        UndoStack,
        TextEditor,
        HeadingList
    };
    static const KUrl StyleManagerURL;
    static const KUrl ListsURL;
    static const KUrl InlineObjectTextManagerURL;
    static const KUrl ChangeTrackerURL;
    static const KUrl UndoStackURL;
    static const KUrl TextEditorURL;
    static const KUrl HeadingListURL;

private:
    QTextDocument *m_document;
};

#endif
