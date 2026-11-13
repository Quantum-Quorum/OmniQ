//
// Created by Goutham Arcot on 17/07/25.
//

#include "CircuitView.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

CircuitView::CircuitView(QWidget *parent)
    : QOpenGLWidget(parent), numQubits_(2), currentStep_(-1),
      offsetX_(0.0f), offsetY_(0.0f), zoom_(1.0f), gateSize_(42),
      qubitSpacing_(65), stepSpacing_(75), isPanning_(false) {
  setMinimumSize(400, 300);
  setMouseTracking(true);
}

CircuitView::~CircuitView() {}

QSize CircuitView::sizeHint() const {
  int maxStep = 10;
  for (const auto &gate : gates_) {
    maxStep = std::max(maxStep, gate.step);
  }
  int width = 150 + stepSpacing_ * (maxStep + 2);
  int height = 100 + qubitSpacing_ * numQubits_;
  return QSize(std::max(width, 700), std::max(height, 350));
}

void CircuitView::setNumQubits(int numQubits) {
  numQubits_ = std::max(1, numQubits);
  update();
}

void CircuitView::addGate(int step, int qubit, const QString &gateType) {
  gates_.append({step, qubit, -1, gateType, 0.0});
  update();
}

void CircuitView::addGate(int step, int qubit, int targetQubit,
                          const QString &gateType, double parameter) {
  gates_.append({step, qubit, targetQubit, gateType, parameter});
  update();
}

void CircuitView::setCurrentStep(int step) {
  currentStep_ = step;
  update();
}

void CircuitView::toggleBreakpoint(int step) {
  if (breakpoints_.contains(step)) {
    breakpoints_.remove(step);
  } else {
    breakpoints_.insert(step);
  }
  emit breakpointToggled(step);
  update();
}

bool CircuitView::hasBreakpoint(int step) const {
  return breakpoints_.contains(step);
}

void CircuitView::clear() {
  gates_.clear();
  currentStep_ = -1;
  update();
}

void CircuitView::initializeGL() {
  initializeOpenGLFunctions();
  glClearColor(0.12f, 0.12f, 0.15f, 1.0f); // Sleek dark theme background
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CircuitView::resizeGL(int w, int h) {
  glViewport(0, 0, w, h);
  projection_.setToIdentity();
  projection_.ortho(0, w, h, 0, -1, 1);
}

void CircuitView::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);

  painter.save();
  painter.translate(offsetX_, offsetY_);
  painter.scale(zoom_, zoom_);

  int maxStep = 10;
  for (const auto &g : gates_) {
    maxStep = std::max(maxStep, g.step + 1);
  }

  // Draw step headers / breakpoint gutter
  drawBreakpoints(painter, maxStep);

  // Draw qubit wire lines
  for (int i = 0; i < numQubits_; ++i) {
    drawQubitLine(painter, i, maxStep);
  }

  // Draw multi-qubit and single-qubit gates
  for (const Gate &gate : gates_) {
    drawGate(painter, gate);
  }

  // Draw vertical execution playhead
  drawPlayhead(painter);

  painter.restore();
}

void CircuitView::drawBreakpoints(QPainter &painter, int maxStep) {
  painter.setFont(QFont("Menlo", 9));
  for (int s = 0; s <= maxStep; ++s) {
    int x = 100 + s * stepSpacing_;
    int y = 25;

    // Step number
    painter.setPen(QColor(130, 140, 160));
    painter.drawText(QRect(x - 25, 5, 50, 16), Qt::AlignCenter, QString::number(s));

    // Breakpoint marker
    if (breakpoints_.contains(s)) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(235, 75, 75));
      painter.drawEllipse(QPointF(x, y), 5.5, 5.5);
    } else {
      painter.setPen(QPen(QColor(60, 65, 80), 1.0));
      painter.setBrush(Qt::NoBrush);
      painter.drawEllipse(QPointF(x, y), 4.5, 4.5);
    }
  }
}

void CircuitView::drawQubitLine(QPainter &painter, int qubitIndex, int maxStep) {
  int y = 65 + qubitIndex * qubitSpacing_;
  int startX = 60;
  int endX = 120 + maxStep * stepSpacing_;

  // Qubit label badge
  QRect badgeRect(10, y - 12, 40, 24);
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(40, 44, 55));
  painter.drawRoundedRect(badgeRect, 4, 4);

  painter.setPen(QColor(200, 210, 230));
  painter.setFont(QFont("Arial", 10, QFont::Bold));
  painter.drawText(badgeRect, Qt::AlignCenter, QString("q[%1]").arg(qubitIndex));

  // Wire
  painter.setPen(QPen(QColor(80, 90, 110), 2.0));
  painter.drawLine(startX, y, endX, y);
}

