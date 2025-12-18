#include <EegPlotItem.h>

EegPlotItem::EegPlotItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    qInfo() << "EegPlotItem created " << this;
    setFlag(ItemHasContents, true);
    custom_plot_ = new QCustomPlot();
    custom_plot_->setBackground(Qt::transparent);
    custom_plot_->setPlottingHint(QCP::phFastPolylines, true);
}

EegPlotItem::~EegPlotItem()
{
    qInfo() << "EegPlotItem destroyed " << this;
    delete custom_plot_;
}

void EegPlotItem::setBackend(EegBackend *backend)
{
    qInfo() << "EegPlotItem set backend " << backend;
    if(backend)
    {
        m_backend = backend;
        connect(m_backend, &EegBackend::channelsChanged, this, &EegPlotItem::onChannelsChanged);
        connect(m_backend, &EegBackend::updateData, this, &EegPlotItem::updateData, Qt::QueuedConnection);
        emit backendChanged();
    }
}

void EegPlotItem::paint(QPainter *painter)
{
    if (!custom_plot_ || buffer_.isNull()) return;

    buffer_.fill(Qt::transparent);
    QCPPainter qcpPainter(&buffer_);
    custom_plot_->toPainter(&qcpPainter);
    painter->drawPixmap(0, 0, buffer_);
}

void EegPlotItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (custom_plot_) {
        custom_plot_->setGeometry(0, 0, newGeometry.width(), newGeometry.height());
        custom_plot_->setViewport(QRect(0, 0, newGeometry.width(), newGeometry.height()));

        if (newGeometry.size() != oldGeometry.size()) {
            buffer_ = QPixmap(newGeometry.width(), newGeometry.height());
        }
    }
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
}

void EegPlotItem::updateData(const std::vector<std::vector<float> > &chunk)
{
    if (!custom_plot_) return;

    for (int i = 0; i < chunk.size(); i++)
    {
        if(custom_plot_->graphCount() <= i) break;

        QVector<double> x, y;
        double currentKey = custom_plot_->graph(i)->dataCount() > 0 ? custom_plot_->graph(i)->dataMainKey(custom_plot_->graph(i)->dataCount()-1) : 0;

        for(float val : chunk[i])
        {
            currentKey += 1.0/128.0;
            x.append(currentKey);
            y.append(val + (i * 100));
        }

        custom_plot_->graph(i)->addData(x, y);
        custom_plot_->graph(i)->data()->removeBefore(currentKey - 30);
    }

    if (custom_plot_->graphCount() > 0)
    {
        double lastKey = custom_plot_->graph(0)->dataMainKey(custom_plot_->graph(0)->dataCount()-1);
        custom_plot_->xAxis->setRange(lastKey - 30.0, lastKey);
    }

    update();
}

void EegPlotItem::onChannelsChanged()
{
    auto channels = m_backend->channels();
    for(auto chan : channels) qInfo() << "EegPlotItem channel: " << chan;

    for(const auto& chan : std::as_const(channels)) custom_plot_->addGraph();
}
