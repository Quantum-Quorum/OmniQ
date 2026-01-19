//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef BLOCHSPHEREWIDGET_H
#define BLOCHSPHEREWIDGET_H

#include <QMatrix4x4>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QTimer>
#include <complex>

class BlochSphereWidget : public QOpenGLWidget, public QOpenGLExtraFunctions {
  Q_OBJECT

public:
  explicit BlochSphereWidget(QWidget *parent = nullptr);
  ~BlochSphereWidget() override;

  void setQuantumState(std::complex<double> alpha, std::complex<double> beta);
  void resetState();

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  // Mouse interaction for rotation
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  void drawSphere(float radius);
  void drawAxes(float length);
  void drawStateVector();

  // Shader program
  QOpenGLShaderProgram *program_;

  // State variables
  float theta_; // Bloch sphere theta (0 to pi)
  float phi_;   // Bloch sphere phi (0 to 2pi)

  // Camera/View
  QMatrix4x4 projection_;
  QMatrix4x4 view_;
  QMatrix4x4 model_;

  // Rotation state
  QPoint lastMousePos_;
  float rotationX_;
  float rotationY_;
  float cameraDistance_;
};

#endif // BLOCHSPHEREWIDGET_H
