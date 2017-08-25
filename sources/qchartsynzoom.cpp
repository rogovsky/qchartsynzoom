/**********************************************************/
/*                                                        */
/*            Реализация класса QChartSynZoom             */
/*                      Версия 1.6.3                      */
/*                                                        */
/* Разработал Мельников Сергей Андреевич,                 */
/* г. Каменск-Уральский Свердловской обл., 2012 г.,       */
/* при поддержке Ю. А. Роговского, г. Новосибирск.        */
/*                                                        */
/* Разрешается свободное использование и распространение. */
/* Упоминание автора обязательно.                         */
/*                                                        */
/**********************************************************/

#include "qchartsynzoom.h"

#include <qwt_scale_widget.h>

#include <math.h>

// Конструктор для графиков в сплиттере
QChartSynZoom::QChartSynZoom(QSplitter *sp) :
    QObject(sp)
{
    common();   // общая часть конструктора

    // получаем главное окно
    mwin = generalParent(sp);
    // и назначаем обработчик событий (фильтр событий)
    mwin->installEventFilter(this);

    splt = sp;  // получаем сплиттер
    // назначаем слот обработки сигнала изменения сплиттера
    connect(splt,SIGNAL(splitterMoved(int,int)),SLOT(splitterMoved()));
}

// Конструктор для отдельного графика
QChartSynZoom::QChartSynZoom(QwtPlot *plt) :
    QObject()
{
    common();   // общая часть конструктора
    // выполняем действия по добавлению нового графика
    appendPlot(plt);
}

// Конструктор для отдельного графика,
// который будет добавлен позже
QChartSynZoom::QChartSynZoom() :
    QObject()
{
    common();   // общая часть конструктора
}

// Деструктор
QChartSynZoom::~QChartSynZoom()
{
    // удаляем интерфейс перемещенния графика
    delete drzmsvc;
    // удаляем интерфейс масштабирования графика
    delete mnzmsvc;
    // для всех графиков удаляем элементы масштабирования
    for (int k=0; k < items->count(); k++) delete items->at(k);
    // очищаем и удаляем
    items->clear(); delete items;   // список имен графиков
}

// Текущий режим масштабирования
QChartSynZoom::QConvType QChartSynZoom::regim() {
    return convType;
}

// Переключение режима масштабирования
void QChartSynZoom::setRegim(QChartSynZoom::QConvType ct) {
    convType = ct;
}

// Количество масштабирующих элементов в списке
int QChartSynZoom::count() {
    return items->count();
}

// Указатель на масштабирующий элемент
QChartZoomItem *QChartSynZoom::at(int ind)
{
    // возвращаем NULL, если некорректный индекс
    if (ind < 0 || ind >= items->count()) return NULL;
    // (иначе) возвращаем указатель на масштабирующий элемент
    return items->at(ind);
}

// Общая часть конструктора
void QChartSynZoom::common()
{
    mwin = NULL;    // главное окно пока не определено
    splt = NULL;    // сплиттер отсутствует

    // создаем список элементов, отвечающих за каждый график
    items = new QList<QChartZoomItem *>();

    // сбрасываем флаг для того, чтобы перед первым изменением масштаба
    // текущие границы графика были зафиксированы в качестве исходных
    isbF = false;
    // сбрасываем признак режима
    convType = ctNone;

    inSync = false; // очищаем флаг обновления шкалы
    tick = NULL;    // и обнуляем указатель на таймер паузы
                    // перед первым обновлением графиков

    // создаем интерфейс масштабирования графика
    mnzmsvc = new QMainSynZoomSvc();
    // и прикрепляем его к менеджеру
    mnzmsvc->attach(this);

    // создаем интерфейс перемещенния графика
    drzmsvc = new QDragSynZoomSvc();
    // и прикрепляем его к менеджеру
    drzmsvc->attach(this);
}

// Определение главного родителя
QObject *QChartSynZoom::generalParent(QObject *sp)
{
    // берем в качестве предыдущего родителя сплиттер
    // (возможен и другой объект в аргументе функции)
    QObject *gp = sp;
    // определяем родителя на текущем уровне
    QObject *tp = gp->parent();
    // пока родитель на текущем уровне не NULL
    while (tp != NULL)
    {
        // понижаем уровень:
        // запоминаем в качестве предыдущего родителя текущий
        gp = tp;
        // определяем родителя на следующем уровне
        tp = gp->parent();
    }
    // возвращаем в качестве главного родителя предыдущий
    return gp;
}

// Назначение отложенного обновления
void QChartSynZoom::initDelayedUpdate()
{
    // если это еще не сделано
    if (tick == NULL)
    {
        // создаем таймер паузы перед первым обновлением графиков
        tick = new QTimer(this);
        // назначаем ему слот обработки сигнала timeout
        connect(tick,SIGNAL(timeout()),this,SLOT(delayedUpdate()));
        // и стартуем таймер на 20 мс
        tick->start(20);
    }
    else    // иначе (таймер уже создан)
    {
        tick->stop();       // останавливаем таймер
        tick->start(20);    // и заново стартуем его на 20 мс
    }
}

// Добавление графика под опеку менеджера
void QChartSynZoom::appendPlot(QwtPlot *plt)
{
//    plt->replot();  // перестраиваем график
    // если главное окно еще не определено, то
    if (mwin == NULL)
    {
        // получаем главное окно
       mwin = generalParent(plt);
       // и назначаем обработчик событий (фильтр событий)
       mwin->installEventFilter(this);
    }
    // создаем для графика масштабирующий элемент
    QChartZoomItem *zit = new QChartZoomItem(plt);
    int n = items->count();
    // при добавлении графиков, начиная со второго
    if (n > 0)
    {
        // получаем указатель на масштабирующий элемент для первого графика
        QChartZoomItem *it0 = items->at(0);
        // получаем карту основной горизонтальной шкалы для него
        QwtScaleMap sm = it0->plot->canvasMap(it0->masterX);
        // устанавливаем для добавляемого графика такие же границы
        // горизонтальной шкалы как и у первого графика
        zit->isb_x->set(sm.s1(),sm.s2());
    }
    plt->replot();  // перестраиваем график
    // при добавлении самого первого графика
    if (n == 0)
    {
        // запоминаем количество делений на горизонтальной шкале
        mstHorDiv = plt->axisMaxMajor(zit->masterX);
        slvHorDiv = plt->axisMaxMajor(zit->slaveX);
    }
    items->append(zit); // добавляем элемент в список
    // и назначаем обработчик событий (фильтр событий)
    plt->installEventFilter(this);
    // для всех шкал графика
    for (int ax=0; ax < QwtPlot::axisCnt; ax++)
    {
        // назначаем обработчик событий (фильтр событий)
        plt->axisWidget(ax)->installEventFilter(this);
        // назначаем слот обработки сигнала изменения шкалы
        connect(plt->axisWidget(ax),SIGNAL(scaleDivChanged()),SLOT(scaleDivChanged()));
    }
    // назначаем отложенное обновление графиков
    initDelayedUpdate();
    // посылаем сигнал - добавлен график
    emit plotAppended(items->count() - 1);
}

// Установка цвета рамки, задающей новый размер графика
void QChartSynZoom::setRubberBandColor(QColor clr) {
    mnzmsvc->setRubberBandColor(clr);
}

// Включение/выключение индикации выделяемой области
void QChartSynZoom::indicateZoom(bool indi) {
    mnzmsvc->indicateZoom(indi);
}

// Включение/выключение легкого режима
void QChartSynZoom::setLightMode(bool lm)
{
    light = lm; // запоминаем значение
    // и устанавливаем его для интерфейса QDragSynZoomSvc
    drzmsvc->setLightMode(lm);
}

// Включение/выключение индикации перемещаемой области графика
// (имеет эффект, если включен легкий режим)
void QChartSynZoom::indicateDragBand(QDragIndiStyle indi) {
    drzmsvc->setIndicatorStyle(indi);
}

// Установка цвета виджета индикатора перемещения
void QChartSynZoom::setDragBandColor(QColor clr) {
    drzmsvc->setDragBandColor(clr);
}

