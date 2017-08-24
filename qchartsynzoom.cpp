/**********************************************************/
/*                                                        */
/*            ���������� ������ QChartSynZoom             */
/*                      ������ 1.6.3                      */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

#include "qchartsynzoom.h"

#include <qwt_scale_widget.h>

// ����������� ��� �������� � ���������
QChartSynZoom::QChartSynZoom(QSplitter *sp) :
    QObject(sp)
{
    common();   // ����� ����� ������������

    // �������� ������� ����
    mwin = generalParent(sp);
    // � ��������� ���������� ������� (������ �������)
    mwin->installEventFilter(this);

    splt = sp;  // �������� ��������
    // ��������� ���� ��������� ������� ��������� ���������
    connect(splt,SIGNAL(splitterMoved(int,int)),SLOT(splitterMoved()));
}

// ����������� ��� ���������� �������
QChartSynZoom::QChartSynZoom(QwtPlot *plt) :
    QObject()
{
    common();   // ����� ����� ������������
    // ��������� �������� �� ���������� ������ �������
    appendPlot(plt);
}

// ����������� ��� ���������� �������,
// ������� ����� �������� �����
QChartSynZoom::QChartSynZoom() :
    QObject()
{
    common();   // ����� ����� ������������
}

// ����������
QChartSynZoom::~QChartSynZoom()
{
    // ������� ��������� ������������ �������
    delete drzmsvc;
    // ������� ��������� ��������������� �������
    delete mnzmsvc;
    // ��� ���� �������� ������� �������� ���������������
    for (int k=0; k < items->count(); k++) delete items->at(k);
    // ������� � �������
    items->clear(); delete items;   // ������ ���� ��������
}

// ������� ����� ���������������
QChartSynZoom::QConvType QChartSynZoom::regim() {
    return convType;
}

// ������������ ������ ���������������
void QChartSynZoom::setRegim(QChartSynZoom::QConvType ct) {
    convType = ct;
}

// ���������� �������������� ��������� � ������
int QChartSynZoom::count() {
    return items->count();
}

// ��������� �� �������������� �������
QChartZoomItem *QChartSynZoom::at(int ind)
{
    // ���������� NULL, ���� ������������ ������
    if (ind < 0 || ind >= items->count()) return NULL;
    // (�����) ���������� ��������� �� �������������� �������
    return items->at(ind);
}

// ����� ����� ������������
void QChartSynZoom::common()
{
    mwin = NULL;    // ������� ���� ���� �� ����������
    splt = NULL;    // �������� �����������

    // ������� ������ ���������, ���������� �� ������ ������
    items = new QList<QChartZoomItem *>();

    // ���������� ���� ��� ����, ����� ����� ������ ���������� ��������
    // ������� ������� ������� ���� ������������� � �������� ��������
    isbF = false;
    // ���������� ������� ������
    convType = ctNone;

    inSync = false; // ������� ���� ���������� �����
    tick = NULL;    // � �������� ��������� �� ������ �����
                    // ����� ������ ����������� ��������

    // ������� ��������� ��������������� �������
    mnzmsvc = new QMainSynZoomSvc();
    // � ����������� ��� � ���������
    mnzmsvc->attach(this);

    // ������� ��������� ������������ �������
    drzmsvc = new QDragSynZoomSvc();
    // � ����������� ��� � ���������
    drzmsvc->attach(this);
}

// ����������� �������� ��������
QObject *QChartSynZoom::generalParent(QObject *sp)
{
    // ����� � �������� ����������� �������� ��������
    // (�������� � ������ ������ � ��������� �������)
    QObject *gp = sp;
    // ���������� �������� �� ������� ������
    QObject *tp = gp->parent();
    // ���� �������� �� ������� ������ �� NULL
    while (tp != NULL)
    {
        // �������� �������:
        // ���������� � �������� ����������� �������� �������
        gp = tp;
        // ���������� �������� �� ��������� ������
        tp = gp->parent();
    }
    // ���������� � �������� �������� �������� ����������
    return gp;
}

// ���������� ����������� ����������
void QChartSynZoom::initDelayedUpdate()
{
    // ���� ��� ��� �� �������
    if (tick == NULL)
    {
        // ������� ������ ����� ����� ������ ����������� ��������
        tick = new QTimer(this);
        // ��������� ��� ���� ��������� ������� timeout
        connect(tick,SIGNAL(timeout()),this,SLOT(delayedUpdate()));
        // � �������� ������ �� 20 ��
        tick->start(20);
    }
    else    // ����� (������ ��� ������)
    {
        tick->stop();       // ������������� ������
        tick->start(20);    // � ������ �������� ��� �� 20 ��
    }
}

// ���������� ������� ��� ����� ���������
void QChartSynZoom::appendPlot(QwtPlot *plt)
{
//    plt->replot();  // ������������� ������
    // ���� ������� ���� ��� �� ����������, ��
    if (mwin == NULL)
    {
        // �������� ������� ����
       mwin = generalParent(plt);
       // � ��������� ���������� ������� (������ �������)
       mwin->installEventFilter(this);
    }
    // ������� ��� ������� �������������� �������
    QChartZoomItem *zit = new QChartZoomItem(plt);
    int n = items->count();
    // ��� ���������� ��������, ������� �� �������
    if (n > 0)
    {
        // �������� ��������� �� �������������� ������� ��� ������� �������
        QChartZoomItem *it0 = items->at(0);
        // �������� ����� �������� �������������� ����� ��� ����
        QwtScaleMap sm = it0->plot->canvasMap(it0->masterX);
        // ������������� ��� ������������ ������� ����� �� �������
        // �������������� ����� ��� � � ������� �������
        zit->isb_x->set(sm.s1(),sm.s2());
    }
    plt->replot();  // ������������� ������
    // ��� ���������� ������ ������� �������
    if (n == 0)
    {
        // ���������� ���������� ������� �� �������������� �����
        mstHorDiv = plt->axisMaxMajor(zit->masterX);
        slvHorDiv = plt->axisMaxMajor(zit->slaveX);
    }
    items->append(zit); // ��������� ������� � ������
    // � ��������� ���������� ������� (������ �������)
    plt->installEventFilter(this);
    // ��� ���� ���� �������
    for (int ax=0; ax < QwtPlot::axisCnt; ax++)
    {
        // ��������� ���������� ������� (������ �������)
        plt->axisWidget(ax)->installEventFilter(this);
        // ��������� ���� ��������� ������� ��������� �����
        connect(plt->axisWidget(ax),SIGNAL(scaleDivChanged()),SLOT(scaleDivChanged()));
    }
    // ��������� ���������� ���������� ��������
    initDelayedUpdate();
    // �������� ������ - �������� ������
    emit plotAppended(items->count() - 1);
}

// ��������� ����� �����, �������� ����� ������ �������
void QChartSynZoom::setRubberBandColor(QColor clr) {
    mnzmsvc->setRubberBandColor(clr);
}

// ���������/���������� ��������� ���������� �������
void QChartSynZoom::indicateZoom(bool indi) {
    mnzmsvc->indicateZoom(indi);
}

// ���������/���������� ������� ������
void QChartSynZoom::setLightMode(bool lm)
{
    light = lm; // ���������� ��������
    // � ������������� ��� ��� ���������� QDragSynZoomSvc
    drzmsvc->setLightMode(lm);
}

// ���������/���������� ��������� ������������ ������� �������
// (����� ������, ���� ������� ������ �����)
void QChartSynZoom::indicateDragBand(QDragIndiStyle indi) {
    drzmsvc->setIndicatorStyle(indi);
}

// ��������� ����� ������� ���������� �����������
void QChartSynZoom::setDragBandColor(QColor clr) {
    drzmsvc->setDragBandColor(clr);
}

// �������� ������� ������ ������� � �������� ��������
void QChartSynZoom::fixBoundaries() {
    // ����� ������ ������������ ���� � ��� �����
    // ����������� �� ������������� ����������� �������
    isbF = false;
    // ����������� �������� ������ ���������� � ������ ������
    // ������-���� �������������� ��� ������ fixBounds()
}

