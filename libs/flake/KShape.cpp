/* This file is part of the KDE project
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2010 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007-2009 Jan Hambrecht <jaham@gmx.net>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KShape.h"
#include "KShape_p.h"
#include "KoShapeContainer.h"
#include "KoShapeLayer.h"
#include "KoShapeContainerModel.h"
#include "KSelection.h"
#include "KPointerEvent.h"
#include "KoInsets.h"
#include "KShapeBorderBase.h"
#include "KShapeBackground.h"
#include "KColorBackground.h"
#include "KGradientBackground.h"
#include "KPatternBackground.h"
#include "KoShapeManager.h"
#include "KoShapeManager_p.h"
#include "KoShapeUserData.h"
#include "KShapeApplicationData.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoViewConverter.h"
#include "KLineBorder.h"
#include "ShapeDeleter_p.h"
#include "KoShapeShadow.h"
#include "KEventAction.h"
#include "KEventActionRegistry.h"
#include "KoOdfWorkaround.h"
#include "KFilterEffectStack.h"

#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include <KUnit.h>
#include <KOdfStylesReader.h>
#include <KOdf.h>
#include <KOdfLoadingContext.h>

#include <QPainter>
#include <QVariant>
#include <QPainterPath>
#include <QList>
#include <QMap>
#include <QByteArray>

#include <kdebug.h>

#include <limits>

KShapePrivate::KShapePrivate(KShape *shape)
    : size(50, 50),
    parent(0),
    userData(0),
    appData(0),
    fill(0),
    border(0),
    q_ptr(shape),
    shadow(0),
    filterEffectStack(0),
    transparency(0.0),
    zIndex(0),
    visible(true),
    printable(true),
    geometryProtected(false),
    keepAspect(false),
    selectable(true),
    detectCollision(false),
    protectContent(false),
    editBlockDepth(0),
    editBlockEndShouldEmit(false)
{
    connectors.append(QPointF(0.5, 0.0));
    connectorPolicies.append(KoShapeConnectionPolicy(KoFlake::EscapeUp, Qt::AlignTop));
    connectors.append(QPointF(1.0, 0.5));
    connectorPolicies.append(KoShapeConnectionPolicy(KoFlake::EscapeRight, Qt::AlignRight));
    connectors.append(QPointF(0.5, 1.0));
    connectorPolicies.append(KoShapeConnectionPolicy(KoFlake::EscapeDown, Qt::AlignBottom));
    connectors.append(QPointF(0.0, 0.5));
    connectorPolicies.append(KoShapeConnectionPolicy(KoFlake::EscapeLeft, Qt::AlignLeft));
}

KShapePrivate::~KShapePrivate()
{
    Q_Q(KShape);
    if (parent)
        parent->removeShape(q);
    foreach(KoShapeManager *manager, shapeManagers) {
        manager->remove(q);
        manager->removeAdditional(q);
    }
    delete userData;
    delete appData;
    if (border && !border->deref())
        delete border;
    if (shadow && !shadow->deref())
        delete shadow;
    if (fill && !fill->deref())
        delete fill;
    if (filterEffectStack && !filterEffectStack->deref())
        delete filterEffectStack;
    qDeleteAll(eventActions);
}

void KShapePrivate::shapeChanged(KShape::ChangeType type)
{
    Q_Q(KShape);
    if (editBlockDepth > 0 && ( type == KShape::PositionChanged
                || type == KShape::RotationChanged || type == KShape::ScaleChanged
                || type == KShape::ShearChanged || type == KShape::SizeChanged
                || type == KShape::GenericMatrixChange)) {
        editBlockEndShouldEmit = true;
        return;
    }

    if (parent)
        parent->model()->childChanged(q, type);
    q->shapeChanged(type, q);
    foreach (KShape * shape, dependees)
        shape->shapeChanged(type, q);
}

void KShapePrivate::updateBorder()
{
    Q_Q(KShape);
    if (border == 0)
        return;
    KoInsets insets;
    border->borderInsets(insets);
    QSizeF inner = q->size();
    // update left
    q->update(QRectF(-insets.left, -insets.top, insets.left,
                inner.height() + insets.top + insets.bottom));
    // update top
    q->update(QRectF(-insets.left, -insets.top,
                inner.width() + insets.left + insets.right, insets.top));
    // update right
    q->update(QRectF(inner.width(), -insets.top, insets.right,
                inner.height() + insets.top + insets.bottom));
    // update bottom
    q->update(QRectF(-insets.left, inner.height(),
                inner.width() + insets.left + insets.right, insets.bottom));
}

void KShapePrivate::addShapeManager(KoShapeManager *manager)
{
    shapeManagers.insert(manager);
}

void KShapePrivate::removeShapeManager(KoShapeManager *manager)
{
    shapeManagers.remove(manager);
}
// static
QString KShapePrivate::getStyleProperty(const char *property, KoShapeLoadingContext &context)
{
    KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
    QString value;

    if (styleStack.hasProperty(KOdfXmlNS::draw, property)) {
        value = styleStack.property(KOdfXmlNS::draw, property);
    }

    return value;
}

void KShapePrivate::addConnection(KoShapeConnection *connection)
{
    connections.append(connection);
    foreach (KoShapeManager *sm, shapeManagers)
        sm->priv()->addShapeConnection(connection);
}

void KShapePrivate::removeConnection(KoShapeConnection *connection)
{
    connections.removeAll(connection);
    // TODO remove from shapeManager ?
}

void KShapePrivate::loadOdfGluePoints(const KXmlElement &gluePoints)
{
    // defaults expect 4 to be there. The subclass should provide those, but if it
    // didn't, make sure we don't have issues with our numbering.
    connectors.resize(4);
    connectorPolicies.resize(4);

    KXmlElement element;
    forEachElement (element, gluePoints) {
        if (element.namespaceURI() != KOdfXmlNS::draw || element.localName() != "glue-point")
            continue;

        const int index = element.attributeNS(KOdfXmlNS::draw, "id").toInt();
        if (index < 4) // defaults, also skip malformed 'id's
            continue;

        KoShapeConnectionPolicy policy;
        const QString escape = element.attributeNS(KOdfXmlNS::draw, "escape-direction");
        if (!escape.isEmpty()) {
            if (escape == "horizontal") {
                policy.setEscapeDirection(KoFlake::EscapeHorizontal);
            } else if (escape == "vertical") {
                policy.setEscapeDirection(KoFlake::EscapeVertical);
            } else if (escape == "left") {
                policy.setEscapeDirection(KoFlake::EscapeLeft);
            } else if (escape == "right") {
                policy.setEscapeDirection(KoFlake::EscapeRight);
            } else if (escape == "up") {
                policy.setEscapeDirection(KoFlake::EscapeUp);
            } else if (escape == "down") {
                policy.setEscapeDirection(KoFlake::EscapeDown);
            }
        }
        const QString align = element.attributeNS(KOdfXmlNS::draw, "align");
        if (!align.isEmpty()) {
            // absolute distances to the edge specified by align
            if (align == "top-left") {
                policy.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            } else if (align == "center") {
                policy.setAlignment(Qt::AlignCenter);
            } else if (align == "top") {
                policy.setAlignment(Qt::AlignTop);
            } else if (align == "top-right") {
                policy.setAlignment(Qt::AlignRight | Qt::AlignTop);
            } else if (align == "left") {
                policy.setAlignment(Qt::AlignLeft);
            } else if (align == "right") {
                policy.setAlignment(Qt::AlignRight);
            } else if (align == "bottom-left") {
                policy.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
            } else if (align == "bottom") {
                policy.setAlignment(Qt::AlignBottom);
            } else if (align == "bottom-right") {
                policy.setAlignment(Qt::AlignRight | Qt::AlignBottom);
            }
        }
        connectorPolicies[index] = policy;

        qreal x = KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x"));
        qreal y = KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y"));

        connectors[index] = QPointF(x, y);
    }
}


// ======== KShape
KShape::KShape()
        : d_ptr(new KShapePrivate(this))
{
    notifyChanged();
}

KShape::KShape(const QColor &color)
        : d_ptr(new KShapePrivate(this))
{
    d_ptr->fill = new KColorBackground(color);
    notifyChanged();
}


KShape::KShape(KShapePrivate &dd)
    : d_ptr(&dd)
{
}

KShape::~KShape()
{
    Q_D(KShape);
    d->shapeChanged(Deleted);
    delete d_ptr;
}

void KShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KShape);
    if (d->fill) {
        applyConversion(painter, converter);
        d->fill->paint(painter, outline());
    }
}

void KShape::paintDecorations(QPainter &, const KoViewConverter &, const KCanvasBase *)
{
}

void KShape::scale(qreal sx, qreal sy)
{
    Q_D(KShape);
    QPointF pos = position();
    QTransform scaleMatrix;
    scaleMatrix.translate(pos.x(), pos.y());
    scaleMatrix.scale(sx, sy);
    scaleMatrix.translate(-pos.x(), -pos.y());
    d->localMatrix = d->localMatrix * scaleMatrix;

    notifyChanged();
    d->shapeChanged(ScaleChanged);
}

void KShape::rotate(qreal angle)
{
    Q_D(KShape);
    QPointF center = d->localMatrix.map(QPointF(0.5 * size().width(), 0.5 * size().height()));
    QTransform rotateMatrix;
    rotateMatrix.translate(center.x(), center.y());
    rotateMatrix.rotate(angle);
    rotateMatrix.translate(-center.x(), -center.y());
    d->localMatrix = d->localMatrix * rotateMatrix;

    notifyChanged();
    d->shapeChanged(RotationChanged);
}

void KShape::shear(qreal sx, qreal sy)
{
    Q_D(KShape);
    QPointF pos = position();
    QTransform shearMatrix;
    shearMatrix.translate(pos.x(), pos.y());
    shearMatrix.shear(sx, sy);
    shearMatrix.translate(-pos.x(), -pos.y());
    d->localMatrix = d->localMatrix * shearMatrix;

    notifyChanged();
    d->shapeChanged(ShearChanged);
}

void KShape::setSize(const QSizeF &newSize)
{
    Q_D(KShape);
    QSizeF oldSize(size());
    if (oldSize == newSize)
        return;

    d->size = newSize;

    notifyChanged();
    d->shapeChanged(SizeChanged);
}

void KShape::setPosition(const QPointF &newPosition)
{
    Q_D(KShape);
    QPointF currentPos = position();
    if (newPosition == currentPos)
        return;
    QTransform translateMatrix;
    translateMatrix.translate(newPosition.x() - currentPos.x(), newPosition.y() - currentPos.y());
    d->localMatrix = d->localMatrix * translateMatrix;

    notifyChanged();
    d->shapeChanged(PositionChanged);
}

bool KShape::hitTest(const QPointF &position) const
{
    Q_D(const KShape);
    if (d->parent && d->parent->isClipped(this) && !d->parent->hitTest(position))
        return false;

    QPointF point = absoluteTransformation(0).inverted().map(position);
    QRectF bb(QPointF(), size());
    if (d->border) {
        KoInsets insets;
        d->border->borderInsets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (bb.contains(point))
        return true;

    // if there is no shadow we can as well just leave
    if (! d->shadow)
        return false;

    // the shadow has an offset to the shape, so we simply
    // check if the position minus the shadow offset hits the shape
    point = absoluteTransformation(0).inverted().map(position - d->shadow->offset());

    return bb.contains(point);
}

QRectF KShape::boundingRect() const
{
    Q_D(const KShape);
    QSizeF mySize = size();
    QTransform transform = absoluteTransformation(0);
    QRectF bb(QPointF(0, 0), mySize);
    if (d->border) {
        KoInsets insets;
        d->border->borderInsets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    bb = transform.mapRect(bb);
    if (d->shadow) {
        KoInsets insets;
        d->shadow->insets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (d->filterEffectStack) {
        QRectF clipRect = d->filterEffectStack->clipRectForBoundingRect(QRectF(QPointF(), mySize));
        bb |= transform.mapRect(clipRect);
    }

    return bb;
}

QTransform KShape::absoluteTransformation(const KoViewConverter *converter) const
{
    Q_D(const KShape);
    QTransform matrix;
    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer * container = d->parent;
    if (container) {
        if (container->inheritsTransform(this)) {
            // We do need to pass the converter here, otherwise the parent's
            // translation is not inherited.
            matrix = container->absoluteTransformation(converter);
        } else {
            QSizeF containerSize = container->size();
            QPointF containerPos = container->absolutePosition() - QPointF(0.5 * containerSize.width(), 0.5 * containerSize.height());
            if (converter)
                containerPos = converter->documentToView(containerPos);
            matrix.translate(containerPos.x(), containerPos.y());
        }
    }

    if (converter) {
        QPointF pos = d->localMatrix.map(QPointF());
        QPointF trans = converter->documentToView(pos) - pos;
        matrix.translate(trans.x(), trans.y());
    }

    return d->localMatrix * matrix;
}

void KShape::applyAbsoluteTransformation(const QTransform &matrix)
{
    QTransform globalMatrix = absoluteTransformation(0);
    // the transformation is relative to the global coordinate system
    // but we want to change the local matrix, so convert the matrix
    // to be relative to the local coordinate system
    QTransform transformMatrix = globalMatrix * matrix * globalMatrix.inverted();
    applyTransformation(transformMatrix);
}

void KShape::applyTransformation(const QTransform &matrix)
{
    Q_D(KShape);
    d->localMatrix = matrix * d->localMatrix;
    notifyChanged();
    d->shapeChanged(GenericMatrixChange);
}

void KShape::setTransformation(const QTransform &matrix)
{
    Q_D(KShape);
    d->localMatrix = matrix;
    notifyChanged();
    d->shapeChanged(GenericMatrixChange);
}

QTransform KShape::transformation() const
{
    Q_D(const KShape);
    return d->localMatrix;
}

bool KShape::compareShapeZIndex(KShape *s1, KShape *s2)
{
    bool foundCommonParent = false;
    KShape *parentShapeS1 = s1;
    KShape *parentShapeS2 = s2;
    int index1 = parentShapeS1->zIndex();
    int index2 = parentShapeS2->zIndex();
    while (parentShapeS1 && !foundCommonParent) {
        parentShapeS2 = s2;
        index2 = parentShapeS2->zIndex();
        while (parentShapeS2) {
            if (parentShapeS2 == parentShapeS1) {
                foundCommonParent = true;
                break;
            }
            index2 = parentShapeS2->zIndex();
            parentShapeS2 = parentShapeS2->parent();
        }

        if (!foundCommonParent) {
            index1 = parentShapeS1->zIndex();
            parentShapeS1 = parentShapeS1->parent();
        }
    }
    if (s1 == parentShapeS2) {
        return true;
    }
    else if (s2 == parentShapeS1) {
        return false;
    }
    return index1 < index2;
}

void KShape::setParent(KoShapeContainer *parent)
{
    Q_D(KShape);
    if (d->parent == parent)
        return;
    KoShapeContainer *oldParent = d->parent;
    d->parent = 0; // avoids recursive removing
    if (oldParent)
        oldParent->removeShape(this);
    if (parent && parent != this) {
        d->parent = parent;
        parent->addShape(this);
    }
    notifyChanged();
    d->shapeChanged(ParentChanged);
}

int KShape::zIndex() const
{
    Q_D(const KShape);
    return d->zIndex;
}

void KShape::update() const
{
    Q_D(const KShape);
    if (!d->shapeManagers.empty()) {
        QRectF rect(boundingRect());
        foreach(KoShapeManager *manager, d->shapeManagers)
            manager->priv()->update(rect, this, true);

        // also ask update for children which inherit transform
        const KoShapeContainer *me = dynamic_cast<const KoShapeContainer*>(this);
        if (me && me->model()) {
            KoShapeContainerModel *model = me->model();
            foreach (KShape *shape, model->shapes()) {
                if (model->inheritsTransform(shape))
                    shape->update();
            }
        }
    }
}

void KShape::update(const QRectF &section) const
{
    Q_D(const KShape);
    if (!d->shapeManagers.empty() && isVisible()) {
        QRectF rect(absoluteTransformation(0).mapRect(section));
        foreach(KoShapeManager *manager, d->shapeManagers) {
            manager->priv()->update(rect);
        }
    }
}

QPainterPath KShape::outline() const
{
    Q_D(const KShape);
    QPainterPath path;
    path.addRect(QRectF(QPointF(0, 0), QSizeF(qMax(d->size.width(), qreal(0.0001)), qMax(d->size.height(), qreal(0.0001)))));
    return path;
}

QPointF KShape::absolutePosition(KoFlake::Position anchor) const
{
    QPointF point;
    switch (anchor) {
    case KoFlake::TopLeftCorner: break;
    case KoFlake::TopRightCorner: point = QPointF(size().width(), 0.0); break;
    case KoFlake::BottomLeftCorner: point = QPointF(0.0, size().height()); break;
    case KoFlake::BottomRightCorner: point = QPointF(size().width(), size().height()); break;
    case KoFlake::CenteredPosition: point = QPointF(size().width() / 2.0, size().height() / 2.0); break;
    }
    return absoluteTransformation(0).map(point);
}

void KShape::setAbsolutePosition(QPointF newPosition, KoFlake::Position anchor)
{
    Q_D(KShape);
    QPointF currentAbsPosition = absolutePosition(anchor);
    QPointF translate = newPosition - currentAbsPosition;
    QTransform translateMatrix;
    translateMatrix.translate(translate.x(), translate.y());
    applyAbsoluteTransformation(translateMatrix);
    notifyChanged();
    d->shapeChanged(PositionChanged);
}

void KShape::copySettings(const KShape *shape)
{
    Q_D(KShape);
    d->size = shape->size();
    d->connectors.clear();
    foreach(const QPointF &point, shape->connectionPoints())
        addConnectionPoint(point);
    d->zIndex = shape->zIndex();
    d->visible = shape->isVisible();

    // Ensure printable is true by default
    if (!d->visible)
        d->printable = true;
    else
        d->printable = shape->isPrintable();

    d->geometryProtected = shape->isGeometryProtected();
    d->protectContent = shape->isContentProtected();
    d->selectable = shape->isSelectable();
    d->keepAspect = shape->keepAspectRatio();
    d->localMatrix = shape->d_ptr->localMatrix;
}

void KShape::notifyChanged()
{
    Q_D(KShape);
    foreach(KoShapeManager * manager, d->shapeManagers) {
        manager->notifyShapeChanged(this);
    }
}

void KShape::setUserData(KoShapeUserData *userData)
{
    Q_D(KShape);
    delete d->userData;
    d->userData = userData;
}

KoShapeUserData *KShape::userData() const
{
    Q_D(const KShape);
    return d->userData;
}

void KShape::setApplicationData(KShapeApplicationData *appData)
{
    Q_D(KShape);
    // appdata is deleted by the application.
    d->appData = appData;
}

KShapeApplicationData *KShape::applicationData() const
{
    Q_D(const KShape);
    return d->appData;
}

bool KShape::hasTransparency() const
{
    Q_D(const KShape);
    if (! d->fill)
        return true;
    else
        return d->fill->hasTransparency() || d->transparency > 0.0;
}

void KShape::setTransparency(qreal transparency)
{
    Q_D(KShape);
    d->transparency = qBound<qreal>(0.0, transparency, 1.0);
}

qreal KShape::transparency(bool recursive) const
{
    Q_D(const KShape);
    if (!recursive || !parent()) {
        return d->transparency;
    } else {
        const qreal parentOpacity = 1.0-parent()->transparency(recursive);
        const qreal childOpacity = 1.0-d->transparency;
        return 1.0-(parentOpacity*childOpacity);
    }
}

void KShape::fetchInsets(KoInsets &insets) const
{
    Q_D(const KShape);
    if (d->border)
        d->border->borderInsets(insets);
    else
        insets.clear();
    // notice that the shadow has 'insets' that go outwards from the shape edge, so
    // they are not relevant for this method.
}

qreal KShape::rotation() const
{
    Q_D(const KShape);
    // try to extract the rotation angle out of the local matrix
    // if it is a pure rotation matrix

    // check if the matrix has shearing mixed in
    if (fabs(fabs(d->localMatrix.m12()) - fabs(d->localMatrix.m21())) > 1e-10)
        return std::numeric_limits<qreal>::quiet_NaN();
    // check if the matrix has scaling mixed in
    if (fabs(d->localMatrix.m11() - d->localMatrix.m22()) > 1e-10)
        return std::numeric_limits<qreal>::quiet_NaN();

    // calculate the angle from the matrix elements
    qreal angle = atan2(-d->localMatrix.m21(), d->localMatrix.m11()) * 180.0 / M_PI;
    if (angle < 0.0)
        angle += 360.0;

    return angle;
}

QSizeF KShape::size() const
{
    Q_D(const KShape);
    return d->size;
}

QPointF KShape::position() const
{
    Q_D(const KShape);
    QSizeF s(size());
    QPointF center(0.5 * qMax(1E-3, s.width()), 0.5 * qMax(1E-3, s.height()));
    return d->localMatrix.map(center) - center;
}

void KShape::addConnectionPoint(const QPointF &point)
{
    Q_D(KShape);
    QSizeF s = size();
    // convert glue point from shape coordinates to factors of size
    d->connectors.append(QPointF(point.x() / s.width(), point.y() / s.height()));
}

QList<QPointF> KShape::connectionPoints() const
{
    Q_D(const KShape);
    QList<QPointF> points;
    QSizeF s = size();
    // convert glue points to shape coordinates
    foreach(const QPointF &cp, d->connectors)
        points.append(QPointF(s.width() * cp.x(), s.height() * cp.y()));

    return points;
}

void KShape::setConnectionPolicy(int connectionIndex, const KoShapeConnectionPolicy &policy)
{
    Q_D(KShape);
    d->connectorPolicies.reserve(connectionIndex);
    d->connectorPolicies.insert(connectionIndex, policy);
}

KoShapeConnectionPolicy KShape::connectionPolicy(int connectionIndex) const
{
    Q_D(const KShape);
    if (d->connectorPolicies.count() - 1 < connectionIndex)
        return KoShapeConnectionPolicy();
    return d->connectorPolicies.value(connectionIndex);
}

void KShape::addEventAction(KEventAction *action)
{
    Q_D(KShape);
    d->eventActions.insert(action);
}

void KShape::removeEventAction(KEventAction *action)
{
    Q_D(KShape);
    d->eventActions.remove(action);
}

QSet<KEventAction *> KShape::eventActions() const
{
    Q_D(const KShape);
    return d->eventActions;
}

void KShape::setBackground(KShapeBackground *fill)
{
    Q_D(KShape);
    if (d->fill)
        d->fill->deref();
    d->fill = fill;
    if (d->fill)
        d->fill->ref();
    d->shapeChanged(BackgroundChanged);
}

KShapeBackground * KShape::background() const
{
    Q_D(const KShape);
    return d->fill;
}

void KShape::setZIndex(int zIndex)
{
    Q_D(KShape);
    notifyChanged();
    d->zIndex = zIndex;
}

void KShape::setVisible(bool on)
{
    Q_D(KShape);
    d->visible = on;
}

bool KShape::isVisible(bool recursive) const
{
    Q_D(const KShape);
    if (! recursive)
        return d->visible;
    if (recursive && ! d->visible)
        return false;

    KoShapeContainer * parentShape = parent();
    while (parentShape) {
        if (! parentShape->isVisible())
            return false;
        parentShape = parentShape->parent();
    }
    return true;
}

void KShape::setPrintable(bool on)
{
    Q_D(KShape);
    d->printable = on;
}

bool KShape::isPrintable() const
{
    Q_D(const KShape);
    if (d->visible)
        return d->printable;
    else
        return false;
}

void KShape::setSelectable(bool selectable)
{
    Q_D(KShape);
    d->selectable = selectable;
}

bool KShape::isSelectable() const
{
    Q_D(const KShape);
    return d->selectable;
}

void KShape::setGeometryProtected(bool on)
{
    Q_D(KShape);
    d->geometryProtected = on;
}

bool KShape::isGeometryProtected() const
{
    Q_D(const KShape);
    return d->geometryProtected;
}

void KShape::setContentProtected(bool protect)
{
    Q_D(KShape);
    d->protectContent = protect;
}

bool KShape::isContentProtected() const
{
    Q_D(const KShape);
    return d->protectContent;
}

KoShapeContainer *KShape::parent() const
{
    Q_D(const KShape);
    return d->parent;
}

void KShape::setKeepAspectRatio(bool keepAspect)
{
    Q_D(KShape);
    d->keepAspect = keepAspect;
}

bool KShape::keepAspectRatio() const
{
    Q_D(const KShape);
    return d->keepAspect;
}

QString KShape::shapeId() const
{
    Q_D(const KShape);
    return d->shapeId;
}

void KShape::setShapeId(const QString &id)
{
    Q_D(KShape);
    d->shapeId = id;
}

void KShape::setCollisionDetection(bool detect)
{
    Q_D(KShape);
    d->detectCollision = detect;
}

bool KShape::collisionDetection()
{
    Q_D(KShape);
    return d->detectCollision;
}

KShapeBorderBase *KShape::border() const
{
    Q_D(const KShape);
    return d->border;
}

void KShape::setBorder(KShapeBorderBase *border)
{
    Q_D(KShape);
    if (border)
        border->ref();
    d->updateBorder();
    if (d->border)
        d->border->deref();
    d->border = border;
    d->updateBorder();
    d->shapeChanged(BorderChanged);
    notifyChanged();
}

void KShape::setShadow(KoShapeShadow *shadow)
{
    Q_D(KShape);
    if (d->shadow)
        d->shadow->deref();
    d->shadow = shadow;
    if (d->shadow) {
        d->shadow->ref();
        // TODO update changed area
    }
    d->shapeChanged(ShadowChanged);
    notifyChanged();
}

KoShapeShadow *KShape::shadow() const
{
    Q_D(const KShape);
    return d->shadow;
}

QTransform KShape::transform() const
{
    Q_D(const KShape);
    return d->localMatrix;
}

void KShape::removeConnectionPoint(int index)
{
    Q_D(KShape);
    if (index < d->connectors.count())
        d->connectors.remove(index);
}

QString KShape::name() const
{
    Q_D(const KShape);
    return d->name;
}

void KShape::setName(const QString &name)
{
    Q_D(KShape);
    d->name = name;
}

void KShape::waitUntilReady(const KoViewConverter &converter, bool asynchronous) const
{
    Q_UNUSED(converter);
    Q_UNUSED(asynchronous);
}

void KShape::deleteLater()
{
    Q_D(KShape);
    foreach(KoShapeManager *manager, d->shapeManagers)
        manager->remove(this);
    d->shapeManagers.clear();
    new ShapeDeleter(this);
}

bool KShape::isEditable() const
{
    Q_D(const KShape);
    if (!d->visible || d->geometryProtected)
        return false;

    if (d->parent && d->parent->isChildLocked(this))
        return false;

    return true;
}

// loading & saving methods
bool KShape::loadOdf(const KXmlElement &, KoShapeLoadingContext &)
{
    kWarning(30006) << "Warning; empty KShape::loadOdf called, reimplement for load to work";
    return true;
}

void KShape::saveOdf(KoShapeSavingContext &context) const
{
    Q_D(const KShape);
    // if nothing else; write out my connectors.
    context.addForWriting(d->connections);
}

QString KShape::saveStyle(KOdfGenericStyle &style, KoShapeSavingContext &context) const
{
    Q_D(const KShape);
    // and fill the style
    KShapeBorderBase *b = border();
    if (b) {
        b->saveOdf(style, context);
    }
    else {
        style.addProperty("draw:stroke", "none");
    }
    KoShapeShadow *s = shadow();
    if (s)
        s->fillStyle(style, context);

    KShapeBackground *bg = background();
    if (bg) {
        bg->fillStyle(style, context);
    }
    else {
        style.addProperty("draw:fill", "none");
    }

    if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml)) {
        style.setAutoStyleInStylesDotXml(true);
    }

    QString value;
    if (isGeometryProtected()) {
        value = "position size";
    }
    if (isContentProtected()) {
        if (! value.isEmpty())
            value += ' ';
        value += "content";
    }
    if (!value.isEmpty()) {
        style.addProperty("style:protect", value);
    }

    QMap<QString, QString>::const_iterator it(d->additionalStyleAttributes.constBegin());
    for (; it != d->additionalStyleAttributes.constEnd(); ++it) {
        style.addProperty(it.key(), it.value());
    }

    if (parent() && parent()->isClipped(this)) {
        /*
         * In KOffice clipping is done using a parent shape which can be rotated, sheared etc
         * and even non-square.  So the ODF interoperability version we write here is really
         * just a very simple version of that...
         */
        qreal top = -position().y();
        qreal left = -position().x();
        qreal right = parent()->size().width() - size().width() - left;
        qreal bottom = parent()->size().height() - size().height() - top;

        style.addProperty("fo:clip", QString("rect(%1pt, %2pt, %3pt, %4pt)")
                .arg(top, 10, 'f').arg(right, 10, 'f')
                .arg(bottom, 10, 'f').arg(left, 10, 'f'));

    }

    return context.mainStyles().insert(style, context.isSet(KoShapeSavingContext::PresentationShape) ? "pr" : "gr");
}

