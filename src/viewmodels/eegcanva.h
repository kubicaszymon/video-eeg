// eegunifiedcanvas.h
#ifndef EEGCANVA_H
#define EEGCANVA_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QVector>
#include "eegviewmodel.h"

class EegCanva : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(EegViewModel* viewModel READ viewModel WRITE setViewModel NOTIFY viewModelChanged)
    Q_PROPERTY(float amplitudeScale READ amplitudeScale WRITE setAmplitudeScale NOTIFY amplitudeScaleChanged)
    Q_PROPERTY(float channelSpacing READ channelSpacing WRITE setChannelSpacing NOTIFY channelSpacingChanged)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
    Q_PROPERTY(float timeWindow READ timeWindow WRITE setTimeWindow NOTIFY timeWindowChanged)

public:
    explicit EegCanva(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    EegViewModel* viewModel() const { return view_model_; }
    void setViewModel(EegViewModel* model);

    float amplitudeScale() const { return amplitude_scale_; }
    void setAmplitudeScale(float scale);

    float channelSpacing() const { return channel_spacing_; }
    void setChannelSpacing(float spacing);

    bool showGrid() const { return show_grid_; }
    void setShowGrid(bool show);

    float timeWindow() const { return time_window_; }
    void setTimeWindow(float seconds);

signals:
    void viewModelChanged();
    void amplitudeScaleChanged();
    void channelSpacingChanged();
    void showGridChanged();
    void timeWindowChanged();

private slots:
    void onShowData();

private:
    void drawGrid(QPainter* painter);
    void drawChannelBaseline(QPainter* painter, int channel_index, qreal y_baseline);
    void drawChannelSignal(QPainter* painter, int channel_index, qreal y_baseline);
    void drawChannelLabel(QPainter* painter, int channel_index, qreal y_baseline);

    EegViewModel* view_model_ = nullptr;
    float amplitude_scale_ = 50.0f;
    float channel_spacing_ = 80.0f;
    bool show_grid_ = true;
    float time_window_ = 5.0f;  // Sekundy do wyświetlenia

    QVector<QColor> channel_colors_;
};

#endif // EEGCANVA_H