// Фиксация текущих границ графика в качестве исходных
void QChartSynZoom::fixBoundaries() {
    // здесь только сбрасывается флаг и тем самым
    // указывается на необходимость фиксировать границы
    isbF = false;
    // фактическая фиксация границ произойдет в момент начала
    // какого-либо преобразования при вызове fixBounds()
}

// Включение/выключение синхронизации дополнительной
// горизонтальной шкалы для графика с индексом ind
void QChartSynZoom::setHorSync(bool hs,int ind)
{
    // выходим, если некорректный индекс
    if (ind < 0 || ind >= items->count()) return;
    // если состояние синхронизации изменилось, то
    // перестраиваем график синхронно с остальными
    if (items->at(ind)->setHSync(hs)) synReplot(ind);
}

// Включение/выключение синхронизации дополнительной
// горизонтальной шкалы для всех графиков
void QChartSynZoom::setHorSync(bool hs)
{
    // пока состояние синхронизации не изменилось
    bool repF = false;
    // перебираем все графики
    for (int k=0; k < items->count(); k++)
        // включаем/выключаем синхронизацию
        repF |= items->at(k)->setHSync(hs);
    // если состояние синхронизации у какого-либо из графиков изменилось,
    // то перестраиваем первый график синхронно с остальными
    if (repF) synReplot(0);
}

// Включение/выключение синхронизации дополнительной
// вертикальной шкалы для графика с индексом ind
void QChartSynZoom::setVerSync(bool vs,int ind)
{
    // выходим, если некорректный индекс
    if (ind < 0 || ind >= items->count()) return;
    // если состояние синхронизации изменилось, то
    // перерисовывем график с выравниванием
    if (items->at(ind)->setVSync(vs)) alignRepaint(ind);
}

// Включение/выключение синхронизации дополнительной
// вертикальной шкалы для всех графиков
void QChartSynZoom::setVerSync(bool vs)
{
    // перебираем все графики
    for (int k=0; k < items->count(); k++)
        // если состояние синхронизации графика изменилось,
        // то перерисовывем его с выравниванием
        if (items->at(k)->setVSync(vs)) alignRepaint(k);
}

// Включение/выключение синхронизации дополнительной горизонтальной
// и вертикальной шкалы для графика с индексом ind
void QChartSynZoom::setSync(bool s,int ind)
{
    // выходим, если некорректный индекс
    if (ind < 0 || ind >= items->count()) return;
    // включаем/выключаем синхронизацию горизонтальной шкалы
    bool hRepF = items->at(ind)->setHSync(s);
    // и вертикальной
    bool vRepF = items->at(ind)->setVSync(s);
    // если изменилось состояние синхронизации горизонтальной шкалы,
    // то перестраиваем график синхронно с остальными
    if (hRepF) synReplot(ind);
    // иначе если изменилось состояние синхронизации вертикальной шкалы,
    // то перерисовывем график с выравниванием
    else if (vRepF) alignRepaint(ind);
}

// Включение/выключение синхронизации дополнительной горизонтальной
// и вертикальной шкалы для всех графиков
void QChartSynZoom::setSync(bool s)
{
    // пока состояние синхронизации горизонтальной шкалы не изменилось
    bool hRepF = false;
    // перебираем все графики
    for (int k=0; k < items->count(); k++)
    {
        // получаем указатель на масштабирующий элемент
        QChartZoomItem *it = items->at(k);
        // включаем/выключаем синхронизацию горизонтальной шкалы
        hRepF |= it->setHSync(s);
        // и вертикальной
        bool vRepF = it->setVSync(s);
        // если изменилось состояние синхронизации вертикальной шкалы
        // и при этом состояние синхронизации горизонтальной шкалы
        // еще не изменилось ни у одного из графиков, то
        // перерисовывем график с выравниванием
        if (vRepF) if (!hRepF) alignRepaint(k);
    }
    // если состояние синхронизации горизонтальной шкалы
    // у какого-либо из графиков изменилось,
    // то перестраиваем первый график синхронно с остальными
    if (hRepF) synReplot(0);
}

// Создание списка ширины меток горизонтальной шкалы
QList<int> *QChartSynZoom::getLabelWidths(QwtPlot *plt,int xAx)
{
    // получаем шрифт, использующийся на горизонтальной шкале
    QFont fnt = plt->axisFont(xAx);

    // получаем список основных меток горизонтальной шкалы
#if   QWT_VERSION < 0x060000   // qwt-5.2.x
    QwtValueList vl = plt->axisScaleDiv(xAx)->ticks(QwtScaleDiv::MajorTick);
#elif QWT_VERSION > 0x060099   // qwt-6.1.x
    QList<double> vl = plt->axisScaleDiv(xAx).ticks(QwtScaleDiv::MajorTick);
#else                          // qwt-6.0.x
    QList<double> vl = plt->axisScaleDiv(xAx)->ticks(QwtScaleDiv::MajorTick);
#endif

    // создаем список ширины меток
    QList<int> *res = new QList<int>();
    // перебираем все метки
    for (int k=0; k < vl.count(); k++)
        // и заполняем список ширины меток
        res->append(plt->axisScaleDraw(xAx)->labelSize(fnt,vl.at(k)).width());
    if (res->count() > 2)   // если в списке больше 2-х меток
    {
        // инициализируем
        int mn = res->at(0);    // минимальную ширину
        int mx = mn;            // и максимальную
        // перебираем оставшиеся метки
        for (int k=1; k < res->count(); k++)
        {
            // берем ширину метки
            int wk = res->at(k);
            // проверяем значение
            if (wk < mn) mn = wk;   // минимальной ширины
            if (wk > mx) mx = wk;   // и максимальной
        }
        // находим в списке минимальную ширину
        int i = res->indexOf(mn);
        // и если нашли, то удаляем ее из списка
        if (i >= 0) res->removeAt(i);
        // находим в списке максимальную ширину
        i = res->indexOf(mx);
        // и если нашли, то удаляем ее из списка
        if (i >= 0) res->removeAt(i);
    }
    // возвращаем список ширины меток
    return res;
}

// Определение средней ширины меток горизонтальной шкалы
int QChartSynZoom::meanLabelWidth(int xAx)
{
    // инициализируем среднюю ширину надписи на одну метку
    int res = 0;
    // получаем список ширины меток
    QList<int> *lbwds = getLabelWidths(items->at(0)->plot,xAx);
    // суммируем ширину надписей всех меток
    for (int k=0; k < lbwds->count(); k++) res += lbwds->at(k);
    // и определяем среднюю ширину
    if (lbwds->count() > 0) res = floor(res / lbwds->count());
    // удаляем список ширины меток
    delete lbwds;
    // возвращаем среднюю ширину надписи на одну метку
    return res;
}

// Определение минимальной ширины графика
int QChartSynZoom::minimumPlotWidth()
{
    // инициализируем минимальное значение ширины графика
    int res = items->at(0)->plot->canvas()->size().width();
    // для оставшихся графиков
    for (int k=1; k < items->count(); k++)
    {
        // берем ширину графика
        int wk = items->at(k)->plot->canvas()->size().width();
        // если она меньше минимальной, то берем ее
        if (wk < res) res = wk; // в качестве минимальной
    }
    // возвращаем минимальное значение ширины графика
    return res;
}

// Обновление одной из пары горизонтальных шкал графиков
bool QChartSynZoom::updateOneHorAxis(int xAx,int *hDiv)
{
    // определяем среднюю ширину надписи на одну метку
    int mwd = meanLabelWidth(xAx);
    // определяем минимальное значение ширины канвы
    int mnw = minimumPlotWidth();
    const int dw = 48;  // минимальное расстояние между метками
    // максимально допустимое число основных меток на шкале равно
    // отношению минимальной ширины канвы к средней ширине надписи
    // на одну метку (с некоторым запасом)
    int dv = floor(mnw/(mwd+dw));
    bool setF = false;  // обновления шкалы пока нет
    // если максимально допустимое число основных меток
    // на шкале изменилось в большую сторону, то
    if (dv > *hDiv)
    {
        // если после применения изменений окажется, что
        // средняя ширина метки увеличилась на 1 и при этом надписи
        // не поместятся на шкале с требуемым интервалом, то
        // декрементируем максимально допустимое число основных меток
        if (dv*(mwd+dw+1) > mnw) dv--;
        // если количество меток все-таки увеличилось, то
        // устанавливаем флаг - требуется обновление шкалы
        if (dv > *hDiv) setF = true;
    }
    // если максимально допустимое число основных меток
    // на шкале изменилось в меньшую сторону, то
    if (dv < *hDiv)
    {
        // если после применения изменений окажется, что
        // средняя ширина метки уменьшилась на 1 и при этом надписи
        // с требуемым интервалом занимают слишком мало места на шкале, то
        // инкрементируем максимально допустимое число основных меток
        if (dv*(mwd+dw-1) < mnw) dv++;
        // если количество меток все-таки уменьшилось, то
        // устанавливаем флаг - требуется обновление шкалы
        if (dv < *hDiv) setF = true;
    }
    // если требуется обновление шкалы, то запоминаем
    // вычисленное максимально допустимое число основных меток
    if (setF) *hDiv = dv;   // для горизонтальной шкалы его
    // и возвращаем флаг обновления
    return setF;
}

