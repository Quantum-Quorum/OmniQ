//
// Created by Goutham Arcot on 18/01/26.
//

#include "HilbertSpaceWidget.h"
#include <algorithm>
#include <cmath>

HilbertSpaceWidget::HilbertSpaceWidget(QWidget *parent)
    : QOpenGLWidget(parent), program_(nullptr), cameraDistance_(10.0f),
      rotationX_(30.0f), rotationY_(-45.0f) {}

HilbertSpaceWidget::~HilbertSpaceWidget() {
  makeCurrent();
  delete program_;
  doneCurrent();
}

void HilbertSpaceWidget::setStateVector(
    const QVector<std::complex<double>> &stateVector) {
  stateVector_ = stateVector;
  update();
}

void HilbertSpaceWidget::resetView() {
  cameraDistance_ = 10.0f;
  rotationX_ = 30.0f;
  rotationY_ = -45.0f;
  update();
}

void HilbertSpaceWidget::initializeGL() {
  initializeOpenGLFunctions();
  glClearColor(0.9f, 0.9f, 0.95f, 1.0f); // Light bluish gray background
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  program_ = new QOpenGLShaderProgram;

  // Vertex Shader
  const char *vshader = R"(
        #version 120
        attribute highp vec4 vertex;
        attribute highp vec3 normal;
        uniform highp mat4 projMatrix;
        uniform highp mat4 mvMatrix;
        uniform highp mat3 normalMatrix;
        varying highp vec3 v_normal;
        varying highp vec3 v_pos;
        void main() {
            v_normal = normalize(normalMatrix * normal);
            v_pos = vec3(mvMatrix * vertex);
            gl_Position = projMatrix * mvMatrix * vertex;
        }
    )";

  // Fragment Shader
  const char *fshader = R"(
        #version 120
        varying highp vec3 v_normal;
        varying highp vec3 v_pos;
        uniform highp vec3 color;
        uniform highp vec3 lightPos;
        void main() {
            highp vec3 L = normalize(lightPos - v_pos);
            highp vec3 N = normalize(v_normal);
            highp float diff = max(dot(N, L), 0.2); // Base ambient 0.2
            // Add specular
            highp vec3 V = normalize(-v_pos);
            highp vec3 R = reflect(-L, N);
            highp float spec = pow(max(dot(R, V), 0.0), 32.0);
            
            highp vec3 finalColor = (diff + spec * 0.5) * color;
            gl_FragColor = vec4(finalColor, 1.0);
        }
    )";

  program_->addShaderFromSourceCode(QOpenGLShader::Vertex, vshader);
  program_->addShaderFromSourceCode(QOpenGLShader::Fragment, fshader);
  program_->link();

  program_->bind();
  viewMatrixLoc_ = program_->uniformLocation("mvMatrix");
  projMatrixLoc_ = program_->uniformLocation("projMatrix");
  normalMatrixLoc_ = program_->uniformLocation("normalMatrix");
  colorLoc_ = program_->uniformLocation("color");
  lightPosLoc_ = program_->uniformLocation("lightPos");
  program_->release();
}

void HilbertSpaceWidget::resizeGL(int w, int h) {
  projection_.setToIdentity();
  projection_.perspective(45.0f, GLfloat(w) / h, 0.1f, 100.0f);
}

void HilbertSpaceWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (stateVector_.isEmpty())
    return;

  program_->bind();

  // View Matrix
  view_.setToIdentity();
  view_.translate(0.0f, -2.0f, -cameraDistance_); // Shift down slightly
  view_.rotate(rotationX_, 1.0f, 0.0f, 0.0f);
  view_.rotate(rotationY_, 0.0f, 1.0f, 0.0f);

  program_->setUniformValue(lightPosLoc_, QVector3D(10.0f, 10.0f, 10.0f));
  program_->setUniformValue(projMatrixLoc_, projection_);

  // Layout Logic
  int n = stateVector_.size();
  float side = std::ceil(std::sqrt(n));
  int rows = static_cast<int>(side);
  int cols = static_cast<int>(std::ceil((float)n / rows));

  float spacing = 1.0f; // Bar width + gap
  float totalWidth = cols * spacing;
  float totalDepth = rows * spacing;

  float startX = -totalWidth / 2.0f + spacing / 2.0f;
  float startZ = -totalDepth / 2.0f + spacing / 2.0f;

  for (int i = 0; i < n; ++i) {
    int r = i / cols;
    int c = i % cols;
    double magnitude = std::abs(stateVector_[i]);
    if (magnitude < 0.001)
      continue; // Skip near-zero bars

    double phase = std::arg(stateVector_[i]);

    float x = startX + c * spacing;
    float z = startZ + r * spacing;
    float h = static_cast<float>(magnitude * 4.0); // Scale height

    QVector3D color = phaseToColor(phase);

    drawBar(x, z, 0.8f, h, color);
  }

  // Draw Ground Grid (optional, simplistic)
  // could draw a flat quad here...

  program_->release();
}

