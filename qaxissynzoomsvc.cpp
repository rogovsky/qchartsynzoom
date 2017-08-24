/**********************************************************/
/*                                                        */
/*           ���������� ������ QAxisSynZoomSvc            */
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

#include "qaxissynzoomsvc.h"

#include <qwt_scale_widget.h>

// �����������
QAxisSynZoomSvc::QAxisSynZoomSvc() :
    QObject()
{
    // �� ��������� ������ ����� ��������
    light = false;
    // ������� ������, ���������� �� ����������� ����������
    zwid = 0;
    // � ��������� ���� �������, ������������� ��������������� �����
    awClr = Qt::black;
    // �� ��������� ��������������� ����� ������������
    // (�� ������ � ��� ������, ���� ������� ������ �����)
    indiAxZ = true;
}

// ������������ ���������� � ��������� ���������������
void QAxisSynZoomSvc::attach(QChartSynZoom *zm)
{
    // ���������� ��������� �� �������� ���������������
    zoom = zm;
    // ��� ���� �������� ��������� �������� �� ���������� ������ �������
    for (int k=0; k < zoom->count(); k++) connectPlot(k);
    // ��������� ���� ������� - ���������� ������ �������
    connect(zm,SIGNAL(plotAppended(int)),SLOT(connectPlot(int)));
}

// ���������/���������� ������� ������
void QAxisSynZoomSvc::setLightMode(bool lm) {
    light = lm;
}

// ��������� ����� �������, ������������� ��������������� �����
void QAxisSynZoomSvc::setAxisRubberBandColor(QColor clr) {
    awClr = clr;
}

// ���������/���������� ��������� ��������������� �����
// (����� ������, ���� ������� ������ �����)
void QAxisSynZoomSvc::indicateAxisZoom(bool indi) {
    indiAxZ = indi;
}

// ���� ������� - ���������� ������ �������
void QAxisSynZoomSvc::connectPlot(int ind)
{
    // ��� ���� ���� �������
    for (int ax=0; ax < QwtPlot::axisCnt; ax++)
        // ��������� ���������� ������� (������ �������)
        zoom->at(ind)->plot->axisWidget(ax)->installEventFilter(this);
}

// ���������� ���� �������
bool QAxisSynZoomSvc::eventFilter(QObject *target,QEvent *event)
{
    int ind = -1;   // ������ ���� �� ������
    int ax = -1;    // �����, ��������������, ����
    // ������������� ������ ��������
    for (int k=0; k < zoom->count(); k++)
        // ��� ������� ������������� ������ ����
        for (int a=0; a < QwtPlot::axisCnt; a++)
            // ���� ������� ��������� ��� ������ �����, ��
            if (target == zoom->at(k)->plot->axisWidget(a))
            {
                ind = k;    // ���������� ����� �������,
                ax = a;     // ����� �����
                break;      // ���������� ����� �����
                break;      // � ����� �������
            }
    // ���� ������ ��� ������, ��
    if (ind >= 0)
        // ���� ��������� ���� �� ������� �� ����, ��
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)
            axisMouseEvent(event,ind,ax);   // �������� ��������������� ����������
    // �������� ���������� ������������ ����������� �������
    return QObject::eventFilter(target,event);
}

// ����������� ������� ����������
int QAxisSynZoomSvc::limitSize(int sz,int bs)
{
    // �������
    int mn = floor(16*bs/31);
    // ����������� ������������ �������
    if (sz < mn) sz = mn;
    // ��������
    int mx = floor(31*bs/16);
    // ����������� ������������ �������
    if (sz > mx) sz = mx;
    return sz;
}