// ���������/���������� ������������� ��������������
// �������������� ����� ��� ������� � �������� ind
void QChartSynZoom::setHorSync(bool hs,int ind)
{
    // �������, ���� ������������ ������
    if (ind < 0 || ind >= items->count()) return;
    // ���� ��������� ������������� ����������, ��
    // ������������� ������ ��������� � ����������
    if (items->at(ind)->setHSync(hs)) synReplot(ind);
}

// ���������/���������� ������������� ��������������
// �������������� ����� ��� ���� ��������
void QChartSynZoom::setHorSync(bool hs)
{
    // ���� ��������� ������������� �� ����������
    bool repF = false;
    // ���������� ��� �������
    for (int k=0; k < items->count(); k++)
        // ��������/��������� �������������
        repF |= items->at(k)->setHSync(hs);
    // ���� ��������� ������������� � ������-���� �� �������� ����������,
    // �� ������������� ������ ������ ��������� � ����������
    if (repF) synReplot(0);
}

// ���������/���������� ������������� ��������������
// ������������ ����� ��� ������� � �������� ind
void QChartSynZoom::setVerSync(bool vs,int ind)
{
    // �������, ���� ������������ ������
    if (ind < 0 || ind >= items->count()) return;
    // ���� ��������� ������������� ����������, ��
    // ������������� ������ � �������������
    if (items->at(ind)->setVSync(vs)) alignRepaint(ind);
}

// ���������/���������� ������������� ��������������
// ������������ ����� ��� ���� ��������
void QChartSynZoom::setVerSync(bool vs)
{
    // ���������� ��� �������
    for (int k=0; k < items->count(); k++)
        // ���� ��������� ������������� ������� ����������,
        // �� ������������� ��� � �������������
        if (items->at(k)->setVSync(vs)) alignRepaint(k);
}

// ���������/���������� ������������� �������������� ��������������
// � ������������ ����� ��� ������� � �������� ind
void QChartSynZoom::setSync(bool s,int ind)
{
    // �������, ���� ������������ ������
    if (ind < 0 || ind >= items->count()) return;
    // ��������/��������� ������������� �������������� �����
    bool hRepF = items->at(ind)->setHSync(s);
    // � ������������
    bool vRepF = items->at(ind)->setVSync(s);
    // ���� ���������� ��������� ������������� �������������� �����,
    // �� ������������� ������ ��������� � ����������
    if (hRepF) synReplot(ind);
    // ����� ���� ���������� ��������� ������������� ������������ �����,
    // �� ������������� ������ � �������������
    else if (vRepF) alignRepaint(ind);
}

// ���������/���������� ������������� �������������� ��������������
// � ������������ ����� ��� ���� ��������
void QChartSynZoom::setSync(bool s)
{
    // ���� ��������� ������������� �������������� ����� �� ����������
    bool hRepF = false;
    // ���������� ��� �������
    for (int k=0; k < items->count(); k++)
    {
        // �������� ��������� �� �������������� �������
        QChartZoomItem *it = items->at(k);
        // ��������/��������� ������������� �������������� �����
        hRepF |= it->setHSync(s);
        // � ������������
        bool vRepF = it->setVSync(s);
        // ���� ���������� ��������� ������������� ������������ �����
        // � ��� ���� ��������� ������������� �������������� �����
        // ��� �� ���������� �� � ������ �� ��������, ��
        // ������������� ������ � �������������
        if (vRepF) if (!hRepF) alignRepaint(k);
    }
    // ���� ��������� ������������� �������������� �����
    // � ������-���� �� �������� ����������,
    // �� ������������� ������ ������ ��������� � ����������
    if (hRepF) synReplot(0);
}

// �������� ������ ������ ����� �������������� �����
QList<int> *QChartSynZoom::getLabelWidths(QwtPlot *plt,int xAx)
{
    // �������� �����, �������������� �� �������������� �����
    QFont fnt = plt->axisFont(xAx);

    // �������� ������ �������� ����� �������������� �����
#if QWT_VERSION < 0x060000
    QwtValueList vl = plt->axisScaleDiv(xAx)->ticks(QwtScaleDiv::MajorTick);
#else
    QList<double> vl = plt->axisScaleDiv(xAx)->ticks(QwtScaleDiv::MajorTick);
#endif

    // ������� ������ ������ �����
    QList<int> *res = new QList<int>();
    // ���������� ��� �����
    for (int k=0; k < vl.count(); k++)
        // � ��������� ������ ������ �����
        res->append(plt->axisScaleDraw(xAx)->labelSize(fnt,vl.at(k)).width());
    if (res->count() > 2)   // ���� � ������ ������ 2-� �����
    {
        // ��������������
        int mn = res->at(0);    // ����������� ������
        int mx = mn;            // � ������������
        // ���������� ���������� �����
        for (int k=1; k < res->count(); k++)
        {
            // ����� ������ �����
            int wk = res->at(k);
            // ��������� ��������
            if (wk < mn) mn = wk;   // ����������� ������
            if (wk > mx) mx = wk;   // � ������������
        }
        // ������� � ������ ����������� ������
        int i = res->indexOf(mn);
        // � ���� �����, �� ������� �� �� ������
        if (i >= 0) res->removeAt(i);
        // ������� � ������ ������������ ������
        i = res->indexOf(mx);
        // � ���� �����, �� ������� �� �� ������
        if (i >= 0) res->removeAt(i);
    }
    // ���������� ������ ������ �����
    return res;
}

// ����������� ������� ������ ����� �������������� �����
int QChartSynZoom::meanLabelWidth(int xAx)
{
    // �������������� ������� ������ ������� �� ���� �����
    int res = 0;
    // �������� ������ ������ �����
    QList<int> *lbwds = getLabelWidths(items->at(0)->plot,xAx);
    // ��������� ������ �������� ���� �����
    for (int k=0; k < lbwds->count(); k++) res += lbwds->at(k);
    // � ���������� ������� ������
    if (lbwds->count() > 0) res = floor(res / lbwds->count());
    // ������� ������ ������ �����
    delete lbwds;
    // ���������� ������� ������ ������� �� ���� �����
    return res;
}

// ����������� ����������� ������ �������
int QChartSynZoom::minimumPlotWidth()
{
    // �������������� ����������� �������� ������ �������
    int res = items->at(0)->plot->canvas()->size().width();
    // ��� ���������� ��������
    for (int k=1; k < items->count(); k++)
    {
        // ����� ������ �������
        int wk = items->at(k)->plot->canvas()->size().width();
        // ���� ��� ������ �����������, �� ����� ��
        if (wk < res) res = wk; // � �������� �����������
    }
    // ���������� ����������� �������� ������ �������
    return res;
}

// ���������� ����� �� ���� �������������� ���� ��������
bool QChartSynZoom::updateOneHorAxis(int xAx,int *hDiv)
{
    // ���������� ������� ������ ������� �� ���� �����
    int mwd = meanLabelWidth(xAx);
    // ���������� ����������� �������� ������ �����
    int mnw = minimumPlotWidth();
    const int dw = 48;  // ����������� ���������� ����� �������
    // ����������� ���������� ����� �������� ����� �� ����� �����
    // ��������� ����������� ������ ����� � ������� ������ �������
    // �� ���� ����� (� ��������� �������)
    int dv = floor(mnw/(mwd+dw));
    bool setF = false;  // ���������� ����� ���� ���
    // ���� ����������� ���������� ����� �������� �����
    // �� ����� ���������� � ������� �������, ��
    if (dv > *hDiv)
    {
        // ���� ����� ���������� ��������� ��������, ���
        // ������� ������ ����� ����������� �� 1 � ��� ���� �������
        // �� ���������� �� ����� � ��������� ����������, ��
        // �������������� ����������� ���������� ����� �������� �����
        if (dv*(mwd+dw+1) > mnw) dv--;
        // ���� ���������� ����� ���-���� �����������, ��
        // ������������� ���� - ��������� ���������� �����
        if (dv > *hDiv) setF = true;
    }
    // ���� ����������� ���������� ����� �������� �����
    // �� ����� ���������� � ������� �������, ��
    if (dv < *hDiv)
    {
        // ���� ����� ���������� ��������� ��������, ���
        // ������� ������ ����� ����������� �� 1 � ��� ���� �������
        // � ��������� ���������� �������� ������� ���� ����� �� �����, ��
        // �������������� ����������� ���������� ����� �������� �����
        if (dv*(mwd+dw-1) < mnw) dv++;
        // ���� ���������� ����� ���-���� �����������, ��
        // ������������� ���� - ��������� ���������� �����
        if (dv < *hDiv) setF = true;
    }
    // ���� ��������� ���������� �����, �� ����������
    // ����������� ����������� ���������� ����� �������� �����
    if (setF) *hDiv = dv;   // ��� �������������� ����� ���
    // � ���������� ���� ����������
    return setF;
}

