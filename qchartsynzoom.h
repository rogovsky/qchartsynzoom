/**********************************************************/
/*                                                        */
/*                  ����� QChartSynZoom                   */
/*                      ������ 1.6.3                      */
/*                                                        */
/* ������������ ��������� ����������� ���������������     */
/* ���������� �������� QwtPlot � ����� ���������� TChart  */
/* (Delphi, C++Builder).                                  */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

#ifndef QCHARTSYNZOOM_H
#define QCHARTSYNZOOM_H

#include <QSplitter>
#include <QMouseEvent>
#include <QTimer>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>

class QChartZoomItem;   // �������, ���������� �� ���� ������
class QScaleBounds;     // ��������� ������ �����
class QMainSynZoomSvc;  // ��������� ��������������� �������
class QDragSynZoomSvc;  // ��������� ����������� �������

class QChartSynZoom : public QObject
{
    Q_OBJECT

public:
    // ����������� ��� �������� � ���������
    explicit QChartSynZoom(QSplitter *);
    // ����������� ��� ���������� �������
    explicit QChartSynZoom(QwtPlot *);
    // ����������� ��� ���������� �������,
    // ������� ����� �������� �����
    explicit QChartSynZoom();
    // ����������
    ~QChartSynZoom();

    // �������� ���� �������� �������������� �������
    // ctNone - ��� ��������������
    // ctZoom - ��������� ��������
    // ctDrag - ����������� �������
    // ctWheel - ����� Wheel (��������� �������� �� ����� ����
        // ��������� ������ ���� ��� ������� ������� Ctrl)
    // ctVerWheel - ����� VerWheel (��������� �������� �� ������������ ���
        // ��������� ������ ���� ��� ������� ����� ������� Shift)
    // ctHorWheel - ����� HorWheel (��������� �������� �� �������������� ���
        // ��������� ������ ���� ��� ������� ������ ������� Shift)
    // ctAxisHL - ����� ��������� ����� �������
    // ctAxisHR - ����� ��������� ������ �������
    // ctAxisVB - ����� ��������� ������ �������
    // ctAxisVT - ����� ��������� ������� �������
    // (��������, ������������ �� ctNone ��������� ������ ����
    // ������ ����� ��� ������ ������ ����, ������� Ctrl ��� Shift)
    enum QConvType {ctNone,ctZoom,ctDrag,ctWheel,ctVerWheel,ctHorWheel,
                    ctAxisHL,ctAxisHR,ctAxisVB,ctAxisVT};

    // ������� ����� ���������������
    QConvType regim();
    // ������������ ������ ���������������
    void setRegim(QConvType);

    // ���������� �������������� ��������� � ������
    int count();
    // ��������� �� �������������� �������
    QChartZoomItem *at(int);

    // ���������� ������� ��� ����� ���������
    void appendPlot(QwtPlot *);
    // ��������� ����� �����, �������� ����� ������ �������
    void setRubberBandColor(QColor);
    // ���������/���������� ��������� ���������� �������
    void indicateZoom(bool);
    // ���������/���������� ������� ������
    void setLightMode(bool);

    // �������� ������ ��������� ����������� �������
    // imNone - ��� ���������
    // imSimple - ������� ��������� (������ ����� ����� �������)
    // imDetailed - ��������� ��������� (����� ����� � ������������ �����)
    enum QDragIndiStyle {disNone,disSimple,disDetailed};

    // ���������/���������� ��������� ������������ ������� �������
    // (����� ������, ���� ������� ������ �����)
    void indicateDragBand(QDragIndiStyle);

    // ��������� ����� ������� ���������� �����������
    void setDragBandColor(QColor);
    // �������� ������� ������ ������� � �������� ��������
    void fixBoundaries();