// Обновление горизонтальной шкалы графиков
bool QChartSynZoom::updateHorAxis()
{
    // получаем указатель на масштабирующий элемент
    QChartZoomItem *it0 = items->at(0);
    bool mstF = updateOneHorAxis(it0->masterX,&mstHorDiv);
    if (mstF)   // если требуется обновление шкалы, то
        // для всех графиков
        for (int k=0; k < items->count(); k++)
        {
            // получаем указатель на
            QChartZoomItem *it = items->at(k);  // масштабирующий элемент
            QwtPlot *plt = it->plot;            // и график
            // устанавливаем вычисленное максимально допустимое число
            // основных меток для основной горизонтальной шкалы
            plt->setAxisMaxMajor(it->masterX,mstHorDiv);
        }
    bool slvF = updateOneHorAxis(it0->slaveX,&slvHorDiv);
    if (slvF)   // если требуется обновление шкалы, то
        // для всех графиков
        for (int k=0; k < items->count(); k++)
        {
            // получаем указатель на
            QChartZoomItem *it = items->at(k);  // масштабирующий элемент
            QwtPlot *plt = it->plot;            // и график
            // устанавливаем вычисленное максимально допустимое число
            // основных меток для дополнительной горизонтальной шкалы
            plt->setAxisMaxMajor(it->slaveX,slvHorDiv);
        }
    // возвращаем общий флаг обновления
    return mstF || slvF;
}

// Обновление вертикальной шкалы графика
bool QChartSynZoom::updateVerAxis(QwtPlot *plt,int yAx,int *vDiv)
{
    // получаем шрифт, использующийся на вертикальной шкале
    QFont fnt = plt->axisFont(yAx);
    // узнаем значение верхней границы вертикальной шкалы
#if QWT_VERSION < 0x060099   // qwt-5.2.x + qwt-6.0.x
    double mxl = plt->axisScaleDiv(yAx)->upperBound();
#else                        // qwt-6.1.x
    double mxl = plt->axisScaleDiv(yAx).upperBound();
#endif

    // определяем размер надписи, соответствующей этому значению при заданном шрифте
#if QWT_VERSION < 0x060000
    QSize szlb = plt->axisScaleDraw(yAx)->labelSize(fnt,mxl);
#else
    QSizeF szlb = plt->axisScaleDraw(yAx)->labelSize(fnt,mxl);
#endif

    // максимально допустимое число основных меток на шкале равно
    // отношению высоты канвы к высоте надписи (с некоторым запасом)
    int dv = floor(plt->canvas()->size().height()/(szlb.height()+8));
    // если вычисленное значение не совпадает с установленным, то
    if (dv != *vDiv)
    {
        // устанавливаем вычисленное максимально допустимое число
        // основных меток для вертикальной шкалы
        plt->setAxisMaxMajor(yAx,dv);
        *vDiv = dv;     // передаем его в вызывающую процедуру
        return true;    // и возвращаем флаг - шкала обновилась
    }
    // иначе возвращаем флаг - шкала не обновлялась
    else return false;
}

// Обновление всех графиков
void QChartSynZoom::updateAllPlots()
{
    // пока нет обновления
    bool repF = false;
    // узаем количество графиков
    int n = items->count();
    // если есть хоть один график, то
    // обновляем горизонтальную шкалу
    if (n > 0) repF = updateHorAxis();
    // для всех графиков
    for (int k=0; k < n; k++)
    {
        // получаем указатель на
        QChartZoomItem *it = items->at(k);  // масштабирующий элемент
        QwtPlot *plt = it->plot;            // и график
        // обновляем вертикальную шкалу
        repF |= updateVerAxis(plt,it->masterY,&it->mstVerDiv);
        repF |= updateVerAxis(plt,it->slaveY,&it->slvVerDiv);
    }
// Если какая-либо из шкал действительно обновилась
// (т.е. изменилось максимальное количество меток на шкале),
// то перестраивается верхний график (синхронно с остальными)
    if (repF) synReplot(0);
}

// Обработчик всех событий
bool QChartSynZoom::eventFilter(QObject *target,QEvent *event)
{
    // если событие произошло для главного окна,
    if (target == mwin)
        // если окно было отображено на экране, или изменились его размеры, то
        if (event->type() == QEvent::Show ||
            event->type() == QEvent::Resize)
            updateAllPlots();   // обновляем все графики
    int ind = -1;   // пока график не найден
    // просматриваем список графиков
    for (int k=0; k < items->count(); k++)
        // если событие произошло для данного графика, то
        if (target == (QObject *)items->at(k)->plot)
        {
            ind = k;    // запоминаем номер графика
            break;      // прекращаем поиск
        }
    // если график был найден, то
    if (ind >= 0)
        // если изменились размеры графика, то
        if (event->type() == QEvent::Resize)
            updateAllPlots();   // обновляем все графики
    // передаем управление стандартному обработчику событий
    return QObject::eventFilter(target,event);
}

// Слот сигнала - Изменение шкалы графика
void QChartSynZoom::scaleDivChanged()
{
    // выходим, если изменение шкалы уже обрабатывается
    if (inSync) return;
    inSync = true;  // устанавливаем флаг обновления шкалы
    int ind = -1;   // график пока не найден
    int axId = -1;  // шкала, соответственно, тоже
    // просматриваем список графиков
    for (int k=0; k < items->count(); k++)
        // для каждого просматриваем список шкал
        for (int ax=0; ax < QwtPlot::axisCnt; ax++)
            // если изменилась данная шкала, то
            if (items->at(k)->plot->axisWidget(ax) == sender())
            {
                ind = k;    // запоминаем номер графика,
                axId = ax;  // номер шкалы
                break;      // прекращаем поиск шкалы
                break;      // и поиск графика
            }
    if (ind >= 0)   // если график был найден,
    {
        // если изменилась горизонтальная шкала, то
        if (axId == QwtPlot::xBottom || axId == QwtPlot::xTop)
        {
            // получаем указатель на изменившийся масштабирующий элемент
            QChartZoomItem *it = items->at(ind);
            // если изменилась основная шкала или дополнительная,
            // но при этом включена синхронизация,то
            if (it->isb_x->affected((QwtPlot::Axis)axId))
                // просматриваем список графиков
                for (int k=0; k < items->count(); k++)
                    if (k != ind)   // для каждого, несовпадающего с изменившимся
                        // устанавливаем количество делений на шкале такое же,
                        // как на изменившемся графике
#if QWT_VERSION < 0x060099   // qwt-5.2.x + qwt-6.0.x
                        items->at(k)->isb_x->setDiv(it->plot->axisScaleDiv(axId));
#else                        // qwt-6.1.x
                    {
                        // TODO: should be reimplemented to avvoid additional copy constructor usage
                        // 1. make copy
                        QwtScaleDiv tmpScaleDiv( it->plot->axisScaleDiv(axId) );
                        // 2. pass the pointer to the copy to the function
                        items->at(k)->isb_x->setDiv( &tmpScaleDiv );
                    }
#endif
        }
        else // иначе (изменилась вертикальная шкала)
        {
            // выравниваем графики по вертикали
            alignVAxes(QwtPlot::yLeft);     // слева
            alignVAxes(QwtPlot::yRight);    // и справа
        }
    }
    inSync = false; // очищаем флаг обновления шкалы
}

