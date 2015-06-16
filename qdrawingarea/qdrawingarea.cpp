#include "qdrawingarea.h"
#include "qdrawingarea_p.h"

#include <QEvent>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

QDrawingArea::QDrawingArea(QWidget *parent) : QWidget(parent),
    d_ptr(new QDrawingAreaPrivate(this))
{

}

QDrawingArea::~QDrawingArea()
{

}

bool QDrawingArea::event(QEvent *e)
{
    Q_D(QDrawingArea);

    QMouseEvent* mouseEvent;
    QTabletEvent *tabletEvent;
    QTouchEvent *touchEvent;
    QPaintEvent *paintEvent;
    QSharedPointer<QDrawingPen> pen;

    switch(e->type())
    {
    case QEvent::TabletPress:
        qDebug() << "QEvent::TabletPress";
    case QEvent::TabletMove:
        qDebug() << "QEvent::TabletMove";
        if(d->flags & IgnoreTablet)
        {
            e->ignore();
        }
        else
        {
            tabletEvent = dynamic_cast<QTabletEvent*>(e);

            pen = findPenFromButtons(tabletEvent->buttons());

            if(pen) {
                if(!d->tabletIdMap.contains(tabletEvent->uniqueId()))
                {
                    d->tabletIdMap[tabletEvent->uniqueId()] = d->tabletIdMap.size() + 0x1000;
                }
                addStrokePoint(d->tabletIdMap[tabletEvent->uniqueId()], pen, tabletEvent->posF().x(), tabletEvent->posF().y(), tabletEvent->pressure());
            }

            e->accept();
        }

        break;
    case QEvent::TabletRelease:
        qDebug() << "QEvent::TabletRelease";

        if(d->flags & IgnoreTablet)
        {
            e->ignore();
        }
        else
        {
            if(d->tabletIdMap.contains(tabletEvent->uniqueId()))
            {
                finishStroke(d->tabletIdMap[tabletEvent->uniqueId()]);
            }

            e->accept();
        }

        break;
    case QEvent::MouseButtonPress:
        qDebug() << "QEvent::MouseButtonPress";
    case QEvent::MouseMove:
        qDebug() << "QEvent::MouseMove";
        if(d->flags & IgnoreMouse)
        {
            e->ignore();
        }
        else
        {
            mouseEvent = dynamic_cast<QMouseEvent*>(e);

            pen = findPenFromButtons(mouseEvent->buttons());

            if(pen) {
                d->heldMouseButtons |= pen->button();

                addStrokePoint(0x1000 + (int)pen->button(), pen, mouseEvent->x(), mouseEvent->y(), 1);
            }

            e->accept();
        }
        break;
    case QEvent::MouseButtonRelease:
        qDebug() << "QEvent::MouseButtonRelease";
        if(d->flags & IgnoreMouse)
        {
            e->ignore();
        }
        else
        {
            mouseEvent = dynamic_cast<QMouseEvent*>(e);

            finishStroke(0x1000 + (int)mouseEvent->button());
            d->heldMouseButtons &= ~mouseEvent->button();

            e->accept();
        }
        break;
    case QEvent::TouchBegin:
        qDebug() << "QEvent::TouchBegin";
        if(d->flags & IgnoreTouch)
        {
            e->ignore();
        }
        else
        {
            touchEvent = dynamic_cast<QTouchEvent*>(e);

//            if(!d->touchPointMap.contains())

            e->accept();
        }

        break;
    case QEvent::TouchEnd:
        qDebug() << "QEvent::TouchEnd";

        break;
    case QEvent::TouchUpdate:
        qDebug() << "QEvent::TouchUpdate";

        break;
    case QEvent::TouchCancel:
        qDebug() << "QEvent::TouchCancel";

        break;
    case QEvent::Paint:
        qDebug() << "QEvent::Paint";
        paintEvent = dynamic_cast<QPaintEvent*>(e);
        e->accept();

    {
        QPainter p(this);
        p.drawPixmap(rect(), d->pixmap);
    }

        break;
    case QEvent::Resize:
        qDebug() << "QEvent::Resize";

        break;
    default:
        qDebug() << e;
        return QWidget::event(e);
    }

    return true;
}

void QDrawingArea::setFlag(int flag, bool enable)
{
    Q_D(QDrawingArea);
    if(enable)
        d->flags |= flag;
    else
        d->flags &= ~flag;
}