void KShape::loadStyle(const KXmlElement &element, KoShapeLoadingContext &context)
{
    Q_D(KShape);

    KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");

    if (d->fill && !d->fill->deref()) {
        delete d->fill;
    }
    d->fill = 0;
    if (d->border && !d->border->deref()) {
        delete d->border;
    }
    d->border = 0;
    if (d->shadow && !d->shadow->deref()) {
        delete d->shadow;
    }
    d->shadow = 0;
    setBackground(loadOdfFill(context));
    setBorder(loadOdfStroke(element, context));
    setShadow(d->loadOdfShadow(context));

    QString protect(styleStack.property(KOdfXmlNS::style, "protect"));
    setGeometryProtected(protect.contains("position") || protect.contains("size"));
    setContentProtected(protect.contains("content"));
}

bool KShape::loadOdfAttributes(const KXmlElement &element, KoShapeLoadingContext &context, int attributes)
{
    Q_D(KShape);
    if (attributes & OdfPosition) {
        QPointF pos(position());
        if (element.hasAttributeNS(KOdfXmlNS::svg, "x"))
            pos.setX(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x", QString())));
        if (element.hasAttributeNS(KOdfXmlNS::svg, "y"))
            pos.setY(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y", QString())));
        setPosition(pos);
    }

    if (attributes & OdfSize) {
        QSizeF s(size());
        if (element.hasAttributeNS(KOdfXmlNS::svg, "width"))
            s.setWidth(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "width")));
        if (element.hasAttributeNS(KOdfXmlNS::svg, "height"))
            s.setHeight(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "height")));
        setSize(s);
    }

    if (attributes & OdfLayer) {
        if (element.hasAttributeNS(KOdfXmlNS::draw, "layer")) {
            KoShapeLayer *layer = context.layer(element.attributeNS(KOdfXmlNS::draw, "layer"));
            if (layer) {
                setParent(layer);
            }
        }
    }

    if (attributes & OdfId) {
        if (element.hasAttributeNS(KOdfXmlNS::draw, "id")) {
            QString id = element.attributeNS(KOdfXmlNS::draw, "id");
            if (!id.isNull()) {
                context.addShapeId(this, id);
            }
        }
    }

    if (attributes & OdfZIndex) {
        if (element.hasAttributeNS(KOdfXmlNS::draw, "z-index")) {
            setZIndex(element.attributeNS(KOdfXmlNS::draw, "z-index").toInt());
        } else {
            setZIndex(context.zIndex());
        }
    }

    if (attributes & OdfName) {
        if (element.hasAttributeNS(KOdfXmlNS::draw, "name")) {
            setName(element.attributeNS(KOdfXmlNS::draw, "name"));
        }
    }

    if (attributes & OdfStyle) {
        KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.save();
        if (element.hasAttributeNS(KOdfXmlNS::draw, "style-name")) {
            context.odfLoadingContext().fillStyleStack(element, KOdfXmlNS::draw, "style-name", "graphic");
        }
        else if (element.hasAttributeNS(KOdfXmlNS::presentation, "style-name")) {
            context.odfLoadingContext().fillStyleStack(element, KOdfXmlNS::presentation, "style-name", "presentation");
        }
        loadStyle(element, context);
        styleStack.restore();
    }

    if (attributes & OdfTransformation) {
        QString transform = element.attributeNS(KOdfXmlNS::draw, "transform");
        if (! transform.isEmpty())
            applyAbsoluteTransformation(parseOdfTransform(transform));
    }

    if (attributes & OdfAdditionalAttributes) {
        QSet<KoShapeLoadingContext::AdditionalAttributeData> additionalAttributeData = KoShapeLoadingContext::additionalAttributeData();
        foreach(const KoShapeLoadingContext::AdditionalAttributeData &attributeData, additionalAttributeData) {
            if (element.hasAttributeNS(attributeData.ns, attributeData.tag)) {
                QString value = element.attributeNS(attributeData.ns, attributeData.tag);
                //kDebug(30006) << "load additional attribute" << attributeData.tag << value;
                setAdditionalAttribute(attributeData.name, value);
            }
        }
    }

    if (attributes & OdfCommonChildElements) {
        const KXmlElement eventActionsElement(KoXml::namedItemNS(element, KOdfXmlNS::office, "event-listeners"));
        if (!eventActionsElement.isNull()) {
            d->eventActions = KEventActionRegistry::instance()->createEventActionsFromOdf(eventActionsElement, context);
        }
        // load glue points (connection points)
        d->loadOdfGluePoints(element);
    }

    return true;
}