// ���������� �������������� ����� ��������
bool QChartSynZoom::updateHorAxis()
{
    // �������� ��������� �� �������������� �������
    QChartZoomItem *it0 = items->at(0);
    bool mstF = updateOneHorAxis(it0->masterX,&mstHorDiv);
    if (mstF)   // ���� ��������� ���������� �����, ��
        // ��� ���� ��������
        for (int k=0; k < items->count(); k++)
        {
            // �������� ��������� ��
            QChartZoomItem *it = items->at(k);  // �������������� �������
            QwtPlot *plt = it->plot;            // � ������
            // ������������� ����������� ����������� ���������� �����
            // �������� ����� ��� �������� �������������� �����
            plt->setAxisMaxMajor(it->masterX,mstHorDiv);
        }
    bool slvF = updateOneHorAxis(it0->slaveX,&slvHorDiv);
    if (slvF)   // ���� ��������� ���������� �����, ��
        // ��� ���� ��������
        for (int k=0; k < items->count(); k++)
        {
            // �������� ��������� ��
            QChartZoomItem *it = items->at(k);  // �������������� �������
            QwtPlot *plt = it->plot;            // � ������
            // ������������� ����������� ����������� ���������� �����
            // �������� ����� ��� �������������� �������������� �����
            plt->setAxisMaxMajor(it->slaveX,slvHorDiv);
        }
    // ���������� ����� ���� ����������
    return mstF || slvF;
}

// ���������� ������������ ����� �������
bool QChartSynZoom::updateVerAxis(QwtPlot *plt,int yAx,int *vDiv)
{
    // �������� �����, �������������� �� ������������ �����
    QFont fnt = plt->axisFont(yAx);
    // ������ �������� ������� ������� ������������ �����
    double mxl = plt->axisScaleDiv(yAx)->upperBound();

    // ���������� ������ �������, ��������������� ����� �������� ��� �������� ������
#if QWT_VERSION < 0x060000
    QSize szlb = plt->axisScaleDraw(yAx)->labelSize(fnt,mxl);
#else
    QSizeF szlb = plt->axisScaleDraw(yAx)->labelSize(fnt,mxl);
#endif

    // ����������� ���������� ����� �������� ����� �� ����� �����
    // ��������� ������ ����� � ������ ������� (� ��������� �������)
    int dv = floor(plt->canvas()->size().height()/(szlb.height()+8));
    // ���� ����������� �������� �� ��������� � �������������, ��
    if (dv != *vDiv)
    {
        // ������������� ����������� ����������� ���������� �����
        // �������� ����� ��� ������������ �����
        plt->setAxisMaxMajor(yAx,dv);
        *vDiv = dv;     // �������� ��� � ���������� ���������
        return true;    // � ���������� ���� - ����� ����������
    }
    // ����� ���������� ���� - ����� �� �����������
    else return false;
}

// ���������� ���� ��������
void QChartSynZoom::updateAllPlots()
{
    // ���� ��� ����������
    bool repF = false;
    // ����� ���������� ��������
    int n = items->count();
    // ���� ���� ���� ���� ������, ��
    // ��������� �������������� �����
    if (n > 0) repF = updateHorAxis();
    // ��� ���� ��������
    for (int k=0; k < n; k++)
    {
        // �������� ��������� ��
        QChartZoomItem *it = items->at(k);  // �������������� �������
        QwtPlot *plt = it->plot;            // � ������
        // ��������� ������������ �����
        repF |= updateVerAxis(plt,it->masterY,&it->mstVerDiv);
        repF |= updateVerAxis(plt,it->slaveY,&it->slvVerDiv);
    }
// ���� �����-���� �� ���� ������������� ����������
// (�.�. ���������� ������������ ���������� ����� �� �����),
// �� ��������������� ������� ������ (��������� � ����������)
    if (repF) synReplot(0);
}

// ���������� ���� �������
bool QChartSynZoom::eventFilter(QObject *target,QEvent *event)
{
    // ���� ������� ��������� ��� �������� ����,
    if (target == mwin)
        // ���� ���� ���� ���������� �� ������, ��� ���������� ��� �������, ��
        if (event->type() == QEvent::Show ||
            event->type() == QEvent::Resize)
            updateAllPlots();   // ��������� ��� �������
    int ind = -1;   // ���� ������ �� ������
    // ������������� ������ ��������
    for (int k=0; k < items->count(); k++)
        // ���� ������� ��������� ��� ������� �������, ��
        if (target == (QObject *)items->at(k)->plot)
        {
            ind = k;    // ���������� ����� �������
            break;      // ���������� �����
        }
    // ���� ������ ��� ������, ��
    if (ind >= 0)
        // ���� ���������� ������� �������, ��
        if (event->type() == QEvent::Resize)
            updateAllPlots();   // ��������� ��� �������
    // �������� ���������� ������������ ����������� �������
    return QObject::eventFilter(target,event);
}

// ���� ������� - ��������� ����� �������
void QChartSynZoom::scaleDivChanged()
{
    // �������, ���� ��������� ����� ��� ��������������
    if (inSync) return;
    inSync = true;  // ������������� ���� ���������� �����
    int ind = -1;   // ������ ���� �� ������
    int axId = -1;  // �����, ��������������, ����
    // ������������� ������ ��������
    for (int k=0; k < items->count(); k++)
        // ��� ������� ������������� ������ ����
        for (int ax=0; ax < QwtPlot::axisCnt; ax++)
            // ���� ���������� ������ �����, ��
            if (items->at(k)->plot->axisWidget(ax) == sender())
            {
                ind = k;    // ���������� ����� �������,
                axId = ax;  // ����� �����
                break;      // ���������� ����� �����
                break;      // � ����� �������
            }
    if (ind >= 0)   // ���� ������ ��� ������,
    {
        // ���� ���������� �������������� �����, ��
        if (axId == QwtPlot::xBottom || axId == QwtPlot::xTop)
        {
            // �������� ��������� �� ������������ �������������� �������
            QChartZoomItem *it = items->at(ind);
            // ���� ���������� �������� ����� ��� ��������������,
            // �� ��� ���� �������� �������������,��
            if (it->isb_x->affected((QwtPlot::Axis)axId))
                // ������������� ������ ��������
                for (int k=0; k < items->count(); k++)
                    if (k != ind)   // ��� �������, �������������� � ������������
                        // ������������� ���������� ������� �� ����� ����� ��,
                        // ��� �� ������������ �������
                        items->at(k)->isb_x->setDiv(it->plot->axisScaleDiv(axId));
        }
        else // ����� (���������� ������������ �����)
        {
            // ����������� ������� �� ���������
            alignVAxes(QwtPlot::yLeft);     // �����
            alignVAxes(QwtPlot::yRight);    // � ������
        }
    }
    inSync = false; // ������� ���� ���������� �����
}