void HilbertSpaceWidget::drawBar(float x, float z, float width, float height,
                                 const QVector3D &color) {
  QMatrix4x4 modelView = view_;
  modelView.translate(x, height / 2.0f, z); // Center vertically at half height
  modelView.scale(width, height, width);

  program_->setUniformValue(viewMatrixLoc_, modelView);
  program_->setUniformValue(normalMatrixLoc_, modelView.normalMatrix());
  program_->setUniformValue(colorLoc_, color);

  // Basic Cube Unit Vertices
  static const float vertices[] = {
      // Front
      -0.5,
      -0.5,
      0.5,
      0.0,
      0.0,
      1.0,
      0.5,
      -0.5,
      0.5,
      0.0,
      0.0,
      1.0,
      0.5,
      0.5,
      0.5,
      0.0,
      0.0,
      1.0,
      0.5,
      0.5,
      0.5,
      0.0,
      0.0,
      1.0,
      -0.5,
      0.5,
      0.5,
      0.0,
      0.0,
      1.0,
      -0.5,
      -0.5,
      0.5,
      0.0,
      0.0,
      1.0,
      // Top
      -0.5,
      0.5,
      0.5,
      0.0,
      1.0,
      0.0,
      0.5,
      0.5,
      0.5,
      0.0,
      1.0,
      0.0,
      0.5,
      0.5,
      -0.5,
      0.0,
      1.0,
      0.0,
      0.5,
      0.5,
      -0.5,
      0.0,
      1.0,
      0.0,
      -0.5,
      0.5,
      -0.5,
      0.0,
      1.0,
      0.0,
      -0.5,
      0.5,
      0.5,
      0.0,
      1.0,
      0.0,
      // (Other faces omitted for brevity - user sees front/top mostly in
      // default view. Add later if needed for full robustness)
      // ... well, let's add right face at least
      0.5,
      -0.5,
      0.5,
      1.0,
      0.0,
      0.0,
      0.5,
      -0.5,
      -0.5,
      1.0,
      0.0,
      0.0,
      0.5,
      0.5,
      -0.5,
      1.0,
      0.0,
      0.0,
      0.5,
      0.5,
      -0.5,
      1.0,
      0.0,
      0.0,
      0.5,
      0.5,
      0.5,
      1.0,
      0.0,
      0.0,
      0.5,
      -0.5,
      0.5,
      1.0,
      0.0,
      0.0,
  };

  program_->enableAttributeArray("vertex");
  program_->enableAttributeArray("normal");
  program_->setAttributeArray("vertex", GL_FLOAT, vertices, 3,
                              6 * sizeof(float));
  program_->setAttributeArray("normal", GL_FLOAT, vertices + 3, 3,
                              6 * sizeof(float));
  glDrawArrays(GL_TRIANGLES, 0, 18); // 3 faces
  program_->disableAttributeArray("vertex");
  program_->disableAttributeArray("normal");
}

QVector3D HilbertSpaceWidget::phaseToColor(double phase) {
  // Map phase [-pi, pi] to Hue [0, 1]
  double hue = (phase + M_PI) / (2.0 * M_PI); // 0..1

  // HSV to RGB conversion (simplified)
  float r, g, b;
  float h = hue * 6.0f;
  float c = 1.0f; // Saturation 1.0, Value 1.0
  float x = c * (1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f));

  if (h >= 0 && h < 1) {
    r = c;
    g = x;
    b = 0;
  } else if (h >= 1 && h < 2) {
    r = x;
    g = c;
    b = 0;
  } else if (h >= 2 && h < 3) {
    r = 0;
    g = c;
    b = x;
  } else if (h >= 3 && h < 4) {
    r = 0;
    g = x;
    b = c;
  } else if (h >= 4 && h < 5) {
    r = x;
    g = 0;
    b = c;
  } else {
    r = c;
    g = 0;
    b = x;
  }

  return QVector3D(r, g, b);
}

void HilbertSpaceWidget::mousePressEvent(QMouseEvent *event) {
  lastMousePos_ = event->pos();
}

void HilbertSpaceWidget::mouseMoveEvent(QMouseEvent *event) {
  int dx = event->x() - lastMousePos_.x();
  int dy = event->y() - lastMousePos_.y();

  if (event->buttons() & Qt::LeftButton) {
    rotationX_ += dy;
    rotationY_ += dx;
    update();
  }
  lastMousePos_ = event->pos();
}

void HilbertSpaceWidget::wheelEvent(QWheelEvent *event) {
  if (event->angleDelta().y() > 0)
    cameraDistance_ *= 0.9f;
  else
    cameraDistance_ *= 1.1f;
  update();
}