KShapeBackground *KShape::loadOdfFill(KoShapeLoadingContext &context) const
{
    QString fill = KShapePrivate::getStyleProperty("fill", context);
    KShapeBackground *bg = 0;
    if (fill == "solid" || fill == "hatch") {
        bg = new KColorBackground();
    } else if (fill == "gradient") {
        QGradient *gradient = new QLinearGradient();
        gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
        bg = new KGradientBackground(gradient);
    } else if (fill == "bitmap") {
        bg = new KPatternBackground(context.imageCollection());
#ifndef NWORKAROUND_ODF_BUGS
    } else if (fill.isEmpty()) {
         bg = KoOdfWorkaround::fixBackgroundColor(this, context);
         return bg;
#endif
    } else {
        return 0;
    }

    if (!bg->loadStyle(context.odfLoadingContext(), size())) {
        delete bg;
        return 0;
    }

    return bg;
}

KShapeBorderBase *KShape::loadOdfStroke(const KXmlElement &element, KoShapeLoadingContext &context) const
{
    KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
    KOdfStylesReader &stylesReader = context.odfLoadingContext().stylesReader();

    QString stroke = KShapePrivate::getStyleProperty("stroke", context);
    if (stroke == "solid" || stroke == "dash") {
        QPen pen = KOdf::loadOdfStrokeStyle(styleStack, stroke, stylesReader);

        KLineBorder *border = new KLineBorder();

        if (styleStack.hasProperty(KOdfXmlNS::koffice, "stroke-gradient")) {
            QString gradientName = styleStack.property(KOdfXmlNS::koffice, "stroke-gradient");
            QBrush brush = KOdf::loadOdfGradientStyleByName(stylesReader, gradientName, size());
            border->setLineBrush(brush);
        } else {
            border->setColor(pen.color());
        }

#ifndef NWORKAROUND_ODF_BUGS
        KoOdfWorkaround::fixPenWidth(pen, context);
#endif
        border->setLineWidth(pen.widthF());
        border->setJoinStyle(pen.joinStyle());
        border->setLineStyle(pen.style(), pen.dashPattern());

        return border;
#ifndef NWORKAROUND_ODF_BUGS
    } else if (stroke.isEmpty()) {
        QPen pen;
        if (KoOdfWorkaround::fixMissingStroke(pen, element, context, this)) {
            KLineBorder *border = new KLineBorder();

            // FIXME: (make it possible to) use a cosmetic pen
            if (pen.widthF() == 0.0)
                border->setLineWidth(0.5);
            else
                border->setLineWidth(pen.widthF());
            border->setJoinStyle(pen.joinStyle());
            border->setLineStyle(pen.style(), pen.dashPattern());
            border->setLineBrush(pen.brush());

            return border;
        }
#endif
    }

    return 0;
}

