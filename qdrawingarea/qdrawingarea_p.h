#ifndef QDRAWINGAREA_P
#define QDRAWINGAREA_P

#include <QMap>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QPixmap>
#include <QTouchDevice>
#include <QMouseEvent>

class QDrawingStroke;
class QDrawingPen;
class QDrawingArea;
struct QDrawingAreaPrivate;
class QAbstractDrawingModel;

class InputProcessor : public QObject
{
    friend class QDrawingArea;

    Q_OBJECT
public:
    explicit InputProcessor(struct QDrawingAreaPrivate *d);
    ~InputProcessor();

    enum {
        RepaintInterval = 100 // ms
    };

    void moveToThread(QThread *targetThread);

signals:
    void repaint(QList<int> modifiedStrokes);
public slots:
    void processPoint(quint32 deviceId, QSharedPointer<QDrawingPen> pen, qreal x, qreal y, qreal pressure);
    void finishPoint(quint32 deviceId);
    void finishAllPoints();
    void repaintTimeout();
    void setRepaintInterval(int timeout);

private:
    void processErasing();

    QTimer repaintTimer;
    QDrawingArea *drawingArea;
    struct QDrawingAreaPrivate *d;
    QMap<quint32, quint32> deviceIdMap;
    QList<int> modifiedStrokes;
};

class Rasterizer : public QObject
{
    Q_OBJECT
public:
    explicit Rasterizer(struct QDrawingAreaPrivate *d);

signals:
    void updateRender(QPixmap img);
public slots:
    void repaint(QList<int> modifiedStrokes);

private:
    inline void renderFullStroke(QPainter &p, QDrawingStroke &stroke);
    inline void renderPartialStroke(QPainter &p, QDrawingStroke &stroke);
    inline void renderStrokeFrom(QPainter &p, QDrawingStroke &stroke, int point);
    struct QDrawingAreaPrivate *d;
};

struct QDrawingAreaPrivate
{
    QDrawingAreaPrivate(QDrawingArea *q);
    ~QDrawingAreaPrivate();

    void generateRandomId();

    typedef QPair<QTouchDevice,QTouchEvent::TouchPoint> TouchInfoPair;
    QDrawingArea *q_ptr;

    int flags;
    int drawingMode;
    bool ignoreFakeMouse;
    InputProcessor *processor;
    QThread *processorThread;
    Rasterizer *rasterizer;
    QThread *rasterizerThread;
    QVector<QDrawingStroke> strokes;
    QMap<quint32, QDrawingStroke> strokeMap;
    quint32 currentId;
    QMap<qint64, quint32> tabletIdMap;
    QList<QSharedPointer<QDrawingPen> > pens;
    Qt::MouseButtons heldMouseButtons;
    QPixmap pixmap;
    QMap<TouchInfoPair, quint32> touchPointMap;
};

struct QAbstractDrawingModelPrivate
{
    QAbstractDrawingModelPrivate(QAbstractDrawingModel *q);
    ~QAbstractDrawingModelPrivate();

    QAbstractDrawingModel *q_ptr;

    QMap<quint32, QDrawingStroke> strokeMap;
    QSizeF documentSize;

    quint32 currentId;
};

#endif // QDRAWINGAREA_P
