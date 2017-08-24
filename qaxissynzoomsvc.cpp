/**********************************************************/
/*                                                        */
/*           Реализация класса QAxisSynZoomSvc            */
/*                      Версия 1.2.2                      */
/*                                                        */
/* Разработал Мельников Сергей Андреевич,                 */
/* г. Каменск-Уральский Свердловской обл., 2012 г.,       */
/* при поддержке Ю. А. Роговского, г. Новосибирск.        */
/*                                                        */
/* Разрешается свободное использование и распространение. */
/* Упоминание автора обязательно.                         */
/*                                                        */
/**********************************************************/

#include "qaxissynzoomsvc.h"

#include <qwt_scale_widget.h>

// Конструктор
QAxisSynZoomSvc::QAxisSynZoomSvc() :
    QObject()
{
    // по умолчанию легкий режим выключен
    light = false;
    // очищаем виджет, отвечающий за отображение индикатора
    zwid = 0;
    // и назначаем цвет виджета, индицирующего масштабирование шкалы
    awClr = Qt::black;
    // по умолчанию масштабирование шкалы индицируется
    // (но только в том случае, если включен легкий режим)
    indiAxZ = true;
}

// Прикрепление интерфейса к менеджеру масштабирования
void QAxisSynZoomSvc::attach(QChartSynZoom *zm)
{
    // запоминаем указатель на менеджер масштабирования
    zoom = zm;
    // для всех графиков выполняем действия по добавлению нового графика
    for (int k=0; k < zoom->count(); k++) connectPlot(k);
    // назначаем слот сигнала - Добавление нового графика
    connect(zm,SIGNAL(plotAppended(int)),SLOT(connectPlot(int)));
}

// Включение/выключение легкого режима
void QAxisSynZoomSvc::setLightMode(bool lm) {
    light = lm;
}

// Установка цвета виджета, индицирующего масштабирование шкалы
void QAxisSynZoomSvc::setAxisRubberBandColor(QColor clr) {
    awClr = clr;
}

// Включение/выключение индикации масштабирования шкалы
// (имеет эффект, если включен легкий режим)
void QAxisSynZoomSvc::indicateAxisZoom(bool indi) {
    indiAxZ = indi;
}

// Слот сигнала - Добавление нового графика
void QAxisSynZoomSvc::connectPlot(int ind)
{
    // для всех шкал графика
    for (int ax=0; ax < QwtPlot::axisCnt; ax++)
        // назначаем обработчик событий (фильтр событий)
        zoom->at(ind)->plot->axisWidget(ax)->installEventFilter(this);
}

// Обработчик всех событий
bool QAxisSynZoomSvc::eventFilter(QObject *target,QEvent *event)
{
    int ind = -1;   // график пока не найден
    int ax = -1;    // шкала, соответственно, тоже
    // просматриваем список графиков
    for (int k=0; k < zoom->count(); k++)
        // для каждого просматриваем список шкал
        for (int a=0; a < QwtPlot::axisCnt; a++)
            // если событие произошло для данной шкады, то
            if (target == zoom->at(k)->plot->axisWidget(a))
            {
                ind = k;    // запоминаем номер графика,
                ax = a;     // номер шкалы
                break;      // прекращаем поиск шкалы
                break;      // и поиск графика
            }
    // если график был найден, то
    if (ind >= 0)
        // если произошло одно из событий от мыши, то
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)
            axisMouseEvent(event,ind,ax);   // вызываем соответствующий обработчик
    // передаем управление стандартному обработчику событий
    return QObject::eventFilter(target,event);
}

// Ограничение размера индикатора
int QAxisSynZoomSvc::limitSize(int sz,int bs)
{
    // минимум
    int mn = floor(16*bs/31);
    // ограничение минимального размера
    if (sz < mn) sz = mn;
    // максимум
    int mx = floor(31*bs/16);
    // ограничение максимальной размера
    if (sz > mx) sz = mx;
    return sz;
}

