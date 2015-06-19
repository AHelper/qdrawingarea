#include "testui.h"
#include "ui_testui.h"

TestUI::TestUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestUI),
    pen(Qt::LeftButton, QColor(Qt::blue), (qreal)0.1, (qreal)15, (qreal)0)
{
    ui->setupUi(this);

    ui->widget->addPen(pen);
    ui->widget->setFlag(QDrawingArea::SmoothCurves | QDrawingArea::D_EmulatePressure);
    ui->widget->setUpdateRate(20);

}

TestUI::~TestUI()
{
    delete ui;
}
