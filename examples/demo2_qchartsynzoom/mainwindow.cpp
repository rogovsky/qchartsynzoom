#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qwt_scale_draw.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Стандартная часть конструктора
    ui->setupUi(this);

    QString tss(    // описываем свойства легенды
//            "background-color: white;"
            "border-left-width: -5px;"
            "border-right-width: -9px;"
            "border-top-width: -7px;"
            "border-bottom-width: -4px;"
            "border-style: inset;");

    // создаем легенду для первого графика
    leg1 = new QwtLegend();
    leg1->setItemMode(QwtLegend::ReadOnlyItem); // легенда только для чтения
    leg1->contentsWidget()->setStyleSheet(tss); // устанавливаем свойства легенды
    // вставляем легенду на график
    ui->plot1->insertLegend(leg1,QwtPlot::TopLegend);

    // создаем легенду для второго графика
    leg2 = new QwtLegend();
    leg2->setItemMode(QwtLegend::ReadOnlyItem); // легенда только для чтения
    leg2->contentsWidget()->setStyleSheet(tss); // устанавливаем свойства легенды
    // вставляем легенду на график
    ui->plot2->insertLegend(leg2,QwtPlot::TopLegend);

    // создаем легенду для третьего графика
    leg3 = new QwtLegend();
    leg3->setItemMode(QwtLegend::ReadOnlyItem); // легенда только для чтения
    leg3->contentsWidget()->setStyleSheet(tss); // устанавливаем свойства легенды
    // вставляем легенду на график
    ui->plot3->insertLegend(leg3,QwtPlot::TopLegend);

    // создаем координатную сетку для первого графика
    grid1 = new QwtPlotGrid;
    // разрешаем линии для дополнительных делений нижней шкалы
    grid1->enableXMin(true);
    // назначаем цвет и стиль линий для основных делений
    grid1->setMajPen(QPen(Qt::black,0,Qt::DotLine));
    // назначаем цвет и стиль линий для дополнительных делений
    grid1->setMinPen(QPen(Qt::gray,0,Qt::DotLine));
    grid1->attach(ui->plot1);   // прикрепляем сетку к графику

    // создаем координатную сетку для второго графика
    grid2 = new QwtPlotGrid;
    // разрешаем линии для дополнительных делений нижней шкалы
    grid2->enableXMin(true);
    // назначаем цвет и стиль линий для основных делений
    grid2->setMajPen(QPen(Qt::black,0,Qt::DotLine));
    // назначаем цвет и стиль линий для дополнительных делений
    grid2->setMinPen(QPen(Qt::gray,0,Qt::DotLine));
    grid2->attach(ui->plot2);   // прикрепляем сетку к графику

    // создаем координатную сетку для первого графика
    grid3 = new QwtPlotGrid;
    // разрешаем линии для дополнительных делений нижней шкалы
    grid3->enableXMin(true);
    // назначаем цвет и стиль линий для основных делений
    grid3->setMajPen(QPen(Qt::black,0,Qt::DotLine));
    // назначаем цвет и стиль линий для дополнительных делений
    grid3->setMinPen(QPen(Qt::gray,0,Qt::DotLine));
    grid3->attach(ui->plot3);   // прикрепляем сетку к графику

