/* This file is part of the KOffice project
 * Copyright (C) 2005-2006,2008 Thomas Zander <zander@kde.org>
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

#ifndef TESTPAGEMANAGER_H
#define TESTPAGEMANAGER_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <qtest_kde.h>

#include <tests/MockShapes.h> // from flake

class TestPageManager : public QObject
{
    Q_OBJECT
private slots: // tests
    void init();
    void getAddPages();
    void getAddPages2();
    void createInsertPages();
    void removePages();
    void pageInfo();
    void testClipToDocument();
    void testOrientationHint();
    void testDirectionHint();
    void testPageNumber();
    void testPageTraversal();
    void testSetPageStyle();
    void testPageCount();
    void testPageSpreadLayout();
    void testInsertPage();
    void testPadding();
    void testPageOffset();
    void testBackgroundRefCount();
    void testAppendPageSpread();
    void testRemovePageSpread();
};

#endif
