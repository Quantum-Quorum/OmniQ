//
// Created by Goutham Arcot on 18/01/26.
//

#include "BlochSphereWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <algorithm>
#include <cmath>

static constexpr float PI_F = 3.14159265358979323846f;

BlochSphereWidget::BlochSphereWidget(QWidget *parent)
    : QWidget(parent), theta_(0.0f), phi_(0.0f), radius_(1.0f),
      qubitIndex_(0), activeStep_(0), isRunning_(false), alpha_(1.0, 0.0),
      beta_(0.0, 0.0), pitch_(22.0f), yaw_(-40.0f), panX_(0.0f), panY_(0.0f),
      zoom_(1.0f) {
  setMinimumSize(160, 160);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setMouseTracking(true);
}

void BlochSphereWidget::setQuantumState(std::complex<double> alpha,
                                        std::complex<double> beta) {
  alpha_ = alpha;
  beta_ = beta;

  double r_alpha = std::abs(alpha);
  double r_beta = std::abs(beta);
  double normSq = r_alpha * r_alpha + r_beta * r_beta;
  if (normSq > 1e-12) {
    double norm = std::sqrt(normSq);
    r_alpha /= norm;
    r_beta /= norm;
    radius_ = 1.0f;
  } else {
    radius_ = 0.0f;
  }

  if (r_alpha > 1.0) r_alpha = 1.0;
  theta_ = static_cast<float>(2.0 * std::acos(r_alpha));

  double phase_alpha = std::arg(alpha);
  double phase_beta = std::arg(beta);
  double phi = phase_beta - phase_alpha;
  while (phi < 0.0) phi += 2.0 * PI_F;
  while (phi >= 2.0 * PI_F) phi -= 2.0 * PI_F;
  phi_ = static_cast<float>(phi);

  update();
}

void BlochSphereWidget::setBlochCoordinates(double theta, double phi, double r) {
  theta_ = static_cast<float>(std::clamp(theta, 0.0, static_cast<double>(PI_F)));
  while (phi < 0.0) phi += 2.0 * PI_F;
  while (phi >= 2.0 * PI_F) phi -= 2.0 * PI_F;
  phi_ = static_cast<float>(phi);
  radius_ = static_cast<float>(std::clamp(r, 0.0, 1.0));

  double halfTheta = theta_ / 2.0;
  alpha_ = std::complex<double>(std::cos(halfTheta), 0.0);
  beta_ = std::polar(std::sin(halfTheta), static_cast<double>(phi_));

  update();
}

void BlochSphereWidget::setQubitIndex(int index) {
  qubitIndex_ = index;
  update();
}

void BlochSphereWidget::resetState() {
  theta_ = 0.0f;
  phi_ = 0.0f;
  radius_ = 1.0f;
  alpha_ = std::complex<double>(1.0, 0.0);
  beta_ = std::complex<double>(0.0, 0.0);
  update();
}

void BlochSphereWidget::resetView() {
  pitch_ = 22.0f;
  yaw_ = -40.0f;
  panX_ = 0.0f;
  panY_ = 0.0f;
  zoom_ = 1.0f;
  update();
}

void BlochSphereWidget::clearTrajectory() {
  markers_.clear();
  update();
}

void BlochSphereWidget::addTrajectoryPoint(int step, const QString &label,
                                          double theta, double phi, double r) {
  TrajectoryMarker marker;
  marker.step = step;
  marker.label = label;
  marker.theta = static_cast<float>(theta);
  marker.phi = static_cast<float>(phi);
  marker.r = static_cast<float>(std::clamp(r, 0.0, 1.0));
  markers_.append(marker);
  update();
}

void BlochSphereWidget::setActiveStep(int step) {
  activeStep_ = step;
  update();
}

void BlochSphereWidget::setRunning(bool isRunning) {
  isRunning_ = isRunning;
  update();
}