// ���� ������� - ��������� ���������
void QChartSynZoom::splitterMoved() {
    // ��������� ���������� ���������� ��������
    initDelayedUpdate();
}

// ���� ������� - ���������� ���������� ��������
void QChartSynZoom::delayedUpdate()
{
    // ��������� ���������� ��������
    updateAllPlots();
    // ����������� ������ ����� �� ������� timeout
    tick->disconnect(SIGNAL(timeout()));
    delete tick;    // ������� ������
    tick = NULL;    // � �������� ��������� �� ����
}

// ����������� �������� ������� ������ �������
// � �������� �������� (���� ���� isbF �������)
void QChartSynZoom::fixBounds()
{
    // ���� ����� ��� �� ���� �������
    if (!isbF)
    {
        // ��� ���� ��������
        for (int k=0; k < items->count(); k++)
        {
            // �������� ��������� �� �������������� �������
            QChartZoomItem *it = items->at(k);
            // ��������� �������
            it->isb_x->fix();   // ��������������
            it->isb_y->fix();   // � ������������
        }
        // ������������� ������ �������� ������ �������
        isbF = true;
    }
}

// �������������� �������� ������ �������
void QChartSynZoom::restBounds(int ind)
{
    // �������� ���������� �������������� ��������� � ������
    int n = items->count();
    // �������, ���� ������������ ������
    if (ind < 0 || ind >= n) return;
    // ��� ���� ��������
    for (int k=0; k < n; k++)
        // ��������������� ����������� ����� ����� � ������
        // ������� ����� ��� �������������� ���
        items->at(k)->isb_x->rest();
    // ��������������� ������ � ������� ������� ������������ �����
    items->at(ind)->isb_y->rest();
    // ������������� ������ (��������� � ����������)
    synReplot(ind);
}

// ���������� ��������� ������ �������������� ����� ��������
void QChartSynZoom::setHorizontScale(double xl,double xr)
{
    // ��� ���� ��������
    for (int k=0; k < items->count(); k++)
        // ������������� �������� ������� �������������� �����
        items->at(k)->isb_x->set(xl,xr);
}

// ������������ �������� ��������� � �������������
void QChartSynZoom::synReplot(int ind)
{
    // �������� ���������� �������������� ��������� � ������
    int n = items->count();
    // �������, ���� ������������ ������
    if (ind < 0 || ind >= n) return;
    // ������������� ������ ��������
    for (int k=0; k < items->count(); k++)
        // ���� ������ �� �������� ��������������, �� ������������� ���
        if (k != ind) items->at(k)->plot->replot();
    // ������������� �������������� ������ � �������������
    alignRepaint(ind);
}

// ����������� ������� � �������������
void QChartSynZoom::alignRepaint(int ind)
{
    // �������, ���� ������������ ������
    if (ind < 0 || ind >= items->count()) return;
    // ������������� �������������� ������
    items->at(ind)->plot->replot();
    // ����������� ������� �� ���������
    alignVAxes(QwtPlot::yLeft);     // �����
    alignVAxes(QwtPlot::yRight);    // � ������
}

// ������������ �������� �� ���������
void QChartSynZoom::alignVAxes(int ax)
{
    // �������, ���� ����� � ��������� �� ������������
    if (ax != QwtPlot::yLeft && ax != QwtPlot::yRight) return;

    // ��������� ������ ������� ����������� ������ �����
    // � �������������� ������������ ��������� ������ �����
#if QWT_VERSION < 0x060000
    QList<int> minExt;
    int maxExt = 0;
#else
    QList<double> minExt;
    double maxExt = 0;
#endif

    bool need = false;  // ���� ��� ����������� � ������������
    // ���������� ��� �������
    for (int k=0; k < items->count(); k++)
    {
        // �������� ��������� �� ������
        QwtPlot *plt = items->at(k)->plot;
        // ��������� ����������� �����
        need |= plt->axisEnabled(ax);
        // �������� ��������� �� ������ �����
        QwtScaleWidget *scaleWidget = plt->axisWidget(ax);
        // � �� ����������� �����
        QwtScaleDraw *sd = scaleWidget->scaleDraw();
        // ���������� � ������ ������� ����������� ������ �����
        minExt.append(sd->minimumExtent());
        // � ���������� ��
        sd->setMinimumExtent(0);

        // ��������� ��������� ������ ������ �����
#if QWT_VERSION < 0x060000
        int ext = sd->extent(QPen(Qt::black,scaleWidget->penWidth(),Qt::SolidLine),scaleWidget->font());
#else
        double ext = sd->extent(scaleWidget->font());
#endif

        // ���� ��� ������ ���� ����������, ��
        // ��������� ������������ ��������� ������ �����
        if (ext > maxExt) maxExt = ext;
    }
    // �������, ���� ��� ����������� � ������������
    if (!need) return;
    // ������ ����� 1 ������
    if (ax == QwtPlot::yRight) maxExt += 1;
    // ��� ���� ��������
    for (int k=0; k < items->count(); k++)
    {
        // �������� ��������� �� ������
        QwtPlot *plt = items->at(k)->plot;
        // �������� ��������� �� ������ �����
        QwtScaleWidget *scaleWidget = plt->axisWidget(ax);

        // ����� ������� ����������� ������ �����
#if QWT_VERSION < 0x060000
        int mE = minExt.at(k);
#else
        double mE = minExt.at(k);
#endif

        // ���� ��� �� ��������� � ���������, ��
        if (mE != maxExt)
        {
            // ������������� ����� �������� ����������� ������ �����
            scaleWidget->scaleDraw()->setMinimumExtent(maxExt);
            // � ��������� ������� ��� �������
            plt->updateLayout();
        }
        // ����� ��������������� ������� �������� ����������� ������ �����
        else scaleWidget->scaleDraw()->setMinimumExtent(mE);
    }
}

/**********************************************************/
/*                                                        */
/*            ���������� ������ QChartZoomItem            */
/*                      ������ 1.2.2                      */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

// �����������
QChartZoomItem::QChartZoomItem(QwtPlot *qp)
{
    // �������� ������, ��� ������� ����� ������������� ��� ��������������
    plot = qp;
    // ������������� ��� ��������, ����������� ������������ ������� �� ����������
    qp->setFocusPolicy(Qt::StrongFocus);

    // ������������ �����
    QwtPlotGrid *grid = NULL;
    // ���, � ������� ��� �����������
    int xAx;    // ��������������
    int yAx;    // ������������
    // �������� ������ ��������� �������
    QwtPlotItemList pil = qp->itemList();
    // ���������� ������ ���������
    for (int k=0; k < pil.count(); k++)
    {
        // �������� ��������� �� �������
        QwtPlotItem *pi = pil.at(k);
        // ���� ��� ������������ �����, ��
        if (pi->rtti() == QwtPlotItem::Rtti_PlotGrid)
        {
            // ���������� ��������� �� ���
            grid = (QwtPlotGrid *)pi;
            // �������� � ����� ��� ��� �����������
            xAx = grid->xAxis();    // �� ���� ��������������
            yAx = grid->yAxis();    // � ���� ������������
            // ���������� �������� ������ ���������
            break;
        }
    }
    // ���� ������������ ����� ���� �������, ��
    if (grid != NULL)
    {
        // ��������� �������� � �������������� �����, ������� ������������
        // ���, � ������� ����������� �����
            // ��������������
        allocAxis(xAx,QwtPlot::xBottom + QwtPlot::xTop - xAx,&masterX,&slaveX);
            // ������������
        allocAxis(yAx,QwtPlot::yLeft + QwtPlot::yRight - yAx,&masterY,&slaveY);
    }
    else    // ����� (������������ ����� �����������)
    {
        // ��������� �������� � �������������� �����, ������� ������������
            // ������ �� ��������������
        allocAxis(QwtPlot::xBottom,QwtPlot::xTop,&masterX,&slaveX);
            // � ����� �� ������������
        allocAxis(QwtPlot::yLeft,QwtPlot::yRight,&masterY,&slaveY);
    }
    // ���������� ������� ������������ ���������� �������� ������� �� ������������ �����
    mstVerDiv = plot->axisMaxMajor(masterY);    // ��������
    slvVerDiv = plot->axisMaxMajor(slaveY);     // � ��������������
    // ������� ���������� ������ �����
    isb_x = new QScaleBounds(qp,masterX,slaveX);    // ��������������
    isb_y = new QScaleBounds(qp,masterY,slaveY);    // � ������������
    qp->replot();   // ������������� ������
}