void QDrawingArea::setUpdateRate(int updatesPerSecond)
{
    Q_D(QDrawingArea);

    emit d->processor->setRepaintInterval(1000.0/updatesPerSecond);
}

void QDrawingArea::addPen(QDrawingPen &p)
{
    Q_D(QDrawingArea);

    d->pens.append(QSharedPointer<QDrawingPen>(new QDrawingPen(p)));
}

void QDrawingArea::updatePixmap(QPixmap p)
{
    Q_D(QDrawingArea);

    qDebug() << "Got a new pixmap";

    d->pixmap = p;

    repaint();
}

void QDrawingArea::addStrokePoint(quint32 deviceId, QSharedPointer<QDrawingPen> pen, qreal x, qreal y, qreal pressure)
{
    Q_D(QDrawingArea);

    qDebug() << thread();
    QMetaObject::invokeMethod(d->processor, "processPoint",
                              Qt::QueuedConnection,
                              Q_ARG(quint32, deviceId),
                              Q_ARG(QSharedPointer<QDrawingPen>, pen),
                              Q_ARG(qreal, x),
                              Q_ARG(qreal, y),
                              Q_ARG(qreal, pressure));
}

void QDrawingArea::finishStroke(quint32 deviceId)
{
    Q_D(QDrawingArea);

    QMetaObject::invokeMethod(d->processor, "finishPoint",
                              Qt::QueuedConnection,
                              Q_ARG(quint32, deviceId));
}

QSharedPointer<QDrawingPen> QDrawingArea::findPenFromButtons(Qt::MouseButtons buttons)
{
    Q_D(QDrawingArea);

    // FIXME: Add in a map of pens or something
    for(int i = 0; i < d->pens.size(); i++)
    {
        if(d->pens[i]->button() & buttons)
        {
            return d->pens[i];
        }
    }
    return QSharedPointer<QDrawingPen>();
}

InputProcessor::InputProcessor(QDrawingAreaPrivate *d) : QObject(),
    d(d)
{
    repaintTimer.setInterval(RepaintInterval);

    connect(&repaintTimer, &QTimer::timeout, this, &InputProcessor::repaintTimeout);
}

InputProcessor::~InputProcessor()
{

}

void InputProcessor::moveToThread(QThread *targetThread)
{
    QObject::moveToThread(targetThread);
    repaintTimer.moveToThread(targetThread);
}

void InputProcessor::processPoint(quint32 deviceId, QSharedPointer<QDrawingPen> pen, qreal x, qreal y, qreal pressure)
{
    QDrawingPoint point(x, y, pressure);

//    if(stroke.id() == (quint32)-1)
    if(!deviceIdMap.contains(deviceId))
    {
        d->generateRandomId();
        deviceIdMap[deviceId] = d->currentId;

        QDrawingStroke &stroke = d->strokeMap[deviceIdMap[deviceId]];
        stroke.setId(d->currentId);
        stroke.setPen(pen);
    }

    QDrawingStroke &stroke = d->strokeMap[deviceIdMap[deviceId]];

    // TODO: Calculate motion of the point smoothly
    // FIXME: Dirty
    if(stroke.size() > 0)
    {
        QDrawingPoint &previous = stroke[stroke.size() - 1];
//        QLineF vector(previous, point);
//        QLineF normal = vector.normalVector().unitVector();

        QVector2D t = (QVector2D)point - (QVector2D)previous;

        t.normalize();
        point.setNormal(t);
    }

    // TODO: Generate cubic curves from recent points

    stroke << point;

    qDebug() << "Added point to stroke" << stroke.id() << "with size" << stroke.size();

    if(!modifiedStrokes.contains(stroke.id()))
        modifiedStrokes << stroke.id();

    // Fire up repaint timer if not already started
    if(!repaintTimer.isActive())
    {
        qDebug() << "Starting";
        qDebug() << thread() << repaintTimer.thread();
        repaintTimer.start();
    }
}

void InputProcessor::finishPoint(quint32 deviceId)
{
    if(deviceIdMap.contains(deviceId))
    {
        qDebug() << "Finished stroke" << deviceIdMap[deviceId] << "from" << deviceId;

        deviceIdMap.remove(deviceId);
    }
}

void InputProcessor::finishAllPoints()
{
    deviceIdMap.clear();
}

