#ifndef EEGPLOTITEM_H
#define EEGPLOTITEM_H

#include <QQuickPaintedItem>
#include "3rdparty/QCustomPlot/qcustomplot.h"
#include "eegviewmodel.h"

class EegPlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(EegViewModel* viewModel READ viewModel WRITE setViewModel NOTIFY viewModelChanged)
public:
    EegPlotItem(QQuickItem* parent = nullptr);
    virtual ~EegPlotItem();

    EegViewModel* viewModel() const { return view_model_; }
    void setViewModel(EegViewModel* model);

    void paint(QPainter* painter) override;

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

signals:
    void viewModelChanged();

public slots:
    void updateData(const std::vector<std::vector<float>>& chunk);

private:
    EegViewModel* view_model_ = nullptr;

    QCustomPlot *custom_plot_;
    QPixmap buffer_;

    void updateBuffer();
};

#endif // EEGPLOTITEM_H
