/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// Description: Template Choose Dialog (header)

/******************************************************************/

#ifndef koTemplateChooseDia_h
#define koTemplateChooseDia_h

#include <qdialog.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbt.h>
#include <qlist.h>
#include <qfileinf.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qevent.h>
#include <qfile.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qsizepolicy.h>

#include <kiconloaderdialog.h>
#include <kapp.h>

class MyIconCanvas : public KIconLoaderCanvas
{
    Q_OBJECT

public:
    MyIconCanvas( QWidget *parent = 0, const QString &name = QString::null )
	: KIconLoaderCanvas( parent, name ) {}

    bool isCurrentValid() { return currentItem(); }

protected:
    void viewportMousePressEvent( QMouseEvent *e ) {
	KIconLoaderCanvas::viewportMousePressEvent( e );
	if ( isCurrentValid() ) {
	    QString s = getCurrent();
	    emit currentChanged( s );
	} else {
	    QString s = "";
	    emit currentChanged( s );
	}
    }

signals:
    void currentChanged( const QString & );

};

class QGridLayout;

class KInstance;

/******************************************************************/
/* Class: KoTemplateChooseDia					  */
/******************************************************************/

class KoTemplateChooseDia : public QDialog
{
    Q_OBJECT

public:
    enum ReturnType {Cancel, Template, File, Empty};

    KoTemplateChooseDia( QWidget *parent, const char *name, const QString& template_type, 
			 KInstance* global, bool _hasCancel, bool _onlyTemplates, 
			 const QString &importFilter, const QString &mimeType );
    ~KoTemplateChooseDia() {;}

    static ReturnType chooseTemplate( const QString& template_type, KInstance* global, QString &_template, 
				      bool _hasCancel, bool _onlyTemplates = true, 
				      const QString &importFilter = QString::null,
				      const QString &mimeType = QString::null );

    QString getTemplate() { return templateName; }
    QString getFullTemplate() { return fullTemplateName; }
    ReturnType getReturnType() { return returnType; }

private:
    struct Group {
	QFileInfo dir;
	QString name;
	QWidget *tab;
	MyIconCanvas *loadWid;
	QLabel *label;
    };

    void getGroups();
    void setupTabs();

    QList<Group> groupList;
    Group *grpPtr;
    QString template_type;
    QString templateName, fullTemplateName;
    bool onlyTemplates;
    QRadioButton *rbTemplates, *rbFile, *rbEmpty;
    QLabel *lFile;
    QPushButton *bFile, *ok;
    QTabWidget *tabs;
    ReturnType returnType;
    QGridLayout *grid;
    QString m_strImportFilter;
    QString m_strMimeType;
    KInstance* global;

private slots:
    void nameChanged( const QString & );
    void chosen();
    void currentChanged( const QString & );

    void openTemplate();
    void openFile();
    void openEmpty();
    void chooseFile();
    void tabsChanged( const QString & )
    { openTemplate(); }

signals:
    void templateChosen( const QString & );

};

#endif

