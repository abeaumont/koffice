/* This file is part of the KDE project
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "SCAnimationStep.h"
#include "SCAnimationSubStep.h"
#include "KoXmlWriter.h"

SCAnimationStep::SCAnimationStep()
{
}

SCAnimationStep::~SCAnimationStep()
{
}

void SCAnimationStep::init(SCAnimationCache *animationCache, int step)
{
    for(int i=0;i < this->animationCount(); i++) {
        QAbstractAnimation * animation = this->animationAt(i);
        if (SCAnimationSubStep * a = dynamic_cast<SCAnimationSubStep*>(animation)) {
            a->init(animationCache, step);
        }
    }
}

bool SCAnimationStep::saveOdf(KoPASavingContext & paContext) const
{
    KoXmlWriter &writer = paContext.xmlWriter();
    writer.startElement("anim:par");
    for (int i=0; i < this->animationCount(); i++) {
        bool startStep = !i;
        QAbstractAnimation *animation = this->animationAt(i);
        if (SCAnimationSubStep *a = dynamic_cast<SCAnimationSubStep*>(animation)) {
            a->saveOdf(paContext, startStep);
        }
    }
    writer.endElement();
    return true;
}

void SCAnimationStep::deactivate()
{
    for (int i=0; i < this->animationCount(); i++) {
        QAbstractAnimation *animation = this->animationAt(i);
        if (SCAnimationSubStep *a = dynamic_cast<SCAnimationSubStep*>(animation)) {
            a->deactivate();
        }
    }
}