// ����������
QChartZoomItem::~QChartZoomItem()
{
    // ������� ���������� ������ �����
    delete isb_x;    // ��������������
    delete isb_y;    // � ������������
}

// ���������/���������� ������������� ��������������
// �������������� ����� ��� ���������� �������
bool QChartZoomItem::setHSync(bool hs)
{
    // �������, ���� ��������� ��������� �������������
    // �������������� �������������� ����� ��� �����������
    if (isb_x->sync == hs) return false;
    // ���������� ����� ��������� �������������
    isb_x->sync = hs;
    // ����������������� ������� �������������� �����
    isb_x->dup();
    // ���������� �������, ��� ���������
    return true;    // ������������� ����������
}

// ���������/���������� ������������� ��������������
// ������������ ����� ��� ���������� �������
bool QChartZoomItem::setVSync(bool vs)
{
    // �������, ���� ��������� ��������� �������������
    // �������������� ������������ ����� ��� �����������
    if (isb_y->sync == vs) return false;
    // ���������� ����� ��������� �������������
    isb_y->sync = vs;
    // ����������������� ������� �������������� �����
    isb_y->dup();
    // ���������� �������, ��� ���������
    return true;    // ������������� ����������
}

// ���������� �������� � �������������� �����
void QChartZoomItem::allocAxis(int pre,int alt,
    QwtPlot::Axis *master,QwtPlot::Axis *slave)
{
    // �������� ����� ���������������� �����
    QwtScaleMap smp = plot->canvasMap(pre); // ���������������� �����
    QwtScaleMap sma = plot->canvasMap(alt); // � ��������������
    // ���� ���������������� ����� �������� ���
    // �������������� ����� ���������� � ��� ����
    // ������� ���������������� ����� �� ��������� ���
    // ������� �������������� ����� ���������, ��
    if ((plot->axisEnabled(pre) ||
        !plot->axisEnabled(alt)) &&
        (smp.s1() != smp.s2() ||
         sma.s1() == sma.s2()))
    {
        // ��������� ���������������� ����� ��������,
        *master = (QwtPlot::Axis)pre;
        // � �������������� ��������������
        *slave = (QwtPlot::Axis)alt;
    }
    else    // �����
            // (���������������� ����� ���������� �
            // �������������� ����� �������� ���
            // ������� ���������������� ����� ��������� �
            // ������� �������������� ����� �� ���������)
    {
        // ��������� �������������� ����� ��������,
        *master = (QwtPlot::Axis)alt;
        // � ���������������� ��������������
        *slave = (QwtPlot::Axis)pre;
    }
}

/**********************************************************/
/*                                                        */
/*             ���������� ������ QScaleBounds             */
/*                      ������ 1.0.1                      */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

// �����������
QScaleBounds::QScaleBounds(QwtPlot *plt,
    QwtPlot::Axis mst,QwtPlot::Axis slv)
{
    // ����������
    plot = plt;     // ��������� ������
    master = mst;   // �������� �����
    slave = slv;    // � ��������������
    fixed = false;  // ������� ��� �� �����������
    sync = false;   // ������������� �� ��������� ���
}

// �������� �������� ������ �����
void QScaleBounds::fix()
{
    // �������� ����� �������� �����
    QwtScaleMap sm = plot->canvasMap(master);
    // � ���������� ������� ����� � ������ ������� �����
    min = sm.s1(); max = sm.s2();
    // �������� ����� �������������� �������������� �����
    sm = plot->canvasMap(slave);
    // ������ �������������� �������� ����� � �������������� � ����
    //     s = a * m + b, ���:
    // ���� ������� �������� ����� �� ���������, ��
    if (min != max)
    {
        // a = (s2 - s1) / (m2 - m1)
        ak = (sm.s2() - sm.s1()) / (max - min);
        // b = (m2*s1 - m1*s2) / (m2 - m1)
        bk = (max * sm.s1() - min * sm.s2()) / (max - min);
    }
    else    // ����� (������� �������� ����� ���������,
            // ������ � �������������� ����)
    {
        // a = 0
        ak = 0;
        // b = s1
        bk = sm.s1();
    }
    fixed = true;   // ������� �����������
}

// ��������� �������� ������ �����
void QScaleBounds::set(double mn,double mx)
{
    // ���� ������� ��� �� �����������, ��������� ��
    if (!fixed) fix();
    // ������������� ������ � ������� ������� �����
    plot->setAxisScale(master,mn,mx);   // ��������
                                        // � ��������������
    if (sync) plot->setAxisScale(slave,mn,mx);
    else plot->setAxisScale(slave,ak*mn+bk,ak*mx+bk);
}

// �������������� �������� ������ �����
void QScaleBounds::rest() {
    // ���� ������� ��� �����������, �� ��������������� ��������
    if (fixed) set(min,max);
}

// ������������� ������ �������������� �����
void QScaleBounds::dup()
{
    // ���� ������� ��� �� �����������, ��������� ��
    if (!fixed) fix();
    // �������� ����� �������� �����
    QwtScaleMap sm = plot->canvasMap(master);
    // � ������������� ������� ��� ��������������
    if (sync) plot->setAxisScale(slave,sm.s1(),sm.s2());
    else plot->setAxisScale(slave,ak*sm.s1()+bk,ak*sm.s2()+bk);
}

// ����������� ������ �� ��������� ����� �� ������
bool QScaleBounds::affected(QwtPlot::Axis ax)
{
    // ���� ����� ��������, �� ������
    if (ax == master) return true;
    // ���� ����� �������������� � �������� �������������, �� ������
    if (ax == slave && sync) return true;
    // ����� (�������������� � ������������� ���������) �� ������
    return false;
}

// ��������� ���������� ������� �� ����� � �������
void QScaleBounds::setDiv(QwtScaleDiv *sdv)
{
    // ������������� ���������� ������� ��� �������� �����,
    plot->setAxisScaleDiv(master,*sdv);
    // � ���� ����� ����� �������������, �� � ��� ��������������
    if (sync) plot->setAxisScaleDiv(slave,*sdv);
}

/**********************************************************/
/*                                                        */
/*           ���������� ������ QMainSynZoomSvc            */
/*                      ������ 1.0.2                      */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

// �����������
QMainSynZoomSvc::QMainSynZoomSvc() :
    QObject()
{
    // ������� ������, ���������� �� ����������� ���������� �������
    zwid = 0;
    // � ��������� ��� ���� (�� ��������� - ������)
    zwClr = Qt::black;
    // �� ��������� ��������� ������� ������������
    indiZ = true;
}

// ������������ ���������� � ��������� ���������������
void QMainSynZoomSvc::attach(QChartSynZoom *zm)
{
    // ���������� ��������� �� �������� ���������������
    zoom = zm;
    // ��� ���� �������� ��������� �������� �� ���������� ������ �������
    for (int k=0; k < zoom->count(); k++) connectPlot(k);
    // ��������� ���� ������� - ���������� ������ �������
    connect(zm,SIGNAL(plotAppended(int)),SLOT(connectPlot(int)));
}

// ��������� ����� �����, �������� ����� ������ �������
void QMainSynZoomSvc::setRubberBandColor(QColor clr) {
    zwClr = clr;
}

// ���������/���������� ��������� ���������� �������
void QMainSynZoomSvc::indicateZoom(bool indi) {
    indiZ = indi;
}