// ��������� ��������� ��������� ��������������� �����
QRect *QAxisSynZoomSvc::axisZoomRect(QPoint evpos,int ind,int ax)
{
    // �������� ��������� �� ������
    QwtPlot *plt = zoom->at(ind)->plot;
    // ���������� (��� ��������) ���������
    QRect gc = plt->canvas()->geometry();       // ����� �������
    QRect gw = plt->axisWidget(ax)->geometry(); // � ������� �����
    // ���������� ������� ��������� ������� ������������ ����� �������
    int x = evpos.x() + gw.x() - gc.x() - scb_pxl;
    int y = evpos.y() + gw.y() - gc.y() - scb_pyt;
    // ���������� (��� ��������)
    int wax = gw.width();   // ������ ������� �����
    int hax = gw.height();  // � ������
    // ������ ����� ���������������
    QChartSynZoom::QConvType ct = zoom->regim();
    // ��������� ��������� ������ �������� ����,
    int wl,wt,ww,wh;    // ������ � ������
    // ���� �������������� �������������� �����, ��
    if (ax == QwtPlot::xBottom ||
        ax == QwtPlot::xTop)
    {
        // ���� ���������� ������ �������, ��
        if (ct == QChartSynZoom::ctAxisHR)
        {
            // ����������� �� ��������� ������� �����
            int mn = floor(scb_pw/16);
            // ���� ������ ������� ������ � ����� �������, ��
            if (x < mn) x = mn;
            // ������ ��������������
            ww = floor(x * scb_pw / scp_x);
            // ��������� �����������
            ww = limitSize(ww,scb_pw);
            // ����� ������ ��������������
            wl = sab_pxl;
        }
        else    // ����� (���������� ����� �������)
        {
            // ����������� �� ��������� ������� ������
            int mx = floor(15*scb_pw/16);
            // ���� ������ ������� ������ � ������ �������, ��
            if (x > mx) x = mx;
            // ������ ��������������
            ww = floor((scb_pw - x) * scb_pw / (scb_pw - scp_x));
            // ��������� �����������
            ww = limitSize(ww,scb_pw);
            // ����� ������ ��������������
            wl = sab_pxl + scb_pw - ww;
        }
        // ������ ��������������
        wh = 4;
        // ������� ������ ��������������
        wt = 10;    // ��� ������ �����
        // ���� �� ���������� �� �����, ������������
        if (wt + wh > hax) wt = hax - wh;
        // ��� ������� ����� �����������
        if (ax == QwtPlot::xTop) wt = hax - wt - wh;
    }
    else    // ����� (�������������� ������������ �����)
    {
        // ���� ���������� ������ �������, ��
        if (ct == QChartSynZoom::ctAxisVB)
        {
            // ����������� �� ��������� ������� ������
            int mn = floor(scb_ph/16);
            // ���� ������ ������� ������ � ������� �������, ��
            if (y < mn) y = mn;
            // ������ ��������������
            wh = floor(y * scb_ph / scp_y);
            // ��������� �����������
            wh = limitSize(wh,scb_ph);
            // ������� ������ ��������������
            wt = sab_pyt;
        }
        else    // ����� (���������� ������� �������)
        {
            // ����������� �� ��������� ������� �����
            int mx = floor(15*scb_ph/16);
            // ���� ������ ������� ������ � ������ �������, ��
            if (y > mx) y = mx;
            // ������ ��������������
            wh = floor((scb_ph - y) * scb_ph / (scb_ph - scp_y));
            // ��������� �����������
            wh = limitSize(wh,scb_ph);
            // ������� ������ �������������� = �������� �������
            wt = sab_pyt + scb_ph - wh;
        }
        // ������ ��������������
        ww = 4;
        // ������� ������ ��������������
        wl = 10;    // ��� ������ �����
        // ���� �� ���������� �� �����, ������������
        if (wl + ww > wax) wl = wax - ww;
        // ��� ����� ����� �����������
        if (ax == QwtPlot::yLeft) wl = wax - wl - ww;
    }
    // ������� � ���������� ��������� �������
    // � ������������ ���������
    return new QRect(wl,wt,ww,wh);
}

// ���������� ������� �������������� �����
void QAxisSynZoomSvc::showZoomWidget(QPoint evpos,int ind,int ax)
{
    // �������� ��������� ��������� ��������������� �����
    QRect *zr = axisZoomRect(evpos,ind,ax);
    // ��� �������� ����������
    int w = zr->width();    // ������
    int hw = floor(w/2);    // ����������
    int h = zr->height();   // ������
    int hh = floor(h/2);    // ����������
    // ������������� ��������� � ������� �������, �������������
    // ��������������� �����, � ������������ � �������� ����������
    zwid->setGeometry(*zr);
    // ������� ��������� ��������� ��������������� �����
    delete zr;
    // ������ ����� ���������������
    QChartSynZoom::QConvType ct = zoom->regim();
    // ��������� ����� ��� �������, ������������� ��������������� �����
    QRegion rw = QRegion(0,0,w,h);  // ������������ �������
    QRegion rs; // ��������� ���������� �������
    // ���� �������������� �������������� �����
    if (ax == QwtPlot::xBottom || ax == QwtPlot::xTop)
    {
        // ���� ���������� ������ ������� �����, ��
        if (ct == QChartSynZoom::ctAxisHR)
            // ���������� ������� ����� �����
            rs = QRegion(1,1,hw-1,h-2);
        // ����� (���������� ����� ������� �����)
        // ���������� ������� ����� ������
        else rs = QRegion(w-hw,1,hw-1,h-2);
    }
    else    // ����� (�������������� ������������ �����)
    {
        // ���� ���������� ������ ������� �����, ��
        if (ct == QChartSynZoom::ctAxisVB)
            // ���������� ������� ����� ������
            rs = QRegion(1,1,w-2,hh-1);
        // ����� (���������� ������� ������� �����)
        // ���������� ������� ����� �����
        else rs = QRegion(1,h-hh,w-2,hh-1);
    }
    // ������������� ����� ����� ��������� �� ������������ ������� ����������
    zwid->setMask(rw.subtracted(rs));
    // ������ ������, ������������ ��������������� �����, �������
    zwid->setVisible(true);
    // �������������� ������
    zwid->repaint();
}