    // ���������/���������� ������������� ��������������
    // �������������� ����� ��� ������� � �������� ind
    void setHorSync(bool,int);
    // ���������/���������� ������������� ��������������
    // �������������� ����� ��� ���� ��������
    void setHorSync(bool);
    // ���������/���������� ������������� ��������������
    // ������������ ����� ��� ������� � �������� ind
    void setVerSync(bool,int);
    // ���������/���������� ������������� ��������������
    // ������������ ����� ��� ���� ��������
    void setVerSync(bool);
    // ���������/���������� ������������� �������������� ��������������
    // � ������������ ����� ��� ������� � �������� ind
    void setSync(bool,int);
    // ���������/���������� ������������� �������������� ��������������
    // � ������������ ����� ��� ���� ��������
    void setSync(bool);

    void updateAllPlots();  // ���������� ���� ��������
    // ����������� �������� ������� ������ �������
    // � �������� �������� (���� ���� isbF �������)
    void fixBounds();
    // �������������� �������� ������ �������
    void restBounds(int);
    // ���������� ��������� ������ �������������� ����� ��������
    void setHorizontScale(double,double);
    // ������������ �������� ��������� � ���������
    void synReplot(int);
    // ����������� ���������� ������� � �������������
    void alignRepaint(int);

signals:
    // ������ - �������� ������
    void plotAppended(int);

protected:
    // ���������� ���� �������
    bool eventFilter(QObject *,QEvent *);

private Q_SLOTS:
    // ���� ������� - ��������� ����� �������
    void scaleDivChanged();

private slots:
    // ���� ������� - ��������� ���������
    void splitterMoved();
    // ���� ������� - ���������� ���������� ��������
    void delayedUpdate();

private:
    QObject *mwin;          // ������� ���� ����������
    QObject *splt;          // �������� - ��������� ��������
    QList<QChartZoomItem *> *items; // ������ ���������, ���������� �� �������
    bool light;             // ������ ����� (� ���������� ����������� �������)

    // ������� �������� ������������� ����������
    // �������� ������� �� �������������� ����� ��������
    int mstHorDiv;  // ��������
    int slvHorDiv;  // ��������������

    bool inSync;    // ���� ���������� �����
    QTimer *tick;   // ������ ����� ����� ���������� ����������� ��������

    // ��������� ��������������� �������
    QMainSynZoomSvc *mnzmsvc;
    // ��������� ����������� �������
    QDragSynZoomSvc *drzmsvc;

    QConvType convType;     // ��� �������� �������������� �������
    bool isbF;              // ����, �������� ������������� ��������� �������� ������� �������
                            // ���� isbF �������, �� ��� ������ ������� �� ������ ������� �������
                            // ������� ����� ��������� � ���������� isb_xl,isb_xr,isb_yb,isb_yt,
                            // � ��� ������ ��������������� � ����������� ������ ����� ������������
                            // ������ � ���� ��������

    // ����� ����� ������������
    void common();
    // ����������� �������� ��������
    QObject *generalParent(QObject *);
    // ���������� ����������� ����������
    void initDelayedUpdate();

    // �������� ������ ������ ����� �������������� �����
    QList<int> *getLabelWidths(QwtPlot *,int);
    // ����������� ������� ������ ����� �������������� �����
    int meanLabelWidth(int);
    // ����������� ����������� ������ �������
    int minimumPlotWidth();
    // ���������� ����� �� ���� �������������� ���� ��������
    bool updateOneHorAxis(int,int *);
    // ���������� �������������� ����� ��������
    bool updateHorAxis();
    // ���������� ������������ ����� �������
    bool updateVerAxis(QwtPlot *,int,int *);

    // ������������ �������� �� ���������
    void alignVAxes(int);
};

/**********************************************************/
/*                                                        */
/*                  ����� QChartZoomItem                  */
/*                      ������ 1.2.2                      */
/*                                                        */
/* ������������ �������������� ��������� ���������������  */
/* � ��������� ��������.                                  */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

