/**********************************************************/
/*                                                        */
/*           Реализация класса QWheelSynZoomSvc           */
/*                      Версия 1.0.3                      */
/*                                                        */
/* Разработал Мельников Сергей Андреевич,                 */
/* г. Каменск-Уральский Свердловской обл., 2012 г.,       */
/* при поддержке Ю. А. Роговского, г. Новосибирск.        */
/*                                                        */
/* Разрешается свободное использование и распространение. */
/* Упоминание автора обязательно.                         */
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

// Конструктор
QWheelSynZoomSvc::QWheelSynZoomSvc() :
    QObject()
{
    // назначаем коэффициент, определяющий изменение масштаба графика
    // при вращении колеса мыши
    sfact = 1.2;
}

// Прикрепление интерфейса к менеджеру масштабирования
void QWheelSynZoomSvc::attach(QChartSynZoom *zm)
{
    // запоминаем указатель на менеджер масштабирования
    zoom = zm;
    // для всех графиков выполняем действия по добавлению нового графика
    for (int k=0; k < zoom->count(); k++) connectPlot(k);
    // назначаем слот сигнала - Добавление нового графика
    connect(zm,SIGNAL(plotAppended(int)),SLOT(connectPlot(int)));
}

// Задание коэффициента масштабирования графика
// при вращении колеса мыши (по умолчанию он равен 1.2)
void QWheelSynZoomSvc::setWheelFactor(double fact) {
    sfact = fact;
}

// Слот сигнала - Добавление нового графика
void QWheelSynZoomSvc::connectPlot(int ind)
{
    // назначаем обработчик событий (фильтр событий)
    zoom->at(ind)->plot->installEventFilter(this);
}

// Обработчик всех событий
bool QWheelSynZoomSvc::eventFilter(QObject *target,QEvent *event)
{
    int ind = -1;   // пока график не найден
    // просматриваем список графиков
    for (int k=0; k < zoom->count(); k++)
        // если событие произошло для данного графика, то
        if (target == (QObject *)zoom->at(k)->plot)
        {
            ind = k;    // запоминаем номер графика
            break;      // прекращаем поиск
        }
    // если график был найден, то
    if (ind >= 0)
    {
        // если произошло одно из событий от клавиатуры, то
        if (event->type() == QEvent::KeyPress ||
            event->type() == QEvent::KeyRelease)
            switchWheel(event); // вызываем соответствующий обработчик
        // если событие вызвано вращением колеса мыши, то
        if (event->type() == QEvent::Wheel)
        {
            procWheel(event,ind);   // вызываем соответствующий обработчик
            zoom->updateAllPlots(); // обновляем все графики
        }
    }
    // передаем управление стандартному обработчику событий
    return QObject::eventFilter(target,event);
}

// Обработчик нажатия/отпускания клавиши Ctrl или Shift
void QWheelSynZoomSvc::switchWheel(QEvent *event)
{
    // читаем режим масштабирования
    QChartSynZoom::QConvType ct = zoom->regim();
    // создаем указатель на событие от клавиатуры
    QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
    // переключаем режим в зависимости от клавиши
    switch (kEvent->key())
    {
    // клавиша Ctrl
    case Qt::Key_Control:
        // в зависимости от события
        switch (event->type())
        {
        // клавиша нажата
        case QEvent::KeyPress:
            // если не включен никакой другой режим,
            if (ct == QChartSynZoom::ctNone)
                // то включаем режим Wheel
                zoom->setRegim(QChartSynZoom::ctWheel);
        // клавиша отпущена
        case QEvent::KeyRelease:
            // если включен режим Wheel,
            if (ct == QChartSynZoom::ctWheel)
                // то выключаем его
                zoom->setRegim(QChartSynZoom::ctNone);
            break;
        // иначе ничего не делаем
        default: ;
        }
        break;
    // клавиша Shift
    case Qt::Key_Shift:
        // в зависимости от события
        switch (event->type())
        {
        // клавиша нажата
        case QEvent::KeyPress:

#ifdef R_SHIFT
// платформа Win или X11
            // если не включен никакой другой режим, то
            if (ct == QChartSynZoom::ctNone)
            {
                // если нажат правый Shift,
                if (kEvent->nativeScanCode() == R_SHIFT)
                    // то включаем режим ctHorWheel
                    zoom->setRegim(QChartSynZoom::ctHorWheel);
                // иначе (нажат левый Shift) включаем режим ctVerWheel
                else zoom->setRegim(QChartSynZoom::ctVerWheel);
            }
#else
// неизвестная платформа
            // если не включен никакой другой режим,
            if (ct == QChartSynZoom::ctNone)
                // то включаем режим ctVerWheel
                zoom->setRegim(QChartSynZoom::ctVerWheel);
#endif

            break;
        // клавиша отпущена
        case QEvent::KeyRelease:

#ifdef R_SHIFT
// платформа Win или X11
            // если отпущен правый Shift,
            if (kEvent->nativeScanCode() == R_SHIFT)
            {
                // если включен режим ctHorWheel,
                if (ct == QChartSynZoom::ctHorWheel)
                    // то выключаем его
                    zoom->setRegim(QChartSynZoom::ctNone);
            }
            // иначе (отпущен левый Shift)
            // если включен режим ctVerWheel,
            else if (ct == QChartSynZoom::ctVerWheel)
                // то выключаем его
                zoom->setRegim(QChartSynZoom::ctNone);
#else
// неизвестная платформа
            // если включен режим ctVerWheel,
            if (ct == QChartSynZoom::ctVerWheel)
                // то выключаем его
                zoom->setRegim(QChartSynZoom::ctNone);
#endif

            break;
        // иначе ничего не делаем
        default: ;
        }
        break;
        // для остальных ничего не делаем
    default: ;
    }
}