// Получение геометрии индикации масштабирования шкалы
QRect *QAxisSynZoomSvc::axisZoomRect(QPoint evpos,int ind,int ax)
{
    // получаем указатель на график
    QwtPlot *plt = zoom->at(ind)->plot;
    // определяем (для удобства) геометрию
    QRect gc = plt->canvas()->geometry();       // канвы графика
    QRect gw = plt->axisWidget(ax)->geometry(); // и виджета шкалы
    // определяем текущее положение курсора относительно канвы графика
    int x = evpos.x() + gw.x() - gc.x() - scb_pxl;
    int y = evpos.y() + gw.y() - gc.y() - scb_pyt;
    // запоминаем (для удобства)
    int wax = gw.width();   // ширину виджета шкалы
    int hax = gw.height();  // и высоту
    // читаем режим масштабирования
    QChartSynZoom::QConvType ct = zoom->regim();
    // объявляем положение левого верхнего угла,
    int wl,wt,ww,wh;    // ширину и высоту
    // если масштабируется горизонтальная шкала, то
    if (ax == QwtPlot::xBottom ||
        ax == QwtPlot::xTop)
    {
        // если изменяется правая граница, то
        if (ct == QChartSynZoom::ctAxisHR)
        {
            // ограничение на положение курсора слева
            int mn = floor(scb_pw/16);
            // если курсор слишком близко к левой границе, то
            if (x < mn) x = mn;
            // ширина прямоугольника
            ww = floor(x * scb_pw / scp_x);
            // применяем ограничения
            ww = limitSize(ww,scb_pw);
            // левый отступ прямоугольника
            wl = sab_pxl;
        }
        else    // иначе (изменяется левая граница)
        {
            // ограничение на положение курсора справа
            int mx = floor(15*scb_pw/16);
            // если курсор слишком близко к правой границе, то
            if (x > mx) x = mx;
            // ширина прямоугольника
            ww = floor((scb_pw - x) * scb_pw / (scb_pw - scp_x));
            // применяем ограничения
            ww = limitSize(ww,scb_pw);
            // левый отступ прямоугольника
            wl = sab_pxl + scb_pw - ww;
        }
        // высота прямоугольника
        wh = 4;
        // верхний отступ прямоугольника
        wt = 10;    // для нижней шкалы
        // если не помещается на шкале, корректируем
        if (wt + wh > hax) wt = hax - wh;
        // для верхней шкалы симметрично
        if (ax == QwtPlot::xTop) wt = hax - wt - wh;
    }
    else    // иначе (масштабируется вертикальная шкала)
    {
        // если изменяется нижняя граница, то
        if (ct == QChartSynZoom::ctAxisVB)
        {
            // ограничение на положение курсора сверху
            int mn = floor(scb_ph/16);
            // если курсор слишком близко к верхней границе, то
            if (y < mn) y = mn;
            // высота прямоугольника
            wh = floor(y * scb_ph / scp_y);
            // применяем ограничения
            wh = limitSize(wh,scb_ph);
            // верхний отступ прямоугольника
            wt = sab_pyt;
        }
        else    // иначе (изменяется верхняя граница)
        {
            // ограничение на положение курсора снизу
            int mx = floor(15*scb_ph/16);
            // если курсор слишком близко к нижней границе, то
            if (y > mx) y = mx;
            // высота прямоугольника
            wh = floor((scb_ph - y) * scb_ph / (scb_ph - scp_y));
            // применяем ограничения
            wh = limitSize(wh,scb_ph);
            // верхний отступ прямоугольника = смещению курсора
            wt = sab_pyt + scb_ph - wh;
        }
        // ширина прямоугольника
        ww = 4;
        // верхний отступ прямоугольника
        wl = 10;    // для правой шкалы
        // если не помещается на шкале, корректируем
        if (wl + ww > wax) wl = wax - ww;
        // для левой шкалы симметрично
        if (ax == QwtPlot::yLeft) wl = wax - wl - ww;
    }
    // создаем и возвращаем геометрию виджета
    // с вычисленными размерами
    return new QRect(wl,wt,ww,wh);
}

