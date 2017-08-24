/**********************************************************/
/*                                                        */
/*           ���������� ������ QWheelSynZoomSvc           */
/*                      ������ 1.0.3                      */
/*                                                        */
/* ���������� ��������� ������ ���������,                 */
/* �. �������-��������� ������������ ���., 2012 �.,       */
/* ��� ��������� �. �. ����������, �. �����������.        */
/*                                                        */
/* ����������� ��������� ������������� � ���������������. */
/* ���������� ������ �����������.                         */
/*                                                        */
/**********************************************************/

#include "qwheelsynzoomsvc.h"

#ifdef Q_WS_WIN
//    #define L_SHIFT 42
    #define R_SHIFT 54
#endif

#ifdef Q_WS_X11
//    #define L_SHIFT 50
    #define R_SHIFT 62
#endif

// �����������
QWheelSynZoomSvc::QWheelSynZoomSvc() :
    QObject()
{
    // ��������� �����������, ������������ ��������� �������� �������
    // ��� �������� ������ ����
    sfact = 1.2;
}

// ������������ ���������� � ��������� ���������������
void QWheelSynZoomSvc::attach(QChartSynZoom *zm)
{
    // ���������� ��������� �� �������� ���������������
    zoom = zm;
    // ��� ���� �������� ��������� �������� �� ���������� ������ �������
    for (int k=0; k < zoom->count(); k++) connectPlot(k);
    // ��������� ���� ������� - ���������� ������ �������
    connect(zm,SIGNAL(plotAppended(int)),SLOT(connectPlot(int)));
}

// ������� ������������ ��������������� �������
// ��� �������� ������ ���� (�� ��������� �� ����� 1.2)
void QWheelSynZoomSvc::setWheelFactor(double fact) {
    sfact = fact;
}

// ���� ������� - ���������� ������ �������
void QWheelSynZoomSvc::connectPlot(int ind)
{
    // ��������� ���������� ������� (������ �������)
    zoom->at(ind)->plot->installEventFilter(this);
}

// ���������� ���� �������
bool QWheelSynZoomSvc::eventFilter(QObject *target,QEvent *event)
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
        // ���� ��������� ���� �� ������� �� ����������, ��
        if (event->type() == QEvent::KeyPress ||
            event->type() == QEvent::KeyRelease)
            switchWheel(event); // �������� ��������������� ����������
        // ���� ������� ������� ��������� ������ ����, ��
        if (event->type() == QEvent::Wheel)
        {
            procWheel(event,ind);   // �������� ��������������� ����������
            zoom->updateAllPlots(); // ��������� ��� �������
        }
    }
    // �������� ���������� ������������ ����������� �������
    return QObject::eventFilter(target,event);
}

// ���������� �������/���������� ������� Ctrl ��� Shift
void QWheelSynZoomSvc::switchWheel(QEvent *event)
{
    // ������ ����� ���������������
    QChartSynZoom::QConvType ct = zoom->regim();
    // ������� ��������� �� ������� �� ����������
    QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
    // ����������� ����� � ����������� �� �������
    switch (kEvent->key())
    {
    // ������� Ctrl
    case Qt::Key_Control:
        // � ����������� �� �������
        switch (event->type())
        {
        // ������� ������
        case QEvent::KeyPress:
            // ���� �� ������� ������� ������ �����,
            if (ct == QChartSynZoom::ctNone)
                // �� �������� ����� Wheel
                zoom->setRegim(QChartSynZoom::ctWheel);
        // ������� ��������
        case QEvent::KeyRelease:
            // ���� ������� ����� Wheel,
            if (ct == QChartSynZoom::ctWheel)
                // �� ��������� ���
                zoom->setRegim(QChartSynZoom::ctNone);
            break;
        // ����� ������ �� ������
        default: ;
        }
        break;
    // ������� Shift
    case Qt::Key_Shift:
        // � ����������� �� �������
        switch (event->type())
        {
        // ������� ������
        case QEvent::KeyPress:

#ifdef R_SHIFT
// ��������� Win ��� X11
            // ���� �� ������� ������� ������ �����, ��
            if (ct == QChartSynZoom::ctNone)
            {
                // ���� ����� ������ Shift,
                if (kEvent->nativeScanCode() == R_SHIFT)
                    // �� �������� ����� ctHorWheel
                    zoom->setRegim(QChartSynZoom::ctHorWheel);
                // ����� (����� ����� Shift) �������� ����� ctVerWheel
                else zoom->setRegim(QChartSynZoom::ctVerWheel);
            }
#else
// ����������� ���������
            // ���� �� ������� ������� ������ �����,
            if (ct == QChartSynZoom::ctNone)
                // �� �������� ����� ctVerWheel
                zoom->setRegim(QChartSynZoom::ctVerWheel);
#endif

            break;
        // ������� ��������
        case QEvent::KeyRelease:

#ifdef R_SHIFT
// ��������� Win ��� X11
            // ���� ������� ������ Shift,
            if (kEvent->nativeScanCode() == R_SHIFT)
            {
                // ���� ������� ����� ctHorWheel,
                if (ct == QChartSynZoom::ctHorWheel)
                    // �� ��������� ���
                    zoom->setRegim(QChartSynZoom::ctNone);
            }
            // ����� (������� ����� Shift)
            // ���� ������� ����� ctVerWheel,
            else if (ct == QChartSynZoom::ctVerWheel)
                // �� ��������� ���
                zoom->setRegim(QChartSynZoom::ctNone);
#else
// ����������� ���������
            // ���� ������� ����� ctVerWheel,
            if (ct == QChartSynZoom::ctVerWheel)
                // �� ��������� ���
                zoom->setRegim(QChartSynZoom::ctNone);
#endif

            break;
        // ����� ������ �� ������
        default: ;
        }
        break;
        // ��� ��������� ������ �� ������
    default: ;
    }
}