// Слот сигнала - Изменение сплиттера
void QChartSynZoom::splitterMoved() {
    // назначаем отложенное обновление графиков
    initDelayedUpdate();
}

// Слот сигнала - Отложенное обновление графиков
void QChartSynZoom::delayedUpdate()
{
    // выполняем обновление графиков
    updateAllPlots();
    // освобождаем таймер паузы от сигнала timeout
    tick->disconnect(SIGNAL(timeout()));
    delete tick;    // удаляем таймер
    tick = NULL;    // и обнуляем указатель на него
}

// Фактическая фиксация текущих границ графика
// в качестве исходных (если флаг isbF сброшен)
void QChartSynZoom::fixBounds()
{
    // если этого еще не было сделано
    if (!isbF)
    {
        // для всех графиков
        for (int k=0; k < items->count(); k++)
        {
            // получаем указатель на масштабирующий элемент
            QChartZoomItem *it = items->at(k);
            // фиксируем границы
            it->isb_x->fix();   // горизонтальные
            it->isb_y->fix();   // и вертикальные
        }
        // устанавливаем флажок фиксации границ графика
        isbF = true;
    }
}

// Восстановление исходных границ графика
void QChartSynZoom::restBounds(int ind)
{
    // получаем количество масштабирующих элементов в списке
    int n = items->count();
    // выходим, если некорректный индекс
    if (ind < 0 || ind >= n) return;
    // для всех графиков
    for (int k=0; k < n; k++)
        // восстанавливаем запомненные ранее левую и правую
        // границы шкалы для горизонтальной оси
        items->at(k)->isb_x->rest();
    // восстанавливаем нижнюю и верхнюю границы вертикальной шкалы
    items->at(ind)->isb_y->rest();
    // перестраиваем график (синхронно с остальными)
    synReplot(ind);
}

// Синхронная установка границ горизонтальной шкалы графиков
void QChartSynZoom::setHorizontScale(double xl,double xr)
{
    // для всех графиков
    for (int k=0; k < items->count(); k++)
        // устанавливаем заданные границы горизонтальной шкалы
        items->at(k)->isb_x->set(xl,xr);
}

// Перестроение графиков синхронно с прикрепленным
void QChartSynZoom::synReplot(int ind)
{
    // получаем количество масштабирующих элементов в списке
    int n = items->count();
    // выходим, если некорректный индекс
    if (ind < 0 || ind >= n) return;
    // просматриваем список графиков
    for (int k=0; k < items->count(); k++)
        // если график не является обрабатываемым, то перестраиваем его
        if (k != ind) items->at(k)->plot->replot();
    // перерисовывем обрабатываемый график с выравниванием
    alignRepaint(ind);
}

// Перерисовка графика с выравниванием
void QChartSynZoom::alignRepaint(int ind)
{
    // выходим, если некорректный индекс
    if (ind < 0 || ind >= items->count()) return;
    // перестраиваем обрабатываемый график
    items->at(ind)->plot->replot();
    // выравниваем графики по вертикали
    alignVAxes(QwtPlot::yLeft);     // слева
    alignVAxes(QwtPlot::yRight);    // и справа
}

// Выравнивание графиков по вертикали
void QChartSynZoom::alignVAxes(int ax)
{
    // выходим, если шкала в аргументе не вертикальная
    if (ax != QwtPlot::yLeft && ax != QwtPlot::yRight) return;

    // объявляем список текущей минимальной ширины шкалы
    // и инициализируем максимальную потребную ширину шкалы
#if QWT_VERSION < 0x060000
    QList<int> minExt;
    int maxExt = 0;
#else
    QList<double> minExt;
    double maxExt = 0;
#endif

    bool need = false;  // пока нет потребности в выравнивании
    // перебираем все графики
    for (int k=0; k < items->count(); k++)
    {
        // получаем указатель на график
        QwtPlot *plt = items->at(k)->plot;
        // проверяем доступность шкалы
        need |= plt->axisEnabled(ax);
        // получаем указатели на виджет шкалы
        QwtScaleWidget *scaleWidget = plt->axisWidget(ax);
        // и на прорисовщик шкалы
        QwtScaleDraw *sd = scaleWidget->scaleDraw();
        // запоминаем в списке текущую минимальную ширину шкалы
        minExt.append(sd->minimumExtent());
        // и сбрасываем ее
        sd->setMinimumExtent(0);

        // вычисляем потребную ширину данной шкалы
#if QWT_VERSION < 0x060000
        int ext = sd->extent(QPen(Qt::black,scaleWidget->penWidth(),Qt::SolidLine),scaleWidget->font());
#else
        double ext = sd->extent(scaleWidget->font());
#endif

        // если она больше всех предыдущих, то
        // обновляем максимальную потребную ширину шкалы
        if (ext > maxExt) maxExt = ext;
    }
    // выходим, если нет потребности в выравнивании
    if (!need) return;
    // справа запас 1 пиксел
    if (ax == QwtPlot::yRight) maxExt += 1;
    // для всех графиков
    for (int k=0; k < items->count(); k++)
    {
        // получаем указатель на график
        QwtPlot *plt = items->at(k)->plot;
        // получаем указатель на виджет шкалы
        QwtScaleWidget *scaleWidget = plt->axisWidget(ax);

        // берем прежнюю минимальную ширину шкалы
#if QWT_VERSION < 0x060000
        int mE = minExt.at(k);
#else
        double mE = minExt.at(k);
#endif

        // если она не совпадает с потребной, то
        if (mE != maxExt)
        {
            // устанавливаем новое значение минимальной ширины шкалы
            scaleWidget->scaleDraw()->setMinimumExtent(maxExt);
            // и обновляем внешний вид графика
            plt->updateLayout();
        }
        // иначе восстанавливаем прежнее значение минимальной ширины шкалы
        else scaleWidget->scaleDraw()->setMinimumExtent(mE);
    }
}

/**********************************************************/
/*                                                        */
/*            Реализация класса QChartZoomItem            */
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

// Конструктор
QChartZoomItem::QChartZoomItem(QwtPlot *qp)
{
    // получаем график, над которым будут производиться все преобразования
    plot = qp;
    // устанавливаем ему свойство, разрешающее обрабатывать события от клавиатуры
    qp->setFocusPolicy(Qt::StrongFocus);

    // Координатная сетка
    QwtPlotGrid *grid = NULL;
    // оси, к которым она прикреплена
    int xAx;    // горизонтальная
    int yAx;    // вертикальная
    // получаем список элементов графика
    QwtPlotItemList pil = qp->itemList();
    // перебираем список элементов
    for (int k=0; k < pil.count(); k++)
    {
        // получаем указатель на элемент
        QwtPlotItem *pi = pil.at(k);
        // если это координатная сетка, то
        if (pi->rtti() == QwtPlotItem::Rtti_PlotGrid)
        {
            // запоминаем указатель на нее
            grid = (QwtPlotGrid *)pi;
            // выясняем к какой оси она прикреплена
            xAx = grid->xAxis();    // из пары горизонтальных
            yAx = grid->yAxis();    // и пары вертикальных
            // прекращаем просмотр списка элементов
            break;
        }
    }
    // если координатная сетка была найдена, то
    if (grid != NULL)
    {
        // назначаем основную и дополнительную шкалу, отдавая предпочтение
        // той, к которой прикреплена сетка
            // горизонтальную
        allocAxis(xAx,QwtPlot::xBottom + QwtPlot::xTop - xAx,&masterX,&slaveX);
            // вертикальную
        allocAxis(yAx,QwtPlot::yLeft + QwtPlot::yRight - yAx,&masterY,&slaveY);
    }
    else    // иначе (координатная сетка отсутствует)
    {
        // назначаем основную и дополнительную шкалу, отдавая предпочтение
            // нижней из горизонтальных
        allocAxis(QwtPlot::xBottom,QwtPlot::xTop,&masterX,&slaveX);
            // и левой из вертикальных
        allocAxis(QwtPlot::yLeft,QwtPlot::yRight,&masterY,&slaveY);
    }
    // запоминаем текущее максимальное количество основных делений на вертикальной шкале
    mstVerDiv = plot->axisMaxMajor(masterY);    // основной
    slvVerDiv = plot->axisMaxMajor(slaveY);     // и дополнительной
    // создаем контейнеры границ шкалы
    isb_x = new QScaleBounds(qp,masterX,slaveX);    // горизонтальной
    isb_y = new QScaleBounds(qp,masterY,slaveY);    // и вертикальной
    qp->replot();   // перестраиваем график
}

