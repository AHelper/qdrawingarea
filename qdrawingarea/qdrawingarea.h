#ifndef QDRAWINGPAD_H
#define QDRAWINGPAD_H

#include <QVector2D>
#include <QWidget>
#include <QAbstractListModel>

class QDrawingAreaPrivate;
class QAbstractDrawingModelPrivate;

class QDrawingPen
{
public:
//    explicit QDrawingPen(Qt::MouseButton button, QColor color, qreal width, qreal orientationLock = qQNaN());
    explicit QDrawingPen(Qt::MouseButton button, QColor color, qreal minWidth, qreal maxWidth = 0, qreal orientationLock = qQNaN());

    explicit QDrawingPen(QDrawingPen & other);

    enum {
        Mode_FeltTipPen,
        Mode_Selector,
        Mode_Eraser,
        Mode_ChiselTip,
        Mode_Highlighter
    };

    Qt::MouseButton button();
    QColor color();
    qreal minWidth();
    qreal maxWidth();
    bool isVariableWidth();
    bool isOrientationLocked();
    qreal orientationLock();

    inline qreal calcWidth(qreal pressure)
    {
        if(m_minWidth == m_maxWidth)
            return m_minWidth;
        else // lerp
            return (m_maxWidth - m_minWidth) * pressure + m_minWidth;
    }

private:
    Qt::MouseButton m_button;
    QColor m_color;
    qreal m_minWidth;
    qreal m_maxWidth;
    qreal m_orientationLock;
};

class QDrawingPoint
{
public:
    QDrawingPoint();
    QDrawingPoint(qreal xpos, qreal ypos, qreal pressure = 1.0, QVector2D normal = QVector2D(0, -1));

    qreal x();
    qreal y();
    qreal pressure();
    QVector2D normal();
    operator QPointF ();

    void setX(qreal x);
    void setY(qreal y);
    void setPressure(qreal pressure);
    void setNormal(QVector2D normal);

protected:
    qreal m_x;
    qreal m_y;
    qreal m_pressure;
    QVector2D m_normal;
};

class QDrawingStroke
{
public:
    QDrawingStroke();

    quint32 id();
    QDrawingPen* pen();

    void setPen(QSharedPointer<QDrawingPen> pen);
    void setId(quint32 id);
    /**
     * @brief setDirty
     * @param dirty
     * @param at First point in the stroke that is dirty. Default 0 marks the entire stroke as dirty.
     */
    void setDirty(bool dirty = true, int at = 0);
    bool dirty();
    int dirtyAt();

    QDrawingStroke& operator<<(const QDrawingPoint &p);
    bool operator&(const QDrawingStroke &s);
    QDrawingPoint& operator[](const unsigned long index);
    unsigned long size() const;

protected:
    QList<int> m_newPoints;
    QVector<QDrawingPoint> m_points;
    quint32 m_id;
    int m_mode;
    QSharedPointer<QDrawingPen> m_pen;
    bool m_dirty;
    int m_dirtyAt;
};

class QAbstractDrawingModel : public QObject
{
    Q_OBJECT
public:
    QAbstractDrawingModel();
    ~QAbstractDrawingModel();

    /**
     * @brief Sets the size of the drawing document in millimeters.
     *
     * The drawing document is an internal area that scales to the size of the QDrawingArea widget.
     * This document uses sizes in mm (millimeters) to provide DPI- and scaling-independent drawing.
     *
     *
     * @param size
     */
    void setDrawingSize(QSizeF &size);
    bool hasIndex(quint32 strokeId);
    const QDrawingStroke& index(quint32 strokeId);
    void append(const QDrawingStroke& stroke);

signals:
    void strokeInserted(const QDrawingStroke& stroke);
    void strokeRemoved(const QDrawingStroke& stroke);

private:
    QAbstractDrawingModelPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QAbstractDrawingModel)
    Q_DISABLE_COPY(QAbstractDrawingModel)
};

class QDrawingArea : public QWidget
{
    Q_OBJECT
public:
    explicit QDrawingArea(QWidget *parent = 0);
    ~QDrawingArea();

    bool event(QEvent *e);

    enum {
        IgnoreTouch      = 0x00000001,
        IgnoreMouse      = 0x00000002,
        IgnoreTablet     = 0x00000004,
        SmoothCurves     = 0x00000008,
        D_EmulateTablet  = 0x01000000,
        D_EmulateTouch   = 0x02000000,
        D_EmulatePressure= 0x04000000,
    };

    void setFlag(int flag, bool enable = true);
    void setUpdateRate(int updatesPerSecond);
    void addPen(QDrawingPen &p);

signals:

public slots:

private slots:
    void updatePixmap(QPixmap p);

protected:
    /**
     * @brief Adds a point to the current stroke being drawn, starting a new stroke if needed.
     * @param x X value in relative coordinate space of widget.
     * @param y Y value in relative coordinate space of widget.
     * @param pressure Pressure applied at the point in the range of [0..1].
     */
    void addStrokePoint(quint32 deviceId, QSharedPointer<QDrawingPen> pen, qreal x, qreal y, qreal pressure);
    void finishStroke(quint32 deviceId);

    QSharedPointer<QDrawingPen> findPenFromButtons(Qt::MouseButtons buttons);

private:
    QDrawingAreaPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QDrawingArea)
};

#endif // QDRAWINGPAD_H
