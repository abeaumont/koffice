/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeFactory.h"

KoShapeFactory::KoShapeFactory(QObject *parent, const QString &id, const QString &name)
: QObject(parent)
, m_id(id)
, m_name(name)
, m_iconName("")
, m_tooltip("")
{
}

const KoID KoShapeFactory::id() const {
    return KoID(m_id, m_name);
}

const QString & KoShapeFactory::toolTip() const {
    return m_tooltip;
}

const QString & KoShapeFactory::icon() const {
    return m_iconName;
}

const QString& KoShapeFactory::name() const {
    return m_name;
}

void KoShapeFactory::addTemplate(KoShapeTemplate params) {
    params.id = shapeId();
    m_templates.append(params);
}

void KoShapeFactory::setToolTip(const QString & tooltip) {
    m_tooltip = tooltip;
}

void KoShapeFactory::setIcon(const QString & iconName) {
    m_iconName = iconName;
}

const QString &KoShapeFactory::shapeId() const {
    return m_id;
}

void KoShapeFactory::setOptionPanels(QList<KoShapeConfigFactory*> &panelFactories) {
    m_configPanels = panelFactories;
}

const QList<KoShapeConfigFactory*> &KoShapeFactory::panelFactories() {
    return m_configPanels;
}

#include "KoShapeFactory.moc"
