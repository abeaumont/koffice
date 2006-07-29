/*
 *  dlg_perftest.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef DLG_PERFTEST_H_
#define DLG_PERFTEST_H_

#include <kdialogbase.h>

class WdgPerfTest;

class DlgPerfTest: public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT

public:

    DlgPerfTest(QWidget * parent = 0,
             const char* name = 0);
    ~DlgPerfTest();

    WdgPerfTest * page();

private slots:

    void okClicked();
    void selectAllClicked();
    void deselectAllClicked();

    void setAllTestCheckBoxes(bool checked);

private:

    WdgPerfTest * m_page;
    double m_oldAngle;
    bool m_lock;

};

#endif // DLG_PERFTEST_H
