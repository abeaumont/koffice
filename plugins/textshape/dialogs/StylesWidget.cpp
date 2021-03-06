/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#include "StylesWidget.h"
#include "StylesModel.h"
#include "ParagraphGeneral.h"
#include "CharacterGeneral.h"
#include "StyleManager.h"

#include <KStyleManager.h>
#include <KCharacterStyle.h>
#include <KParagraphStyle.h>
#include <KCanvasBase.h>

#include <KDE/KDebug>
#include <KDE/KInputDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QFormLayout>
#include <QtGui/QRadioButton>
#include <QtCore/QPointer>

#define DEBUG_CHANGED

StylesWidget::StylesWidget(QWidget *parent)
        : QWidget(parent),
        m_styleManager(0),
        m_stylesModel(new StylesModel(0, this)),
        m_blockSignals(false),
        m_isEmbedded(false),
        m_canvasBase(0)
{
    widget.setupUi(this);
    widget.stylesView->setRootIsDecorated(false);
    widget.stylesView->setModel(m_stylesModel);
    widget.stylesView->header()->swapSections(0, 1);
    widget.stylesView->header()->resizeSection(1, 16);
    widget.stylesView->header()->hide();
    widget.stylesView->setExpandsOnDoubleClick(false);

    widget.newStyle->setIcon(KIcon("list-add"));
    widget.deleteStyle->setIcon(KIcon("list-remove"));
    widget.modifyStyle->setIcon(KIcon("configure"));
    widget.applyStyle->setIcon(KIcon("dialog-ok-apply"));

    setCurrent(QModelIndex()); // register that we don't have a selection at startup

    connect(widget.newStyle, SIGNAL(clicked()), this, SLOT(newStyleClicked()));
    connect(widget.deleteStyle, SIGNAL(clicked()), this, SLOT(deleteStyleClicked()));
    connect(widget.modifyStyle, SIGNAL(clicked()), this, SLOT(editStyle()));
    connect(widget.applyStyle, SIGNAL(clicked()), this, SLOT(applyStyle()));
    connect(widget.stylesView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(setCurrent(const QModelIndex&)));
    connect(widget.stylesView, SIGNAL(doubleClicked(const QModelIndex&)),
                this, SLOT(applyStyle(const QModelIndex&)));
    connect(m_stylesModel, SIGNAL(isMultiLevel(bool)), this, SLOT(setStylesAreNested(bool)));
}

void StylesWidget::setEmbedded(bool embed)
{
    m_isEmbedded = embed;

    widget.newStyle->setVisible(!embed);
    widget.deleteStyle->setVisible(!embed);
    widget.modifyStyle->setVisible(!embed);
    widget.applyStyle->setVisible(!embed);
    widget.stylesView->header()->resizeSection(1, 0); // don't show the bubbles
    widget.stylesView->expandAll();
}

void StylesWidget::setStyleManager(KStyleManager *sm)
{
    m_styleManager = sm;
    m_stylesModel->setStyleManager(sm);
}

