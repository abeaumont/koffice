/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __KWInsertLink__
#define __KWInsertLink__

#include <kdialogbase.h>

class QLineEdit;
class KURLRequester;


class internetLinkPage : public QWidget
{
  Q_OBJECT
public:
  internetLinkPage( QWidget *parent = 0, char *name = 0 );
  QString linkName();
  QString hrefName();
private:
  QString createInternetLink();
  QLineEdit* m_linkName, *m_hrefName;
};

class mailLinkPage : public QWidget
{
  Q_OBJECT
public:
  mailLinkPage( QWidget *parent = 0, char *name = 0 );
  QString linkName();
  QString hrefName();
private:
  QString createMailLink();
  QLineEdit* m_linkName, *m_hrefName;
};

class fileLinkPage : public QWidget
{
  Q_OBJECT
public:
  fileLinkPage( QWidget *parent = 0, char *name = 0 );
  QString linkName();
  QString hrefName();
private:
  QString createFileLink();
  QLineEdit* m_linkName;
  KURLRequester* m_hrefName;
};

class KWInsertLinkDia : public KDialogBase
{
    Q_OBJECT
public:
    KWInsertLinkDia( QWidget *parent, const char *name = 0 );
    static bool createLinkDia(QString & linkName, QString & hrefName);

    //internal
    QString linkName();
    QString hrefName();

protected slots:
    virtual void slotOk();
    void slotTextChanged ( const QString & );

private:
    fileLinkPage *fileLink;
    mailLinkPage *mailLink;
    internetLinkPage *internetLink;
};

#endif
