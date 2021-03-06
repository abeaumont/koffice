/* This file is part of the KDE project
 * Copyright (C) 2007,2010 Thomas Zander <zander@kde.org>
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

#include "KoPageLayoutDialog.h"

#include "KoPageLayoutWidget.h"
#include "KoPagePreviewWidget.h"

#include <klocale.h>
#include <kdebug.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTimer>

class KoPageLayoutDialog::Private
{
public:
    Private() : pageLayoutWidget(0), documentCheckBox(0) {}
    KOdfPageLayoutData layout;
    KoPageLayoutWidget *pageLayoutWidget;
    QCheckBox *documentCheckBox;
};


KoPageLayoutDialog::KoPageLayoutDialog(QWidget *parent, const KOdfPageLayoutData &layout)
    : KPageDialog(parent)
    , d(new Private)
{
    setWindowTitle(i18n("Page Layout"));
    setFaceType(KPageDialog::Tabbed);

    QWidget *widget = new QWidget(this);
    addPage(widget, i18n("Page"));

    QHBoxLayout *lay = new QHBoxLayout(widget);
    lay->setMargin(0);
    widget->setLayout(lay);

    d->pageLayoutWidget = new KoPageLayoutWidget(widget, layout);
    d->pageLayoutWidget->showUnitchooser(false);
    d->pageLayoutWidget->layout()->setMargin(0);
    lay->addWidget(d->pageLayoutWidget);
    d->layout = d->pageLayoutWidget->pageLayout();


    KoPagePreviewWidget *prev = new KoPagePreviewWidget(widget);
    prev->setPageLayout(d->layout);
    lay->addWidget(prev);

    connect (d->pageLayoutWidget, SIGNAL(layoutChanged(const KOdfPageLayoutData&)),
            prev, SLOT(setPageLayout(const KOdfPageLayoutData&)));
    connect (d->pageLayoutWidget, SIGNAL(layoutChanged(const KOdfPageLayoutData&)),
            this, SLOT(setPageLayout(const KOdfPageLayoutData&)));
    connect (d->pageLayoutWidget, SIGNAL(unitChanged(const KUnit&)),
            this, SIGNAL(unitChanged(const KUnit&)));
}

KoPageLayoutDialog::~KoPageLayoutDialog()
{
    delete d;
}

KOdfPageLayoutData KoPageLayoutDialog::pageLayout() const
{
    return d->layout;
}

void KoPageLayoutDialog::setPageLayout(const KOdfPageLayoutData &layout)
{
    d->layout = layout;
}

void KoPageLayoutDialog::accept()
{
    KPageDialog::accept();
    deleteLater();
}

void KoPageLayoutDialog::reject()
{
    KPageDialog::reject();
    deleteLater();
}

bool KoPageLayoutDialog::applyToDocument() const
{
    return d->documentCheckBox && d->documentCheckBox->isChecked();
}

void KoPageLayoutDialog::showApplyToDocument(bool on)
{
    if (on && d->documentCheckBox == 0) {
        for (int i = 0; i < children().count(); ++i) {
            if (QDialogButtonBox *buttonBox = qobject_cast<QDialogButtonBox*>(children()[i])) {
                d->documentCheckBox = new QCheckBox(i18n("Apply to document"), buttonBox);
                d->documentCheckBox->setChecked(true);
                buttonBox->addButton(d->documentCheckBox, QDialogButtonBox::ResetRole);
                break;
            }
        }

        Q_ASSERT(d->pageLayoutWidget);
        connect (d->documentCheckBox, SIGNAL(toggled(bool)),
                d->pageLayoutWidget, SLOT(setApplyToDocument(bool)));
    } else if (d->documentCheckBox) {
        d->documentCheckBox->setVisible(on);
    }
}

void KoPageLayoutDialog::showTextDirection(bool on)
{
    d->pageLayoutWidget->showTextDirection(on);
}

KOdfText::Direction KoPageLayoutDialog::textDirection() const
{
    return d->pageLayoutWidget->textDirection();
}

void KoPageLayoutDialog::setTextDirection(KOdfText::Direction direction)
{
    d->pageLayoutWidget->setTextDirection(direction);
}

void KoPageLayoutDialog::showPageSpread(bool on)
{
    d->pageLayoutWidget->showPageSpread(on);
}

void KoPageLayoutDialog::setPageSpread(bool pageSpread)
{
    d->pageLayoutWidget->setPageSpread(pageSpread);
}

void KoPageLayoutDialog::showUnitchooser(bool on)
{
    d->pageLayoutWidget->showUnitchooser(on);
}

void KoPageLayoutDialog::setUnit(const KUnit &unit)
{
    d->pageLayoutWidget->setUnit(unit);
}

