#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qdrawingarea.h>

namespace Ui {
class TestUI;
}

class QDrawingPen;

class TestUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestUI(QWidget *parent = 0);
    ~TestUI();

private:
    Ui::TestUI *ui;
    QDrawingPen pen;
};

#endif // MAINWINDOW_H