KoShapeShadow *KShapePrivate::loadOdfShadow(KoShapeLoadingContext &context) const
{
    KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
    QString shadowStyle = KShapePrivate::getStyleProperty("shadow", context);
    if (shadowStyle == "visible" || shadowStyle == "hidden") {
        KoShapeShadow *shadow = new KoShapeShadow();
        QColor shadowColor(styleStack.property(KOdfXmlNS::draw, "shadow-color"));
        qreal offsetX = KUnit::parseValue(styleStack.property(KOdfXmlNS::draw, "shadow-offset-x"));
        qreal offsetY = KUnit::parseValue(styleStack.property(KOdfXmlNS::draw, "shadow-offset-y"));
        shadow->setOffset(QPointF(offsetX, offsetY));

        QString opacity = styleStack.property(KOdfXmlNS::draw, "shadow-opacity");
        if (! opacity.isEmpty() && opacity.right(1) == "%")
            shadowColor.setAlphaF(opacity.left(opacity.length() - 1).toFloat() / 100.0);
        shadow->setColor(shadowColor);
        shadow->setVisible(shadowStyle == "visible");

        return shadow;
    }
    return 0;
}

QTransform KShape::parseOdfTransform(const QString &transform)
{
    QTransform matrix;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transform.split(')', QString::SkipEmptyParts);
    QStringList::ConstIterator it = subtransforms.constBegin();
    QStringList::ConstIterator end = subtransforms.constEnd();
    for (; it != end; ++it) {
        QStringList subtransform = (*it).split('(', QString::SkipEmptyParts);

        subtransform[0] = subtransform[0].trimmed().toLower();
        subtransform[1] = subtransform[1].simplified();
        QRegExp reg("[,( ]");
        QStringList params = subtransform[1].split(reg, QString::SkipEmptyParts);

        if (subtransform[0].startsWith(';') || subtransform[0].startsWith(','))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

        QString cmd = subtransform[0].toLower();

        if (cmd == "rotate") {
            QTransform rotMatrix;
            if (params.count() == 3) {
                qreal x = KUnit::parseValue(params[1]);
                qreal y = KUnit::parseValue(params[2]);

                rotMatrix.translate(x, y);
                // oo2 rotates by radians
                rotMatrix.rotate(-params[0].toDouble()*180.0 / M_PI);
                rotMatrix.translate(-x, -y);
            } else {
                // oo2 rotates by radians
                rotMatrix.rotate(-params[0].toDouble()*180.0 / M_PI);
            }
            matrix = matrix * rotMatrix;
        } else if (cmd == "translate") {
            QTransform moveMatrix;
            if (params.count() == 2) {
                qreal x = KUnit::parseValue(params[0]);
                qreal y = KUnit::parseValue(params[1]);
                moveMatrix.translate(x, y);
            } else   // Spec : if only one param given, assume 2nd param to be 0
                moveMatrix.translate(KUnit::parseValue(params[0]) , 0);
            matrix = matrix * moveMatrix;
        } else if (cmd == "scale") {
            QTransform scaleMatrix;
            if (params.count() == 2)
                scaleMatrix.scale(params[0].toDouble(), params[1].toDouble());
            else    // Spec : if only one param given, assume uniform scaling
                scaleMatrix.scale(params[0].toDouble(), params[0].toDouble());
            matrix = matrix * scaleMatrix;
        } else if (cmd == "skewx") {
            QPointF p = absolutePosition(KoFlake::TopLeftCorner);
            QTransform shearMatrix;
            shearMatrix.translate(p.x(), p.y());
            shearMatrix.shear(tan(-params[0].toDouble()), 0.0F);
            shearMatrix.translate(-p.x(), -p.y());
            matrix = matrix * shearMatrix;
        } else if (cmd == "skewy") {
            QPointF p = absolutePosition(KoFlake::TopLeftCorner);
            QTransform shearMatrix;
            shearMatrix.translate(p.x(), p.y());
            shearMatrix.shear(0.0F, tan(-params[0].toDouble()));
            shearMatrix.translate(-p.x(), -p.y());
            matrix = matrix * shearMatrix;
        } else if (cmd == "matrix") {
            QTransform m;
            if (params.count() >= 6) {
                m.setMatrix(params[0].toDouble(), params[1].toDouble(), 0,
                        params[2].toDouble(), params[3].toDouble(), 0,
                        KUnit::parseValue(params[4]), KUnit::parseValue(params[5]), 1);
            }
            matrix = matrix * m;
        }
    }

    return matrix;
}