// ����������� ������ ������� �����
double QAxisSynZoomSvc::limitScale(double sz,double bs)
{
    // ��������
    double mx = 16*bs;
    if (light)  // ���� ������ �����, ��
    {
        // ������ �������
        double mn = 16*bs/31;
        // ����������� ������������ �������
        if (sz < mn) sz = mn;
        // ������ ��������
        mx = 31*bs/16;
    }
    // ����������� ������������� �������
    if (sz > mx) sz = mx;
    return sz;
}

// ���������� ����������� ����������� ������� �����
void QAxisSynZoomSvc::axisApplyMove(QPoint evpos,int ind,int ax)
{
    // �������� ��������� ��
    QChartZoomItem *it = zoom->at(ind);         // �������������� �������
    QwtPlot *plt = it->plot;                    // ������
    // ���������� (��� ��������) ���������
    QRect gc = plt->canvas()->geometry();       // ����� �������
    QRect gw = plt->axisWidget(ax)->geometry(); // � ������� �����
    // ���������� ������� ��������� ������� ������������ �����
    // (�� ������� �������� �������)
    int x = evpos.x() + gw.x() - gc.x() - scb_pxl;
    int y = evpos.y() + gw.y() - gc.y() - scb_pyt;
    // ��������� ��������� ������ �������
    // bcNone - ������ �� ����������
    // bcHor  - ���������� ������� �� �������������� �����
    // bcVer  - ���������� ������� �� ������������ �����
    enum QBoundChanged {bcNone,bcHor,bcVer};
    QBoundChanged bndCh = bcNone;   // ���� ������ �� ����������
    // ������ ����� ���������������
    QChartSynZoom::QConvType ct = zoom->regim();
    // � ����������� �� ����������� ������ ��������� ��������� ��������
    switch (ct)
    {
        // ����� ��������� ����� �������
    case QChartSynZoom::ctAxisHL:
    {
        // ����������� �� ��������� ������� ������
        if (x >= scb_pw) x = scb_pw-1;
        // ��������� ����� ������ �����
        double wx = scb_wx * (scb_pw - scp_x) / (scb_pw - x);
        // ��������� �����������
        wx = limitScale(wx,scb_wx);
        // ��������� ����� ����� �������
        double xl = scb_xr - wx;
        // �������� ����� �������
        zoom->setHorizontScale(xl,scb_xr);
        bndCh = bcHor;  // ���������� ������� �� �������������� �����
        break;
    }
        // ����� ��������� ������ �������
    case QChartSynZoom::ctAxisHR:
    {
        // ����������� �� ��������� ������� �����
        if (x <= 0) x = 1;
        // ��������� ����� ������ �����
        double wx = scb_wx * scp_x / x;
        // ��������� �����������
        wx = limitScale(wx,scb_wx);
        // ��������� ����� ������ �������
        double xr = scb_xl + wx;
        // �������� ������ �������
        zoom->setHorizontScale(scb_xl,xr);
        bndCh = bcHor;  // ���������� ������� �� �������������� �����
        break;
    }
        // ����� ��������� ������ �������
    case QChartSynZoom::ctAxisVB:
    {
        // ����������� �� ��������� ������� ������
        if (y <= 0) y = 1;
        // ��������� ����� ������ �����
        double hy = scb_hy * scp_y / y;
        // ��������� �����������
        hy = limitScale(hy,scb_hy);
        // ��������� ����� ������ �������
        double yb = scb_yt - hy;
        // ������������� �� ��� ������������ �����
        it->isb_y->set(yb,scb_yt);
        bndCh = bcVer;  // ���������� ������� �� ������������ �����
        break;
    }
        // ����� ��������� ������� �������
    case QChartSynZoom::ctAxisVT:
    {
        // ����������� �� ��������� ������� �����
        if (y >= scb_ph) y = scb_ph-1;
        // ��������� ����� ������ �����
        double hy = scb_hy * (scb_ph - scp_y) / (scb_ph - y);
        // ��������� �����������
        hy = limitScale(hy,scb_hy);
        // ��������� ����� ������� �������
        double yt = scb_yb + hy;
        // ������������� �� ��� ������������ �����
        it->isb_y->set(scb_yb,yt);
        bndCh = bcVer;  // ���������� ������� �� ������������ �����
        break;
    }
        // ��� ������ ������� ������ �� ������
    default: ;
    }
    // �������, ��� ����������
    switch (bndCh)
    {
        // ������� �� �������������� �����:
        // ������������� ������ (��������� � ����������)
    case bcHor: zoom->synReplot(ind); break;
        // ������� �� ������������ �����:
        // ������������� ������
    case bcVer: plt->replot(); break;
        // ����� ������ �� ������
    default: ;
    }
}