void InputProcessor::repaintTimeout()
{
    qDebug() << "Timeout";
    if(modifiedStrokes.empty())
    {
        repaintTimer.stop();
    }
    else
    {
        qDebug() << "Triggering repaint";
        // TODO: Other work?
        emit repaint(modifiedStrokes);

        modifiedStrokes.clear();
    }
}

void InputProcessor::setRepaintInterval(int timeout)
{
    repaintTimer.setInterval(timeout);
}

void InputProcessor::processErasing()
{
    // TODO: Look at a stroke and check if it collides with another stroke.  Might not need the entire stroke, just the segment from the last point to the current
    // Also handle if it is a dot. Somehow.
}

QDrawingPoint::QDrawingPoint() :
    m_x(0),
    m_y(0),
    m_pressure(0),
    m_normal(QVector2D())
{

}

QDrawingPoint::QDrawingPoint(qreal xpos, qreal ypos, qreal pressure, QVector2D normal) :
    m_x(xpos),
    m_y(ypos),
    m_pressure(pressure),
    m_normal(normal)
{
}

qreal QDrawingPoint::x()
{
    return m_x;
}

qreal QDrawingPoint::y()
{
    return m_y;
}

qreal QDrawingPoint::pressure()
{
    return m_pressure;
}

QVector2D QDrawingPoint::normal()
{
    return m_normal;
}

QDrawingPoint::operator QPointF()
{
    return QPointF(m_x, m_y);
}

void QDrawingPoint::setX(qreal x)
{
    m_x = x;
}

void QDrawingPoint::setY(qreal y)
{
    m_y = y;
}

void QDrawingPoint::setPressure(qreal pressure)
{
    m_pressure = pressure;
}

void QDrawingPoint::setNormal(QVector2D normal)
{
    m_normal = normal;
}


QDrawingStroke::QDrawingStroke() :
    m_id(-1)
{

}

quint32 QDrawingStroke::id()
{
    return m_id;
}

QDrawingPen *QDrawingStroke::pen()
{
    return m_pen.data();
}

void QDrawingStroke::setPen(QSharedPointer<QDrawingPen> pen)
{
    m_pen = pen;
}

QDrawingStroke &QDrawingStroke::operator<<(const QDrawingPoint &p)
{
    m_newPoints.append(m_points.size());
    m_points.append(p);

    return *this;
}

bool QDrawingStroke::operator&(const QDrawingStroke &s)
{
    return false;
}

QDrawingPoint &QDrawingStroke::operator[](const unsigned long index)
{
    return m_points[index];
}

unsigned long QDrawingStroke::size() const
{
    return m_points.size();
}

void QDrawingStroke::setId(quint32 id)
{
    m_id = id;
}

void QDrawingStroke::setDirty(bool dirty, int at)
{
    m_dirty = dirty;

    if(at == -1)
        m_dirtyAt = m_points.size() - 1;
    else
        m_dirtyAt = at;
}

bool QDrawingStroke::dirty()
{
    return m_dirty;
}

int QDrawingStroke::dirtyAt()
{
    return m_dirtyAt;
}

QDrawingAreaPrivate::QDrawingAreaPrivate(QDrawingArea *q) : q_ptr(q), flags(0), drawingMode(0), ignoreFakeMouse(false), pixmap(1,1) {
    qRegisterMetaType<QSharedPointer<QDrawingPen> >("QSharedPointer<QDrawingPen>");
    processorThread = new QThread;
    processor = new InputProcessor(this);

    processorThread->start();
    processor->moveToThread(processorThread);

    rasterizerThread = new QThread;
    rasterizer = new Rasterizer(this);

    QObject::connect(q, &QDrawingArea::destroyed, processor, &QObject::deleteLater);
    QObject::connect(q, &QDrawingArea::destroyed, rasterizer, &QObject::deleteLater);
    QObject::connect(q, &QDrawingArea::destroyed, processorThread, &QObject::deleteLater);
    QObject::connect(q, &QDrawingArea::destroyed, rasterizerThread, &QObject::deleteLater);

    // TODO: Connect repaint signal from input processor to rasterizer
    QObject::connect(processor, &InputProcessor::repaint, rasterizer, &Rasterizer::repaint, Qt::QueuedConnection);
    QObject::connect(rasterizer, &Rasterizer::updateRender, q, &QDrawingArea::updatePixmap);
}

QDrawingAreaPrivate::~QDrawingAreaPrivate() {
    processor->deleteLater();
    processorThread->deleteLater();
}

