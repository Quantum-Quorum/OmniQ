//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>

class CircuitView : public QWidget
{
    Q_OBJECT

public:
    explicit CircuitView(QWidget *parent = nullptr);
    ~CircuitView();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void drawCircuit(QPainter &painter);
    void drawGate(QPainter &painter, int x, int y, const QString &gateType);
    
    int gateSize_;
    int qubitSpacing_;
    int stepSpacing_;
    QPoint lastMousePos_;
};

#endif // CIRCUITVIEW_H
