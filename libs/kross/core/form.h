/***************************************************************************
 * form.h
 * This file is part of the KDE project
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_FORM_H
#define KROSS_FORM_H

#include <QString>
#include <QWidget>

#include <kpagedialog.h>
#include <kfiledialog.h>

#include <koffice_export.h>

namespace Kross {

    /// \internal implementation of the customized KFileDialog
    class FormFileWidgetImpl;

    /**
     * The FormFileWidget class provides a in a widget embedded KFileDialog.
     */
    class KROSS_EXPORT FormFileWidget : public QWidget
    {
            Q_OBJECT
            Q_ENUMS(Mode)

        public:
            typedef enum Mode { Other = 0, Opening, Saving };

            FormFileWidget(QWidget* parent, const QString& startDirOrVariable);
            virtual ~FormFileWidget();

        public slots:

            void setMode(const QString& mode);

            QString currentFilter() const;
            void setFilter(QString filter);

            QString currentMimeFilter() const;
            void setMimeFilter(const QStringList& filter);

            QString selectedFile() const;
            //QStringList selectedFiles() const { return KFileDialog::selectedFiles(); }
            //QString selectedUrl() const { return KFileDialog::selectedUrl().toLocalFile(); }

        private:
            virtual bool eventFilter(QObject* watched, QEvent* e);
            virtual void showEvent(QShowEvent* event);
            virtual void hideEvent(QHideEvent* event);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * The FormDialog class provides access to KDialog objects as
     * top-level containers.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * mydialog = Kross.forms().createDialog("MyDialog")
     * mydialog.setButtons("Ok|Cancel")
     * mywidget = Kross.forms().createWidgetFromUIFile(mydialog, "./mywidget.ui")
     * mywidget["QLineEdit"].setText("some string")
     * if mydialog.exec_loop():
     *     if mydialog.result() == "Ok":
     *         print mywidget["QLineEdit"].text
     * \endcode
     */
    class KROSS_EXPORT FormDialog : public KPageDialog
    {
            Q_OBJECT

        public:
            FormDialog(const QString& caption);
            virtual ~FormDialog();

        public slots:

            /**
             * Set the buttons.
             *
             * \param buttons string that defines the displayed buttons. For example the
             * string may look like "Ok" or "Ok|Cancel" or "Yes|No|Cancel".
             * \return true if the passed \p buttons string was valid and setting the
             * buttons was successfully else false is returned.
             */
            bool setButtons(const QString& buttons);

            /**
             * Set the face type of the dialog.
             *
             * \param facetype the face type which could be "Auto", "Plain", "List",
             * "Tree" or "Tabbed" as defined in \a KPageView::FaceType .
             */
            bool setFaceType(const QString& facetype);

            QString currentPage() const;
            void setCurrentPage(const QString& name);

            QWidget* page(const QString& name) const;
            QWidget* addPage(const QString& name, const QString& header, const QString& iconname);

            /**
             * Shows the dialog as a modal dialog, blocking until the user
             * closes it and returns the execution result.
             *
             * \return >=1 if the dialog was accepted (e.g. "Ok" pressed) else
             * the user rejected the dialog (e.g. by pressing "Cancel" or just
             * closing the dialog by pressing the escape-key).
             */
            int exec() { return KDialog::exec(); }

            /**
             * Same as the \a exec() method above provided for Python-lovers (python
             * does not like functions named "exec" and PyQt named it "exec_loop", so
             * just let's do the same).
             */
            int exec_loop() { return exec(); }

            /**
             * \return the result. The result may for example "Ok", "Cancel", "Yes" or "No".
             */
            QString result();

        protected:
            virtual void showEvent(QShowEvent* event);
            virtual void hideEvent(QHideEvent* event);

        private slots:
            virtual void slotButtonClicked(int button);
            void slotCurrentPageChanged(KPageWidgetItem* current);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * The FormModule provides access to UI functionality.
     */
    class KROSS_EXPORT FormModule : public QObject
    {
            Q_OBJECT

        public:
            FormModule(QObject* parent);
            virtual ~FormModule();

        public slots:

            /**
             * \return the active modal widget. Modal widgets are special top-level
             * widgets which are subclasses of QDialog and are modal.
             */
            QWidget* activeModalWidget();

            /**
             * \return the application top-level window that has the keyboard input
             * focus, or NULL if no application window has the focus.
             */
            QWidget* activeWindow();

            /**
             * Create and return a new \a FormDialog instance.
             *
             * \param caption The displayed caption of the dialog.
             */
            QWidget* createDialog(const QString& caption);

#if 0
            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the new QWidget is a child of parent.
             * \param classname the name of the class that should be
             * created. For example "QComboBox" or "QPushButton".
             * \return the new QWidget instance or NULL.
             */
            QWidget* createWidget(QWidget* parent, const QString& classname);
#endif

            /**
             * Create and return a new \a FormFileWidget instance.
             *
             * \param parent the parent QWidget the new \a FormFileWidget instance
             * is a child of.
             * \param startDirOrVariable the start-directory or -variable.
             * \return the new \a FormFileWidget instance or NULL.
             */
            QWidget* createFileWidget(QWidget* parent, const QString& startDirOrVariable);

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the new QWidget is a child of parent.
             * \param xml the UI XML string used to construct the new widget.
             * \return the new QWidget instance or NULL.
             */
            QWidget* createWidgetFromUI(QWidget* parent, const QString& xml);

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the new QWidget is a child of parent.
             * \param filename the full filename of the UI file which is readed
             * and it's UI XML content is used to construct the new widget.
             */
            QWidget* createWidgetFromUIFile(QWidget* parent, const QString& filename);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };
}

#endif