// Деструктор
QChartZoomItem::~QChartZoomItem()
{
    // удаляем контейнеры границ шкалы
    delete isb_x;    // горизонтальной
    delete isb_y;    // и вертикальной
}

// Включение/выключение синхронизации дополнительной
// горизонтальной шкалы для опекаемого графика
bool QChartZoomItem::setHSync(bool hs)
{
    // выходим, если потребное состояние синхронизации
    // дополнительной горизонтальной шкалы уже установлено
    if (isb_x->sync == hs) return false;
    // запоминаем новое состояние синхронизации
    isb_x->sync = hs;
    // переустанавливаем границы дополнительной шкалы
    isb_x->dup();
    // возвращаем признак, что состояние
    return true;    // синхронизации изменилось
}

// Включение/выключение синхронизации дополнительной
// вертикальной шкалы для опекаемого графика
bool QChartZoomItem::setVSync(bool vs)
{
    // выходим, если потребное состояние синхронизации
    // дополнительной вертикальной шкалы уже установлено
    if (isb_y->sync == vs) return false;
    // запоминаем новое состояние синхронизации
    isb_y->sync = vs;
    // переустанавливаем границы дополнительной шкалы
    isb_y->dup();
    // возвращаем признак, что состояние
    return true;    // синхронизации изменилось
}

// Назначение основной и дополнительной шкалы
void QChartZoomItem::allocAxis(int pre,int alt,
    QwtPlot::Axis *master,QwtPlot::Axis *slave)
{
    // получаем карту предпочтительной шкалы
    QwtScaleMap smp = plot->canvasMap(pre); // предпочтительной шкалы
    QwtScaleMap sma = plot->canvasMap(alt); // и альтернативной
    // если предпочтительная шкала доступна или
    // альтернативная шкала недоступна и при этом
    // границы предпочтительной шкалы не совпадают или
    // границы альтернативной шкалы совпадают, то
    if ((plot->axisEnabled(pre) ||
        !plot->axisEnabled(alt)) &&
        (smp.s1() != smp.s2() ||
         sma.s1() == sma.s2()))
    {
        // назначаем предпочтительную шкалу основной,
        *master = (QwtPlot::Axis)pre;
        // а альтернативную дополнительной
        *slave = (QwtPlot::Axis)alt;
    }
    else    // иначе
            // (предпочтительная шкала недоступна и
            // альтернативная шкала доступна или
            // границы предпочтительной шкалы совпадают и
            // границы альтернативной шкалы не совпадают)
    {
        // назначаем альтернативную шкалу основной,
        *master = (QwtPlot::Axis)alt;
        // а предпочтительную дополнительной
        *slave = (QwtPlot::Axis)pre;
    }
}

/**********************************************************/
/*                                                        */
/*             Реализация класса QScaleBounds             */
/*                      Версия 1.0.1                      */
/*                                                        */
/* Разработал Мельников Сергей Андреевич,                 */
/* г. Каменск-Уральский Свердловской обл., 2012 г.,       */
/* при поддержке Ю. А. Роговского, г. Новосибирск.        */
/*                                                        */
/* Разрешается свободное использование и распространение. */
/* Упоминание автора обязательно.                         */
/*                                                        */
/**********************************************************/

// Конструктор
QScaleBounds::QScaleBounds(QwtPlot *plt,
    QwtPlot::Axis mst,QwtPlot::Axis slv)
{
    // запоминаем
    plot = plt;     // опекаемый график
    master = mst;   // основную шкалу
    slave = slv;    // и дополнительную
    fixed = false;  // границы еще не фиксированы
    sync = false;   // синхронизации по умолчанию нет
}

// Фиксация исходных границ шкалы
void QScaleBounds::fix()
{
    // получаем карту основной шкалы
    QwtScaleMap sm = plot->canvasMap(master);
    // и запоминаем текущие левую и правую границы шкалы
    min = sm.s1(); max = sm.s2();
    // получаем карту дополнительной горизонтальной шкалы
    sm = plot->canvasMap(slave);
    // строим преобразование основной шкалы в дополнительную в виде
    //     s = a * m + b, где:
    // если границы основной шкалы не совпадают, то
    if (min != max)
    {
        // a = (s2 - s1) / (m2 - m1)
        ak = (sm.s2() - sm.s1()) / (max - min);
        // b = (m2*s1 - m1*s2) / (m2 - m1)
        bk = (max * sm.s1() - min * sm.s2()) / (max - min);
    }
    else    // иначе (границы основной шкалы совпадают,
            // значит и дополнительной тоже)
    {
        // a = 0
        ak = 0;
        // b = s1
        bk = sm.s1();
    }
    fixed = true;   // границы фиксированы
}

// Установка заданных границ шкалы
void QScaleBounds::set(double mn,double mx)
{
    // если границы еще не фиксированы, фиксируем их
    if (!fixed) fix();
    // устанавливаем нижнюю и верхнюю границы шкалы
    plot->setAxisScale(master,mn,mx);   // основной
                                        // и дополнительной
    if (sync) plot->setAxisScale(slave,mn,mx);
    else plot->setAxisScale(slave,ak*mn+bk,ak*mx+bk);
}

// Восстановление исходных границ шкалы
void QScaleBounds::rest() {
    // если границы уже фиксированы, то восстанавливаем исходные
    if (fixed) set(min,max);
}

// Переустановка границ дополнительной шкалы
void QScaleBounds::dup()
{
    // если границы еще не фиксированы, фиксируем их
    if (!fixed) fix();
    // получаем карту основной шкалы
    QwtScaleMap sm = plot->canvasMap(master);
    // и устанавливаем границы для дополнительной
    if (sync) plot->setAxisScale(slave,sm.s1(),sm.s2());
    else plot->setAxisScale(slave,ak*sm.s1()+bk,ak*sm.s2()+bk);
}

// Определение влияет ли указанная шкала на другие
bool QScaleBounds::affected(QwtPlot::Axis ax)
{
    // если шкала основная, то вляиет
    if (ax == master) return true;
    // если шкала дополнительная и включена синхронизация, то вляиет
    if (ax == slave && sync) return true;
    // иначе (дополнительная и синхронизация выключена) не влияет
    return false;
}

// Установка количества делений на шкале с образца
void QScaleBounds::setDiv(QwtScaleDiv *sdv)
{
    // устанавливаем количество делений для основной шкалы,
    plot->setAxisScaleDiv(master,*sdv);
    // а если задан режим синхронизации, то и для дополнительной
    if (sync) plot->setAxisScaleDiv(slave,*sdv);
}

/**********************************************************/
/*                                                        */
/*           Реализация класса QMainSynZoomSvc            */
/*                      Версия 1.0.2                      */
/*                                                        */
/* Разработал Мельников Сергей Андреевич,                 */
/* г. Каменск-Уральский Свердловской обл., 2012 г.,       */
/* при поддержке Ю. А. Роговского, г. Новосибирск.        */
/*                                                        */
/* Разрешается свободное использование и распространение. */
/* Упоминание автора обязательно.                         */
/*                                                        */
/**********************************************************/

// Конструктор
QMainSynZoomSvc::QMainSynZoomSvc() :
    QObject()
{
    // очищаем виджет, отвечающий за отображение выделенной области
    zwid = 0;
    // и назначаем ему цвет (по умолчанию - черный)
    zwClr = Qt::black;
    // по умолчанию выделение области индицируется
    indiZ = true;
}

// Прикрепление интерфейса к менеджеру масштабирования
void QMainSynZoomSvc::attach(QChartSynZoom *zm)
{
    // запоминаем указатель на менеджер масштабирования
    zoom = zm;
    // для всех графиков выполняем действия по добавлению нового графика
    for (int k=0; k < zoom->count(); k++) connectPlot(k);
    // назначаем слот сигнала - Добавление нового графика
    connect(zm,SIGNAL(plotAppended(int)),SLOT(connectPlot(int)));
}

