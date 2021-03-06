/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#ifndef INSERTVARIABLEACTION_H
#define INSERTVARIABLEACTION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KOdfText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "InsertInlineObjectActionBase_p.h"

class KCanvasBase;
class KProperties;
class KInlineObjectFactoryBase;
struct KoInlineObjectTemplate;

class InsertVariableAction : public InsertInlineObjectActionBase
{
public:
    InsertVariableAction(KCanvasBase *base, KInlineObjectFactoryBase *factory, const KoInlineObjectTemplate &templ);

private:
    virtual KInlineObject *createInlineObject();

    KInlineObjectFactoryBase *const m_factory;
    const QString m_templateId;
    const KProperties *const m_properties;
    QString m_templateName;
};

#endif