// Изменение размера шрифта шкал графиков
    // получаем шрифт нижней шкалы первого графика
    // (у остальных шкал такие же шрифты)
    QFont af = ui->plot1->axisFont(QwtPlot::xBottom);
    af.setPointSize(8); // назначаем новый размер шрифта - 8 пунктов

    // устанавливаем измененный шрифт на первом графике для
    ui->plot1->setAxisFont(QwtPlot::xBottom,af);    // нижней шкалы
    ui->plot1->setAxisFont(QwtPlot::yLeft,af);      // левой шкалы

    // устанавливаем измененный шрифт на втором графике для
    ui->plot2->setAxisFont(QwtPlot::xBottom,af);    // нижней шкалы
    ui->plot2->setAxisFont(QwtPlot::yLeft,af);      // левой шкалы

    // устанавливаем измененный шрифт на третьем графике для
    ui->plot3->setAxisFont(QwtPlot::xBottom,af);    // нижней шкалы
    ui->plot3->setAxisFont(QwtPlot::yLeft,af);      // левой шкалы

    // устанавливаем минимальную и максимальную границы
        // на первом графике для
    ui->plot1->setAxisScale(QwtPlot::xBottom,-0.125,8.125); // нижней шкалы
    ui->plot1->setAxisScale(QwtPlot::yLeft,-0.9,0.9);       // левой шкалы
        // на втором графике для
    ui->plot2->setAxisScale(QwtPlot::xBottom,-0.125,8.125); // нижней шкалы
    ui->plot2->setAxisScale(QwtPlot::yLeft,-2.5,2.5);       // левой шкалы
        // на третьем графике для
    ui->plot3->setAxisScale(QwtPlot::xBottom,-0.125,8.125); // нижней шкалы
    ui->plot3->setAxisScale(QwtPlot::yLeft,-5,5);           // левой шкалы

    // создаем первую кривую с наименованием "U1(t)"
    curv1 = new QwtPlotCurve(QString("U1(t)"));
    // сглаживание при прорисовке не разрешаем, т.к. точек на кривой будет много
    // назначаем цвет прорисовки - темно-зеленый
    curv1->setPen(QPen(Qt::darkGreen));

    // создаем вторую кривую с наименованием "U2(t)"
    curv2 = new QwtPlotCurve(QString("U2(t)"));
    // разрешаем для нее сглаживание при прорисовке
    curv2->setRenderHint(QwtPlotItem::RenderAntialiased);
    // назначаем цвет прорисовки - красный
    curv2->setPen(QPen(Qt::red));

    // создаем маркеры для точек второй кривой
#if QWT_VERSION < 0x060000
    // инструкции для библиотеки Qwt версии 5.x.x
    QwtSymbol symbol2;                      // объявляем экземпляр класса QwtSymbol
    symbol2.setStyle(QwtSymbol::Ellipse);   // назначаем маркерам стиль – эллипс
    symbol2.setPen(QColor(Qt::black));      // цвет прорисовки – черный
    symbol2.setSize(5);                     // размер – 5
    curv2->setSymbol(symbol2);              // прикрепляем его к кривой
#else
    // инструкции для библиотеки Qwt версии 6.x.x
    QwtSymbol *symbol2 = new QwtSymbol();
    symbol2->setStyle(QwtSymbol::Ellipse);
    symbol2->setPen(QColor(Qt::black));
    symbol2->setSize(4);
    curv2->setSymbol(symbol2);
#endif

    // создаем третью кривую с наименованием "U3(t)"
    curv3 = new QwtPlotCurve(QString("U3(t)"));
    // сглаживание при прорисовке не разрешаем, т.к. точек на кривой будет достаточно много
    // назначаем цвет прорисовки - темно-зеленый
    curv3->setPen(QPen(Qt::blue));

    // количество данных
    const int N1 = 262144;  // для первой кривой
    const int N2 = 128;     // для второй кривой
    const int N3 = 32768;   // для третьей кривой

    // выделяем блок памяти под размещение данных Х, Y ((общий для всех массивов))
    double *X1 = (double *)malloc((2*N1+2*N2+2*N3)*sizeof(double));
    double *Y1 = X1 + N1;
    double *X2 = Y1 + N1;
    double *Y2 = X2 + N2;
    double *X3 = Y2 + N2;
    double *Y3 = X3 + N3;

    // подготовка данных для первой кривой
    // X1 принимает значения от 0 до 8
    // шаг приращения по X1 для первой кривой
    // (зависит от количества точек N1)
    double h = 8./(N1-1);
    for (int k = 0; k < N1; k++)
    {
        X1[k] = k*h;
        // Y1(X1) – синусоида с заданными амплитудой, частотой и начальной фазой
        // амплитуда        0.7
        // частота          4
        // начальная фаза   PI/9
        Y1[k] = 0.7 * cos(8*M_PI*X1[k]+M_PI/9);
    }
    // подготовка данных для второй кривой
    // X2 принимает значения от 0 до 8
    // шаг приращения по X2 для второй кривой
    // (зависит от количества точек N2)
    h = 8./(N2-1);
    for (int k = 0; k < N2; k++)
    {
        X2[k] = k*h;
        // Y2(X2) – синусоида с заданными амплитудой, частотой и начальной фазой
        // амплитуда        2
        // частота          0.5
        // начальная фаза   -5*PI/12
        Y2[k] = 2 * cos(M_PI*X2[k]-5*M_PI/12);
    }
    // подготовка данных для третьей кривой
    // X3 принимает значения от 0 до 8
    // шаг приращения по X3 для третьей кривой
    // (зависит от количества точек N3)
    h = 8./(N3-1);
    for (int k = 0; k < N3; k++)
    {
        X3[k] = k*h;
        // Y3(X3) – синусоида с заданными амплитудой, частотой и начальной фазой
        // амплитуда        4
        // частота          1.5
        // начальная фаза   5*PI/7
        Y3[k] = 4 * cos(3*M_PI*X3[k]+5*M_PI/7);
    }

    // передаем кривым подготовленные данные