BlochSphereWidget::ProjectedPoint
BlochSphereWidget::project(const Point3D &p, float cx, float cy,
                           float radius) const {
  float yawRad = yaw_ * PI_F / 180.0f;
  float pitchRad = pitch_ * PI_F / 180.0f;

  // Rotate around vertical Z axis (quantization axis) by yaw
  float x1 = p.x * std::cos(yawRad) + p.y * std::sin(yawRad);
  float y1 = -p.x * std::sin(yawRad) + p.y * std::cos(yawRad);
  float z1 = p.z;

  // Rotate around horizontal axis by pitch (tilt up/down)
  float x2 = x1;
  float y2 = y1 * std::cos(pitchRad) - z1 * std::sin(pitchRad);
  float z2 = y1 * std::sin(pitchRad) + z1 * std::cos(pitchRad);

  // Perspective factor
  float d = 4.2f;
  float persp = d / (d - y2 * 0.75f);

  float sx = cx + panX_ + x2 * radius * zoom_ * persp;
  float sy = cy + panY_ - z2 * radius * zoom_ * persp;

  return {QPointF(sx, sy), y2};
}

void BlochSphereWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);

  // Background
  painter.fillRect(rect(), QColor(14, 17, 24));

  float cx = width() / 2.0f;
  float cy = height() / 2.0f;
  float R = std::min(width(), height()) * 0.35f;
  if (R < 30.0f) R = 30.0f;

  // 1. Shaded 3D Sphere Body with Radial Light Gradient
  QPointF sphereCenter(cx + panX_, cy + panY_);
  float effR = R * zoom_;
  QRadialGradient sphereGrad(sphereCenter.x() - effR * 0.28f,
                            sphereCenter.y() - effR * 0.30f, effR * 1.25f);
  sphereGrad.setColorAt(0.0, QColor(30, 41, 59, 140)); // Specular highlight
  sphereGrad.setColorAt(0.55, QColor(15, 23, 42, 185)); // Body navy
  sphereGrad.setColorAt(0.92, QColor(8, 12, 22, 220));
  sphereGrad.setColorAt(1.0, QColor(2, 6, 18, 245));   // Rim shadow

  painter.setPen(QPen(QColor(56, 189, 248, 50), 1.2));
  painter.setBrush(sphereGrad);
  painter.drawEllipse(sphereCenter, effR, effR);

  // 2. Draw Grid Rings
  drawLatitudeRings(painter, cx, cy, R);
  drawLongitudeRings(painter, cx, cy, R);

  // 3. Draw Cartesian 3D Axes
  drawAxes(painter, cx, cy, R);

  // 4. Draw Quantum Trajectory Path & Waypoint Markers
  drawTrajectory(painter, cx, cy, R);

  // 5. Draw Quantum State Vector & Projections
  drawStateVector(painter, cx, cy, R);

  // 6. Draw HUD and Playback Overlay Controls
  drawHUD(painter);
  drawPlaybackOverlay(painter);
}

void BlochSphereWidget::drawLatitudeRings(QPainter &painter, float cx, float cy,
                                         float radius) {
  const int segments = 48;
  const float lats[] = {30.0f, 60.0f, 90.0f, 120.0f, 150.0f};

  for (float latDeg : lats) {
    bool isEquator = (std::abs(latDeg - 90.0f) < 0.1f);
    float latRad = latDeg * PI_F / 180.0f;
    float z = std::cos(latRad);
    float ringR = std::sin(latRad);

    QPen penBack = isEquator
                       ? QPen(QColor(0, 229, 255, 65), 1.6, Qt::DashLine)
                       : QPen(QColor(148, 163, 184, 30), 1.0, Qt::DotLine);
    QPen penFront = isEquator
                        ? QPen(QColor(0, 229, 255, 230), 2.4, Qt::SolidLine)
                        : QPen(QColor(148, 163, 184, 85), 1.2, Qt::SolidLine);

    for (int i = 0; i < segments; ++i) {
      float psi1 = 2.0f * PI_F * float(i) / float(segments);
      float psi2 = 2.0f * PI_F * float(i + 1) / float(segments);

      Point3D p1 = {ringR * std::cos(psi1), ringR * std::sin(psi1), z};
      Point3D p2 = {ringR * std::cos(psi2), ringR * std::sin(psi2), z};

      ProjectedPoint pr1 = project(p1, cx, cy, radius);
      ProjectedPoint pr2 = project(p2, cx, cy, radius);

      bool isFront = (pr1.depth + pr2.depth) >= 0.0f;
      painter.setPen(isFront ? penFront : penBack);
      painter.drawLine(pr1.screenPos, pr2.screenPos);
    }
  }
}