// Установка цвета рамки, задающей новый размер графика
void QMainSynZoomSvc::setRubberBandColor(QColor clr) {
    zwClr = clr;
}

// Включение/выключение индикации выделяемой области
void QMainSynZoomSvc::indicateZoom(bool indi) {
    indiZ = indi;
}

// Слот сигнала - Добавление нового графика
void QMainSynZoomSvc::connectPlot(int ind)
{
    // назначаем обработчик событий (фильтр событий)
    zoom->at(ind)->plot->installEventFilter(this);
}

// Обработчик всех событий
bool QMainSynZoomSvc::eventFilter(QObject *target,QEvent *event)
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
        // если произошло одно из событий от мыши, то
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)
            procMouseEvent(event,ind);  // вызываем соответствующий обработчик
    }
    // передаем управление стандартному обработчику событий
    return QObject::eventFilter(target,event);
}

// Прорисовка виджета выделенной области
void QMainSynZoomSvc::showZoomWidget(QRect zr)
{
    // устанавливаем положение и размеры виджета, отображающего выделенную область
    zwid->setGeometry(zr);
    // запоминаем для удобства
    int dw = zr.width();    // ширину области
    int dh = zr.height();   // и высоту
    // формируем маску для виджета, отображающего выделенную область
    QRegion rw(0,0,dw,dh);      // непрозрачная область
    QRegion rs(1,1,dw-2,dh-2);  // прозрачная область
    // устанавливаем маску путем вычитания из непрозрачной области прозрачной
    zwid->setMask(rw.subtracted(rs));
    // делаем виджет, отображающий выделенную область, видимым
    zwid->setVisible(true);
    // перерисовываем виджет
    zwid->repaint();
}

// Обработчик обычных событий от мыши
void QMainSynZoomSvc::procMouseEvent(QEvent *event,int k)
{
    // создаем указатель на событие от мыши
    QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
    // в зависимости от типа события вызываем соответствующий обработчик
    switch (event->type())
    {
        // нажата кнопка мыши
    case QEvent::MouseButtonPress: startZoom(mEvent,k); break;
        // перемещение мыши
    case QEvent::MouseMove: selectZoomRect(mEvent,k); break;
        // отпущена кнопка мыши
    case QEvent::MouseButtonRelease: procZoom(mEvent,k); break;
        // для прочих событий ничего не делаем
    default: ;
    }
}

// Обработчик нажатия на кнопку мыши
// (включение изменения масштаба)
void QMainSynZoomSvc::startZoom(QMouseEvent *mEvent,int ind)
{
    // фиксируем исходные границы графика (если этого еще не было сделано)
    zoom->fixBounds();
    // если в данный момент еще не включен ни один из режимов
    if (zoom->regim() == QChartSynZoom::ctNone)
    {
        // получаем указатели на
        QwtPlot *plt = zoom->at(ind)->plot; // график
#if QWT_VERSION < 0x060099   // qwt-5.2.x + qwt-6.0.x
        QwtPlotCanvas *cv = plt->canvas();  // и канву
#else                        // qwt-6.1.x
        QWidget *cv  = plt->canvas();       // и канву
#endif
        // получаем геометрию канвы графика
        QRect cg = cv->geometry();
        // определяем текущее положение курсора (относительно канвы графика)
        scp_x = mEvent->pos().x() - cg.x();
        scp_y = mEvent->pos().y() - cg.y();
        // если курсор находится над канвой графика
        if (scp_x >= 0 && scp_x < cg.width() &&
            scp_y >= 0 && scp_y < cg.height())
            // если нажата левая кнопка мыши, то
            if (mEvent->button() == Qt::LeftButton)
            {
                // прописываем соответствующий признак режима
                zoom->setRegim(QChartSynZoom::ctZoom);
                // запоминаем текущий курсор
                tCursor = cv->cursor();
                // устанавливаем курсор Cross
                cv->setCursor(Qt::CrossCursor);
                // если включена индикация, то
                if (indiZ)
                {
                    // создаем виджет, который будет отображать выделенную область
                    // (он будет прорисовываться на том же виджете, что и график)
                    zwid = new QWidget(plt->parentWidget());
                    // и назначаем ему цвет
                    zwid->setStyleSheet(QString(
                        "background-color:rgb(%1,%2,%3);").arg(
                        zwClr.red()).arg(zwClr.green()).arg(zwClr.blue()));
                }
            }
    }
}

// Обработчик перемещения мыши
// (выделение новых границ графика)
void QMainSynZoomSvc::selectZoomRect(QMouseEvent *mEvent,int ind) {
    // если включен режим изменения масштаба и индикация, то
    if (zoom->regim() == QChartSynZoom::ctZoom && indiZ)
    {
        // получаем указатель на график
        QwtPlot *plt = zoom->at(ind)->plot;
        // получаем геометрию графика
        QRect pg = plt->geometry();
        // и геометрию канвы графика
        QRect cg = plt->canvas()->geometry();
        // scp_x - координата курсора в пикселах по горизонтальной оси
        //     в начальный момент времени (когда была нажата левая кнопка мыши)
        // mEvent->pos().x() - cg.x() - координата курсора в пикселах
        //     по горизонтальной оси в текущий момент времени
        // mEvent->pos().x() - cg.x() - scp_x - смещение курсора в пикселах
        //     по горизонтальной оси от начального положения и соответственно
        //     ширина dx выделенной области
        int dx = mEvent->pos().x() - cg.x() - scp_x;
        // pg.x() - положение графика по горизонтальной оси
        //     относительно виджета, его содержащего
        // pg.x() + cg.x() - положение канвы графика по горизонтальной оси
        //     относительно виджета, его содержащего
        // pg.x() + cg.x() + scp_x - положение gx0 начальной точки по горизонтальной оси
        //     относительно виджета, содержащего график, она нужна в качестве опоры
        //     для отображения выделенной области
        int gx0 = pg.x() + cg.x() + scp_x;
        // если ширина выделенной области отрицательна, то текущая точка находится левее начальной,
        //     и тогда именно ее мы используем в качестве опоры для отображения выделенной области
        if (dx < 0) {dx = -dx; gx0 -= dx;}
        // иначе если ширина равна нулю, то для того чтобы выделенная область все-таки отбражалась,
        //     принудительно сделаем ее равной единице
        else if (dx == 0) dx = 1;
        // аналогично определяем высоту dy выделенной области
        int dy = mEvent->pos().y() - cg.y() - scp_y;
        // и положение gy0 начальной точки по вертикальной оси
        int gy0 = pg.y() + cg.y() + scp_y;
        // если высота выделенной области отрицательна, то текущая точка находится выше начальной,
        //     и тогда именно ее мы используем в качестве опоры для отображения выделенной области
        if (dy < 0) {dy = -dy; gy0 -= dy;}
        // иначе если высота равна нулю, то для того чтобы выделенная область все-таки отбражалась,
        //     принудительно сделаем ее равной единице
        else if (dy == 0) dy = 1;
        // отображаем выделенную область
        showZoomWidget(QRect(gx0,gy0,dx,dy));
    }
}

