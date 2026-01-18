//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QString>
#include <QVector>
#include <QWheelEvent>

class QPainter;

class CircuitView : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit CircuitView(QWidget *parent = nullptr);
  ~CircuitView() override;

  void setNumQubits(int numQubits);
  void addGate(int step, int qubit, const QString &gateType);
  void clear();

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  void drawQubitLine(QPainter &painter, int qubitIndex);
  void drawGate(QPainter &painter, int x, int y, const QString &gateType);
  void drawText(QPainter &painter, int x, int y, const QString &text);

  struct Gate {
    int step;
    int qubit;
    QString type;
  };

  // Circuit data
  int numQubits_;
  QVector<Gate> gates_;

  // View parameters
  float offsetX_;
  float offsetY_;
  float zoom_;

  // Rendering parameters
  int gateSize_;
  int qubitSpacing_;
  int stepSpacing_;

  // Interaction
  QPoint lastMousePos_;
  bool isPanning_;

  // OpenGL
  QMatrix4x4 projection_;
};

#endif // CIRCUITVIEW_H
