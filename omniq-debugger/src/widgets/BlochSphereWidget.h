//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef BLOCHSPHEREWIDGET_H
#define BLOCHSPHEREWIDGET_H

#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QWidget>
#include <complex>

struct TrajectoryMarker {
  int step;
  QString label;
  float theta;
  float phi;
  float r;
  QPointF screenPos;
  float depth;
  QRectF badgeRect;
};

class BlochSphereWidget : public QWidget {
  Q_OBJECT

public:
  explicit BlochSphereWidget(QWidget *parent = nullptr);
  ~BlochSphereWidget() override = default;

  void setQuantumState(std::complex<double> alpha, std::complex<double> beta);
  void setBlochCoordinates(double theta, double phi, double r = 1.0);
  void setQubitIndex(int index);
  void resetState();
  void resetView();

  void clearTrajectory();
  void addTrajectoryPoint(int step, const QString &label, double theta,
                          double phi, double r);
  void setActiveStep(int step);
  void setRunning(bool isRunning);

  QSize sizeHint() const override { return QSize(380, 380); }
  QSize minimumSizeHint() const override { return QSize(160, 160); }

signals:
  void stepSelected(int step);
  void runRequested();
  void pauseRequested();
  void stepForwardRequested();
  void resetRequested();

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  struct Point3D {
    float x;
    float y;
    float z;
  };

  struct ProjectedPoint {
    QPointF screenPos;
    float depth;
  };

  ProjectedPoint project(const Point3D &p, float cx, float cy,
                         float radius) const;
  void drawLatitudeRings(QPainter &painter, float cx, float cy, float radius);
  void drawLongitudeRings(QPainter &painter, float cx, float cy, float radius);
  void drawAxes(QPainter &painter, float cx, float cy, float radius);
  void drawTrajectory(QPainter &painter, float cx, float cy, float radius);
  void drawStateVector(QPainter &painter, float cx, float cy, float radius);
  void drawFloatingBadge(QPainter &painter, const QPointF &pos,
                         const QString &text, const QColor &bg,
                         const QColor &fg);
  void drawHUD(QPainter &painter);
  void drawPlaybackOverlay(QPainter &painter);

  // Quantum state parameters
  float theta_;  // Polar angle [0, pi]
  float phi_;    // Azimuthal angle [0, 2pi]
  float radius_; // Bloch vector length r in [0, 1]
  int qubitIndex_;
  int activeStep_;
  bool isRunning_;
  std::complex<double> alpha_;
  std::complex<double> beta_;

  // Camera angles & view parameters
  float pitch_; // Rotation around X (tilt up/down in degrees)
  float yaw_;   // Rotation around Z (spin left/right in degrees)
  float panX_;
  float panY_;
  float zoom_;

  QPoint lastMousePos_;

  // Trajectory markers
  QVector<TrajectoryMarker> markers_;

  // Playback hit-test rects
  QRectF runBtnRect_;
  QRectF stepBtnRect_;
  QRectF resetBtnRect_;
};

#endif // BLOCHSPHEREWIDGET_H