void BlochSphereWidget::drawLongitudeRings(QPainter &painter, float cx, float cy,
                                          float radius) {
  const int segments = 48;
  const float longs[] = {0.0f, 45.0f, 90.0f, 135.0f};

  for (float longDeg : longs) {
    bool isPrime = (std::abs(longDeg) < 0.1f);
    float longRad = longDeg * PI_F / 180.0f;
    float cosLong = std::cos(longRad);
    float sinLong = std::sin(longRad);

    QPen penBack = isPrime
                       ? QPen(QColor(192, 132, 252, 65), 1.5, Qt::DashLine)
                       : QPen(QColor(148, 163, 184, 25), 1.0, Qt::DotLine);
    QPen penFront = isPrime
                        ? QPen(QColor(192, 132, 252, 220), 2.2, Qt::SolidLine)
                        : QPen(QColor(148, 163, 184, 75), 1.0, Qt::SolidLine);

    for (int i = 0; i < segments; ++i) {
      float psi1 = 2.0f * PI_F * float(i) / float(segments);
      float psi2 = 2.0f * PI_F * float(i + 1) / float(segments);

      Point3D p1 = {std::sin(psi1) * cosLong, std::sin(psi1) * sinLong,
                    std::cos(psi1)};
      Point3D p2 = {std::sin(psi2) * cosLong, std::sin(psi2) * sinLong,
                    std::cos(psi2)};

      ProjectedPoint pr1 = project(p1, cx, cy, radius);
      ProjectedPoint pr2 = project(p2, cx, cy, radius);

      bool isFront = (pr1.depth + pr2.depth) >= 0.0f;
      painter.setPen(isFront ? penFront : penBack);
      painter.drawLine(pr1.screenPos, pr2.screenPos);
    }
  }
}

