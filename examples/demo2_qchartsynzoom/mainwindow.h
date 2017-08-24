#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "qchartsynzoom.h"
#include "qaxissynzoomsvc.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QwtLegend *leg1,*leg2,*leg3;        // легенды для графиков
    QwtPlotGrid *grid1,*grid2,*grid3;   // координатные сетки для графиков
    QwtPlotCurve *curv1,*curv2,*curv3;  // кривые, отображаемые на графиках

// Менеджер синхронного масштабирования и перемещения графиков в стиле TChart
    QChartSynZoom *zoom;
// Интерфейс изменения одной из границ графика (дополнение к менеджеру)
    QAxisSynZoomSvc *axzmsvc;
};

#endif // MAINWINDOW_H