void StylesWidget::setCurrentFormat(const QTextBlockFormat &format)
{
    if (format == m_currentBlockFormat)
        return;
    m_currentBlockFormat = format;
    int id = m_currentBlockFormat.intProperty(KParagraphStyle::StyleId);
    bool unchanged = true;
    KParagraphStyle *usedStyle = 0;
    if (m_styleManager)
        usedStyle = m_styleManager->paragraphStyle(id);
    if (usedStyle) {
	QMap<int, QVariant> props = m_currentBlockFormat.properties();
	for (QMap<int, QVariant>::const_iterator it = props.constBegin(); it != props.constEnd(); ++it) {
            int property = it.key();
            if (property == QTextFormat::ObjectIndex || property == KParagraphStyle::ListStyleId
                    || property == KParagraphStyle::MasterPageName
                    || property == QTextFormat::PageBreakPolicy)
                continue;
            if (property == KParagraphStyle::OutlineLevel) {
                // OutlineLevel itself is not present in the style, but DefaultOutlineLevel is.
                property = KParagraphStyle::DefaultOutlineLevel;
            }
            QVariant cur(m_currentBlockFormat.property(property));
            QVariant style(usedStyle->value(property));
            if (cur == style)
                continue;
            if (cur.type() == QVariant::Int && cur.toInt() == 0)
                continue;
            if (cur.type() == QVariant::Double && cur.toDouble() == 0.)
                continue;
            if (style.type() == QVariant::Int && style.toInt() == 0)
                continue;
            if (style.type() == QVariant::Double && style.toDouble() == 0.)
                continue;
#ifdef DEBUG_CHANGED
            QString type;
            if (m_currentBlockFormat.hasProperty(property)) {
                if (usedStyle->hasProperty(property))
                    type = "Changed";
                else
                    type = "New";
            } else {
                type = "Removed";
            }
            type += " 0x%1";
            type = type.arg(property, 4, 16);
            if (property >= QTextFormat::UserProperty)
                type += QString(" User+%2").arg(property - QTextFormat::UserProperty);
            kDebug() << "parag--" << type;
#endif
            unchanged = false;
            break;
        }
    }

    m_blockSignals = true;
    QModelIndex mi = m_stylesModel->setCurrentParagraphStyle(id, unchanged);
    widget.stylesView->scrollTo(mi);
    m_blockSignals = false;
}

void StylesWidget::setCurrentFormat(const QTextCharFormat &format)
{
    if (format == m_currentCharFormat)
        return;
    m_currentCharFormat = format;

    int id = m_currentCharFormat.intProperty(KCharacterStyle::StyleId);
    bool unchanged = true;
    KCharacterStyle *usedStyle = 0;
    if (m_styleManager)
        usedStyle = m_styleManager->characterStyle(id);
    if (usedStyle) {
        QTextCharFormat defaultFormat;
        usedStyle->unapplyStyle(defaultFormat); // sets the default properties.
	QMap<int, QVariant> props = m_currentCharFormat.properties();
        for (QMap<int, QVariant>::const_iterator it = props.constBegin(); it != props.constEnd(); ++it) {
            int property = it.key();
            if (property == QTextFormat::ObjectIndex || property == QTextFormat::ObjectType
                    || property == QTextFormat::ForegroundBrush
                    || property == QTextFormat::FontFamily
                    || property == KCharacterStyle::InlineInstanceId)
                continue;
            if (m_currentCharFormat.property(property) != usedStyle->value(property)
                    && m_currentCharFormat.property(property) != defaultFormat.property(property)) {
#ifdef DEBUG_CHANGED
                QString type;
                if (m_currentBlockFormat.hasProperty(property)) {
                    if (usedStyle->hasProperty(property))
                        type = "Changed";
                    else
                        type = "New";
                } else {
                    type = "Removed";
                }
                type += " 0x%1";
                type = type.arg(property, 4, 16);
                if (property >= QTextFormat::UserProperty)
                    type += QString(" User+%2").arg(property - QTextFormat::UserProperty);
                kDebug() << "char--" << type;
#endif
                unchanged = false;
                break;
            }
        }
    }

    m_blockSignals = true;
    m_stylesModel->setCurrentCharacterStyle(id, unchanged);
    m_blockSignals = false;
}

void StylesWidget::newStyleClicked()
{
    KDialog *dialog = new KDialog(this);
    QWidget *root = new QWidget(dialog);
    QFormLayout *lay = new QFormLayout(root);
    KLineEdit *name = new KLineEdit(root);
    name->setText(i18n("new style"));
    lay->addRow(i18n("Name:"), name);
    QRadioButton *pr = new QRadioButton(i18n("Paragraph style"), root);
    pr->setChecked(true);
    lay->addRow(i18n("Type:"), pr);
    QRadioButton *cr = new QRadioButton(i18n("Character style"), root);
    lay->addRow(0, cr);
    root->setLayout(lay);

    dialog->setCaption(i18n("Create New Style"));
    dialog->setMainWidget(root);
    if (dialog->exec() == KDialog::Accepted) {
        QString styleName = name->text();
        if (styleName.isEmpty())
            styleName = i18n("new style");
        if (cr->isChecked()) {
            KCharacterStyle *style = new KCharacterStyle();
            style->setName(styleName);
            m_styleManager->add(style);
        } else {
            KParagraphStyle *style = new KParagraphStyle();
            style->setName(styleName);
            m_styleManager->add(style);
        }
    }
}