// Прорисовка виджета масштабируемой шкалы
void QAxisSynZoomSvc::showZoomWidget(QPoint evpos,int ind,int ax)
{
    // получаем геометрию индикации масштабирования шкалы
    QRect *zr = axisZoomRect(evpos,ind,ax);
    // для удобства запоминаем
    int w = zr->width();    // ширину
    int hw = floor(w/2);    // полуширину
    int h = zr->height();   // высоту
    int hh = floor(h/2);    // полувысоту
    // устанавливаем положение и размеры виджета, индицирующего
    // масштабирование шкалы, в соответствии с найденой геометрией
    zwid->setGeometry(*zr);
    // удаляем геометрию индикации масштабирования шкалы
    delete zr;
    // читаем режим масштабирования
    QChartSynZoom::QConvType ct = zoom->regim();
    // формируем маску для виджета, индицирующего масштабирование шкалы
    QRegion rw = QRegion(0,0,w,h);  // непрозрачная область
    QRegion rs; // объявляем прозрачную область
    // если масштабируется горизонтальная шкала
    if (ax == QwtPlot::xBottom || ax == QwtPlot::xTop)
    {
        // если изменяется правая граница шкалы, то
        if (ct == QChartSynZoom::ctAxisHR)
            // прозрачная область будет слева
            rs = QRegion(1,1,hw-1,h-2);
        // иначе (изменяется левая граница шкалы)
        // прозрачная область будет справа
        else rs = QRegion(w-hw,1,hw-1,h-2);
    }
    else    // иначе (масштабируется вертикальная шкала)
    {
        // если изменяется нижняя граница шкалы, то
        if (ct == QChartSynZoom::ctAxisVB)
            // прозрачная область будет сверху
            rs = QRegion(1,1,w-2,hh-1);
        // иначе (изменяется верхняя граница шкалы)
        // прозрачная область будет снизу
        else rs = QRegion(1,h-hh,w-2,hh-1);
    }
    // устанавливаем маску путем вычитания из непрозрачной области прозрачной
    zwid->setMask(rw.subtracted(rs));
    // делаем виджет, индицирующий масштабирование шкалы, видимым
    zwid->setVisible(true);
    // перерисовываем виджет
    zwid->repaint();
}

// Ограничение нового размера шкалы
double QAxisSynZoomSvc::limitScale(double sz,double bs)
{
    // максимум
    double mx = 16*bs;
    if (light)  // если легкий режим, то
    {
        // легкий минимум
        double mn = 16*bs/31;
        // ограничение минимального размера
        if (sz < mn) sz = mn;
        // легкий максимум
        mx = 31*bs/16;
    }
    // ограничение максимального размера
    if (sz > mx) sz = mx;
    return sz;
}

// Применение результатов перемещения границы шкалы
void QAxisSynZoomSvc::axisApplyMove(QPoint evpos,int ind,int ax)
{
    // получаем указатели на
    QChartZoomItem *it = zoom->at(ind);         // масштабирующий элемент
    QwtPlot *plt = it->plot;                    // график
    // определяем (для удобства) геометрию
    QRect gc = plt->canvas()->geometry();       // канвы графика
    QRect gw = plt->axisWidget(ax)->geometry(); // и виджета шкалы
    // определяем текущее положение курсора относительно канвы
    // (за вычетом смещений графика)
    int x = evpos.x() + gw.x() - gc.x() - scb_pxl;
    int y = evpos.y() + gw.y() - gc.y() - scb_pyt;
    // возможные изменения границ графика
    // bcNone - ничего не изменилось
    // bcHor  - изменилась граница на горизонтальной шкале
    // bcVer  - изменилась граница на вертикальной шкале
    enum QBoundChanged {bcNone,bcHor,bcVer};
    QBoundChanged bndCh = bcNone;   // пока ничего не изменилось
    // читаем режим масштабирования
    QChartSynZoom::QConvType ct = zoom->regim();
    // в зависимости от включенного режима выполняем некоторые действия
    switch (ct)
    {
        // режим изменения левой границы
    case QChartSynZoom::ctAxisHL:
    {
        // ограничение на положение курсора справа
        if (x >= scb_pw) x = scb_pw-1;
        // вычисляем новую ширину шкалы
        double wx = scb_wx * (scb_pw - scp_x) / (scb_pw - x);
        // применяем ограничения
        wx = limitScale(wx,scb_wx);
        // вычисляем новую левую границу
        double xl = scb_xr - wx;
        // изменяем левую границу
        zoom->setHorizontScale(xl,scb_xr);
        bndCh = bcHor;  // изменилась граница на горизонтальной шкале
        break;
    }
        // режим изменения правой границы
    case QChartSynZoom::ctAxisHR:
    {
        // ограничение на положение курсора слева
        if (x <= 0) x = 1;
        // вычисляем новую ширину шкалы
        double wx = scb_wx * scp_x / x;
        // применяем ограничения
        wx = limitScale(wx,scb_wx);
        // вычисляем новую правую границу
        double xr = scb_xl + wx;
        // изменяем правую границу
        zoom->setHorizontScale(scb_xl,xr);
        bndCh = bcHor;  // изменилась граница на горизонтальной шкале
        break;
    }
        // режим изменения нижней границы
    case QChartSynZoom::ctAxisVB:
    {
        // ограничение на положение курсора сверху
        if (y <= 0) y = 1;
        // вычисляем новую высоту шкалы
        double hy = scb_hy * scp_y / y;
        // применяем ограничения
        hy = limitScale(hy,scb_hy);
        // вычисляем новую нижнюю границу
        double yb = scb_yt - hy;
        // устанавливаем ее для вертикальной шкалы
        it->isb_y->set(yb,scb_yt);
        bndCh = bcVer;  // изменилась граница на вертикальной шкале
        break;
    }
        // режим изменения верхней границы
    case QChartSynZoom::ctAxisVT:
    {
        // ограничение на положение курсора снизу
        if (y >= scb_ph) y = scb_ph-1;
        // вычисляем новую высоту шкалы
        double hy = scb_hy * (scb_ph - scp_y) / (scb_ph - y);
        // применяем ограничения
        hy = limitScale(hy,scb_hy);
        // вычисляем новую верхнюю границу
        double yt = scb_yb + hy;
        // устанавливаем ее для вертикальной шкалы
        it->isb_y->set(scb_yb,yt);
        bndCh = bcVer;  // изменилась граница на вертикальной шкале
        break;
    }
        // для прочих режимов ничего не делаем
    default: ;
    }
    // смотрим, что изменилось
    switch (bndCh)
    {
        // граница на горизонтальной шкале:
        // перестраиваем график (синхронно с остальными)
    case bcHor: zoom->synReplot(ind); break;
        // граница на вертикальной шкале:
        // перестраиваем график
    case bcVer: plt->replot(); break;
        // иначе ничего не делаем
    default: ;
    }
}

