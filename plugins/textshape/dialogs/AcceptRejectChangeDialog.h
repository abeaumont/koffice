/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
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

#ifndef ACCEPT_REJECT_CHANGE_DIALOG_H
#define ACCEPT_REJECT_CHANGE_DIALOG_H

#include <QtCore/QObject>
#include <KDE/KDialog>
#include <ui_AcceptRejectChangeDialog.h>

class KChangeTracker;

class AcceptRejectChangeDialog:public KDialog
{
    Q_OBJECT

    public:
        typedef enum{
            eDialogCancelled = 0,
            eChangeAccepted,
            eChangeRejected
        }AcceptRejectResult;

        AcceptRejectChangeDialog(KChangeTracker *changeTracker, int changeId);
        ~AcceptRejectChangeDialog();
        
    private:
        Ui::AcceptRejectChangeDialog ui;

    private slots:
        void changeAccepted();
        void changeRejected();
        void dialogCancelled();
};

#endif