// Обработчик отпускания кнопки мыши
// (выполнение изменения масштаба)
void QMainSynZoomSvc::procZoom(QMouseEvent *mEvent,int ind)
{
    // если включен режим изменения масштаба или режим перемещения графика
    if (zoom->regim() == QChartSynZoom::ctZoom)
        // если отпущена левая кнопка мыши, то
        if (mEvent->button() == Qt::LeftButton)
        {
            // получаем указатели на
            QChartZoomItem *it = zoom->at(ind); // масштабирующий элемент
            QwtPlot *plt = it->plot;            // график
#if QWT_VERSION < 0x060099   // qwt-5.2.x + qwt-6.0.x
            QwtPlotCanvas *cv = plt->canvas();  // и канву
#else                        // qwt-6.1.x
            QWidget *cv  = plt->canvas();       // и канву
#endif
            // восстанавливаем курсор
            cv->setCursor(tCursor);
            // если включена индикация, то удаляем виджет,
            if (indiZ) delete zwid; // отображающий выделенную область
            // получаем геометрию канвы графика
            QRect cg = cv->geometry();
            // определяем положение курсора, т.е. координаты xp и yp
            // конечной точки выделенной области (в пикселах относительно канвы QwtPlot)
            int xp = mEvent->pos().x() - cg.x();
            int yp = mEvent->pos().y() - cg.y();
            // если выделение производилось справа налево или снизу вверх,
            // то восстанавливаем исходные границы графика (отменяем увеличение)
            if (xp < scp_x || yp < scp_y) zoom->restBounds(ind);
            // иначе если размер выделенной области достаточен, то изменяем масштаб
            else if (xp - scp_x >= 8 && yp - scp_y >= 8)
            {
                // устанавливаем левую границу горизонтальной шкалы
                // по начальной точке, а правую по конечной
                zoom->setHorizontScale(plt->invTransform(it->masterX,scp_x),
                    plt->invTransform(it->masterX,xp));
                // получаем основную вертикальную шкалу
                QwtPlot::Axis mY = it->masterY;
                // определяем нижнюю границу вертикальной шкалы по конечной точке
                double bt = plt->invTransform(mY,yp);
                // определяем верхнюю границу вертикальной шкалы по начальной точке
                double tp = plt->invTransform(mY,scp_y);
                // устанавливаем нижнюю и верхнюю границы вертикальной шкалы
                it->isb_y->set(bt,tp);
                // перестраиваем график (синхронно с остальными)
                zoom->synReplot(ind);
            }
            // очищаем признак режима
            zoom->setRegim(QChartSynZoom::ctNone);
        }
}

/**********************************************************/
/*                                                        */
/*           Реализация класса QDragSynZoomSvc            */
/*                      Версия 1.0.1                      */
/*                                                        */
/* Разработал Мельников Сергей Андреевич,                 */
/* г. Каменск-Уральский Свердловской обл., 2012 г.,       */
/* при поддержке Ю. А. Роговского, г. Новосибирск.        */
/*                                                        */
/* Разрешается свободное использование и распространение. */
/* Упоминание автора обязательно.                         */
/*                                                        */
/**********************************************************/

// Конструктор
QDragSynZoomSvc::QDragSynZoomSvc() :
    QObject()
{
    // по умолчанию легкий режим выключен
    light = false;
    // очищаем виджет, отвечающий за отображение индикатора перемещения
    zwid = 0;
    // и назначаем ему цвет (по умолчанию черный)
    dwClr = Qt::black;
    // по умолчанию включен простой режим индикации перемещение графика
    // (но только в том случае, если включен легкий режим)
    indiDrB = QChartSynZoom::disSimple;
}

// Прикрепление интерфейса к менеджеру масштабирования
void QDragSynZoomSvc::attach(QChartSynZoom *zm)
{
    // запоминаем указатель на менеджер масштабирования
    zoom = zm;
    // для всех графиков выполняем действия по добавлению нового графика
    for (int k=0; k < zoom->count(); k++) connectPlot(k);
    // назначаем слот сигнала - Добавление нового графика
    connect(zm,SIGNAL(plotAppended(int)),SLOT(connectPlot(int)));
}

// Включение/выключение легкого режима
void QDragSynZoomSvc::setLightMode(bool lm) {
    light = lm;
}

// Установка цвета виджета индикатора перемещения
void QDragSynZoomSvc::setDragBandColor(QColor clr) {
    dwClr = clr;
}

// Включение/выключение индикации перемещаемой области
// (имеет эффект, если включен легкий режим)
void QDragSynZoomSvc::setIndicatorStyle(QChartSynZoom::QDragIndiStyle indi) {
    indiDrB = indi;
}

// Слот сигнала - Добавление нового графика
void QDragSynZoomSvc::connectPlot(int ind)
{
    // назначаем обработчик событий (фильтр событий)
    zoom->at(ind)->plot->installEventFilter(this);
}

// Обработчик всех событий
bool QDragSynZoomSvc::eventFilter(QObject *target,QEvent *event)
{
    int ind = -1;   // график пока не найден
    // просматриваем список графиков
    for (int k=0; k < zoom->count(); k++)
        // если событие произошло для данной графика, то
        if (target == zoom->at(k)->plot)
        {
            ind = k;    // запоминаем номер графика,
            break;      // прекращаем поиск графика
        }
    // если график был найден, то
    if (ind >= 0)
    {
        // если произошло одно из событий от мыши, то
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)
            dragMouseEvent(event,ind);  // вызываем соответствующий обработчик
    }
    // передаем управление стандартному обработчику событий
    return QObject::eventFilter(target,event);
}

// Добавление в маску индикатора вертикальных линий сетки для меток горизонтальной шкалы
QRegion QDragSynZoomSvc::addHorTicks(QRegion rw,QwtScaleDiv::TickType tt,QChartZoomItem *it)
{
    // получаем указатель на график
    QwtPlot *plt = it->plot;
    // получаем список основных меток горизонтальной шкалы
#if   QWT_VERSION < 0x060000   // qwt-5.2.x
    QwtValueList vl = plt->axisScaleDiv(it->masterX)->ticks(tt);
#elif QWT_VERSION > 0x060099   // qwt-6.1.x
    QList<double> vl = plt->axisScaleDiv(it->masterX).ticks(tt);
#else                          // qwt-6.0.x
    QList<double> vl = plt->axisScaleDiv(it->masterX)->ticks(tt);
#endif

    // перебираем все метки горизонтальной шкалы
    for (int k=0; k < vl.count(); k++)
    {
        // вычисляем смещение метки относительно канвы
        int x = plt->transform(it->masterX,vl.at(k));
        // формируем вертикальную линию сетки
        QRegion rs(x-1,1,1,rw.boundingRect().height()-2);
        // добавляем ее в маску
        rw = rw.united(rs);
    }
    // возвращаем измененную маску
    return rw;
}

// Добавление в маску индикатора горизонтальных линий сетки для меток вертикальной шкалы
QRegion QDragSynZoomSvc::addVerTicks(QRegion rw,QwtScaleDiv::TickType tt,QChartZoomItem *it)
{
    // получаем указатель на график
    QwtPlot *plt = it->plot;
    // получаем список основных меток вертикальной шкалы
#if   QWT_VERSION < 0x060000   // qwt-5.2.x
    QwtValueList vl = plt->axisScaleDiv(it->masterY)->ticks(tt);
#elif QWT_VERSION > 0x060099   // qwt-6.1.x
    QList<double> vl = plt->axisScaleDiv(it->masterY).ticks(tt);
#else                          // qwt-6.0.x
    QList<double> vl = plt->axisScaleDiv(it->masterY)->ticks(tt);
#endif

    // перебираем все метки вертикальной шкалы
    for (int k=0; k < vl.count(); k++)
    {
        // вычисляем смещение метки относительно канвы
        int y = plt->transform(it->masterY,vl.at(k));
        // формируем горизонтальную линию сетки
        QRegion rs(1,y-1,rw.boundingRect().width()-2,1);
        // добавляем ее в маску
        rw = rw.united(rs);
    }
    // возвращаем измененную маску
    return rw;
}

// Прорисовка изображения индикатора перемещения
void QDragSynZoomSvc::showDragWidget(QPoint evpos,int ind)
{
    // получаем указатели на
    QChartZoomItem *it = zoom->at(ind); // масштабирующий элемент
    QwtPlot *plt = it->plot;            // и график
    // получаем геометрию графика
    QRect pg = plt->geometry();             // графика
    QRect cg = plt->canvas()->geometry();   // и канвы графика
    // запоминаем для удобства
    int ww = cg.width() - 2;    // ширину канвы
    int wh = cg.height() - 2;   // и высоту
    // формируем положение и размер рамки канвы
    QRect wg(pg.x()+1+evpos.x()-scp_x,pg.y()+1+evpos.y()-scp_y,ww,wh);
    // устанавливаем положение и размеры виджета индикатора
    zwid->setGeometry(wg);
    // объявляем составляющие маски для виджета индикатора
    QRegion rw(0,0,ww,wh);      // непрозрачная область
    QRegion rs(1,1,ww-2,wh-2);  // прозрачная область
    // формируем маску путем вычитания из непрозрачной области прозрачной
    rw = rw.subtracted(rs);
    // если включен подробный режим индикации, то
    if (indiDrB == QChartSynZoom::disDetailed)
    {
        // добавляем в маску вертикальные линии сетки для делений горизонтальной шкалы
        rw = addHorTicks(rw,QwtScaleDiv::MajorTick,it);     // основных
        rw = addHorTicks(rw,QwtScaleDiv::MediumTick,it);    // средних
        rw = addHorTicks(rw,QwtScaleDiv::MinorTick,it);     // минимальных
        // добавляем в маску горизонтальные линии сетки для основных делений
        rw = addVerTicks(rw,QwtScaleDiv::MajorTick,it); // вертикальной шкалы
    }
    // устанавливаем маску
    zwid->setMask(rw);
    // делаем виджет, индицирующий перемещение графика, видимым
    zwid->setVisible(true);
    // перерисовываем виджет
    zwid->repaint();
}