// ���������� ��������� �� �������� ������ ����
void QWheelSynZoomSvc::applyWheel(QEvent *event,bool ax,bool ay,int ind)
{
    // �������� ��� QEvent � QWheelEvent
    QWheelEvent *wEvent = static_cast<QWheelEvent *>(event);
    // ���� ��������� ������������ ������ ����
    if (wEvent->orientation() == Qt::Vertical)
    {
        // ���������� ���� �������� ������ ����
        // (�������� 120 ������������� ���� �������� 15�)
        int wd = wEvent->delta();
        // ��������� �������������� ���������
        // (�� ������� ��� ����� ��������/�������� ������)
        double kw = sfact*wd/120;
        if (wd != 0)    // ���� ������ ���������, ��
        {
            // ��������� �������� ������� ������� (���� ����� ��� �� ���� �������)
            zoom->fixBounds();
            // �������� ��������� ��
            QChartZoomItem *it = zoom->at(ind); // �������������� �������
            QwtPlot *plt = it->plot;            // � ������
            if (ax) // ���� ������ ��������������� �� �����������
            {
                // �������� ����� �������� �������������� �����
                QwtScaleMap sm = plt->canvasMap(it->masterX);
                // ���������� ����� ������������� �� ����� x ���������
                double mx = (sm.s1()+sm.s2())/2;
                // � ���������� ���������
                double dx = (sm.s2()-sm.s1())/2;
                // � ����������� �� ����� ���� �������� ������ ����
                // ��������� ���������� ������������ ���������� � kw ���
                if (wd > 0) dx /= kw;
                // ��� ����������� ���������� ������������ ���������� � -kw ���
                else dx *= -kw;
                // ������������� ����� ����� � ������ ������� ����� ��� ��� x
                // (����� ������������ ����� ������� �������� �� �����,
                // � ������� ��������� �� ������, �.�. ����������� ������� �����������)
                zoom->setHorizontScale(mx-dx,mx+dx);
            }
            if (ay) // ���� ������ ��������������� �� ���������
            {
                // �������� ����� �������� ������������ �����
                QwtScaleMap sm = plt->canvasMap(it->masterY);
                // ���������� ����� ������������� �� ����� y ���������
                double my = (sm.s1()+sm.s2())/2;
                // � ���������� ���������
                double dy = (sm.s2()-sm.s1())/2;
                // � ����������� �� ����� ���� �������� ������ ����
                // ��������� ���������� ������������ ���������� � kw ���
                if (wd > 0) dy /= kw;
                // ����������� ���������� ������������ ���������� � -kw ���
                else dy *= -kw;
                // ������������� ����� ������ � ������� ������� ������������ �����
                // (����� ������������ ����� ������� �������� �� �����,
                // � ������� ��������� �� ������, �.�. ����������� ������� �����������)
                it->isb_y->set(my-dy,my+dy);
            }
            // ������������� ������ (��������� � ����������)
            zoom->synReplot(ind);
        }
    }
}

// ���������� �������� ������ ����
void QWheelSynZoomSvc::procWheel(QEvent *event,int ind)
{
    // � ����������� �� ����������� ������ ��������
    // ��������������� � ���������������� �����������
    switch (zoom->regim())
    {
        // Wheel - ���� ������ ������� Ctrl -
            // ��������������� �� ����� ����
    case QChartSynZoom::ctWheel: applyWheel(event,true,true,ind); break;
        //VerWheel - ���� ������ ����� ������� Shift -
            // ��������������� ������ �� ������������ ���
    case QChartSynZoom::ctVerWheel: applyWheel(event,false,true,ind); break;
        // HorWheel - ���� ������ ������ ������� Shift -
            // ��������������� ������ �� �������������� ���
    case QChartSynZoom::ctHorWheel: applyWheel(event,true,false,ind); break;
        // ��� ������ ������� ������ �� ������
    default: ;
    }
}