void CircuitView::drawGate(QPainter &painter, const Gate &gate) {
  int x = 100 + gate.step * stepSpacing_;
  int y1 = 65 + gate.qubit * qubitSpacing_;

  if (gate.type == "CNOT") {
    int y2 = (gate.targetQubit != -1) ? (65 + gate.targetQubit * qubitSpacing_)
                                      : (y1 + qubitSpacing_);
    // Connector line
    painter.setPen(QPen(QColor(0, 220, 180), 2.5));
    painter.drawLine(x, y1, x, y2);

    // Control dot (•)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 220, 180));
    painter.drawEllipse(QPointF(x, y1), 6.0, 6.0);

    // Target circle with cross (⊕)
    painter.setPen(QPen(QColor(0, 220, 180), 2.5));
    painter.setBrush(QColor(25, 45, 50));
    painter.drawEllipse(QPointF(x, y2), 12.0, 12.0);
    painter.drawLine(x - 12, y2, x + 12, y2);
    painter.drawLine(x, y2 - 12, x, y2 + 12);
    return;
  }

  if (gate.type == "SWAP") {
    int y2 = (gate.targetQubit != -1) ? (65 + gate.targetQubit * qubitSpacing_)
                                      : (y1 + qubitSpacing_);
    painter.setPen(QPen(QColor(240, 160, 60), 2.5));
    painter.drawLine(x, y1, x, y2);

    // Cross markers (✕)
    for (int y : {y1, y2}) {
      painter.drawLine(x - 6, y - 6, x + 6, y + 6);
      painter.drawLine(x - 6, y + 6, x + 6, y - 6);
    }
    return;
  }

  if (gate.type == "CP") {
    int y2 = (gate.targetQubit != -1) ? (65 + gate.targetQubit * qubitSpacing_)
                                      : (y1 + qubitSpacing_);
    painter.setPen(QPen(QColor(180, 120, 240), 2.5));
    painter.drawLine(x, y1, x, y2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(180, 120, 240));
    painter.drawEllipse(QPointF(x, y1), 6.0, 6.0);
    painter.drawEllipse(QPointF(x, y2), 6.0, 6.0);
    return;
  }

  // Single qubit gate box
  QRectF gateRect(x - gateSize_ / 2.0, y1 - gateSize_ / 2.0, gateSize_, gateSize_);

  QColor bgColor(70, 80, 100);
  QColor borderColor(100, 120, 150);

  if (gate.type == "H") {
    bgColor = QColor(45, 105, 190);
    borderColor = QColor(80, 150, 240);
  } else if (gate.type == "X" || gate.type == "RX") {
    bgColor = QColor(190, 55, 60);
    borderColor = QColor(240, 90, 95);
  } else if (gate.type == "Y" || gate.type == "RY") {
    bgColor = QColor(180, 105, 30);
    borderColor = QColor(230, 145, 50);
  } else if (gate.type == "Z" || gate.type == "RZ" || gate.type == "PHASE") {
    bgColor = QColor(120, 55, 180);
    borderColor = QColor(170, 95, 235);
  }

  painter.setPen(QPen(borderColor, 1.8));
  painter.setBrush(bgColor);
  painter.drawRoundedRect(gateRect, 6.0, 6.0);

  painter.setPen(Qt::white);
  painter.setFont(QFont("Arial", 11, QFont::Bold));
  painter.drawText(gateRect, Qt::AlignCenter, gate.type);
}

void CircuitView::drawPlayhead(QPainter &painter) {
  if (currentStep_ < 0) return;

  int x = 100 + currentStep_ * stepSpacing_;
  int topY = 20;
  int bottomY = 75 + numQubits_ * qubitSpacing_;

  // Playhead vertical line
  painter.setPen(QPen(QColor(0, 229, 255, 220), 2.5));
  painter.drawLine(x, topY, x, bottomY);

  // Playhead arrowhead at top
  QPolygonF arrow;
  arrow << QPointF(x - 7, topY - 8)
        << QPointF(x + 7, topY - 8)
        << QPointF(x, topY);
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(0, 229, 255));
  painter.drawPolygon(arrow);
}

void CircuitView::mousePressEvent(QMouseEvent *event) {
  QPointF localPos = (event->position() - QPointF(offsetX_, offsetY_)) / zoom_;

  if (event->button() == Qt::LeftButton) {
    // Check if clicked in breakpoint gutter (y < 45)
    if (localPos.y() >= 0 && localPos.y() <= 45) {
      int step = std::round((localPos.x() - 100) / stepSpacing_);
      if (step >= 0) {
        toggleBreakpoint(step);
        return;
      }
    }

    // Check if clicked near a step to jump/seek
    int step = std::round((localPos.x() - 100) / stepSpacing_);
    if (step >= 0 && std::abs(localPos.x() - (100 + step * stepSpacing_)) < 25) {
      emit stepSelected(step);
    }

    lastMousePos_ = event->pos();
    isPanning_ = true;
  }
}

void CircuitView::mouseMoveEvent(QMouseEvent *event) {
  if (isPanning_ && (event->buttons() & Qt::LeftButton)) {
    QPoint delta = event->pos() - lastMousePos_;
    offsetX_ += delta.x();
    offsetY_ += delta.y();
    lastMousePos_ = event->pos();
    update();
  }
}

void CircuitView::wheelEvent(QWheelEvent *event) {
  float zoomFactor = 1.1f;
  if (event->angleDelta().y() > 0) {
    zoom_ *= zoomFactor;
  } else {
    zoom_ /= zoomFactor;
  }
  zoom_ = qBound(0.2f, zoom_, 4.0f);
  update();
}