void KShape::saveOdfAttributes(KoShapeSavingContext &context, int attributes) const
{
    Q_D(const KShape);
    if (attributes & OdfStyle) {
        KOdfGenericStyle style;
        // all items that should be written to 'draw:frame' and any other 'draw:' object that inherits this shape
        if (context.isSet(KoShapeSavingContext::PresentationShape)) {
            style = KOdfGenericStyle(KOdfGenericStyle::PresentationAutoStyle, "presentation");
            context.xmlWriter().addAttribute("presentation:style-name", saveStyle(style, context));
        } else {
            style = KOdfGenericStyle(KOdfGenericStyle::GraphicAutoStyle, "graphic");
            context.xmlWriter().addAttribute("draw:style-name", saveStyle(style, context));
        }
    }

    if (attributes & OdfId)  {
        if (context.isSet(KoShapeSavingContext::DrawId)) {
            context.xmlWriter().addAttribute("draw:id", context.drawId(this));
        }
    }

    if (attributes & OdfName) {
        if (! name().isEmpty())
            context.xmlWriter().addAttribute("draw:name", name());
    }

    if (attributes & OdfLayer) {
        KShape *parent = d->parent;
        while (parent) {
            if (dynamic_cast<KoShapeLayer*>(parent)) {
                context.xmlWriter().addAttribute("draw:layer", parent->name());
                break;
            }
            parent = parent->parent();
        }
    }

    if (attributes & OdfSize) {
        QSizeF s(size());
        if (parent() && parent()->isClipped(this)) { // being clipped shrinks our visible size
            // clipping in ODF is done using a combination of visual size and content cliprect.
            // A picture of 10cm x 10cm displayed in a box of 2cm x 4cm will be scaled (out
            // of proportion in this case).  If we then add a fo:clip like;
            // fo:clip="rect(2cm, 3cm, 4cm, 5cm)" (top, right, bottom, left)
            // our original 10x10 is clipped to 2cm x 4cm  and *then* fitted in that box.

            // TODO do this properly by subtracting rects
            s = parent()->size();
        }
        context.xmlWriter().addAttributePt("svg:width", s.width());
        context.xmlWriter().addAttributePt("svg:height", s.height());
    }

    // The position is implicitly stored in the transformation matrix
    // if the transformation is saved as well
    if ((attributes & OdfPosition) && !(attributes & OdfTransformation)) {
        const QPointF p(position() * context.shapeOffset(this));
        context.xmlWriter().addAttributePt("svg:x", p.x());
        context.xmlWriter().addAttributePt("svg:y", p.y());
    }

    if (attributes & OdfTransformation) {
        QTransform matrix = absoluteTransformation(0) * context.shapeOffset(this);
        if (! matrix.isIdentity()) {
            if (qAbs(matrix.m11() - 1) < 1E-5           // 1
                    && qAbs(matrix.m12()) < 1E-5        // 0
                    && qAbs(matrix.m21()) < 1E-5        // 0
                    && qAbs(matrix.m22() - 1) < 1E-5) { // 1
                context.xmlWriter().addAttributePt("svg:x", matrix.dx());
                context.xmlWriter().addAttributePt("svg:y", matrix.dy());
            } else {
                QString m = QString("matrix(%1 %2 %3 %4 %5pt %6pt)")
                            .arg(matrix.m11(), 0, 'f', 11)
                            .arg(matrix.m12(), 0, 'f', 11)
                            .arg(matrix.m21(), 0, 'f', 11)
                            .arg(matrix.m22(), 0, 'f', 11)
                            .arg(matrix.dx(), 0, 'f', 11)
                            .arg(matrix.dy(), 0, 'f', 11);
                context.xmlWriter().addAttribute("draw:transform", m);
            }
        }
    }

    if (attributes & OdfViewbox) {
        const QSizeF s(size());
        QString viewBox = QString("0 0 %1 %2").arg(qRound(s.width())).arg(qRound(s.height()));
        context.xmlWriter().addAttribute("svg:viewBox", viewBox);
    }

    if (attributes & OdfAdditionalAttributes) {
        QMap<QString, QString>::const_iterator it(d->additionalAttributes.constBegin());
        for (; it != d->additionalAttributes.constEnd(); ++it) {
            context.xmlWriter().addAttribute(it.key().toUtf8(), it.value());
        }
    }
}