// Обработчик событий от мыши для шкалы
void QAxisSynZoomSvc::axisMouseEvent(QEvent *event,int k,int a)
{
    // создаем указатель на событие от мыши
    QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
    // в зависимости от типа события вызываем соответствующий обработчик
    switch (event->type())
    {
        // нажата кнопка мыши
    case QEvent::MouseButtonPress: startAxisZoom(mEvent,k,a); break;
        // перемещение мыши
    case QEvent::MouseMove: procAxisZoom(mEvent,k,a); break;
        // отпущена кнопка мыши
    case QEvent::MouseButtonRelease: endAxisZoom(mEvent,k,a); break;
        // для прочих событий ничего не делаем
    default: ;
    }
}

// Обработчик нажатия на кнопку мыши над шкалой
// (включение изменения масштаба шкалы)
void QAxisSynZoomSvc::startAxisZoom(QMouseEvent *mEvent,int ind,int ax)
{
    // фиксируем исходные границы графика (если этого еще не было сделано)
    zoom->fixBounds();
    // если в данный момент еще не включен ни один из режимов
    if (zoom->regim() == QChartSynZoom::ctNone)
    {
        // получаем указатель на масштабирующий элемент
        QChartZoomItem *it = zoom->at(ind);
        // если нажата левая кнопка мыши, то
        // включаем один из режимов масштабирования
        if (mEvent->button() == Qt::LeftButton)
        {
            // получаем указатели на
            QwtPlot *plt = it->plot;                    // график
            QwtScaleWidget *sw = plt->axisWidget(ax);   // виджет шкалы
            // получаем карту основной горизонтальной шкалы
            QwtScaleMap sm = plt->canvasMap(it->masterX);
            // для того чтобы фиксировать начальные левую и правую границы
            scb_xl = sm.s1(); scb_xr = sm.s2(); scb_wx = sm.sDist();
            // аналогично получаем карту основной вертикальной шкалы
            sm = plt->canvasMap(it->masterY);
            // для того чтобы фиксировать начальные нижнюю и верхнюю границы
            scb_yb = sm.s1(); scb_yt = sm.s2(); scb_hy = sm.sDist();
            // определяем (для удобства) геометрию
            QRect gc = plt->canvas()->geometry();   // канвы графика
            QRect gw = sw->geometry();              // и виджета шкалы
            // текущее левое смещение графика (в пикселах относительно канвы)
            scb_pxl = plt->transform(it->masterX,scb_xl);
            // текущая ширина графика (в пикселах)
            scb_pw = plt->transform(it->masterX,scb_xr) - scb_pxl;
            // текущее левое смещение графика
            // (в пикселах относительно виджета шкалы)
            sab_pxl = scb_pxl + gc.x() - gw.x();
            // текущее верхнее смещение графика (в пикселах относительно канвы)
            scb_pyt = plt->transform(it->masterY,scb_yt);
            // текущая высота графика (в пикселах)
            scb_ph = plt->transform(it->masterY,scb_yb) - scb_pyt;
            // текущее верхнее смещение графика
            // (в пикселах относительно виджета шкалы)
            sab_pyt = scb_pyt + gc.y() - gw.y();
            // запоминаем текущее положение курсора относительно канвы
            // (за вычетом смещений графика)
            scp_x = mEvent->pos().x() - sab_pxl;
            scp_y = mEvent->pos().y() - sab_pyt;
            // если масштабируется горизонтальная шкала
            if (ax == QwtPlot::xBottom ||
                ax == QwtPlot::xTop)
            {
                // если левая граница меньше правой,
                if (scb_wx > 0)
                    // если ширина канвы больше минимума,
                    if (scb_pw > 36)
                    {
                        // в зависимости от положения курсора
                        // (правее или левее середины шкалы)
                        // включаем соответствующий режим - изменение
                        if (scp_x >= floor(scb_pw/2))
                            zoom->setRegim(QChartSynZoom::ctAxisHR);    // правой границы
                        else zoom->setRegim(QChartSynZoom::ctAxisHL);   // или левой
                    }
            }
            else    // иначе (масштабируется вертикальная шкала)
            {
                // если нижняя граница меньше верхней,
                if (scb_hy > 0)
                    // если высота канвы больше минимума,
                    if (scb_ph > 18)
                    {
                        // в зависимости от положения курсора
                        // (ниже или выше середины шкалы)
                        // включаем соответствующий режим - изменение
                        if (scp_y >= floor(scb_ph/2))
                            zoom->setRegim(QChartSynZoom::ctAxisVB);    // нижней границы
                        else zoom->setRegim(QChartSynZoom::ctAxisVT);   // или верхней
                    }
            }
            // если один из режимов был включен
            if (zoom->regim() != QChartSynZoom::ctNone)
            {
                // запоминаем текущий курсор
                tCursor = sw->cursor();
                // устанавливаем курсор PointingHand
                sw->setCursor(Qt::PointingHandCursor);
                // если легкий режим и включена индикация, то
                if (light && indiAxZ)
                {
                    // создаем виджет, индицирующий масштабирование шкалы
                    zwid = new QWidget(plt->axisWidget(ax));
                    // назначаем ему цвет
                    zwid->setStyleSheet(QString(
                        "background-color:rgb(%1,%2,%3);").arg(
                        awClr.red()).arg(awClr.green()).arg(awClr.blue()));
                    // и прорисовываем
                    showZoomWidget(mEvent->pos(),ind,ax);
                }
            }
        }
    }
}