#if QWT_VERSION < 0x060000
    // инструкции для библиотеки Qwt версии 5.x.x
    curv1->setData(X1,Y1,N1);
    curv2->setData(X2,Y2,N2);
    curv3->setData(X3,Y3,N3);
#else
    // инструкции для библиотеки Qwt версии 6.x.x
    curv1->setSamples(X1,Y1,N1);
    curv2->setSamples(X2,Y2,N2);
    curv3->setSamples(X3,Y3,N3);
#endif

    // освобождаем выделенную память
    free((void *)X1);

    // помещаем кривые на графики
    curv1->attach(ui->plot1);
    curv2->attach(ui->plot2);
    curv3->attach(ui->plot3);

    // назначаем тип курсора для канвы
    ui->plot1->canvas()->setCursor(Qt::ArrowCursor);    // первого графика
    ui->plot2->canvas()->setCursor(Qt::ArrowCursor);    // второго графика
    ui->plot3->canvas()->setCursor(Qt::ArrowCursor);    // третьего графика

    // создаем менеджер масштабирования и перемещения графиков
    zoom = new QChartSynZoom();
    // устанавливаем цвет рамки, задающей новый размер графика - серый
    zoom->setRubberBandColor(Qt::gray);
    // включаем легкий режим
    zoom->setLightMode(true);
    // выбираем подробную индикацию
    zoom->indicateDragBand(QChartSynZoom::disDetailed);
    // устанавливаем цвета виджета индикатора перемещения
    zoom->setDragBandColor(Qt::gray);

    // отдаем под опеку менеджера масштабирования
    zoom->appendPlot(ui->plot1);    // первый график
    zoom->appendPlot(ui->plot2);    // второй график
    zoom->appendPlot(ui->plot3);    // третий график

    // создаем интерфейс изменения одной из границ графика
    axzmsvc = new QAxisSynZoomSvc();
    // включаем ему легкий режим
    axzmsvc->setLightMode(true);
    // устанавливаем цвет виджета, индицирующего масштабирование шкалы
    axzmsvc->setAxisRubberBandColor(Qt::gray);
    // прикрепляем интерфейс к менеджеру масштабирования
    axzmsvc->attach(zoom);
}

MainWindow::~MainWindow()
{
    // удаляем все, что было создано в конструкторе:
        // легенду
    delete leg1;    // первого графика
    delete leg2;    // второго графика
    delete leg3;    // третьего графика
        // координатную сетку
    delete grid1;   // первого графика
    delete grid2;   // второго графика
    delete grid3;   // третьего графика
        // кривую
    delete curv1;   // первую
    delete curv2;   // вторую
    delete curv3;   // третью

    delete axzmsvc;
    delete zoom;    // менеджер масштабирования

// Стандартная часть деструктора
    delete ui;
}