void KShape::saveOdfCommonChildElements(KoShapeSavingContext &context) const
{
    Q_D(const KShape);
    // save event listeners see ODF 9.2.21 Event Listeners
    if (d->eventActions.size() > 0) {
        context.xmlWriter().startElement("office:event-listeners");
        foreach(KEventAction * action, d->eventActions) {
            action->saveOdf(context);
        }
        context.xmlWriter().endElement();
    }

    // those go on the same level, so delay writing them till the end.
    context.addForWriting(d->connections);

    // TODO: save glue points see ODF 9.2.19 Glue Points
}

// end loading & saving methods

// static
void KShape::applyConversion(QPainter &painter, const KoViewConverter &converter)
{
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
}

QPointF KShape::shapeToDocument(const QPointF &point) const
{
    return absoluteTransformation(0).map(point);
}

QRectF KShape::shapeToDocument(const QRectF &rect) const
{
    return absoluteTransformation(0).mapRect(rect);
}

QPointF KShape::documentToShape(const QPointF &point) const
{
    return absoluteTransformation(0).inverted().map(point);
}

QRectF KShape::documentToShape(const QRectF &rect) const
{
    return absoluteTransformation(0).inverted().mapRect(rect);
}

bool KShape::addDependee(KShape *shape)
{
    Q_D(KShape);
    if (! shape)
        return false;

    // refuse to establish a circular dependency
    if (shape->hasDependee(this))
        return false;

    d->dependees.insert(shape);

    return true;
}