// Применение изменений по вращении колеса мыши
void QWheelSynZoomSvc::applyWheel(QEvent *event,bool ax,bool ay,int ind)
{
    // приводим тип QEvent к QWheelEvent
    QWheelEvent *wEvent = static_cast<QWheelEvent *>(event);
    // если вращается вертикальное колесо мыши
    if (wEvent->orientation() == Qt::Vertical)
    {
        // определяем угол поворота колеса мыши
        // (значение 120 соответствует углу поворота 15°)
        int wd = wEvent->delta();
        // вычисляем масштабирующий множитель
        // (во сколько раз будет увеличен/уменьшен график)
        double kw = sfact*wd/120;
        if (wd != 0)    // если колесо вращалось, то
        {
            // фиксируем исходные границы графика (если этого еще не было сделано)
            zoom->fixBounds();
            // получаем указатели на
            QChartZoomItem *it = zoom->at(ind); // масштабирующий элемент
            QwtPlot *plt = it->plot;            // и график
            if (ax) // если задано масштабирование по горизонтали
            {
                // получаем карту основной горизонтальной шкалы
                QwtScaleMap sm = plt->canvasMap(it->masterX);
                // определяем центр отображаемого на шкале x интервала
                double mx = (sm.s1()+sm.s2())/2;
                // и полуширину интервала
                double dx = (sm.s2()-sm.s1())/2;
                // в зависимости от знака угла поворота колеса мыши
                // уменьшаем полуширину отображаемых интервалов в kw раз
                if (wd > 0) dx /= kw;
                // или увеличиваем полуширину отображаемых интервалов в -kw раз
                else dx *= -kw;
                // устанавливаем новые левую и правую границы шкалы для оси x
                // (центр изображаемой части графика остается на месте,
                // а границы удаляются от центра, т.о. изображение графика уменьшается)
                zoom->setHorizontScale(mx-dx,mx+dx);
            }
            if (ay) // если задано масштабирование по вертикали
            {
                // получаем карту основной вертикальной шкалы
                QwtScaleMap sm = plt->canvasMap(it->masterY);
                // определяем центр отображаемого на шкале y интервала
                double my = (sm.s1()+sm.s2())/2;
                // и полуширину интервала
                double dy = (sm.s2()-sm.s1())/2;
                // в зависимости от знака угла поворота колеса мыши
                // уменьшаем полуширину отображаемых интервалов в kw раз
                if (wd > 0) dy /= kw;
                // увеличиваем полуширину отображаемых интервалов в -kw раз
                else dy *= -kw;
                // устанавливаем новые нижнюю и верхнюю границы вертикальной шкалы
                // (центр изображаемой части графика остается на месте,
                // а границы удаляются от центра, т.о. изображение графика уменьшается)
                it->isb_y->set(my-dy,my+dy);
            }
            // перестраиваем график (синхронно с остальными)
            zoom->synReplot(ind);
        }
    }
}

// Обработчик вращения колеса мыши
void QWheelSynZoomSvc::procWheel(QEvent *event,int ind)
{
    // в зависимости от включенного режима вызываем
    // масштабирование с соответствующими параметрами
    switch (zoom->regim())
    {
        // Wheel - была нажата клавиша Ctrl -
            // масштабирование по обеим осям
    case QChartSynZoom::ctWheel: applyWheel(event,true,true,ind); break;
        //VerWheel - была нажата левая клавиша Shift -
            // масштабирование только по вертикальной оси
    case QChartSynZoom::ctVerWheel: applyWheel(event,false,true,ind); break;
        // HorWheel - была нажата правая клавиша Shift -
            // масштабирование только по горизонтальной оси
    case QChartSynZoom::ctHorWheel: applyWheel(event,true,false,ind); break;
        // для прочих режимов ничего не делаем
    default: ;
    }
}
