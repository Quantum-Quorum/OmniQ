//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef HILBERTSPACEWIDGET_H
#define HILBERTSPACEWIDGET_H

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QVector>
#include <QWheelEvent>
#include <QWidget>
#include <complex>

class HilbertSpaceWidget : public QOpenGLWidget, public QOpenGLExtraFunctions {
  Q_OBJECT

public:
  explicit HilbertSpaceWidget(QWidget *parent = nullptr);
  ~HilbertSpaceWidget() override;

  void setStateVector(const QVector<std::complex<double>> &stateVector);
  void resetView();

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  void drawBar(float x, float z, float width, float height,
               const QVector3D &color);
  QVector3D phaseToColor(double phase);

  // Data
  QVector<std::complex<double>> stateVector_;

  // OpenGL resources
  QOpenGLShaderProgram *program_;
  int viewMatrixLoc_;
  int projMatrixLoc_;
  int normalMatrixLoc_;
  int colorLoc_;
  int lightPosLoc_;

  // Camera/View
  QMatrix4x4 projection_;
  QMatrix4x4 view_;

  float cameraDistance_;
  float rotationX_;
  float rotationY_;
  QPoint lastMousePos_;
};

#endif // HILBERTSPACEWIDGET_H