// ���� ������� - ���������� ������ �������
void QMainSynZoomSvc::connectPlot(int ind)
{
    // ��������� ���������� ������� (������ �������)
    zoom->at(ind)->plot->installEventFilter(this);
}

// ���������� ���� �������
bool QMainSynZoomSvc::eventFilter(QObject *target,QEvent *event)
{
    int ind = -1;   // ���� ������ �� ������
    // ������������� ������ ��������
    for (int k=0; k < zoom->count(); k++)
        // ���� ������� ��������� ��� ������� �������, ��
        if (target == (QObject *)zoom->at(k)->plot)
        {
            ind = k;    // ���������� ����� �������
            break;      // ���������� �����
        }
    // ���� ������ ��� ������, ��
    if (ind >= 0)
    {
        // ���� ��������� ���� �� ������� �� ����, ��
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)
            procMouseEvent(event,ind);  // �������� ��������������� ����������
    }
    // �������� ���������� ������������ ����������� �������
    return QObject::eventFilter(target,event);
}

// ���������� ������� ���������� �������
void QMainSynZoomSvc::showZoomWidget(QRect zr)
{
    // ������������� ��������� � ������� �������, ������������� ���������� �������
    zwid->setGeometry(zr);
    // ���������� ��� ��������
    int dw = zr.width();    // ������ �������
    int dh = zr.height();   // � ������
    // ��������� ����� ��� �������, ������������� ���������� �������
    QRegion rw(0,0,dw,dh);      // ������������ �������
    QRegion rs(1,1,dw-2,dh-2);  // ���������� �������
    // ������������� ����� ����� ��������� �� ������������ ������� ����������
    zwid->setMask(rw.subtracted(rs));
    // ������ ������, ������������ ���������� �������, �������
    zwid->setVisible(true);
    // �������������� ������
    zwid->repaint();
}

// ���������� ������� ������� �� ����
void QMainSynZoomSvc::procMouseEvent(QEvent *event,int k)
{
    // ������� ��������� �� ������� �� ����
    QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
    // � ����������� �� ���� ������� �������� ��������������� ����������
    switch (event->type())
    {
        // ������ ������ ����
    case QEvent::MouseButtonPress: startZoom(mEvent,k); break;
        // ����������� ����
    case QEvent::MouseMove: selectZoomRect(mEvent,k); break;
        // �������� ������ ����
    case QEvent::MouseButtonRelease: procZoom(mEvent,k); break;
        // ��� ������ ������� ������ �� ������
    default: ;
    }
}

// ���������� ������� �� ������ ����
// (��������� ��������� ��������)
void QMainSynZoomSvc::startZoom(QMouseEvent *mEvent,int ind)
{
    // ��������� �������� ������� ������� (���� ����� ��� �� ���� �������)
    zoom->fixBounds();
    // ���� � ������ ������ ��� �� ������� �� ���� �� �������
    if (zoom->regim() == QChartSynZoom::ctNone)
    {
        // �������� ��������� ��
        QwtPlot *plt = zoom->at(ind)->plot; // ������
        QwtPlotCanvas *cv = plt->canvas();  // � �����
        // �������� ��������� ����� �������
        QRect cg = cv->geometry();
        // ���������� ������� ��������� ������� (������������ ����� �������)
        scp_x = mEvent->pos().x() - cg.x();
        scp_y = mEvent->pos().y() - cg.y();
        // ���� ������ ��������� ��� ������ �������
        if (scp_x >= 0 && scp_x < cg.width() &&
            scp_y >= 0 && scp_y < cg.height())
            // ���� ������ ����� ������ ����, ��
            if (mEvent->button() == Qt::LeftButton)
            {
                // ����������� ��������������� ������� ������
                zoom->setRegim(QChartSynZoom::ctZoom);
                // ���������� ������� ������
                tCursor = cv->cursor();
                // ������������� ������ Cross
                cv->setCursor(Qt::CrossCursor);
                // ���� �������� ���������, ��
                if (indiZ)
                {
                    // ������� ������, ������� ����� ���������� ���������� �������
                    // (�� ����� ��������������� �� ��� �� �������, ��� � ������)
                    zwid = new QWidget(plt->parentWidget());
                    // � ��������� ��� ����
                    zwid->setStyleSheet(QString(
                        "background-color:rgb(%1,%2,%3);").arg(
                        zwClr.red()).arg(zwClr.green()).arg(zwClr.blue()));
                }
            }
    }
}

// ���������� ����������� ����
// (��������� ����� ������ �������)
void QMainSynZoomSvc::selectZoomRect(QMouseEvent *mEvent,int ind) {
    // ���� ������� ����� ��������� �������� � ���������, ��
    if (zoom->regim() == QChartSynZoom::ctZoom && indiZ)
    {
        // �������� ��������� �� ������
        QwtPlot *plt = zoom->at(ind)->plot;
        // �������� ��������� �������
        QRect pg = plt->geometry();
        // � ��������� ����� �������
        QRect cg = plt->canvas()->geometry();
        // scp_x - ���������� ������� � �������� �� �������������� ���
        //     � ��������� ������ ������� (����� ���� ������ ����� ������ ����)
        // mEvent->pos().x() - cg.x() - ���������� ������� � ��������
        //     �� �������������� ��� � ������� ������ �������
        // mEvent->pos().x() - cg.x() - scp_x - �������� ������� � ��������
        //     �� �������������� ��� �� ���������� ��������� � ��������������
        //     ������ dx ���������� �������
        int dx = mEvent->pos().x() - cg.x() - scp_x;
        // pg.x() - ��������� ������� �� �������������� ���
        //     ������������ �������, ��� �����������
        // pg.x() + cg.x() - ��������� ����� ������� �� �������������� ���
        //     ������������ �������, ��� �����������
        // pg.x() + cg.x() + scp_x - ��������� gx0 ��������� ����� �� �������������� ���
        //     ������������ �������, ����������� ������, ��� ����� � �������� �����
        //     ��� ����������� ���������� �������
        int gx0 = pg.x() + cg.x() + scp_x;
        // ���� ������ ���������� ������� ������������, �� ������� ����� ��������� ����� ���������,
        //     � ����� ������ �� �� ���������� � �������� ����� ��� ����������� ���������� �������
        if (dx < 0) {dx = -dx; gx0 -= dx;}
        // ����� ���� ������ ����� ����, �� ��� ���� ����� ���������� ������� ���-���� �����������,
        //     ������������� ������� �� ������ �������
        else if (dx == 0) dx = 1;
        // ���������� ���������� ������ dy ���������� �������
        int dy = mEvent->pos().y() - cg.y() - scp_y;
        // � ��������� gy0 ��������� ����� �� ������������ ���
        int gy0 = pg.y() + cg.y() + scp_y;
        // ���� ������ ���������� ������� ������������, �� ������� ����� ��������� ���� ���������,
        //     � ����� ������ �� �� ���������� � �������� ����� ��� ����������� ���������� �������
        if (dy < 0) {dy = -dy; gy0 -= dy;}
        // ����� ���� ������ ����� ����, �� ��� ���� ����� ���������� ������� ���-���� �����������,
        //     ������������� ������� �� ������ �������
        else if (dy == 0) dy = 1;
        // ���������� ���������� �������
        showZoomWidget(QRect(gx0,gy0,dx,dy));
    }
}