void BlochSphereWidget::drawAxes(QPainter &painter, float cx, float cy,
                                float radius) {
  float len = 1.35f;

  auto drawAxisLine = [&](const Point3D &from, const Point3D &to,
                          const QColor &color) {
    ProjectedPoint pFrom = project(from, cx, cy, radius);
    ProjectedPoint pTo = project(to, cx, cy, radius);

    painter.setPen(QPen(color, 2.2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(pFrom.screenPos, pTo.screenPos);
  };

  // Z Axis (Quantization axis, Sky Blue: +Z = |0⟩, -Z = |1⟩)
  drawAxisLine({0.0f, 0.0f, -len}, {0.0f, 0.0f, len}, QColor(56, 189, 248));

  // X Axis (Coral Red: +X = |+⟩, -X = |-⟩)
  drawAxisLine({-len, 0.0f, 0.0f}, {len, 0.0f, 0.0f}, QColor(248, 113, 113));

  // Y Axis (Emerald Green: +Y = |+i⟩, -Y = |-i⟩)
  drawAxisLine({0.0f, -len, 0.0f}, {0.0f, len, 0.0f}, QColor(52, 211, 153));

  // Floating Badges for Poles and Axis Extremities
  ProjectedPoint ptZPos = project({0.0f, 0.0f, len + 0.12f}, cx, cy, radius);
  ProjectedPoint ptZNeg = project({0.0f, 0.0f, -len - 0.12f}, cx, cy, radius);
  ProjectedPoint ptXPos = project({len + 0.12f, 0.0f, 0.0f}, cx, cy, radius);
  ProjectedPoint ptXNeg = project({-len - 0.12f, 0.0f, 0.0f}, cx, cy, radius);
  ProjectedPoint ptYPos = project({0.0f, len + 0.12f, 0.0f}, cx, cy, radius);
  ProjectedPoint ptYNeg = project({0.0f, -len - 0.12f, 0.0f}, cx, cy, radius);

  drawFloatingBadge(painter, ptZPos.screenPos, "|0⟩ (+Z)",
                    QColor(14, 116, 144, 220), QColor(224, 242, 254));
  drawFloatingBadge(painter, ptZNeg.screenPos, "|1⟩ (-Z)",
                    QColor(14, 116, 144, 220), QColor(224, 242, 254));
  drawFloatingBadge(painter, ptXPos.screenPos, "|+⟩ (+X)",
                    QColor(159, 18, 57, 220), QColor(255, 228, 230));
  drawFloatingBadge(painter, ptXNeg.screenPos, "|-⟩ (-X)",
                    QColor(159, 18, 57, 220), QColor(255, 228, 230));
  drawFloatingBadge(painter, ptYPos.screenPos, "|+i⟩ (+Y)",
                    QColor(6, 95, 70, 220), QColor(209, 250, 229));
  drawFloatingBadge(painter, ptYNeg.screenPos, "|-i⟩ (-Y)",
                    QColor(6, 95, 70, 220), QColor(209, 250, 229));
}

void BlochSphereWidget::drawTrajectory(QPainter &painter, float cx, float cy,
                                      float radius) {
  if (markers_.isEmpty())
    return;

  // Project all waypoints
  for (auto &m : markers_) {
    float r = std::clamp(m.r, 0.0f, 1.0f);
    float mx = r * std::sin(m.theta) * std::cos(m.phi);
    float my = r * std::sin(m.theta) * std::sin(m.phi);
    float mz = r * std::cos(m.theta);

    ProjectedPoint pr = project({mx, my, mz}, cx, cy, radius);
    m.screenPos = pr.screenPos;
    m.depth = pr.depth;
  }

  // 1. Draw connecting path between consecutive steps
  for (int i = 0; i < markers_.size() - 1; ++i) {
    const auto &m1 = markers_[i];
    const auto &m2 = markers_[i + 1];

    if (i < activeStep_) {
      // Executed path: Solid glowing gold
      painter.setPen(
          QPen(QColor(251, 191, 36, 210), 3.0, Qt::SolidLine, Qt::RoundCap));
    } else {
      // Future planned path: Elegant dashed purple
      painter.setPen(
          QPen(QColor(168, 85, 247, 130), 1.8, Qt::DashLine, Qt::RoundCap));
    }
    painter.drawLine(m1.screenPos, m2.screenPos);
  }

  // 2. Draw waypoint nodes and interactive step badges
  QFont badgeFont("Menlo", 8, QFont::Bold);
  QFontMetrics fm(badgeFont);

  for (auto &m : markers_) {
    bool isActive = (m.step == activeStep_);
    bool isPast = (m.step < activeStep_);

    // Draw waypoint sphere node
    if (isActive) {
      // Glowing aura
      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(254, 240, 138, 120));
      painter.drawEllipse(m.screenPos, 9.0, 9.0);

      // Gold core
      painter.setBrush(QColor(251, 191, 36));
      painter.drawEllipse(m.screenPos, 5.5, 5.5);
      painter.setBrush(Qt::white);
      painter.drawEllipse(m.screenPos, 2.5, 2.5);
    } else if (isPast) {
      // Cyan node
      painter.setPen(QPen(QColor(6, 182, 212), 1.2));
      painter.setBrush(QColor(8, 145, 178));
      painter.drawEllipse(m.screenPos, 4.5, 4.5);
    } else {
      // Future: Subtle purple node
      painter.setPen(QPen(QColor(168, 85, 247, 160), 1.0));
      painter.setBrush(QColor(147, 51, 234, 180));
      painter.drawEllipse(m.screenPos, 3.5, 3.5);
    }

    // Step Badge text
    QString badgeText = isActive ? QString("S%1: %2").arg(m.step).arg(m.label)
                                 : QString("%1: %2").arg(m.step).arg(m.label);

    int textW = fm.horizontalAdvance(badgeText);
    int textH = fm.height();
    QPointF badgeCenter = m.screenPos + QPointF(12.0, -8.0);
    QRectF badgeRect(badgeCenter.x() - 4.0, badgeCenter.y() - textH / 2.0 - 2.0,
                    textW + 8.0, textH + 4.0);
    m.badgeRect = badgeRect;

    // Draw Badge
    if (isActive) {
      painter.setPen(QPen(QColor(254, 240, 138), 1.2));
      painter.setBrush(QColor(217, 119, 6, 235));
    } else if (isPast) {
      painter.setPen(QPen(QColor(6, 182, 212, 180), 1.0));
      painter.setBrush(QColor(15, 23, 42, 220));
    } else {
      painter.setPen(QPen(QColor(168, 85, 247, 130), 1.0));
      painter.setBrush(QColor(15, 23, 42, 180));
    }
    painter.drawRoundedRect(badgeRect, 4.0, 4.0);

    painter.setFont(badgeFont);
    painter.setPen(isActive ? QColor(254, 243, 199)
                            : (isPast ? QColor(103, 232, 249)
                                      : QColor(192, 132, 252, 200)));
    painter.drawText(badgeRect, Qt::AlignCenter, badgeText);
  }
}

void BlochSphereWidget::drawStateVector(QPainter &painter, float cx, float cy,
                                       float radius) {
  float r = std::clamp(radius_, 0.0f, 1.0f);
  if (r < 0.001f) r = 0.001f;

  float vx = r * std::sin(theta_) * std::cos(phi_);
  float vy = r * std::sin(theta_) * std::sin(phi_);
  float vz = r * std::cos(theta_);

  ProjectedPoint pCenter = project({0.0f, 0.0f, 0.0f}, cx, cy, radius);
  ProjectedPoint pTip = project({vx, vy, vz}, cx, cy, radius);
  ProjectedPoint pEquator = project({vx, vy, 0.0f}, cx, cy, radius);

  // 1. Dropped vertical projection line from tip down to equator plane
  painter.setPen(QPen(QColor(250, 204, 21, 140), 1.6, Qt::DashLine));
  painter.drawLine(pTip.screenPos, pEquator.screenPos);

  // 2. Radial line on equator plane from origin to projection
  painter.setPen(QPen(QColor(250, 204, 21, 120), 1.4, Qt::DotLine));
  painter.drawLine(pCenter.screenPos, pEquator.screenPos);

  // 3. Equator projection anchor point
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(250, 204, 21, 180));
  painter.drawEllipse(pEquator.screenPos, 4.0, 4.0);

  // 4. Main Quantum State Vector Arrow: Glowing Amber / Gold
  QPen vectorPen(QColor(251, 191, 36), 3.8, Qt::SolidLine, Qt::RoundCap);
  painter.setPen(vectorPen);
  painter.drawLine(pCenter.screenPos, pTip.screenPos);

  // 5. Arrowhead at tip
  QPointF dir = pTip.screenPos - pCenter.screenPos;
  double len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
  if (len > 8.0) {
    QPointF u(dir.x() / len, dir.y() / len);
    QPointF normal(-u.y(), u.x());
    QPointF headBase = pTip.screenPos - u * 12.0;
    QPointF pA = headBase + normal * 5.5;
    QPointF pB = headBase - normal * 5.5;

    QPainterPath headPath;
    headPath.moveTo(pTip.screenPos);
    headPath.lineTo(pA);
    headPath.lineTo(pB);
    headPath.closeSubpath();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(251, 191, 36));
    painter.drawPath(headPath);
  }

  // 6. 3D Shaded Glowing Tip Sphere
  QRadialGradient tipGrad(pTip.screenPos.x() - 2.0, pTip.screenPos.y() - 2.0,
                          8.0);
  tipGrad.setColorAt(0.0, QColor(255, 255, 255));
  tipGrad.setColorAt(0.4, QColor(251, 191, 36));
  tipGrad.setColorAt(1.0, QColor(180, 83, 9));

  painter.setPen(QPen(QColor(255, 255, 255, 180), 1.0));
  painter.setBrush(tipGrad);
  painter.drawEllipse(pTip.screenPos, 6.5, 6.5);
}