void KShape::removeDependee(KShape *shape)
{
    Q_D(KShape);
    d->dependees.remove(shape);
}

bool KShape::hasDependee(KShape *shape) const
{
    Q_D(const KShape);
    return d->dependees.contains(shape);
}

void KShape::shapeChanged(ChangeType type, KShape *shape)
{
    Q_UNUSED(type);
    Q_UNUSED(shape);
}

void KShape::setAdditionalAttribute(const QString &name, const QString &value)
{
    Q_D(KShape);
    d->additionalAttributes.insert(name, value);
}

void KShape::removeAdditionalAttribute(const QString &name)
{
    Q_D(KShape);
    d->additionalAttributes.remove(name);
}

bool KShape::hasAdditionalAttribute(const QString &name) const
{
    Q_D(const KShape);
    return d->additionalAttributes.contains(name);
}

QString KShape::additionalAttribute(const QString &name) const
{
    Q_D(const KShape);
    return d->additionalAttributes.value(name);
}

void KShape::setAdditionalStyleAttribute(const QString &name, const QString &value)
{
    Q_D(KShape);
    d->additionalStyleAttributes.insert(name, value);
}

void KShape::removeAdditionalStyleAttribute(const QString &name)
{
    Q_D(KShape);
    d->additionalStyleAttributes.remove(name);
}

