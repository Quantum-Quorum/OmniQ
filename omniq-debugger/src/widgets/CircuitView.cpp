//
// Created by Goutham Arcot on 17/07/25.
//

#include "CircuitView.h"
#include <QDebug> // Added QDebug include
#include <QMouseEvent>
#include <QPainter>
#include <cmath>

CircuitView::CircuitView(QWidget *parent)
    : QOpenGLWidget(parent), numQubits_(4), offsetX_(0.0f), offsetY_(0.0f),
      zoom_(1.0f), gateSize_(40), qubitSpacing_(60), stepSpacing_(80),
      isPanning_(false) {
  setMinimumSize(400, 300);
  setMouseTracking(true);

  // Add some example gates
  gates_.append({0, 0, "H"});
  gates_.append({1, 0, "X"});
  gates_.append({2, 0, "CNOT"});
  qDebug() << "CircuitView initialized"; // Replaced printf with qDebug
}

CircuitView::~CircuitView() {}

QSize CircuitView::sizeHint() const {
  int width = 200 + stepSpacing_ * 20; // Default width for 20 steps
  if (!gates_.isEmpty()) {
    int maxStep = 0;
    for (const auto &gate : gates_) {
      maxStep = std::max(maxStep, gate.step);
    }
    width = 200 + stepSpacing_ * (maxStep + 1);
  }
  int height = 100 + qubitSpacing_ * numQubits_;
  return QSize(std::max(width, 800), std::max(height, 400));
}

void CircuitView::setNumQubits(int numQubits) {
  numQubits_ = numQubits;
  update();
}

void CircuitView::addGate(int step, int qubit, const QString &gateType) {
  gates_.append({step, qubit, gateType});
  update();
}

void CircuitView::clear() {
  gates_.clear();
  update();
}

void CircuitView::initializeGL() {
  initializeOpenGLFunctions();
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void CircuitView::resizeGL(int w, int h) {
  glViewport(0, 0, w, h);

  // Set up 2D orthographic projection
  projection_.setToIdentity();
  projection_.ortho(0, w, h, 0, -1, 1); // Top-left origin like QPainter
}

void CircuitView::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);

  // Use QPainter for convenience with OpenGL backend
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Apply transformations
  painter.save();
  painter.translate(offsetX_, offsetY_);
  painter.scale(zoom_, zoom_);

  // Draw qubit lines
  for (int i = 0; i < numQubits_; ++i) {
    drawQubitLine(painter, i);
  }

  // Draw gates
  for (const Gate &gate : gates_) {
    int x = 100 + gate.step * stepSpacing_;
    int y = 50 + gate.qubit * qubitSpacing_;
    drawGate(painter, x, y, gate.type);
  }

  painter.restore();
}

void CircuitView::drawQubitLine(QPainter &painter, int qubitIndex) {
  int y = 50 + qubitIndex * qubitSpacing_;
  int lineWidth = width() / zoom_;

  painter.setPen(QPen(Qt::black, 2.0 / zoom_));
  painter.drawLine(50, y, lineWidth - 50, y);

  // Draw qubit label
  painter.setPen(Qt::black);
  painter.setFont(QFont("Arial", static_cast<int>(10 / zoom_)));
  painter.drawText(QRect(10, y - 10, 30, 20), Qt::AlignCenter,
                   QString("q%1").arg(qubitIndex));
}

void CircuitView::drawGate(QPainter &painter, int x, int y,
                           const QString &gateType) {
  float adjustedGateSize = gateSize_;
  QRectF gateRect(x - adjustedGateSize / 2, y - adjustedGateSize / 2,
                  adjustedGateSize, adjustedGateSize);

  // Different colors for different gate types
  QColor gateColor = Qt::lightGray;
  if (gateType == "H")
    gateColor = QColor(100, 150, 255);
  else if (gateType == "X")
    gateColor = QColor(255, 100, 100);
  else if (gateType == "CNOT")
    gateColor = QColor(100, 255, 150);
  else if (gateType == "Z")
    gateColor = QColor(255, 255, 100);

  // Draw gate background
  painter.setBrush(QBrush(gateColor));
  painter.setPen(QPen(Qt::black, 1.5 / zoom_));
  painter.drawRoundedRect(gateRect, 3, 3);

  // Draw gate text
  painter.setPen(Qt::black);
  painter.setFont(QFont("Arial", static_cast<int>(12 / zoom_), QFont::Bold));
  painter.drawText(gateRect, Qt::AlignCenter, gateType);
}

void CircuitView::drawText(QPainter &painter, int x, int y,
                           const QString &text) {
  painter.setPen(Qt::black);
  painter.setFont(QFont("Arial", static_cast<int>(10 / zoom_)));
  painter.drawText(x, y, text);
}

void CircuitView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
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
  // Zoom in/out with mouse wheel
  float zoomFactor = 1.1f;
  if (event->angleDelta().y() > 0) {
    zoom_ *= zoomFactor;
  } else {
    zoom_ /= zoomFactor;
  }

  // Clamp zoom
  zoom_ = qBound(0.1f, zoom_, 5.0f);

  update();
}