void BlochSphereWidget::drawFloatingBadge(QPainter &painter, const QPointF &pos,
                                         const QString &text, const QColor &bg,
                                         const QColor &fg) {
  if (pos.x() < -50 || pos.x() > width() + 50 || pos.y() < -50 ||
      pos.y() > height() + 50)
    return;

  QFont labelFont("Menlo", 9, QFont::Bold);
  painter.setFont(labelFont);
  QFontMetrics fm(labelFont);
  int textW = fm.horizontalAdvance(text);
  int textH = fm.height();
  QRectF rect(pos.x() - textW / 2.0 - 4.0, pos.y() - textH / 2.0 - 2.0,
              textW + 8.0, textH + 4.0);

  painter.setPen(Qt::NoPen);
  painter.setBrush(bg);
  painter.drawRoundedRect(rect, 4.0, 4.0);

  painter.setPen(fg);
  painter.drawText(rect, Qt::AlignCenter, text);
}

void BlochSphereWidget::drawHUD(QPainter &painter) {
  // Sleek Glassmorphism HUD Card in Top-Left
  if (width() >= 230 && height() >= 160) {
    QRect cardRect(10, 10, 220, 106);
    painter.setPen(QPen(QColor(255, 255, 255, 28), 1.0));
    painter.setBrush(QColor(15, 23, 42, 225));
    painter.drawRoundedRect(cardRect, 8.0, 8.0);

    // Header: Qubit index
    painter.setFont(QFont("Menlo", 11, QFont::Bold));
    painter.setPen(QColor(56, 189, 248)); // Cyan
    painter.drawText(cardRect.adjusted(12, 8, -12, -78),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString("Qubit q%1 State").arg(qubitIndex_));

    // Angles
    double thetaDeg = theta_ * 180.0 / PI_F;
    double phiDeg = phi_ * 180.0 / PI_F;
    painter.setFont(QFont("Menlo", 9));
    painter.setPen(QColor(226, 232, 240));
    painter.drawText(cardRect.adjusted(12, 30, -12, -58),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString("θ = %1°   φ = %2°")
                         .arg(thetaDeg, 0, 'f', 1)
                         .arg(phiDeg, 0, 'f', 1));

    // Probabilities
    double p0 = std::norm(alpha_);
    double p1 = std::norm(beta_);
    painter.drawText(cardRect.adjusted(12, 50, -12, -38),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString("P(|0⟩): %1%   P(|1⟩): %2%")
                         .arg(p0 * 100.0, 0, 'f', 1)
                         .arg(p1 * 100.0, 0, 'f', 1));

    // Amplitudes & Bloch purity radius
    painter.setPen(QColor(250, 204, 21)); // Gold
    painter.drawText(cardRect.adjusted(12, 70, -12, -18),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString("α: %1  β: %2 (r: %3)")
                         .arg(std::abs(alpha_), 0, 'f', 2)
                         .arg(std::abs(beta_), 0, 'f', 2)
                         .arg(radius_, 0, 'f', 2));
  }

  // Navigation Hint at Bottom-Right
  if (width() >= 300 && height() >= 100) {
    painter.setFont(QFont("Menlo", 8));
    painter.setPen(QColor(100, 116, 139, 210));
    painter.drawText(
        QRect(10, height() - 20, width() - 20, 16),
        Qt::AlignRight | Qt::AlignVCenter,
        "Click Marker: Jump Step | Drag: Rotate | Wheel: Zoom");
  }
}

