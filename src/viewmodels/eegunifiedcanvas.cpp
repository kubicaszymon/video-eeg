// eegunifiedcanvas.cpp - POPRAWNA WERSJA
#include "eegunifiedcanvas.h"
#include <QPainterPath>
#include <QFontMetrics>

EegUnifiedCanvas::EegUnifiedCanvas(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setPerformanceHint(QQuickPaintedItem::FastFBOResizing);
    setAntialiasing(false);

    channel_colors_ << QColor("#FFFFFF")
                    << QColor("#FF6B6B")
                    << QColor("#4ECDC4")
                    << QColor("#FFD93D")
                    << QColor("#95E1D3");
}

void EegUnifiedCanvas::setViewModel(EegViewModel* model)
{
    if(view_model_ == model) return;

    if(view_model_)
        disconnect(view_model_, nullptr, this, nullptr);

    view_model_ = model;

    if(view_model_)
    {
        connect(view_model_, &EegViewModel::dataUpdated,
                this, &EegUnifiedCanvas::onDataUpdated, Qt::QueuedConnection);
    }

    emit viewModelChanged();
}

void EegUnifiedCanvas::setAmplitudeScale(float scale)
{
    if(qFuzzyCompare(amplitude_scale_, scale)) return;
    amplitude_scale_ = scale;
    emit amplitudeScaleChanged();
    update();
}

void EegUnifiedCanvas::setChannelSpacing(float spacing)
{
    if(qFuzzyCompare(channel_spacing_, spacing)) return;
    channel_spacing_ = spacing;
    emit channelSpacingChanged();
    update();
}

void EegUnifiedCanvas::setShowGrid(bool show)
{
    if(show_grid_ == show) return;
    show_grid_ = show;
    emit showGridChanged();
    update();
}

void EegUnifiedCanvas::setTimeWindow(float seconds)
{
    if(qFuzzyCompare(time_window_, seconds)) return;
    time_window_ = seconds;
    emit timeWindowChanged();
    update();
}

void EegUnifiedCanvas::onDataUpdated()
{
    update();
}

void EegUnifiedCanvas::paint(QPainter *painter)
{
    if(!view_model_) return;

    painter->setRenderHint(QPainter::Antialiasing, false);

    qreal w = width();
    qreal h = height();

    // Background
    painter->fillRect(0, 0, w, h, QColor("#1E1E1E"));

    int channel_count = view_model_->GetChannelCount();
    if(channel_count == 0) return;

    // Draw grid first
    if(show_grid_)
        drawGrid(painter);

    // Draw each channel
    for(int ch = 0; ch < channel_count; ++ch)
    {
        qreal y_baseline = 40.0 + (ch * channel_spacing_);

        drawChannelLabel(painter, ch, y_baseline);
        drawChannelBaseline(painter, ch, y_baseline);
        drawChannelSignal(painter, ch, y_baseline);
    }
}

void EegUnifiedCanvas::drawGrid(QPainter* painter)
{
    qreal w = width();
    qreal h = height();

    painter->setPen(QPen(QColor("#2A2A2A"), 1));

    // Vertical lines - co sekundę
    int grid_lines = static_cast<int>(time_window_);
    for(int i = 0; i <= grid_lines; ++i)
    {
        qreal x = (i / time_window_) * w;
        painter->drawLine(QPointF(x, 0), QPointF(x, h));
    }

    // Horizontal lines - co 5 kanałów
    int channel_count = view_model_->GetChannelCount();
    painter->setPen(QPen(QColor("#3A3A3A"), 1, Qt::DotLine));
    for(int ch = 0; ch < channel_count; ch += 5)
    {
        qreal y = 40.0 + (ch * channel_spacing_);
        painter->drawLine(QPointF(0, y), QPointF(w, y));
    }
}

void EegUnifiedCanvas::drawChannelLabel(QPainter* painter, int channel_index, qreal y_baseline)
{
    QVariantList names = view_model_->GetChannelNames();
    if(channel_index >= names.size()) return;

    QString name = names[channel_index].toString();

    painter->setPen(QColor("#CCCCCC"));
    painter->setFont(QFont("Arial", 9));

    QRectF label_rect(5, y_baseline - 10, 60, 20);
    painter->drawText(label_rect, Qt::AlignLeft | Qt::AlignVCenter, name);
}

void EegUnifiedCanvas::drawChannelBaseline(QPainter* painter, int channel_index, qreal y_baseline)
{
    qreal w = width();

    // Linia zera (baseline) - wyraźna, ciągła
    painter->setPen(QPen(QColor("#5A5A5A"), 1.5));
    painter->drawLine(QPointF(70, y_baseline), QPointF(w, y_baseline));
}

void EegUnifiedCanvas::drawChannelSignal(QPainter* painter, int channel_index, qreal y_baseline)
{
    QVector<float> data = view_model_->getChannelData(channel_index);
    if(data.isEmpty()) return;

    qreal w = width();
    qreal signal_start_x = 70;
    qreal signal_width = w - signal_start_x - 10;

    int total_points = data.size();

    // Oblicz ile sampli powinno być w oknie czasowym
    int sample_rate = view_model_->GetSampleRate();
    int samples_in_window = static_cast<int>(time_window_ * sample_rate);

    // Jeśli mamy więcej danych niż okno - pokaż tylko ostatnie
    int start_index = 0;
    int points_to_draw = total_points;

    if(total_points > samples_in_window)
    {
        start_index = total_points - samples_in_window;
        points_to_draw = samples_in_window;
    }

    // KRYTYCZNE: Decimation - więcej punktów dla gładkich krzywych
    int step = qMax(1, points_to_draw / 5000);  // 5000 zamiast 2000!
    int visible_points = (points_to_draw + step - 1) / step;

    // Użyj QVector<QPointF>
    QVector<QPointF> points;
    points.reserve(visible_points);

    qreal x_scale = signal_width / qMax(1.0, static_cast<qreal>(points_to_draw - 1));

    for(int i = 0; i < points_to_draw; i += step)
    {
        int data_index = start_index + i;
        if(data_index >= total_points) break;

        qreal x = signal_start_x + (i * x_scale);
        qreal y = y_baseline - (data[data_index] * amplitude_scale_);

        points.append(QPointF(x, y));
    }

    // Kolory
    QColor signal_color = QColor("#00BCD4");

    QVariantList names = view_model_->GetChannelNames();
    if(channel_index < names.size())
    {
        QString name = names[channel_index].toString();
        if(name.contains("Fz") || name.contains("Cz") || name.contains("Pz"))
        {
            signal_color = QColor("#FF6B6B");
        }
    }

    painter->setPen(QPen(signal_color, 1.0));

    if(!points.isEmpty())
    {
        painter->drawPolyline(points);
    }
}