// ��������� ��������, ����������� �� ���� ������
class QChartZoomItem
{
public:
    // �����������
    explicit QChartZoomItem(QwtPlot *);
    // ����������
    ~QChartZoomItem();

    QwtPlot *plot;          // ��������� ������

    // ������� �������� ������������� ����������
    // �������� ������� �� ������������ ����� ��������
    int mstVerDiv;  // ��������
    int slvVerDiv;  // ��������������

    // �������������� �����
    QwtPlot::Axis masterX;  // ��������
    QwtPlot::Axis slaveX;   // ��������������
    // ������������ �����
    QwtPlot::Axis masterY;  // ��������
    QwtPlot::Axis slaveY;   // ��������������

    // ���������� ������ �����
    // (������������ � ��������������)
    QScaleBounds *isb_x,*isb_y;

    // ���������/���������� ������������� ��������������
    // �������������� ����� ��� ���������� �������
    bool setHSync(bool);
    // ���������/���������� ������������� ��������������
    // ������������ ����� ��� ���������� �������
    bool setVSync(bool);

private:
    // ���������� �������� � �������������� �����
    void allocAxis(int,int,QwtPlot::Axis *,QwtPlot::Axis *);
};

/**********************************************************/
/*                                                        */
/*                   ����� QScaleBounds                   */
/*                      ������ 1.0.1                      */
/*                                                        */
/* �������� �������� ������� �������� ����� � ����������� */
/* ����� �������� � �������������� ������.                */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

class QScaleBounds
{
public:
    // �����������
    explicit QScaleBounds(QwtPlot *,QwtPlot::Axis,QwtPlot::Axis);

    double min,max;     // �������� ������� �������� �����
    double ak,bk;       // ������������, �������� �������� �����������
                        // ����� �������� ������ � ��������������

    bool sync;  // ������� ������������� �������������� ����� � ��������

    // �������� �������� ������ �����
    void fix();
    // ��������� �������� ������ �����
    void set(double,double);
    // �������������� �������� ������ �����
    void rest();
    // ������������� ������ �������������� �����
    void dup();
    // ����������� ������ �� ��������� ����� �� ������
    bool affected(QwtPlot::Axis);
    // ��������� ���������� ������� �� �������� ����� � �������
    void setDiv(QwtScaleDiv *);

private:
    QwtPlot *plot;          // ��������� ������
    QwtPlot::Axis master;   // �������� �����
    QwtPlot::Axis slave;    // ��������������
    bool fixed;             // ������� �������� ������
};

/**********************************************************/
/*                                                        */
/*                 ����� QMainSynZoomSvc                  */
/*                      ������ 1.0.2                      */
/*                                                        */
/* ������������ ��������� ����������� ���������������     */
/* ������� ��� ���� �� �������� ������� ������            */
/* QChartSynZoom.                                         */
/* ������� � ��������� �����, ������� � ������ 1.5.0.     */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

class QMainSynZoomSvc : public QObject
{
    Q_OBJECT

public:
    // �����������
    explicit QMainSynZoomSvc();

    // ������������ ���������� � ��������� ���������������
    void attach(QChartSynZoom *);

    // ��������� ����� �����, �������� ����� ������ �������
    void setRubberBandColor(QColor);
    // ���������/���������� ��������� ���������� �������
    void indicateZoom(bool);

protected:
    // ���������� ���� �������
    bool eventFilter(QObject *,QEvent *);

private slots:
    // ���� ������� - ���������� ������ �������
    void connectPlot(int);

private:
    QChartSynZoom *zoom;    // ��������� �������� ���������������
    QWidget *zwid;          // ������ ��� ����������� ���������� ����������� �������
    QColor zwClr;           // ���� �����, �������� ����� ������ �������
    bool indiZ;             // ���� ��������� ���������� �������
    QCursor tCursor;        // ����� ��� ���������� �������� �������

