#ifndef EEGPLOTITEM_H
#define EEGPLOTITEM_H

#include <QQuickPaintedItem>
#include "3rdparty/QCustomPlot/qcustomplot.h"
#include "EegBackend.h"

// SK: TODO
// If somebody will have time it would be nice to refactor this EEG graphs entire to use some custom GPU render for better performance etc
// QCustomPlot is allright but it always can be better

class EegPlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(EegBackend* backend READ backend WRITE setBackend NOTIFY backendChanged)
public:
    EegPlotItem(QQuickItem* parent = nullptr);
    virtual ~EegPlotItem();

    EegBackend* backend() const { return m_backend; }
    void setBackend(EegBackend* backend);

    void paint(QPainter* painter) override;

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

signals:
    void backendChanged();

public slots:
    void updateData(const std::vector<std::vector<float>>& chunk);

private:
    EegBackend* m_backend = nullptr;

    QCustomPlot *custom_plot_;
    QPixmap buffer_;

    void updateBuffer();
};

#endif // EEGPLOTITEM_H