// Применение результатов перемещения графика
void QDragSynZoomSvc::applyDrag(QPoint evpos,int ind)
{
    // получаем указатели на
    QChartZoomItem *it = zoom->at(ind); // масштабирующий элемент
    QwtPlot *plt = it->plot;            // и график
    // получаем геометрию канвы графика
    QRect cg = plt->canvas()->geometry();
    // scp_x - координата курсора в пикселах по горизонтальной оси
    //     в начальный момент времени (когда была нажата правая кнопка мыши)
    // evpos.x() - cg.x() - координата курсора
    //     в пикселах по горизонтальной оси в текущий момент времени
    // evpos.x() - cg.x() - scp_x - смещение курсора в пикселах
    //     по горизонтальной оси от начального положения
    // (evpos.x() - cg.x() - scp_x) * cs_kx -  это же смещение,
    //     но уже в единицах горизонтальной шкалы
    // dx - смещение границ по горизонтальной оси берется с обратным знаком
    //     (чтобы график относительно границ переместился вправо, сами границы следует сместить влево)
    double dx = -(evpos.x() - cg.x() - scp_x) * cs_kx;
    // устанавливаем новые левую и правую границы шкалы для горизонтальной оси
    //     новые границы = начальные границы + смещение
    zoom->setHorizontScale(scb_xl + dx,scb_xr + dx);
    // аналогично определяем dy - смещение границ по вертикальной оси
    double dy = -(evpos.y() - cg.y() - scp_y) * cs_ky;
    // устанавливаем новые нижнюю и верхнюю границы вертикальной шкалы
    it->isb_y->set(scb_yb + dy,scb_yt + dy);
    // перестраиваем график (синхронно с остальными)
    zoom->synReplot(ind);
}

// Обработчик событий от мыши
void QDragSynZoomSvc::dragMouseEvent(QEvent *event,int k)
{
    // создаем указатель на событие от мыши
    QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
    // в зависимости от типа события вызываем соответствующий обработчик
    switch (event->type())
    {
        // нажата кнопка мыши
    case QEvent::MouseButtonPress: startDrag(mEvent,k); break;
        // перемещение мыши
    case QEvent::MouseMove: procDrag(mEvent,k); break;
        // отпущена кнопка мыши
    case QEvent::MouseButtonRelease: endDrag(mEvent,k); break;
        // для прочих событий ничего не делаем
    default: ;
    }
}

// Обработчик нажатия на кнопку мыши
// (включение перемещения графика)
void QDragSynZoomSvc::startDrag(QMouseEvent *mEvent,int ind)
{
    // фиксируем исходные границы графика (если этого еще не было сделано)
    zoom->fixBounds();
    // если в данный момент еще не включен ни один из режимов
    if (zoom->regim() == QChartSynZoom::ctNone)
    {
        // получаем указатели на
        QChartZoomItem *it = zoom->at(ind); // масштабирующий элемент
        QwtPlot *plt = it->plot;            // график
#if QWT_VERSION < 0x060099   // qwt-5.2.x + qwt-6.0.x
        QwtPlotCanvas *cv = plt->canvas();  // и канву
#else                        // qwt-6.1.x
        QWidget *cv  = plt->canvas();       // и канву
#endif
        // получаем геометрию канвы графика
        QRect cg = cv->geometry();
        // определяем текущее положение курсора (относительно канвы графика)
        scp_x = mEvent->pos().x() - cg.x();
        scp_y = mEvent->pos().y() - cg.y();
        // если курсор находится над канвой графика
        if (scp_x >= 0 && scp_x < cg.width() &&
            scp_y >= 0 && scp_y < cg.height())
            // если нажата правая кнопка мыши, то
            if (mEvent->button() == Qt::RightButton)
            {
                // прописываем соответствующий признак режима
                zoom->setRegim(QChartSynZoom::ctDrag);
                // запоминаем текущий курсор
                tCursor = cv->cursor();
                // устанавливаем курсор OpenHand
                cv->setCursor(Qt::OpenHandCursor);
                // определяем текущий масштабирующий множитель по горизонтальной оси
                // (т.е. узнаем на сколько изменяется координата по шкале x
                // при перемещении курсора вправо на один пиксел)
                cs_kx = plt->invTransform(it->masterX,scp_x + 1) -
                    plt->invTransform(it->masterX,scp_x);
                // получаем основную вертикальную шкалу
                QwtPlot::Axis mY = it->masterY;
                // определяем текущий масштабирующий множитель по вертикальной оси
                // (аналогично)
                cs_ky = plt->invTransform(mY,scp_y + 1) -
                    plt->invTransform(mY,scp_y);
                // получаем карту основной горизонтальной шкалы
                QwtScaleMap sm = plt->canvasMap(it->masterX);
                // для того чтобы фиксировать начальные левую и правую границы
                scb_xl = sm.s1(); scb_xr = sm.s2();
                // аналогично получаем карту основной вертикальной шкалы
                sm = plt->canvasMap(mY);
                // для того чтобы фиксировать начальные нижнюю и верхнюю границы
                scb_yb = sm.s1(); scb_yt = sm.s2();
                // если легкий режим и включена индикация, то
                if (light and indiDrB != QChartSynZoom::disNone)
                {
                    // создаем виджет, индицирующий перемещение графика
                    zwid = new QWidget(plt->parentWidget());
                    // назначаем ему цвет
                    zwid->setStyleSheet(QString(
                        "background-color:rgb(%1,%2,%3);").arg(
                        dwClr.red()).arg(dwClr.green()).arg(dwClr.blue()));
                    // прорисовываем изображение индикатора перемещения
                    showDragWidget(mEvent->pos(),ind);
                }
            }
    }
}

// Обработчик перемещения мыши
// (выполнение перемещения или выбор нового положения графика)
void QDragSynZoomSvc::procDrag(QMouseEvent *mEvent,int ind)
{
    // если включен режим перемещения графика, то
    if (zoom->regim() == QChartSynZoom::ctDrag)
    {
        // устанавливаем курсор ClosedHand
        zoom->at(ind)->plot->canvas()->setCursor(Qt::ClosedHandCursor);
        if (light)  // если легкий режим, то
        {
            // если включена индикация, то
            if (indiDrB != QChartSynZoom::disNone)
                // прорисовываем изображение индикатора перемещения
                showDragWidget(mEvent->pos(),ind);
        }
        // иначе применяем результаты перемещения графика
        else applyDrag(mEvent->pos(),ind);
    }
}

// Обработчик отпускания кнопки мыши
// (выключение перемещения графика)
void QDragSynZoomSvc::endDrag(QMouseEvent *mEvent,int ind)
{
    // если включен режим изменения масштаба или режим перемещения графика
    if (zoom->regim() == QChartSynZoom::ctDrag)
        // если отпущена правая кнопка мыши, то
        if (mEvent->button() == Qt::RightButton)
        {
            if (light)  // если легкий режим, то
            {
                // если включена индикация, то удаляем виджет индикатора
                if (indiDrB != QChartSynZoom::disNone) delete zwid;
                // применяем результаты перемещения графика
                applyDrag(mEvent->pos(),ind);
            }
            // восстанавливаем курсор
            zoom->at(ind)->plot->canvas()->setCursor(tCursor);
            zoom->setRegim(QChartSynZoom::ctNone);  // и очищаем признак режима
        }
}