void BlochSphereWidget::drawPlaybackOverlay(QPainter &painter) {
  if (width() < 280 || height() < 200)
    return;

  // Floating Pill Control Bar in Bottom-Center
  int pillW = 210;
  int pillH = 32;
  int pillX = (width() - pillW) / 2;
  int pillY = height() - pillH - 24;
  QRectF pillRect(pillX, pillY, pillW, pillH);

  // Outer glow and rounded pill container
  painter.setPen(QPen(QColor(255, 255, 255, 35), 1.0));
  painter.setBrush(QColor(15, 23, 42, 230));
  painter.drawRoundedRect(pillRect, pillH / 2.0, pillH / 2.0);

  // Sub-button rects
  float btnW = (pillW - 16.0f) / 3.0f;
  resetBtnRect_ = QRectF(pillX + 4.0f, pillY + 3.0f, btnW, pillH - 6.0f);
  runBtnRect_ = QRectF(pillX + 8.0f + btnW, pillY + 3.0f, btnW, pillH - 6.0f);
  stepBtnRect_ =
      QRectF(pillX + 12.0f + 2.0f * btnW, pillY + 3.0f, btnW, pillH - 6.0f);

  QFont btnFont("Menlo", 9, QFont::Bold);
  painter.setFont(btnFont);

  // 1. Reset button
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(51, 65, 85, 180));
  painter.drawRoundedRect(resetBtnRect_, 4.0, 4.0);
  painter.setPen(QColor(226, 232, 240));
  painter.drawText(resetBtnRect_, Qt::AlignCenter, "↺ Reset");

  // 2. Run / Pause button (Highlighted)
  painter.setPen(Qt::NoPen);
  painter.setBrush(isRunning_ ? QColor(217, 119, 6) : QColor(16, 185, 129));
  painter.drawRoundedRect(runBtnRect_, 4.0, 4.0);
  painter.setPen(Qt::white);
  painter.drawText(runBtnRect_, Qt::AlignCenter,
                   isRunning_ ? "⏸ Pause" : "▶ Run");

  // 3. Step button
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(51, 65, 85, 180));
  painter.drawRoundedRect(stepBtnRect_, 4.0, 4.0);
  painter.setPen(QColor(226, 232, 240));
  painter.drawText(stepBtnRect_, Qt::AlignCenter, "Step ⏭");
}

