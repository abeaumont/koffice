/*
 * Copyright (c) 2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "dialog.h"
#include "dialog.moc"

#include <qhbox.h>
#include <qvgroupbox.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kdebug.h>


//-----------------------------------------------------------------------------
SelectionRange::SelectionRange(const QString &s)
{
    // fill
    QValueVector<QPair<uint, uint> > r;
    QStringList list = QStringList::split(',', s);
    QRegExp range("^([0-9]+)\\-([0-9]+)$");
    QRegExp one("^[0-9]+$");
    for (QStringList::iterator it = list.begin(); it!=list.end(); ++it) {
        if ( one.exactMatch(*it) ) {
            uint p = (*it).toUInt();
            r.append( qMakePair(p, p) );
        } else if ( range.exactMatch(*it) ) {
            uint p1 = range.cap(1).toUInt();
            uint p2 = range.cap(2).toUInt();
            if ( p1>p2 ) continue;
            r.append( qMakePair(p1, p2) );
        }
    }

    // order
    QPair<uint, uint> tmp;
    for (uint i=1; i<r.size(); i++)
        if ( r[i].first<r[i-1].first )
            qSwap(r[i-1], r[i]);

    // coalesce
    for (uint i=0; i<r.size(); i++)
        if ( i!=0 && r[i].first<=tmp.second )
            tmp.second = kMax(tmp.second, r[i].second);
        else {
            _ranges.append(r[i]);
            tmp = r[i];
            kdDebug(30516) << "selection range: (" << tmp.first << ","
                           << tmp.second << ") " << endl;
        }
}

uint SelectionRange::nbPages() const
{
    uint nb = 0;
    for (uint i=0; i<_ranges.size(); i++)
        nb += _ranges[i].second - _ranges[i].first + 1;
    return nb;
}

SelectionRangeIterator::SelectionRangeIterator(const SelectionRange &range)
    : _ranges(range._ranges)
{
    toFirst();
}

int SelectionRangeIterator::toFirst()
{
    if ( _ranges.size()==0 ) _current = -1;
    else {
        _index = 0;
        _current = _ranges[0].first;
    }
    return _current;
}

int SelectionRangeIterator::next()
{
    if ( _current==-1 ) return -1;
    if ( _current==int(_ranges[_index].second) ) {
        _index++;
        _current = (_index==_ranges.size() ? -1
                    : int(_ranges[_index].first));
    } else _current++;
    return _current;
}

//-----------------------------------------------------------------------------
PdfImportDialog::PdfImportDialog(uint nbPages, bool isEncrypted,
                                 QWidget *widget)
    : KDialogBase(Plain, i18n("KWord's PDF Import Filter"), Ok|Cancel, Ok,
                  widget, "pdf_import_dialog"), _nbPages(nbPages)
{
    QApplication::restoreOverrideCursor();

    QVBoxLayout *top = new QVBoxLayout(plainPage(), KDialogBase::marginHint(),
                                       KDialogBase::spacingHint());

    // page selection
    QVGroupBox *gbox = new QVGroupBox(i18n("Page Selection"), plainPage());
    gbox->setInsideSpacing(KDialogBase::spacingHint());
    top->addWidget(gbox);
    _group = new QButtonGroup;
    _allButton = new QRadioButton(i18n("All (%1 pages)").arg(nbPages), gbox);
    _allButton->setChecked(true);
    _group->insert(_allButton);
    QHBox *hbox = new QHBox(gbox);
    _rangeButton = new QRadioButton(i18n("Range"), hbox);
    _group->insert(_rangeButton);
    _range = new KLineEdit(hbox);
    _range->setFocus();
    connect(_range, SIGNAL(textChanged(const QString &)),
            SLOT(rangeChanged(const QString &)));

    // passwords
    gbox = new QVGroupBox(i18n("Passwords"), plainPage());
    top->addWidget(gbox);
    QGrid *grid = new QGrid(2, gbox);
    grid->setSpacing(KDialogBase::spacingHint());
    (void)new QLabel(i18n("Owner"), grid);
    _owner = new KLineEdit(grid);
    _owner->setEchoMode(QLineEdit::Password);
    (void)new QLabel(i18n("User"), grid);
    _user = new KLineEdit(grid);
    _user->setEchoMode(QLineEdit::Password);
    grid->setEnabled(isEncrypted);
}

PdfImportDialog::~PdfImportDialog()
{
    delete _group;
    QApplication::setOverrideCursor(Qt::waitCursor);
}

void PdfImportDialog::rangeChanged(const QString &)
{
    _rangeButton->setChecked(true);
}

SelectionRange PdfImportDialog::range() const
{
    return SelectionRange( (_allButton->isChecked() ?
                            QString("1-%1").arg(_nbPages) : _range->text()) );
}