    int scp_x,scp_y;        // ��������� ������� � ������ ������ ��������������
                            // (� �������� ������������ ����� �������)

    // ���������� ������� ���������� �������
    void showZoomWidget(QRect);

    // ���������� ������� ������� �� ����
    void procMouseEvent(QEvent *,int);

    // ���������� ������� �� ������ ����
    // (��������� ��������� ��������)
    void startZoom(QMouseEvent *,int);
    // ���������� ����������� ����
    // (��������� ����� ������ �������)
    void selectZoomRect(QMouseEvent *,int);
    // ���������� ���������� ������ ����
    // (���������� ��������� ��������)
    void procZoom(QMouseEvent *,int);
};

/**********************************************************/
/*                                                        */
/*                 ����� QDragSynZoomSvc                  */
/*                      ������ 1.0.1                      */
/*                                                        */
/* ������������ ��������� ����������� ����������� ������� */
/* ��� ���� �� �������� ������� ������ QChartSynZoom.     */
/* ������� � ��������� �����, ������� � ������ 1.5.0.     */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

class QDragSynZoomSvc : public QObject
{
    Q_OBJECT

public:
    // �����������
    explicit QDragSynZoomSvc();

    // ������������ ���������� � ��������� ���������������
    void attach(QChartSynZoom *);

    // ���������/���������� ������� ������
    void setLightMode(bool);
    // ���������/���������� ��������� ������������ �������
    // (����� ������, ���� ������� ������ �����)
    void setIndicatorStyle(QChartSynZoom::QDragIndiStyle);
    // ��������� ����� ������� ���������� �����������
    void setDragBandColor(QColor);

protected:
    // ���������� ���� �������
    bool eventFilter(QObject *,QEvent *);

private slots:
    // ���� ������� - ���������� ������ �������
    void connectPlot(int);

private:
    QChartSynZoom *zoom;    // ��������� �������� ���������������
    bool light;             // ������ ����� (� ���������� �����������)
    QChartSynZoom::QDragIndiStyle indiDrB;  // ����� ��������� ����������� �������
    QWidget *zwid;          // ������ ��� ����������� ���������� ����������� �������
    QColor dwClr;           // ���� �������, ������������� ����������� �������
    QCursor tCursor;        // ����� ��� ���������� �������� �������

    double scb_xl,scb_xr;   // ������� ������� ������� �� �������������� ���
                            // � ������ ������ ��������������
    double scb_yb,scb_yt;   // ������� ������� ������� �� ������������ ���
                            // � ������ ������ ��������������
    double cs_kx,cs_ky;     // ������� �������������� ��������� �� ����� ����
                            // (��������� ���������� ��� ����������� �� ���� ������)
    int scp_x,scp_y;        // ��������� ������� � ������ ������ ��������������
                            // (� �������� ������������ ����� �������)

    // ���������� � ����� ���������� ������������ ����� �����
    // ��� ����� �������������� �����
    QRegion addHorTicks(QRegion,QwtScaleDiv::TickType,QChartZoomItem *);
    // ���������� � ����� ���������� �������������� ����� �����
    // ��� ����� ������������ �����
    QRegion addVerTicks(QRegion,QwtScaleDiv::TickType,QChartZoomItem *);
    // ���������� ����������� ���������� �����������
    void showDragWidget(QPoint,int);
    // ���������� ����������� ����������� �������
    void applyDrag(QPoint,int);

    // ���������� ������� �� ����
    void dragMouseEvent(QEvent *,int);

    // ���������� ������� �� ������ ����
    // (��������� ����������� �������)
    void startDrag(QMouseEvent *,int);
    // ���������� ����������� ����
    // (���������� ����������� ��� ����� ������ ��������� �������)
    void procDrag(QMouseEvent *,int);
    // ���������� ���������� ������ ����
    // (���������� ����������� �������)
    void endDrag(QMouseEvent *,int);
};

#endif // QCHARTSYNZOOM_H