#include <EegPlotItem.h>

EegPlotItem::EegPlotItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    qInfo() << "EegPlotItem created " << this;
    setFlag(ItemHasContents, true);

    m_custom_plot = new QCustomPlot();
    m_custom_plot->setBackground(Qt::transparent);
    m_custom_plot->setPlottingHint(QCP::phFastPolylines, true);

    m_custom_plot->yAxis->setLabel("");
    m_custom_plot->yAxis->setTicks(false);
    m_custom_plot->yAxis->setTickLabels(false);

    m_custom_plot->xAxis->setRange(0, 10);
    m_custom_plot->yAxis->setLabel("");

    m_custom_plot->replot();
}

EegPlotItem::~EegPlotItem()
{
    qInfo() << "EegPlotItem destroyed " << this;
    delete m_custom_plot;
}

void EegPlotItem::setBackend(EegBackend *backend)
{
    if(m_backend)
    {
        qFatal("EegPlotitem already has backend");
        return;
    }
    qInfo() << "EegPlotItem set backend " << backend;
    if(backend)
    {
        m_backend = backend;
        connect(m_backend, &EegBackend::channelsChanged, this, &EegPlotItem::onChannelsChanged);
        connect(m_backend, &EegBackend::updateData, this, &EegPlotItem::updateData, Qt::QueuedConnection);
    }
}

void EegPlotItem::paint(QPainter *painter)
{
    if (!m_custom_plot || buffer_.isNull()) return;

    buffer_.fill(Qt::transparent);
    QCPPainter qcpPainter(&buffer_);
    m_custom_plot->toPainter(&qcpPainter);
    painter->drawPixmap(0, 0, buffer_);
}

void EegPlotItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (m_custom_plot) {
        m_custom_plot->setGeometry(0, 0, newGeometry.width(), newGeometry.height());
        m_custom_plot->setViewport(QRect(0, 0, newGeometry.width(), newGeometry.height()));

        if (newGeometry.size() != oldGeometry.size()) {
            buffer_ = QPixmap(newGeometry.width(), newGeometry.height());
        }
    }
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
}

void EegPlotItem::updateData(const std::vector<std::vector<float> > &chunk)
{
    if (!m_custom_plot) return;

    for (int i = 0; i < chunk.size(); i++)
    {
        if(m_custom_plot->graphCount() <= i) break;

        QVector<double> x, y;
        double currentKey = m_custom_plot->graph(i)->dataCount() > 0 ? m_custom_plot->graph(i)->dataMainKey(m_custom_plot->graph(i)->dataCount()-1) : 0;

        for(float val : chunk[i])
        {
            currentKey += 1.0/128.0;
            x.append(currentKey);
            y.append(val + (i * 100));
        }

        m_custom_plot->graph(i)->addData(x, y);
        m_custom_plot->graph(i)->data()->removeBefore(currentKey - 30);
    }

    if (m_custom_plot->graphCount() > 0)
    {
        double lastKey = m_custom_plot->graph(0)->dataMainKey(m_custom_plot->graph(0)->dataCount()-1);
        m_custom_plot->xAxis->setRange(lastKey - 30.0, lastKey);
    }

    update();
}

void EegPlotItem::onChannelsChanged()
{
    auto channels = m_backend->channels();

    for(auto chan : channels) qInfo() << "EegPlotItem channel: " << chan; //DEBUG

    //m_custom_plot->clearItems();
    //for(const auto& chan : std::as_const(channels)) m_custom_plot->addGraph();

    // DEBUG
    QVector<double> x(251), y0(251), y1(251);
    for (int i=0; i<251; ++i)
    {
        x[i] = i;
        y0[i] = qExp(-i/150.0)*qCos(i/10.0); // exponentially decaying cosine
        y1[i] = qExp(-i/150.0);              // exponential envelope
    }

    m_custom_plot->addGraph();
    m_custom_plot->graph(0)->setData(x, y0);

    m_custom_plot->addGraph();
    m_custom_plot->graph(1)->setData(x, y1);

    m_custom_plot->replot();
}