// Обработчик перемещения мыши
// (выделение новых границ шкалы)
void QAxisSynZoomSvc::procAxisZoom(QMouseEvent *mEvent,int ind,int ax)
{
    // читаем режим масштабирования
    QChartSynZoom::QConvType ct = zoom->regim();
    // выходим, если не включен ни один из режимов изменения шкалы
    if (ct != QChartSynZoom::ctAxisHL &&
        ct != QChartSynZoom::ctAxisHR &&
        ct != QChartSynZoom::ctAxisVB &&
        ct != QChartSynZoom::ctAxisVT) return;
    if (light)  // если легкий режим, то
    {
        // если включена индикация, то прорисовываем виджет индикатора
        if (indiAxZ) showZoomWidget(mEvent->pos(),ind,ax);
    }
    // иначе применяем результаты перемещения границы шкалы
    else axisApplyMove(mEvent->pos(),ind,ax);
}

// Обработчик отпускания кнопки мыши
// (выполнение изменения масштаба шкалы)
void QAxisSynZoomSvc::endAxisZoom(QMouseEvent *mEvent,int ind,int ax)
{
    // читаем режим масштабирования
    QChartSynZoom::QConvType ct = zoom->regim();
    // выходим, если не включен ни один из режимов изменения шкалы
    if (ct != QChartSynZoom::ctAxisHL &&
        ct != QChartSynZoom::ctAxisHR &&
        ct != QChartSynZoom::ctAxisVB &&
        ct != QChartSynZoom::ctAxisVT) return;
    // если отпущена левая кнопка мыши, то
    if (mEvent->button() == Qt::LeftButton)
    {
        if (light)  // если легкий режим, то
        {
            // если включена индикация, то удаляем виджет индикатора
            if (indiAxZ) delete zwid;
            // применяем результаты перемещения границы шкалы
            axisApplyMove(mEvent->pos(),ind,ax);
        }
        // воостанавливаем курсор
        zoom->at(ind)->plot->axisWidget(ax)->setCursor(tCursor);
        // выключаем режим масштабирования
        zoom->setRegim(QChartSynZoom::ctNone);
    }
}
