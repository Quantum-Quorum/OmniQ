//
// Created by Goutham Arcot on 18/01/26.
//

#include "BlochSphereWidget.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <cmath>

BlochSphereWidget::BlochSphereWidget(QWidget *parent)
    : QOpenGLWidget(parent), program_(nullptr), theta_(0.0f), phi_(0.0f),
      rotationX_(0.0f), rotationY_(0.0f), cameraDistance_(5.0f) {
  setMinimumSize(400, 400);
}

BlochSphereWidget::~BlochSphereWidget() {
  makeCurrent();
  delete program_;
  doneCurrent();
}

void BlochSphereWidget::setQuantumState(std::complex<double> alpha,
                                        std::complex<double> beta) {
  // Convert amplitudes to Bloch angles
  // |psi> = cos(theta/2)|0> + exp(i*phi)sin(theta/2)|1>

  double r_alpha = std::abs(alpha);
  double r_beta = std::abs(beta);

  // Calculate theta: 2 * acos(|alpha|)
  // (assuming normalized state)
  if (r_alpha > 1.0)
    r_alpha = 1.0;
  theta_ = 2.0f * std::acos(r_alpha);

  // Calculate phi: phase(beta) - phase(alpha)
  double phase_alpha = std::arg(alpha);
  double phase_beta = std::arg(beta);
  phi_ = phase_beta - phase_alpha;

  update();
}

void BlochSphereWidget::resetState() {
  theta_ = 0.0f;
  phi_ = 0.0f;
  update();
}

void BlochSphereWidget::initializeGL() {
  initializeOpenGLFunctions();

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  // Enable simple lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);

  // Since we are using fixed pipeline for simplicity in this prototype,
  // we don't strictly need a shader yet, but we'll set up a dummy one if we go
  // programmable later.
}

void BlochSphereWidget::resizeGL(int w, int h) {
  glViewport(0, 0, w, h);

  projection_.setToIdentity();
  projection_.perspective(45.0f, float(w) / float(h), 0.1f, 100.0f);
}

void BlochSphereWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projection_.constData());

  view_.setToIdentity();
  view_.translate(0.0f, 0.0f, -cameraDistance_);
  view_.rotate(rotationX_, 1.0f, 0.0f, 0.0f);
  view_.rotate(rotationY_, 0.0f, 1.0f, 0.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_.constData());

  // Draw the sphere wireframe
  glColor3f(0.3f, 0.3f, 0.3f); // Gray for sphere
  drawSphere(1.0f);

  // Draw axes
  drawAxes(1.5f);

  // Draw state vector
  drawStateVector();
}

void BlochSphereWidget::drawSphere(float radius) {
  // Draw standard glut-like wire sphere manually or use simple loops
  const int rings = 20;
  const int sectors = 20;
  const float PI = 3.14159265359f;

  // Draw longitude lines
  for (int j = 0; j < sectors; ++j) {
    float phi = 2.0f * PI * float(j) / float(sectors);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= rings; ++i) {
      float theta = PI * float(i) / float(rings);
      float x = radius * std::sin(theta) * std::cos(phi);
      float y = radius * std::cos(theta);
      float z = radius * std::sin(theta) * std::sin(phi);
      glVertex3f(x, y, z);
    }
    glEnd();
  }

  // Draw latitude rings
  for (int i = 1; i < rings; ++i) {
    float theta = PI * float(i) / float(rings);
    glBegin(GL_LINE_LOOP);
    for (int j = 0; j < sectors; ++j) {
      float phi = 2.0f * PI * float(j) / float(sectors);
      float x = radius * std::sin(theta) * std::cos(phi);
      float y = radius * std::cos(theta); // Y is up
      float z = radius * std::sin(theta) * std::sin(phi);
      glVertex3f(x, y, z);
    }
    glEnd();
  }
}

void BlochSphereWidget::drawAxes(float length) {
  glLineWidth(2.0f);
  glBegin(GL_LINES);

  // X Axis (Red)
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(-length, 0.0f, 0.0f);
  glVertex3f(length, 0.0f, 0.0f);

  // Y Axis (Green) - In Bloch sphere, typically Z is up (poles), so Y is
  // horizontal But in OpenGL Y is typically up. Let's align Z as up for Bloch
  // convention Let's map Bloch(x, y, z) -> OpenGL(x, z, -y) or something
  // similar. Standard Bloch: x = sin(theta)cos(phi) y = sin(theta)sin(phi) z =
  // cos(theta)

  // Let's map Bloch Z to OpenGL Y (Up)
  // Bloch X to OpenGL X (Right)
  // Bloch Y to OpenGL Z (Forward/Back)

  // OpenGL Y Axis (Green) -> Bloch Z (|0> and |1>)
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, -length, 0.0f);
  glVertex3f(0.0f, length, 0.0f);

  // OpenGL Z Axis (Blue) -> Bloch Y
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, -length);
  glVertex3f(0.0f, 0.0f, length);

  glEnd();
  glLineWidth(1.0f);
}

void BlochSphereWidget::drawStateVector() {
  // Convert spherical (theta, phi) to Cartesian
  // Coordinate mapping: Z is up (OpenGL Y), X is right (OpenGL X), Y is depth
  // (OpenGL Z)

  float x = std::sin(theta_) * std::cos(phi_);
  float y = std::cos(theta_); // This is Z in Bloch, but Y in OpenGL
  float z =
      std::sin(theta_) * std::sin(phi_); // This is Y in Bloch, but Z in OpenGL

  glColor3f(1.0f, 1.0f, 0.0f); // Yellow vector
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(x, y, z);
  glEnd();
  glLineWidth(1.0f);

  // Draw point at tip
  glPointSize(8.0f);
  glBegin(GL_POINTS);
  glVertex3f(x, y, z);
  glEnd();
  glPointSize(1.0f);
}

void BlochSphereWidget::mousePressEvent(QMouseEvent *event) {
  lastMousePos_ = event->pos();
}

void BlochSphereWidget::mouseMoveEvent(QMouseEvent *event) {
  float dx = event->x() - lastMousePos_.x();
  float dy = event->y() - lastMousePos_.y();

  if (event->buttons() & Qt::LeftButton) {
    rotationX_ += dy;
    rotationY_ += dx;
    update();
  }

  lastMousePos_ = event->pos();
}

void BlochSphereWidget::wheelEvent(QWheelEvent *event) {
  if (event->angleDelta().y() > 0) {
    cameraDistance_ *= 0.9f;
  } else {
    cameraDistance_ *= 1.1f;
  }

  // Clamp zoom
  if (cameraDistance_ < 1.0f)
    cameraDistance_ = 1.0f;
  if (cameraDistance_ > 50.0f)
    cameraDistance_ = 50.0f;

  update();
}