KFilterEffectStack *KShape::filterEffectStack() const
{
    Q_D(const KShape);
    return d->filterEffectStack;
}

void KShape::setFilterEffectStack(KFilterEffectStack *filterEffectStack)
{
    Q_D(KShape);
    if (d->filterEffectStack)
        d->filterEffectStack->deref();
    d->filterEffectStack = filterEffectStack;
    if (d->filterEffectStack) {
        d->filterEffectStack->ref();
    }
    notifyChanged();
}

QSet<KShape*> KShape::toolDelegates() const
{
    Q_D(const KShape);
    return d->toolDelegates;
}

void KShape::beginEditBlock()
{
    Q_D(KShape);
    d->editBlockDepth++;
}

void KShape::endEditBlock()
{
    Q_D(KShape);
    if (d->editBlockDepth == 0) {
        kWarning(30006) << "out of sync endEditBlock called, already ended block";
        return;
    }
    if (--d->editBlockDepth == 0 && d->editBlockEndShouldEmit) {
        d->shapeChanged(GenericMatrixChange);
        d->editBlockEndShouldEmit = false;
    }
}

void KShape::setToolDelegates(const QSet<KShape*> &delegates)
{
    Q_D(KShape);
    d->toolDelegates = delegates;
}

KShapePrivate *KShape::priv()
{
    Q_D(KShape);
    return d;
}

const KShapePrivate *KShape::priv() const
{
    Q_D(const KShape);
    return d;
}