void QDrawingAreaPrivate::generateRandomId()
{
    if(strokeMap.contains(currentId))
        currentId++;

    while(strokeMap.contains(currentId))
    {
        currentId = (quint32)qrand();
    }
}

//QDrawingPen::QDrawingPen(Qt::MouseButton button, QColor color, qreal width, qreal orientationLock) :
//    m_button(button),
//    m_color(color),
//    m_minWidth(width),
//    m_maxWidth(width),
//    m_orientationLock(orientationLock)
//{

//}

QDrawingPen::QDrawingPen(Qt::MouseButton button, QColor color, qreal minWidth, qreal maxWidth, qreal orientationLock) :
    m_button(button),
    m_color(color),
    m_minWidth(minWidth),
    m_maxWidth(maxWidth == 0 ? minWidth : maxWidth),
    m_orientationLock(orientationLock)
{

}

QDrawingPen::QDrawingPen(QDrawingPen &other) :
    m_button(other.m_button),
    m_color(other.m_color),
    m_minWidth(other.m_minWidth),
    m_maxWidth(other.m_maxWidth)
{

}

Qt::MouseButton QDrawingPen::button()
{
    return m_button;
}

QColor QDrawingPen::color()
{
    return m_color;
}

qreal QDrawingPen::minWidth()
{
    return m_minWidth;
}

qreal QDrawingPen::maxWidth()
{
    return m_maxWidth;
}

bool QDrawingPen::isVariableWidth()
{
    // No fuzzy comparison needed
    return m_minWidth != m_maxWidth;
}

bool QDrawingPen::isOrientationLocked()
{
    return m_orientationLock == qQNaN();
}


Rasterizer::Rasterizer(QDrawingAreaPrivate *d) :
    d(d)
{

}

void Rasterizer::repaint(QList<int> modifiedStrokes)
{
    QPixmap pix;
    qDebug() << "Rasterize";

    // Copy current pixmap if the size matches
    if(d->q_ptr->rect() == d->pixmap.rect())
    {
        pix = d->pixmap;

        QPainter p(&pix);

        for(int i = 0; i < modifiedStrokes.size(); i++)
        {
            renderPartialStroke(p, d->strokeMap[modifiedStrokes[i]]);
        }

        p.end();
    }
    else
    {
        pix = QPixmap(d->q_ptr->size());
        pix.fill(d->q_ptr->palette().color(d->q_ptr->backgroundRole()));

        QPainter p(&pix);
        QMap<quint32, QDrawingStroke>::iterator itr = d->strokeMap.begin();

        for(; itr != d->strokeMap.end(); ++itr)
        {
            renderFullStroke(p, itr.value());
        }

        p.end();
    }


    emit updateRender(pix);
}

void Rasterizer::renderFullStroke(QPainter &p, QDrawingStroke &stroke)
{
    renderStrokeFrom(p, stroke, 0);
}

void Rasterizer::renderPartialStroke(QPainter &p, QDrawingStroke &stroke)
{
    if(stroke.dirty())
        renderStrokeFrom(p, stroke, stroke.dirtyAt());
}

void Rasterizer::renderStrokeFrom(QPainter &p, QDrawingStroke &stroke, int point)
{
    QPen pen(stroke.pen()->color());
    pen.setCapStyle(Qt::RoundCap);

    if(d->flags & QDrawingArea::SmoothCurves)
        p.setRenderHint(QPainter::Antialiasing);

    // TODO: Cubic curves

    for(int i = point; i < stroke.size() - 1; i++)
    {
        // TODO: Variable-width lines
        pen.setWidthF(stroke.pen()->calcWidth(stroke[i+1].pressure()));
        p.setPen(pen);
        p.drawLine(stroke[i], stroke[i+1]);
    }
}


QAbstractDrawingModelPrivate::QAbstractDrawingModelPrivate(QAbstractDrawingModel *q) : q_ptr(q)
{

}

QAbstractDrawingModelPrivate::~QAbstractDrawingModelPrivate()
{

}


QAbstractDrawingModel::QAbstractDrawingModel()
{

}

QAbstractDrawingModel::~QAbstractDrawingModel()
{

}

bool QAbstractDrawingModel::hasIndex(quint32 strokeId)
{

}

const QDrawingStroke &QAbstractDrawingModel::index(quint32 strokeId)
{

}

void QAbstractDrawingModel::append(const QDrawingStroke &stroke)
{

}
