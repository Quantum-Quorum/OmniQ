//
// Created by Goutham Arcot on 17/07/25.
//

#include "CircuitView.h"
#include <QPainter>
#include <QMouseEvent>

CircuitView::CircuitView(QWidget *parent)
    : QWidget(parent)
    , gateSize_(40)
    , qubitSpacing_(60)
    , stepSpacing_(80)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
}

CircuitView::~CircuitView()
{
}

void CircuitView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    painter.fillRect(rect(), Qt::white);
    
    // Draw circuit
    drawCircuit(painter);
}

void CircuitView::mousePressEvent(QMouseEvent *event)
{
    lastMousePos_ = event->pos();
    update();
}

void CircuitView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        // Handle dragging
        update();
    }
}

void CircuitView::drawCircuit(QPainter &painter)
{
    // Draw qubit lines
    painter.setPen(QPen(Qt::black, 2));
    for (int i = 0; i < 4; ++i) {
        int y = 50 + i * qubitSpacing_;
        painter.drawLine(50, y, width() - 50, y);
    }
    
    // Draw some example gates
    drawGate(painter, 100, 50, "H");
    drawGate(painter, 180, 50, "X");
    drawGate(painter, 260, 50, "CNOT");
    drawGate(painter, 340, 50, "H");
}

void CircuitView::drawGate(QPainter &painter, int x, int y, const QString &gateType)
{
    QRect gateRect(x - gateSize_/2, y - gateSize_/2, gateSize_, gateSize_);
    
    // Draw gate background
    painter.setBrush(QBrush(Qt::lightGray));
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(gateRect);
    
    // Draw gate text
    painter.setPen(Qt::black);
    painter.drawText(gateRect, Qt::AlignCenter, gateType);
}