// ���������� ������� �� ���� ��� �����
void QAxisSynZoomSvc::axisMouseEvent(QEvent *event,int k,int a)
{
    // ������� ��������� �� ������� �� ����
    QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
    // � ����������� �� ���� ������� �������� ��������������� ����������
    switch (event->type())
    {
        // ������ ������ ����
    case QEvent::MouseButtonPress: startAxisZoom(mEvent,k,a); break;
        // ����������� ����
    case QEvent::MouseMove: procAxisZoom(mEvent,k,a); break;
        // �������� ������ ����
    case QEvent::MouseButtonRelease: endAxisZoom(mEvent,k,a); break;
        // ��� ������ ������� ������ �� ������
    default: ;
    }
}

// ���������� ������� �� ������ ���� ��� ������
// (��������� ��������� �������� �����)
void QAxisSynZoomSvc::startAxisZoom(QMouseEvent *mEvent,int ind,int ax)
{
    // ��������� �������� ������� ������� (���� ����� ��� �� ���� �������)
    zoom->fixBounds();
    // ���� � ������ ������ ��� �� ������� �� ���� �� �������
    if (zoom->regim() == QChartSynZoom::ctNone)
    {
        // �������� ��������� �� �������������� �������
        QChartZoomItem *it = zoom->at(ind);
        // ���� ������ ����� ������ ����, ��
        // �������� ���� �� ������� ���������������
        if (mEvent->button() == Qt::LeftButton)
        {
            // �������� ��������� ��
            QwtPlot *plt = it->plot;                    // ������
            QwtScaleWidget *sw = plt->axisWidget(ax);   // ������ �����
            // �������� ����� �������� �������������� �����
            QwtScaleMap sm = plt->canvasMap(it->masterX);
            // ��� ���� ����� ����������� ��������� ����� � ������ �������
            scb_xl = sm.s1(); scb_xr = sm.s2(); scb_wx = sm.sDist();
            // ���������� �������� ����� �������� ������������ �����
            sm = plt->canvasMap(it->masterY);
            // ��� ���� ����� ����������� ��������� ������ � ������� �������
            scb_yb = sm.s1(); scb_yt = sm.s2(); scb_hy = sm.sDist();
            // ���������� (��� ��������) ���������
            QRect gc = plt->canvas()->geometry();   // ����� �������
            QRect gw = sw->geometry();              // � ������� �����
            // ������� ����� �������� ������� (� �������� ������������ �����)
            scb_pxl = plt->transform(it->masterX,scb_xl);
            // ������� ������ ������� (� ��������)
            scb_pw = plt->transform(it->masterX,scb_xr) - scb_pxl;
            // ������� ����� �������� �������
            // (� �������� ������������ ������� �����)
            sab_pxl = scb_pxl + gc.x() - gw.x();
            // ������� ������� �������� ������� (� �������� ������������ �����)
            scb_pyt = plt->transform(it->masterY,scb_yt);
            // ������� ������ ������� (� ��������)
            scb_ph = plt->transform(it->masterY,scb_yb) - scb_pyt;
            // ������� ������� �������� �������
            // (� �������� ������������ ������� �����)
            sab_pyt = scb_pyt + gc.y() - gw.y();
            // ���������� ������� ��������� ������� ������������ �����
            // (�� ������� �������� �������)
            scp_x = mEvent->pos().x() - sab_pxl;
            scp_y = mEvent->pos().y() - sab_pyt;
            // ���� �������������� �������������� �����
            if (ax == QwtPlot::xBottom ||
                ax == QwtPlot::xTop)
            {
                // ���� ����� ������� ������ ������,
                if (scb_wx > 0)
                    // ���� ������ ����� ������ ��������,
                    if (scb_pw > 36)
                    {
                        // � ����������� �� ��������� �������
                        // (������ ��� ����� �������� �����)
                        // �������� ��������������� ����� - ���������
                        if (scp_x >= floor(scb_pw/2))
                            zoom->setRegim(QChartSynZoom::ctAxisHR);    // ������ �������
                        else zoom->setRegim(QChartSynZoom::ctAxisHL);   // ��� �����
                    }
            }
            else    // ����� (�������������� ������������ �����)
            {
                // ���� ������ ������� ������ �������,
                if (scb_hy > 0)
                    // ���� ������ ����� ������ ��������,
                    if (scb_ph > 18)
                    {
                        // � ����������� �� ��������� �������
                        // (���� ��� ���� �������� �����)
                        // �������� ��������������� ����� - ���������
                        if (scp_y >= floor(scb_ph/2))
                            zoom->setRegim(QChartSynZoom::ctAxisVB);    // ������ �������
                        else zoom->setRegim(QChartSynZoom::ctAxisVT);   // ��� �������
                    }
            }
            // ���� ���� �� ������� ��� �������
            if (zoom->regim() != QChartSynZoom::ctNone)
            {
                // ���������� ������� ������
                tCursor = sw->cursor();
                // ������������� ������ PointingHand
                sw->setCursor(Qt::PointingHandCursor);
                // ���� ������ ����� � �������� ���������, ��
                if (light && indiAxZ)
                {
                    // ������� ������, ������������ ��������������� �����
                    zwid = new QWidget(plt->axisWidget(ax));
                    // ��������� ��� ����
                    zwid->setStyleSheet(QString(
                        "background-color:rgb(%1,%2,%3);").arg(
                        awClr.red()).arg(awClr.green()).arg(awClr.blue()));
                    // � �������������
                    showZoomWidget(mEvent->pos(),ind,ax);
                }
            }
        }
    }
}