// ���������� ���������� ������ ����
// (���������� ��������� ��������)
void QMainSynZoomSvc::procZoom(QMouseEvent *mEvent,int ind)
{
    // ���� ������� ����� ��������� �������� ��� ����� ����������� �������
    if (zoom->regim() == QChartSynZoom::ctZoom)
        // ���� �������� ����� ������ ����, ��
        if (mEvent->button() == Qt::LeftButton)
        {
            // �������� ��������� ��
            QChartZoomItem *it = zoom->at(ind); // �������������� �������
            QwtPlot *plt = it->plot;            // ������
            QwtPlotCanvas *cv = plt->canvas();  // � �����
            // ��������������� ������
            cv->setCursor(tCursor);
            // ���� �������� ���������, �� ������� ������,
            if (indiZ) delete zwid; // ������������ ���������� �������
            // �������� ��������� ����� �������
            QRect cg = cv->geometry();
            // ���������� ��������� �������, �.�. ���������� xp � yp
            // �������� ����� ���������� ������� (� �������� ������������ ����� QwtPlot)
            int xp = mEvent->pos().x() - cg.x();
            int yp = mEvent->pos().y() - cg.y();
            // ���� ��������� ������������� ������ ������ ��� ����� �����,
            // �� ��������������� �������� ������� ������� (�������� ����������)
            if (xp < scp_x || yp < scp_y) zoom->restBounds(ind);
            // ����� ���� ������ ���������� ������� ����������, �� �������� �������
            else if (xp - scp_x >= 8 && yp - scp_y >= 8)
            {
                // ������������� ����� ������� �������������� �����
                // �� ��������� �����, � ������ �� ��������
                zoom->setHorizontScale(plt->invTransform(it->masterX,scp_x),
                    plt->invTransform(it->masterX,xp));
                // �������� �������� ������������ �����
                QwtPlot::Axis mY = it->masterY;
                // ���������� ������ ������� ������������ ����� �� �������� �����
                double bt = plt->invTransform(mY,yp);
                // ���������� ������� ������� ������������ ����� �� ��������� �����
                double tp = plt->invTransform(mY,scp_y);
                // ������������� ������ � ������� ������� ������������ �����
                it->isb_y->set(bt,tp);
                // ������������� ������ (��������� � ����������)
                zoom->synReplot(ind);
            }
            // ������� ������� ������
            zoom->setRegim(QChartSynZoom::ctNone);
        }
}

/**********************************************************/
/*                                                        */
/*           ���������� ������ QDragSynZoomSvc            */
/*                      ������ 1.0.1                      */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

// �����������
QDragSynZoomSvc::QDragSynZoomSvc() :
    QObject()
{
    // �� ��������� ������ ����� ��������
    light = false;
    // ������� ������, ���������� �� ����������� ���������� �����������
    zwid = 0;
    // � ��������� ��� ���� (�� ��������� ������)
    dwClr = Qt::black;
    // �� ��������� ������� ������� ����� ��������� ����������� �������
    // (�� ������ � ��� ������, ���� ������� ������ �����)
    indiDrB = QChartSynZoom::disSimple;
}

// ������������ ���������� � ��������� ���������������
void QDragSynZoomSvc::attach(QChartSynZoom *zm)
{
    // ���������� ��������� �� �������� ���������������
    zoom = zm;
    // ��� ���� �������� ��������� �������� �� ���������� ������ �������
    for (int k=0; k < zoom->count(); k++) connectPlot(k);
    // ��������� ���� ������� - ���������� ������ �������
    connect(zm,SIGNAL(plotAppended(int)),SLOT(connectPlot(int)));
}

// ���������/���������� ������� ������
void QDragSynZoomSvc::setLightMode(bool lm) {
    light = lm;
}

// ��������� ����� ������� ���������� �����������
void QDragSynZoomSvc::setDragBandColor(QColor clr) {
    dwClr = clr;
}

// ���������/���������� ��������� ������������ �������
// (����� ������, ���� ������� ������ �����)
void QDragSynZoomSvc::setIndicatorStyle(QChartSynZoom::QDragIndiStyle indi) {
    indiDrB = indi;
}

// ���� ������� - ���������� ������ �������
void QDragSynZoomSvc::connectPlot(int ind)
{
    // ��������� ���������� ������� (������ �������)
    zoom->at(ind)->plot->installEventFilter(this);
}

// ���������� ���� �������
bool QDragSynZoomSvc::eventFilter(QObject *target,QEvent *event)
{
    int ind = -1;   // ������ ���� �� ������
    // ������������� ������ ��������
    for (int k=0; k < zoom->count(); k++)
        // ���� ������� ��������� ��� ������ �������, ��
        if (target == zoom->at(k)->plot)
        {
            ind = k;    // ���������� ����� �������,
            break;      // ���������� ����� �������
        }
    // ���� ������ ��� ������, ��
    if (ind >= 0)
    {
        // ���� ��������� ���� �� ������� �� ����, ��
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)
            dragMouseEvent(event,ind);  // �������� ��������������� ����������
    }
    // �������� ���������� ������������ ����������� �������
    return QObject::eventFilter(target,event);
}

// ���������� � ����� ���������� ������������ ����� ����� ��� ����� �������������� �����
QRegion QDragSynZoomSvc::addHorTicks(QRegion rw,QwtScaleDiv::TickType tt,QChartZoomItem *it)
{
    // �������� ��������� �� ������
    QwtPlot *plt = it->plot;
    // �������� ������ �������� ����� �������������� �����
#if QWT_VERSION < 0x060000
    QwtValueList vl = plt->axisScaleDiv(it->masterX)->ticks(tt);
#else
    QList<double> vl = plt->axisScaleDiv(it->masterX)->ticks(tt);
#endif

    // ���������� ��� ����� �������������� �����
    for (int k=0; k < vl.count(); k++)
    {
        // ��������� �������� ����� ������������ �����
        int x = plt->transform(it->masterX,vl.at(k));
        // ��������� ������������ ����� �����
        QRegion rs(x-1,1,1,rw.boundingRect().height()-2);
        // ��������� �� � �����
        rw = rw.united(rs);
    }
    // ���������� ���������� �����
    return rw;
}

// ���������� � ����� ���������� �������������� ����� ����� ��� ����� ������������ �����
QRegion QDragSynZoomSvc::addVerTicks(QRegion rw,QwtScaleDiv::TickType tt,QChartZoomItem *it)
{
    // �������� ��������� �� ������
    QwtPlot *plt = it->plot;
    // �������� ������ �������� ����� ������������ �����
#if QWT_VERSION < 0x060000
    QwtValueList vl = plt->axisScaleDiv(it->masterY)->ticks(tt);
#else
    QList<double> vl = plt->axisScaleDiv(it->masterY)->ticks(tt);
#endif

    // ���������� ��� ����� ������������ �����
    for (int k=0; k < vl.count(); k++)
    {
        // ��������� �������� ����� ������������ �����
        int y = plt->transform(it->masterY,vl.at(k));
        // ��������� �������������� ����� �����
        QRegion rs(1,y-1,rw.boundingRect().width()-2,1);
        // ��������� �� � �����
        rw = rw.united(rs);
    }
    // ���������� ���������� �����
    return rw;
}

// ���������� ����������� ���������� �����������
void QDragSynZoomSvc::showDragWidget(QPoint evpos,int ind)
{
    // �������� ��������� ��
    QChartZoomItem *it = zoom->at(ind); // �������������� �������
    QwtPlot *plt = it->plot;            // � ������
    // �������� ��������� �������
    QRect pg = plt->geometry();             // �������
    QRect cg = plt->canvas()->geometry();   // � ����� �������
    // ���������� ��� ��������
    int ww = cg.width() - 2;    // ������ �����
    int wh = cg.height() - 2;   // � ������
    // ��������� ��������� � ������ ����� �����
    QRect wg(pg.x()+1+evpos.x()-scp_x,pg.y()+1+evpos.y()-scp_y,ww,wh);
    // ������������� ��������� � ������� ������� ����������
    zwid->setGeometry(wg);
    // ��������� ������������ ����� ��� ������� ����������
    QRegion rw(0,0,ww,wh);      // ������������ �������
    QRegion rs(1,1,ww-2,wh-2);  // ���������� �������
    // ��������� ����� ����� ��������� �� ������������ ������� ����������
    rw = rw.subtracted(rs);
    // ���� ������� ��������� ����� ���������, ��
    if (indiDrB == QChartSynZoom::disDetailed)
    {
        // ��������� � ����� ������������ ����� ����� ��� ������� �������������� �����
        rw = addHorTicks(rw,QwtScaleDiv::MajorTick,it);     // ��������
        rw = addHorTicks(rw,QwtScaleDiv::MediumTick,it);    // �������
        rw = addHorTicks(rw,QwtScaleDiv::MinorTick,it);     // �����������
        // ��������� � ����� �������������� ����� ����� ��� �������� �������
        rw = addVerTicks(rw,QwtScaleDiv::MajorTick,it); // ������������ �����
    }
    // ������������� �����
    zwid->setMask(rw);
    // ������ ������, ������������ ����������� �������, �������
    zwid->setVisible(true);
    // �������������� ������
    zwid->repaint();
}

