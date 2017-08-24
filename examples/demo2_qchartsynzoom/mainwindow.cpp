#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qwt_scale_draw.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // ����������� ����� ������������
    ui->setupUi(this);

    QString tss(    // ��������� �������� �������
//            "background-color: white;"
            "border-left-width: -5px;"
            "border-right-width: -9px;"
            "border-top-width: -7px;"
            "border-bottom-width: -4px;"
            "border-style: inset;");

    // ������� ������� ��� ������� �������
    leg1 = new QwtLegend();
    leg1->setItemMode(QwtLegend::ReadOnlyItem); // ������� ������ ��� ������
    leg1->contentsWidget()->setStyleSheet(tss); // ������������� �������� �������
    // ��������� ������� �� ������
    ui->plot1->insertLegend(leg1,QwtPlot::TopLegend);

    // ������� ������� ��� ������� �������
    leg2 = new QwtLegend();
    leg2->setItemMode(QwtLegend::ReadOnlyItem); // ������� ������ ��� ������
    leg2->contentsWidget()->setStyleSheet(tss); // ������������� �������� �������
    // ��������� ������� �� ������
    ui->plot2->insertLegend(leg2,QwtPlot::TopLegend);

    // ������� ������� ��� �������� �������
    leg3 = new QwtLegend();
    leg3->setItemMode(QwtLegend::ReadOnlyItem); // ������� ������ ��� ������
    leg3->contentsWidget()->setStyleSheet(tss); // ������������� �������� �������
    // ��������� ������� �� ������
    ui->plot3->insertLegend(leg3,QwtPlot::TopLegend);

    // ������� ������������ ����� ��� ������� �������
    grid1 = new QwtPlotGrid;
    // ��������� ����� ��� �������������� ������� ������ �����
    grid1->enableXMin(true);
    // ��������� ���� � ����� ����� ��� �������� �������
    grid1->setMajPen(QPen(Qt::black,0,Qt::DotLine));
    // ��������� ���� � ����� ����� ��� �������������� �������
    grid1->setMinPen(QPen(Qt::gray,0,Qt::DotLine));
    grid1->attach(ui->plot1);   // ����������� ����� � �������

    // ������� ������������ ����� ��� ������� �������
    grid2 = new QwtPlotGrid;
    // ��������� ����� ��� �������������� ������� ������ �����
    grid2->enableXMin(true);
    // ��������� ���� � ����� ����� ��� �������� �������
    grid2->setMajPen(QPen(Qt::black,0,Qt::DotLine));
    // ��������� ���� � ����� ����� ��� �������������� �������
    grid2->setMinPen(QPen(Qt::gray,0,Qt::DotLine));
    grid2->attach(ui->plot2);   // ����������� ����� � �������

    // ������� ������������ ����� ��� ������� �������
    grid3 = new QwtPlotGrid;
    // ��������� ����� ��� �������������� ������� ������ �����
    grid3->enableXMin(true);
    // ��������� ���� � ����� ����� ��� �������� �������
    grid3->setMajPen(QPen(Qt::black,0,Qt::DotLine));
    // ��������� ���� � ����� ����� ��� �������������� �������
    grid3->setMinPen(QPen(Qt::gray,0,Qt::DotLine));
    grid3->attach(ui->plot3);   // ����������� ����� � �������

// ��������� ������� ������ ���� ��������
    // �������� ����� ������ ����� ������� �������
    // (� ��������� ���� ����� �� ������)
    QFont af = ui->plot1->axisFont(QwtPlot::xBottom);
    af.setPointSize(8); // ��������� ����� ������ ������ - 8 �������

    // ������������� ���������� ����� �� ������ ������� ���
    ui->plot1->setAxisFont(QwtPlot::xBottom,af);    // ������ �����
    ui->plot1->setAxisFont(QwtPlot::yLeft,af);      // ����� �����

    // ������������� ���������� ����� �� ������ ������� ���
    ui->plot2->setAxisFont(QwtPlot::xBottom,af);    // ������ �����
    ui->plot2->setAxisFont(QwtPlot::yLeft,af);      // ����� �����

    // ������������� ���������� ����� �� ������� ������� ���
    ui->plot3->setAxisFont(QwtPlot::xBottom,af);    // ������ �����
    ui->plot3->setAxisFont(QwtPlot::yLeft,af);      // ����� �����

    // ������������� ����������� � ������������ �������
        // �� ������ ������� ���
    ui->plot1->setAxisScale(QwtPlot::xBottom,-0.125,8.125); // ������ �����
    ui->plot1->setAxisScale(QwtPlot::yLeft,-0.9,0.9);       // ����� �����
        // �� ������ ������� ���
    ui->plot2->setAxisScale(QwtPlot::xBottom,-0.125,8.125); // ������ �����
    ui->plot2->setAxisScale(QwtPlot::yLeft,-2.5,2.5);       // ����� �����
        // �� ������� ������� ���
    ui->plot3->setAxisScale(QwtPlot::xBottom,-0.125,8.125); // ������ �����
    ui->plot3->setAxisScale(QwtPlot::yLeft,-5,5);           // ����� �����

    // ������� ������ ������ � ������������� "U1(t)"
    curv1 = new QwtPlotCurve(QString("U1(t)"));
    // ����������� ��� ���������� �� ���������, �.�. ����� �� ������ ����� �����
    // ��������� ���� ���������� - �����-�������
    curv1->setPen(QPen(Qt::darkGreen));

    // ������� ������ ������ � ������������� "U2(t)"
    curv2 = new QwtPlotCurve(QString("U2(t)"));
    // ��������� ��� ��� ����������� ��� ����������
    curv2->setRenderHint(QwtPlotItem::RenderAntialiased);
    // ��������� ���� ���������� - �������
    curv2->setPen(QPen(Qt::red));

    // ������� ������� ��� ����� ������ ������
#if QWT_VERSION < 0x060000
    // ���������� ��� ���������� Qwt ������ 5.x.x
    QwtSymbol symbol2;                      // ��������� ��������� ������ QwtSymbol
    symbol2.setStyle(QwtSymbol::Ellipse);   // ��������� �������� ����� � ������
    symbol2.setPen(QColor(Qt::black));      // ���� ���������� � ������
    symbol2.setSize(5);                     // ������ � 5
    curv2->setSymbol(symbol2);              // ����������� ��� � ������
#else
    // ���������� ��� ���������� Qwt ������ 6.x.x
    QwtSymbol *symbol2 = new QwtSymbol();
    symbol2->setStyle(QwtSymbol::Ellipse);
    symbol2->setPen(QColor(Qt::black));
    symbol2->setSize(4);
    curv2->setSymbol(symbol2);
#endif

    // ������� ������ ������ � ������������� "U3(t)"
    curv3 = new QwtPlotCurve(QString("U3(t)"));
    // ����������� ��� ���������� �� ���������, �.�. ����� �� ������ ����� ���������� �����
    // ��������� ���� ���������� - �����-�������
    curv3->setPen(QPen(Qt::blue));

    // ���������� ������
    const int N1 = 262144;  // ��� ������ ������
    const int N2 = 128;     // ��� ������ ������
    const int N3 = 32768;   // ��� ������� ������

    // �������� ���� ������ ��� ���������� ������ �, Y ((����� ��� ���� ��������))
    double *X1 = (double *)malloc((2*N1+2*N2+2*N3)*sizeof(double));
    double *Y1 = X1 + N1;
    double *X2 = Y1 + N1;
    double *Y2 = X2 + N2;
    double *X3 = Y2 + N2;
    double *Y3 = X3 + N3;

    // ���������� ������ ��� ������ ������
    // X1 ��������� �������� �� 0 �� 8
    // ��� ���������� �� X1 ��� ������ ������
    // (������� �� ���������� ����� N1)
    double h = 8./(N1-1);
    for (int k = 0; k < N1; k++)
    {
        X1[k] = k*h;
        // Y1(X1) � ��������� � ��������� ����������, �������� � ��������� �����
        // ���������        0.7
        // �������          4
        // ��������� ����   PI/9
        Y1[k] = 0.7 * cos(8*M_PI*X1[k]+M_PI/9);
    }
    // ���������� ������ ��� ������ ������
    // X2 ��������� �������� �� 0 �� 8
    // ��� ���������� �� X2 ��� ������ ������
    // (������� �� ���������� ����� N2)
    h = 8./(N2-1);
    for (int k = 0; k < N2; k++)
    {
        X2[k] = k*h;
        // Y2(X2) � ��������� � ��������� ����������, �������� � ��������� �����
        // ���������        2
        // �������          0.5
        // ��������� ����   -5*PI/12
        Y2[k] = 2 * cos(M_PI*X2[k]-5*M_PI/12);
    }
    // ���������� ������ ��� ������� ������
    // X3 ��������� �������� �� 0 �� 8
    // ��� ���������� �� X3 ��� ������� ������
    // (������� �� ���������� ����� N3)
    h = 8./(N3-1);
    for (int k = 0; k < N3; k++)
    {
        X3[k] = k*h;
        // Y3(X3) � ��������� � ��������� ����������, �������� � ��������� �����
        // ���������        4
        // �������          1.5
        // ��������� ����   5*PI/7
        Y3[k] = 4 * cos(3*M_PI*X3[k]+5*M_PI/7);
    }

    // �������� ������ �������������� ������
#if QWT_VERSION < 0x060000
    // ���������� ��� ���������� Qwt ������ 5.x.x
    curv1->setData(X1,Y1,N1);
    curv2->setData(X2,Y2,N2);
    curv3->setData(X3,Y3,N3);
#else
    // ���������� ��� ���������� Qwt ������ 6.x.x
    curv1->setSamples(X1,Y1,N1);
    curv2->setSamples(X2,Y2,N2);
    curv3->setSamples(X3,Y3,N3);
#endif

    // ����������� ���������� ������
    free((void *)X1);

    // �������� ������ �� �������
    curv1->attach(ui->plot1);
    curv2->attach(ui->plot2);
    curv3->attach(ui->plot3);

    // ��������� ��� ������� ��� �����
    ui->plot1->canvas()->setCursor(Qt::ArrowCursor);    // ������� �������
    ui->plot2->canvas()->setCursor(Qt::ArrowCursor);    // ������� �������
    ui->plot3->canvas()->setCursor(Qt::ArrowCursor);    // �������� �������

    // ������� �������� ��������������� � ����������� ��������
    zoom = new QChartSynZoom();
    // ������������� ���� �����, �������� ����� ������ ������� - �����
    zoom->setRubberBandColor(Qt::gray);
    // �������� ������ �����
    zoom->setLightMode(true);
    // �������� ��������� ���������
    zoom->indicateDragBand(QChartSynZoom::disDetailed);
    // ������������� ����� ������� ���������� �����������
    zoom->setDragBandColor(Qt::gray);

    // ������ ��� ����� ��������� ���������������
    zoom->appendPlot(ui->plot1);    // ������ ������
    zoom->appendPlot(ui->plot2);    // ������ ������
    zoom->appendPlot(ui->plot3);    // ������ ������

    // ������� ��������� ��������� ����� �� ������ �������
    axzmsvc = new QAxisSynZoomSvc();
    // �������� ��� ������ �����
    axzmsvc->setLightMode(true);
    // ������������� ���� �������, ������������� ��������������� �����
    axzmsvc->setAxisRubberBandColor(Qt::gray);
    // ����������� ��������� � ��������� ���������������
    axzmsvc->attach(zoom);
}

MainWindow::~MainWindow()
{
    // ������� ���, ��� ���� ������� � ������������:
        // �������
    delete leg1;    // ������� �������
    delete leg2;    // ������� �������
    delete leg3;    // �������� �������
        // ������������ �����
    delete grid1;   // ������� �������
    delete grid2;   // ������� �������
    delete grid3;   // �������� �������
        // ������
    delete curv1;   // ������
    delete curv2;   // ������
    delete curv3;   // ������

    delete axzmsvc;
    delete zoom;    // �������� ���������������

// ����������� ����� �����������
    delete ui;
}