// ���������� ����������� ����
// (��������� ����� ������ �����)
void QAxisSynZoomSvc::procAxisZoom(QMouseEvent *mEvent,int ind,int ax)
{
    // ������ ����� ���������������
    QChartSynZoom::QConvType ct = zoom->regim();
    // �������, ���� �� ������� �� ���� �� ������� ��������� �����
    if (ct != QChartSynZoom::ctAxisHL &&
        ct != QChartSynZoom::ctAxisHR &&
        ct != QChartSynZoom::ctAxisVB &&
        ct != QChartSynZoom::ctAxisVT) return;
    if (light)  // ���� ������ �����, ��
    {
        // ���� �������� ���������, �� ������������� ������ ����������
        if (indiAxZ) showZoomWidget(mEvent->pos(),ind,ax);
    }
    // ����� ��������� ���������� ����������� ������� �����
    else axisApplyMove(mEvent->pos(),ind,ax);
}

// ���������� ���������� ������ ����
// (���������� ��������� �������� �����)
void QAxisSynZoomSvc::endAxisZoom(QMouseEvent *mEvent,int ind,int ax)
{
    // ������ ����� ���������������
    QChartSynZoom::QConvType ct = zoom->regim();
    // �������, ���� �� ������� �� ���� �� ������� ��������� �����
    if (ct != QChartSynZoom::ctAxisHL &&
        ct != QChartSynZoom::ctAxisHR &&
        ct != QChartSynZoom::ctAxisVB &&
        ct != QChartSynZoom::ctAxisVT) return;
    // ���� �������� ����� ������ ����, ��
    if (mEvent->button() == Qt::LeftButton)
    {
        if (light)  // ���� ������ �����, ��
        {
            // ���� �������� ���������, �� ������� ������ ����������
            if (indiAxZ) delete zwid;
            // ��������� ���������� ����������� ������� �����
            axisApplyMove(mEvent->pos(),ind,ax);
        }
        // ��������������� ������
        zoom->at(ind)->plot->axisWidget(ax)->setCursor(tCursor);
        // ��������� ����� ���������������
        zoom->setRegim(QChartSynZoom::ctNone);
    }
}