void BlochSphereWidget::mousePressEvent(QMouseEvent *event) {
  QPointF pos = event->position();

  // 1. Hit-test Playback Controls
  if (runBtnRect_.contains(pos)) {
    if (isRunning_) {
      emit pauseRequested();
    } else {
      emit runRequested();
    }
    return;
  }
  if (stepBtnRect_.contains(pos)) {
    emit stepForwardRequested();
    return;
  }
  if (resetBtnRect_.contains(pos)) {
    emit resetRequested();
    return;
  }

  // 2. Hit-test Trajectory Markers
  for (const auto &m : markers_) {
    if (m.badgeRect.adjusted(-3, -3, 3, 3).contains(pos)) {
      emit stepSelected(m.step);
      return;
    }
  }

  // 3. Default view drag
  lastMousePos_ = event->pos();
}

void BlochSphereWidget::mouseMoveEvent(QMouseEvent *event) {
  float dx = static_cast<float>(event->position().x() - lastMousePos_.x());
  float dy = static_cast<float>(event->position().y() - lastMousePos_.y());

  if (event->buttons() & Qt::LeftButton) {
    yaw_ += dx * 0.45f;
    pitch_ += dy * 0.45f;
    pitch_ = std::clamp(pitch_, -88.0f, 88.0f);
    update();
  } else if (event->buttons() & Qt::RightButton) {
    panX_ += dx;
    panY_ += dy;
    update();
  }

  lastMousePos_ = event->pos();
}

void BlochSphereWidget::mouseDoubleClickEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  resetView();
}

void BlochSphereWidget::wheelEvent(QWheelEvent *event) {
  if (event->angleDelta().y() > 0) {
    zoom_ *= 1.08f;
  } else {
    zoom_ *= 0.92f;
  }

  zoom_ = std::clamp(zoom_, 0.5f, 3.5f);
  update();
}