void StylesWidget::deleteStyleClicked()
{
    QModelIndex index = widget.stylesView->currentIndex();
    Q_ASSERT(index.isValid());
    widget.stylesView->clearSelection();
    KParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
    if (paragraphStyle) {
        KCharacterStyle *s = paragraphStyle->characterStyle();
        m_styleManager->remove(paragraphStyle);
        bool inUse = false;
        foreach(KParagraphStyle *ps, m_styleManager->paragraphStyles()) {
            if (ps->characterStyle() == s) {
                inUse = true;
                break;
            }
        }
        if (!inUse)
            m_styleManager->remove(s);
    } else
        m_styleManager->remove(m_stylesModel->characterStyleForIndex(index));
}

void StylesWidget::editStyle()
{
    QModelIndex index = widget.stylesView->currentIndex();
    Q_ASSERT(index.isValid());

    QWidget *widget;
    KParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);

    if (paragraphStyle) {
        StyleManager *styleManager = new StyleManager();
        styleManager->hideSelector();
        styleManager->setStyleManager(m_styleManager);
        if (m_canvasBase)
            styleManager->setUnit(m_canvasBase->unit());
        styleManager->setParagraphStyle(paragraphStyle);
        styleManager->layout()->setMargin(0);
        widget = styleManager;
    } else {
        KCharacterStyle *characterStyle = m_stylesModel->characterStyleForIndex(index);
        Q_ASSERT(characterStyle);
        CharacterGeneral *c = new CharacterGeneral;
        c->setStyle(characterStyle);
        if (m_canvasBase)
            c->setUnit(m_canvasBase->unit());
        connect(c, SIGNAL(styleAltered(const KCharacterStyle*)),
                m_styleManager, SLOT(alteredStyle(const KCharacterStyle*)));
        widget = c;
    }

    if (widget) {
        QPointer<KDialog> dialog = new KDialog(this);
        dialog->setCaption(paragraphStyle ? i18n("Edit Paragraph Style") : i18n("Edit Character Style"));
        dialog->setMainWidget(widget);
        connect(dialog, SIGNAL(okClicked()), widget, SLOT(save()));
        dialog->exec();
        delete dialog;
    }
}

void StylesWidget::applyStyle()
{
    QModelIndex index = widget.stylesView->currentIndex();
    Q_ASSERT(index.isValid());
    applyStyle(index);
}

void StylesWidget::applyStyle(const QModelIndex &index)
{
    KParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
    if (paragraphStyle) {
        emit paragraphStyleSelected(paragraphStyle);
        emit doneWithFocus();
        return;
    }

    KCharacterStyle *characterStyle = m_stylesModel->characterStyleForIndex(index);
    if (characterStyle) {
        emit characterStyleSelected(characterStyle);
        emit doneWithFocus();
        return;
    }
}

void StylesWidget::setCurrent(const QModelIndex &index)
{
    widget.modifyStyle->setEnabled(index.isValid());
    widget.applyStyle->setEnabled(index.isValid());

    bool canDelete = index.isValid();
    if (canDelete) {
        canDelete = !index.parent().isValid();
        KParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
        if (!canDelete) // there is one other way its deletable, if its a parag style
            canDelete = paragraphStyle;
        // but not if its the default paragraph style.
        if (canDelete && (paragraphStyle && paragraphStyle->styleId() == 100))
            canDelete = false;
    }
    widget.deleteStyle->setEnabled(canDelete);

    if (index.isValid() && m_isEmbedded) {
        KParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
        if (paragraphStyle) {
            emit paragraphStyleSelected(paragraphStyle, canDelete);
            return;
        }

        KCharacterStyle *characterStyle = m_stylesModel->characterStyleForIndex(index);
        if (characterStyle) {
            emit characterStyleSelected(characterStyle, canDelete);
            return;
        }
    }
}

void StylesWidget::setStylesAreNested(bool on)
{
    widget.stylesView->setRootIsDecorated(on);
}

#include <StylesWidget.moc>