// ���������� ����������� ����������� �������
void QDragSynZoomSvc::applyDrag(QPoint evpos,int ind)
{
    // �������� ��������� ��
    QChartZoomItem *it = zoom->at(ind); // �������������� �������
    QwtPlot *plt = it->plot;            // � ������
    // �������� ��������� ����� �������
    QRect cg = plt->canvas()->geometry();
    // scp_x - ���������� ������� � �������� �� �������������� ���
    //     � ��������� ������ ������� (����� ���� ������ ������ ������ ����)
    // evpos.x() - cg.x() - ���������� �������
    //     � �������� �� �������������� ��� � ������� ������ �������
    // evpos.x() - cg.x() - scp_x - �������� ������� � ��������
    //     �� �������������� ��� �� ���������� ���������
    // (evpos.x() - cg.x() - scp_x) * cs_kx -  ��� �� ��������,
    //     �� ��� � �������� �������������� �����
    // dx - �������� ������ �� �������������� ��� ������� � �������� ������
    //     (����� ������ ������������ ������ ������������ ������, ���� ������� ������� �������� �����)
    double dx = -(evpos.x() - cg.x() - scp_x) * cs_kx;
    // ������������� ����� ����� � ������ ������� ����� ��� �������������� ���
    //     ����� ������� = ��������� ������� + ��������
    zoom->setHorizontScale(scb_xl + dx,scb_xr + dx);
    // ���������� ���������� dy - �������� ������ �� ������������ ���
    double dy = -(evpos.y() - cg.y() - scp_y) * cs_ky;
    // ������������� ����� ������ � ������� ������� ������������ �����
    it->isb_y->set(scb_yb + dy,scb_yt + dy);
    // ������������� ������ (��������� � ����������)
    zoom->synReplot(ind);
}

// ���������� ������� �� ����
void QDragSynZoomSvc::dragMouseEvent(QEvent *event,int k)
{
    // ������� ��������� �� ������� �� ����
    QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
    // � ����������� �� ���� ������� �������� ��������������� ����������
    switch (event->type())
    {
        // ������ ������ ����
    case QEvent::MouseButtonPress: startDrag(mEvent,k); break;
        // ����������� ����
    case QEvent::MouseMove: procDrag(mEvent,k); break;
        // �������� ������ ����
    case QEvent::MouseButtonRelease: endDrag(mEvent,k); break;
        // ��� ������ ������� ������ �� ������
    default: ;
    }
}

// ���������� ������� �� ������ ����
// (��������� ����������� �������)
void QDragSynZoomSvc::startDrag(QMouseEvent *mEvent,int ind)
{
    // ��������� �������� ������� ������� (���� ����� ��� �� ���� �������)
    zoom->fixBounds();
    // ���� � ������ ������ ��� �� ������� �� ���� �� �������
    if (zoom->regim() == QChartSynZoom::ctNone)
    {
        // �������� ��������� ��
        QChartZoomItem *it = zoom->at(ind); // �������������� �������
        QwtPlot *plt = it->plot;            // ������
        QwtPlotCanvas *cv = plt->canvas();  // � �����
        // �������� ��������� ����� �������
        QRect cg = cv->geometry();
        // ���������� ������� ��������� ������� (������������ ����� �������)
        scp_x = mEvent->pos().x() - cg.x();
        scp_y = mEvent->pos().y() - cg.y();
        // ���� ������ ��������� ��� ������ �������
        if (scp_x >= 0 && scp_x < cg.width() &&
            scp_y >= 0 && scp_y < cg.height())
            // ���� ������ ������ ������ ����, ��
            if (mEvent->button() == Qt::RightButton)
            {
                // ����������� ��������������� ������� ������
                zoom->setRegim(QChartSynZoom::ctDrag);
                // ���������� ������� ������
                tCursor = cv->cursor();
                // ������������� ������ OpenHand
                cv->setCursor(Qt::OpenHandCursor);
                // ���������� ������� �������������� ��������� �� �������������� ���
                // (�.�. ������ �� ������� ���������� ���������� �� ����� x
                // ��� ����������� ������� ������ �� ���� ������)
                cs_kx = plt->invTransform(it->masterX,scp_x + 1) -
                    plt->invTransform(it->masterX,scp_x);
                // �������� �������� ������������ �����
                QwtPlot::Axis mY = it->masterY;
                // ���������� ������� �������������� ��������� �� ������������ ���
                // (����������)
                cs_ky = plt->invTransform(mY,scp_y + 1) -
                    plt->invTransform(mY,scp_y);
                // �������� ����� �������� �������������� �����
                QwtScaleMap sm = plt->canvasMap(it->masterX);
                // ��� ���� ����� ����������� ��������� ����� � ������ �������
                scb_xl = sm.s1(); scb_xr = sm.s2();
                // ���������� �������� ����� �������� ������������ �����
                sm = plt->canvasMap(mY);
                // ��� ���� ����� ����������� ��������� ������ � ������� �������
                scb_yb = sm.s1(); scb_yt = sm.s2();
                // ���� ������ ����� � �������� ���������, ��
                if (light and indiDrB != QChartSynZoom::disNone)
                {
                    // ������� ������, ������������ ����������� �������
                    zwid = new QWidget(plt->parentWidget());
                    // ��������� ��� ����
                    zwid->setStyleSheet(QString(
                        "background-color:rgb(%1,%2,%3);").arg(
                        dwClr.red()).arg(dwClr.green()).arg(dwClr.blue()));
                    // ������������� ����������� ���������� �����������
                    showDragWidget(mEvent->pos(),ind);
                }
            }
    }
}

// ���������� ����������� ����
// (���������� ����������� ��� ����� ������ ��������� �������)
void QDragSynZoomSvc::procDrag(QMouseEvent *mEvent,int ind)
{
    // ���� ������� ����� ����������� �������, ��
    if (zoom->regim() == QChartSynZoom::ctDrag)
    {
        // ������������� ������ ClosedHand
        zoom->at(ind)->plot->canvas()->setCursor(Qt::ClosedHandCursor);
        if (light)  // ���� ������ �����, ��
        {
            // ���� �������� ���������, ��
            if (indiDrB != QChartSynZoom::disNone)
                // ������������� ����������� ���������� �����������
                showDragWidget(mEvent->pos(),ind);
        }
        // ����� ��������� ���������� ����������� �������
        else applyDrag(mEvent->pos(),ind);
    }
}

// ���������� ���������� ������ ����
// (���������� ����������� �������)
void QDragSynZoomSvc::endDrag(QMouseEvent *mEvent,int ind)
{
    // ���� ������� ����� ��������� �������� ��� ����� ����������� �������
    if (zoom->regim() == QChartSynZoom::ctDrag)
        // ���� �������� ������ ������ ����, ��
        if (mEvent->button() == Qt::RightButton)
        {
            if (light)  // ���� ������ �����, ��
            {
                // ���� �������� ���������, �� ������� ������ ����������
                if (indiDrB != QChartSynZoom::disNone) delete zwid;
                // ��������� ���������� ����������� �������
                applyDrag(mEvent->pos(),ind);
            }
            // ��������������� ������
            zoom->at(ind)->plot->canvas()->setCursor(tCursor);
            zoom->setRegim(QChartSynZoom::ctNone);  // � ������� ������� ������
        }
}
