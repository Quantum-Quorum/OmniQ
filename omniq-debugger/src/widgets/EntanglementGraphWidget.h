//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef ENTANGLEMENTGRAPHWIDGET_H
#define ENTANGLEMENTGRAPHWIDGET_H

#include <QMap>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QVector3D>
#include <QVector>
#include <QWheelEvent>
#include <QWidget>

struct EntanglementLink {
  int qubit1;
  int qubit2;
  double strength; // 0.0 to 1.0 (e.g., concurrence)
};

class EntanglementGraphWidget : public QOpenGLWidget,
                                public QOpenGLExtraFunctions {
  Q_OBJECT

public:
  explicit EntanglementGraphWidget(QWidget *parent = nullptr);
  ~EntanglementGraphWidget() override;

  void setNumQubits(int numQubits);
  void setEntanglementData(const QVector<EntanglementLink> &links);
  void resetView();

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  void calculateLayout();
  void drawSphere(const QVector3D &position, float radius,
                  const QVector3D &color);
  void drawCylinder(const QVector3D &start, const QVector3D &end, float width,
                    const QVector3D &color);

  // Data
  int numQubits_;
  QVector<EntanglementLink> links_;
  QVector<QVector3D> nodePositions_;

  // OpenGL resources
  QOpenGLShaderProgram *program_;
  int viewMatrixLoc_;
  int modelMatrixLoc_;
  int projMatrixLoc_;
  int colorLoc_;
  int normalMatrixLoc_;
  int lightPosLoc_;

  // Camera/View
  QMatrix4x4 projection_;
  QMatrix4x4 view_;
  QMatrix4x4 model_; // Global model rotation

  float cameraDistance_;
  QPoint lastMousePos_;
  float rotationX_;
  float rotationY_;
};

#endif // ENTANGLEMENTGRAPHWIDGET_H
