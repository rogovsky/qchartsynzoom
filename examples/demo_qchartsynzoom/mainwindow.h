#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "qchartsynzoom.h"
#include "qwheelsynzoomsvc.h"
#include "qaxissynzoomsvc.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

// ��������� �������� ������ ���������
    void setSplitSize();

private:
    Ui::MainWindow *ui;

    QwtLegend *leg1,*leg2,*leg3;        // ������� ��� ��������
    QwtPlotGrid *grid1,*grid2,*grid3;   // ������������ ����� ��� ��������
    QwtPlotCurve *curv1,*curv2,*curv3;  // ������, ������������ �� ��������

// �������� ����������� ��������������� � ����������� �������� � ����� TChart
    QChartSynZoom *zoom;
// ��������� ��������������� ������� ��������� ������ ���� (���������� � ���������)
    QWheelSynZoomSvc *whlzmsvc;
// ��������� ��������� ����� �� ������ ������� (���������� � ���������)
    QAxisSynZoomSvc *axzmsvc;
};

#endif // MAINWINDOW_H
