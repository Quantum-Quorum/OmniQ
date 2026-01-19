//
// Created by Goutham Arcot on 17/07/25.
//

#include "QuantumStateViewer.h"
#include "ClickableLabel.h"
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <algorithm>
#include <cmath>
#include <complex>

// Density Matrix Widget Implementation
DensityMatrixWidget::DensityMatrixWidget(QWidget *parent)
    : QWidget(parent), showDensityMatrix_(true), selectedRow_(-1),
      selectedCol_(-1) {
  setMinimumSize(300, 300);
  setMouseTracking(true);
}

void DensityMatrixWidget::setDensityMatrix(
    const QVector<QVector<std::complex<double>>> &matrix) {
  densityMatrix_ = matrix;
  showDensityMatrix_ = true;
  update();
}

void DensityMatrixWidget::setStateVector(
    const QVector<std::complex<double>> &state) {
  stateVector_ = state;
  showDensityMatrix_ = false;
  update();
}

void DensityMatrixWidget::setBasisLabels(const QStringList &labels) {
  basisLabels_ = labels;
  update();
}

void DensityMatrixWidget::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  if (showDensityMatrix_) {
    drawMatrix(painter);
  } else {
    drawStateVector(painter);
  }
  drawBasisLabels(painter);
}

void DensityMatrixWidget::drawMatrix(QPainter &painter) {
  if (densityMatrix_.isEmpty())
    return;

  int size = densityMatrix_.size();
  int cellSize = qMin(width(), height()) / (size + 2);
  int startX = (width() - size * cellSize) / 2;
  int startY = (height() - size * cellSize) / 2;

  // Find maximum value for normalization
  double maxValue = 0.0;
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      maxValue = qMax(maxValue, std::abs(densityMatrix_[i][j]));
    }
  }

  // Draw matrix cells
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      int x = startX + j * cellSize;
      int y = startY + i * cellSize;
      QRect cellRect(x, y, cellSize, cellSize);

      // Color based on magnitude
      double magnitude = std::abs(densityMatrix_[i][j]);
      QColor color = getMatrixColor(magnitude, maxValue);

      painter.setBrush(QBrush(color));
      painter.setPen(QPen(Qt::black, 1));
      painter.drawRect(cellRect);

      // Draw value
      QString value = formatComplexNumber(densityMatrix_[i][j]);
      painter.setPen(Qt::black);
      painter.drawText(cellRect, Qt::AlignCenter, value);
    }
  }
}

void DensityMatrixWidget::drawStateVector(QPainter &painter) {
  if (stateVector_.isEmpty())
    return;

  int size = stateVector_.size();
  int barWidth = width() / (size + 1);
  int maxHeight = height() - 50;

  // Find maximum magnitude for normalization
  double maxMagnitude = 0.0;
  for (const auto &amp : stateVector_) {
    maxMagnitude = qMax(maxMagnitude, std::abs(amp));
  }

  // Draw state vector bars
  for (int i = 0; i < size; ++i) {
    int x = (i + 1) * barWidth;
    double magnitude = std::abs(stateVector_[i]);
    int barHeight =
        static_cast<int>((magnitude / (maxMagnitude + 1e-9)) * maxHeight);
    int y = maxHeight - barHeight + 5; // Draw from bottom up

    QRect barRect(x - barWidth / 2, y, barWidth - 2, barHeight);

    // Color based on magnitude
    QColor color = getMatrixColor(magnitude, maxMagnitude);
    painter.setBrush(QBrush(color));
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(barRect);

    // Draw basis label
    QString basis;
    if (!basisLabels_.isEmpty() && i < basisLabels_.size()) {
      basis = basisLabels_[i];
    } else {
      basis = QString("|%1⟩").arg(i, 0, 2);
    }

    painter.setPen(Qt::black);
    painter.drawText(x - barWidth / 2, maxHeight + 10, barWidth, 20,
                     Qt::AlignCenter, basis);
  }
}

void DensityMatrixWidget::drawBasisLabels(QPainter &painter) {
  painter.setPen(Qt::black);
  painter.setFont(QFont("Arial", 8));

  if (showDensityMatrix_ && !densityMatrix_.isEmpty()) {
    int size = densityMatrix_.size();
    int cellSize = qMin(width(), height()) / (size + 2);
    int startX = (width() - size * cellSize) / 2;
    int startY = (height() - size * cellSize) / 2;

    // Column labels
    for (int j = 0; j < size; ++j) {
      int x = startX + j * cellSize;
      QString label = (!basisLabels_.isEmpty() && j < basisLabels_.size())
                          ? basisLabels_[j]
                          : QString("|%1⟩").arg(j);
      painter.drawText(x, startY - 20, cellSize, 20, Qt::AlignCenter, label);
    }

    // Row labels
    for (int i = 0; i < size; ++i) {
      int y = startY + i * cellSize;
      QString label = (!basisLabels_.isEmpty() && i < basisLabels_.size())
                          ? basisLabels_[i]
                          : QString("⟨%1|").arg(i);
      painter.drawText(startX - 30, y, 30, cellSize, Qt::AlignCenter,
                       label); // Simplified <x| label logic
    }
  }
}

QColor DensityMatrixWidget::getMatrixColor(double value, double maxValue) {
  if (maxValue == 0.0)
    return Qt::white;

  double normalized = value / maxValue;
  int intensity = static_cast<int>(normalized * 255);

  // Blue to red gradient
  if (normalized < 0.5) {
    return QColor(0, 0, 255 - intensity * 2);
  } else {
    return QColor(intensity * 2 - 255, 0, 0);
  }
}

QString
DensityMatrixWidget::formatComplexNumber(const std::complex<double> &z) {
  double real = z.real();
  double imag = z.imag();

  if (std::abs(imag) < 1e-10) {
    return QString::number(real, 'f', 3);
  } else if (std::abs(real) < 1e-10) {
    return QString("%1i").arg(imag, 0, 'f', 3);
  } else {
    return QString("%1 %2 %3i")
        .arg(real, 0, 'f', 3)
        .arg(imag >= 0 ? "+" : "-")
        .arg(std::abs(imag), 0, 'f', 3);
  }
}

void DensityMatrixWidget::mousePressEvent(QMouseEvent *event) {
  if (showDensityMatrix_ && !densityMatrix_.isEmpty()) {
    int size = densityMatrix_.size();
    int cellSize = qMin(width(), height()) / (size + 2);
    int startX = (width() - size * cellSize) / 2;
    int startY = (height() - size * cellSize) / 2;

    int x = (event->pos().x() - startX) / cellSize;
    int y = (event->pos().y() - startY) / cellSize;

    if (x >= 0 && x < size && y >= 0 && y < size) {
      selectedRow_ = y;
      selectedCol_ = x;
      update();
    }
  }
}

// Quantum State Viewer Implementation
QuantumStateViewer::QuantumStateViewer(QWidget *parent) : QWidget(parent) {
  setupUI();
  // Animation setup removed - now handled by MainWindow
}

QuantumStateViewer::~QuantumStateViewer() {}

QSize QuantumStateViewer::sizeHint() const {
  // Base size for tables and controls
  int width = 800;
  int height = 600;

  // If we have a large state vector, increase suggested size
  if (stateVector_.size() > 16) {
    height = 800;
  }

  return QSize(width, height);
}

void QuantumStateViewer::setupUI() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Title
  titleLabel_ = new QLabel("Quantum State Analyzer", this);
  titleLabel_->setAlignment(Qt::AlignCenter);
  titleLabel_->setStyleSheet("font-weight: bold; font-size: 14px;");
  mainLayout->addWidget(titleLabel_);

  // View controls
  QHBoxLayout *controlsLayout = new QHBoxLayout();
  controlsLayout->addWidget(new QLabel("View:"));
  viewModeSelector_ = new QComboBox(this);
  viewModeSelector_->addItems({"State Vector", "Density Matrix",
                               "Entanglement Graph", "Hilbert Space"});
  connect(viewModeSelector_, &QComboBox::currentTextChanged, this,
          &QuantumStateViewer::onViewModeChanged);
  controlsLayout->addWidget(viewModeSelector_);

  controlsLayout->addWidget(new QLabel("Basis:"));
  basisSelector_ = new QComboBox(this);
  basisSelector_->addItems({"Computational", "Bell", "GHZ", "W"});
  connect(basisSelector_, &QComboBox::currentTextChanged, this,
          &QuantumStateViewer::onBasisChanged);
  controlsLayout->addWidget(basisSelector_);

  controlsLayout->addStretch();

  animateButton_ = new QPushButton("Animate", this);
  animateButton_->setCheckable(true);
  connect(animateButton_, &QPushButton::clicked, this,
          &QuantumStateViewer::onAnimateClicked);
  controlsLayout->addWidget(animateButton_);

  exportButton_ = new QPushButton("Export", this);
  connect(exportButton_, &QPushButton::clicked, this,
          &QuantumStateViewer::onExportState);
  controlsLayout->addWidget(exportButton_);

  mainLayout->addLayout(controlsLayout);

  // State display group
  stateGroup_ = new QGroupBox("State Representation", this);
  QVBoxLayout *stateOuterLayout = new QVBoxLayout(stateGroup_);

  QHBoxLayout *stateTitleLayout = new QHBoxLayout();
  stateTitleLayout->addStretch();
  stateTitleLayout->addWidget(createInfoIcon(
      "<b>State Representation</b><br/>"
      "Visualizes the current quantum state $|\psi\rangle$. "
      "<ul><li><b>State Vector:</b> Shows complex amplitudes $\alpha_i$ where "
      "$|\psi\rangle = \sum \alpha_i |i\rangle$.</li>"
      "<li><b>Density Matrix:</b> Visualizes $\rho = \sum p_i "
      "|\psi_i\rangle\langle\psi_i|$ for mixed states.</li></ul>"));
  stateOuterLayout->addLayout(stateTitleLayout);

  QHBoxLayout *stateLayout = new QHBoxLayout();
  stateOuterLayout->addLayout(stateLayout);

  stateTextEdit_ = new QTextEdit(this);
  stateTextEdit_->setReadOnly(true);
  stateTextEdit_->setMaximumHeight(150);
  stateLayout->addWidget(stateTextEdit_);

  densityMatrixWidget_ = new DensityMatrixWidget(this);
  stateLayout->addWidget(densityMatrixWidget_);

  entanglementGraph_ = new EntanglementGraphWidget(this);
  entanglementGraph_->setVisible(false);
  stateLayout->addWidget(entanglementGraph_);

  hilbertSpace_ = new HilbertSpaceWidget(this);
  hilbertSpace_->setVisible(false);
  stateLayout->addWidget(hilbertSpace_);

  mainLayout->addWidget(stateGroup_);

  // Properties group
  propertiesGroup_ = new QGroupBox("State Properties", this);
  QVBoxLayout *propsLayout = new QVBoxLayout(propertiesGroup_);

  QHBoxLayout *propsTitleLayout = new QHBoxLayout();
  propsTitleLayout->addStretch();
  propsTitleLayout->addWidget(createInfoIcon(
      "<b>State Properties</b><br/>"
      "Calculates physical metrics of the state:<br/>"
      "• <b>Purity</b>: Measures how much the state is 'pure' vs 'mixed'.<br/>"
      "• <b>Fidelity</b>: Comparison with a target state.<br/>"
      "• <b>Coherence</b>: Measures the 'quantumness' of the superposition."));
  propsLayout->addLayout(propsTitleLayout);

  propertiesTable_ = new QTableWidget(this);
  propertiesTable_->setColumnCount(2);
  propertiesTable_->setHorizontalHeaderLabels({"Property", "Value"});
  propertiesTable_->horizontalHeader()->setStretchLastSection(true);
  propsLayout->addWidget(propertiesTable_);

  // Property labels
  QHBoxLayout *propLabelsLayout = new QHBoxLayout();
  purityLabel_ = new QLabel("Purity: 1.000", this);
  fidelityLabel_ = new QLabel("Fidelity: 1.000", this);
  vonNeumannLabel_ = new QLabel("S: 0.000", this);
  propLabelsLayout->addWidget(purityLabel_);
  propLabelsLayout->addWidget(fidelityLabel_);
  propLabelsLayout->addWidget(vonNeumannLabel_);
  propLabelsLayout->addStretch();

  coherenceBar_ = new QProgressBar(this);
  coherenceBar_->setRange(0, 100);
  coherenceBar_->setValue(100);
  coherenceBar_->setFormat("Coherence: %p%");
  propLabelsLayout->addWidget(coherenceBar_);

  propsLayout->addLayout(propLabelsLayout);
  mainLayout->addWidget(propertiesGroup_);

  // Entanglement group
  entanglementGroup_ = new QGroupBox("Entanglement Analysis", this);
  QVBoxLayout *entLayout = new QVBoxLayout(entanglementGroup_);

  QHBoxLayout *entTitleLayout = new QHBoxLayout();
  entTitleLayout->addStretch();
  entTitleLayout->addWidget(createInfoIcon(
      "<b>Entanglement Analysis</b><br/>"
      "Quantifies correlations between qubits. 3D Graph shows strength of "
      "concurrence/entanglement entropy for multi-qubit systems."));
  entLayout->addLayout(entTitleLayout);

  QHBoxLayout *entControlsLayout = new QHBoxLayout();
  entanglementButton_ = new QPushButton("Calculate Entanglement", this);
  connect(entanglementButton_, &QPushButton::clicked, this,
          &QuantumStateViewer::onCalculateEntanglement);
  entControlsLayout->addWidget(entanglementButton_);
  entControlsLayout->addStretch();

  entanglementBar_ = new QProgressBar(this);
  entanglementBar_->setRange(0, 100);
  entanglementBar_->setValue(0);
  entanglementBar_->setFormat("Entanglement: %p%");
  entControlsLayout->addWidget(entanglementBar_);

  entLayout->addLayout(entControlsLayout);

  entanglementTable_ = new QTableWidget(this);
  entanglementTable_->setColumnCount(3);
  entanglementTable_->setHorizontalHeaderLabels(
      {"Measure", "Value", "Description"});
  entanglementTable_->horizontalHeader()->setStretchLastSection(true);
  entLayout->addWidget(entanglementTable_);

  mainLayout->addWidget(entanglementGroup_);

  // Tomography group
  tomographyGroup_ = new QGroupBox("State Tomography", this);
  QVBoxLayout *tomLayout = new QVBoxLayout(tomographyGroup_);

  QHBoxLayout *tomTitleLayout = new QHBoxLayout();
  tomTitleLayout->addStretch();
  tomTitleLayout->addWidget(createInfoIcon(
      "<b>Quantum State Tomography</b><br/>"
      "The process of reconstructing the full quantum state by performing "
      "multiple measurements in different bases (X, Y, Z)."));
  tomLayout->addLayout(tomTitleLayout);

  QHBoxLayout *tomControlsLayout = new QHBoxLayout();
  tomographyButton_ = new QPushButton("Perform Tomography", this);
  connect(tomographyButton_, &QPushButton::clicked, this,
          &QuantumStateViewer::onPerformTomography);
  tomControlsLayout->addWidget(tomographyButton_);
  tomControlsLayout->addStretch();

  tomLayout->addLayout(tomControlsLayout);

  tomographyTable_ = new QTableWidget(this);
  tomographyTable_->setColumnCount(4);
  tomographyTable_->setHorizontalHeaderLabels(
      {"Measurement", "Expected", "Observed", "Error"});
  tomographyTable_->horizontalHeader()->setStretchLastSection(true);
  tomLayout->addWidget(tomographyTable_);

  mainLayout->addWidget(tomographyGroup_);

  setLayout(mainLayout);
}

void QuantumStateViewer::updateState(const QString &stateString) {
  currentStateString_ = stateString;
  stateTextEdit_->setText(stateString);
  calculateStateProperties();
}

void QuantumStateViewer::updateStateVector(
    const QVector<std::complex<double>> &stateVector) {
  stateVector_ = stateVector;

  // Apply current basis view
  onBasisChanged(basisSelector_->currentText());

  calculateStateProperties();
}

void QuantumStateViewer::updateDensityMatrix(
    const QVector<QVector<std::complex<double>>> &densityMatrix) {
  densityMatrix_ = densityMatrix;
  densityMatrixWidget_->setDensityMatrix(densityMatrix);
  calculateStateProperties();
}

void QuantumStateViewer::updateEntanglementMeasures(
    const QMap<QString, double> &measures) {
  entanglementMeasures_ = measures;
  updateEntanglementTable();
}

void QuantumStateViewer::updateTomographyData(
    const QVector<double> &tomographyData) {
  tomographyData_ = tomographyData;
  updateTomographyTable();
}

void QuantumStateViewer::setViewMode(const QString &mode) {
  viewModeSelector_->setCurrentText(mode);
}

void QuantumStateViewer::onViewModeChanged(const QString &mode) {
  if (mode == "State Vector") {
    entanglementGraph_->setVisible(false);
    hilbertSpace_->setVisible(false);
  } else if (mode == "Density Matrix") {
    entanglementGraph_->setVisible(false);
    hilbertSpace_->setVisible(false);
  } else if (mode == "Entanglement Graph") {
    densityMatrixWidget_->setVisible(false);
    entanglementGraph_->setVisible(true);
    hilbertSpace_->setVisible(false);
    onCalculateEntanglement();
  } else if (mode == "Hilbert Space") {
    densityMatrixWidget_->setVisible(false);
    entanglementGraph_->setVisible(false);
    hilbertSpace_->setVisible(true);
    hilbertSpace_->setStateVector(
        stateVector_); // Fallback, onBasisChanged will overwrite
  }

  // Refresh with current basis
  onBasisChanged(basisSelector_->currentText());
}

void QuantumStateViewer::onBasisChanged(const QString &basis) {
  QVector<std::complex<double>> transformedState = getStateInBasis(basis);
  QVector<QVector<std::complex<double>>> transformedDensityMatrix;

  // Set labels
  QStringList labels;
  if (basis == "Bell" &&
      (stateVector_.size() == 4 || densityMatrix_.size() == 4)) {
    labels = {"|Φ⁺⟩", "|Ψ⁺⟩", "|Φ⁻⟩", "|Ψ⁻⟩"}; // Matches our transform order
  } else if (basis == "GHZ" &&
             (stateVector_.size() == 8 || densityMatrix_.size() == 8)) {
    labels = {"|GHZ₀⟩", "|1⟩", "|2⟩", "|3⟩", "|4⟩", "|5⟩", "|6⟩", "|GHZ₁⟩"};
  } else if (basis == "W" &&
             (stateVector_.size() == 8 || densityMatrix_.size() == 8)) {
    labels = {"|000⟩", "|W⟩", "|W⟂₁⟩", "|W⟂₂⟩",
              "|111⟩", "|W̅⟩", "|W̅⟂₁⟩", "|W̅⟂₂⟩"};
  } else if (basis == "Computational") {
    // Default to empty (widgets handle |i>) or generate explicit
    // labels.clear();
  }

  if (viewModeSelector_->currentText() == "Density Matrix") {
    transformedDensityMatrix = getDensityMatrixInBasis(basis);
    densityMatrixWidget_->setDensityMatrix(transformedDensityMatrix);
    densityMatrixWidget_->setBasisLabels(labels);
    densityMatrixWidget_->setVisible(true);
  } else if (viewModeSelector_->currentText() == "State Vector") {
    densityMatrixWidget_->setStateVector(transformedState);
    densityMatrixWidget_->setBasisLabels(labels);
    densityMatrixWidget_->setVisible(true);
    updateStateDisplay();
  } else if (viewModeSelector_->currentText() == "Hilbert Space") {
    hilbertSpace_->setStateVector(transformedState);
    // hilbertSpace_->setLabels(labels); // Assuming it might not handle labels
    // yet
    updateStateDisplay();
  }
}

QVector<QVector<std::complex<double>>>
QuantumStateViewer::getDensityMatrixInBasis(const QString &basis) const {
  if (densityMatrix_.isEmpty())
    return densityMatrix_;

  if (basis == "Computational")
    return densityMatrix_;

  int n = densityMatrix_.size();

  if (basis == "Bell" && n == 4) {
    // ... (Existing Bell Logic) ...
    QVector<QVector<std::complex<double>>> rho = densityMatrix_;
    std::swap(rho[2], rho[3]);
    for (int i = 0; i < 4; ++i)
      std::swap(rho[i][2], rho[i][3]);

    QVector<QVector<std::complex<double>>> res(
        4, QVector<std::complex<double>>(4));
    for (int r = 0; r < 2; ++r) {
      for (int c = 0; c < 2; ++c) {
        std::complex<double> A = rho[r][c];
        std::complex<double> B = rho[r][c + 2];
        std::complex<double> C = rho[r + 2][c];
        std::complex<double> D = rho[r + 2][c + 2];

        res[r][c] = 0.5 * (A + B + C + D);
        res[r][c + 2] = 0.5 * (A - B + C - D);
        res[r + 2][c] = 0.5 * (A + B - C - D);
        res[r + 2][c + 2] = 0.5 * (A - B - C + D);
      }
    }
    return res;
  } else if (basis == "GHZ" && n == 8) {
    // GHZ Inverse on Density Matrix: CX(1,2) -> CX(0,1) -> H(0)
    QVector<QVector<std::complex<double>>> rho = densityMatrix_;

    // 1. CX(1,2): Swap 2<->3 (010<->011) and 6<->7 (110<->111)
    std::swap(rho[2], rho[3]);
    std::swap(rho[6], rho[7]);    // Rows
    for (int i = 0; i < 8; ++i) { // Cols
      std::swap(rho[i][2], rho[i][3]);
      std::swap(rho[i][6], rho[i][7]);
    }

    // 2. CX(0,1): Swap 4<->6 (100<->110) and 5<->7 (101<->111)
    std::swap(rho[4], rho[6]);
    std::swap(rho[5], rho[7]);    // Rows
    for (int i = 0; i < 8; ++i) { // Cols
      std::swap(rho[i][4], rho[i][6]);
      std::swap(rho[i][5], rho[i][7]);
    }

    // 3. H(0): Block Hadamard on MSB (indices 0-3 vs 4-7)
    QVector<QVector<std::complex<double>>> res(
        8, QVector<std::complex<double>>(8));
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        std::complex<double> A = rho[r][c];
        std::complex<double> B = rho[r][c + 4];
        std::complex<double> C = rho[r + 4][c];
        std::complex<double> D = rho[r + 4][c + 4];

        res[r][c] = 0.5 * (A + B + C + D);
        res[r][c + 4] = 0.5 * (A - B + C - D);
        res[r + 4][c] = 0.5 * (A + B - C - D);
        res[r + 4][c + 4] = 0.5 * (A - B - C + D);
      }
    }
    return res;
  }

  return densityMatrix_;
}

QVector<std::complex<double>>
QuantumStateViewer::getStateInBasis(const QString &basis) const {
  if (stateVector_.isEmpty())
    return stateVector_;

  int n = stateVector_.size();
  int numQubits = static_cast<int>(std::log2(n));
  QVector<std::complex<double>> result = stateVector_;

  if (basis == "Computational") {
    return result;
  } else if (basis == "Bell" && numQubits == 2) {
    // ... (Bell logic existing) ...
    // Bell Basis Transformation: Inverse of (H(0) -> CNOT(0,1))
    QVector<std::complex<double>> temp = result;
    std::swap(temp[2], temp[3]);
    result[0] = (temp[0] + temp[2]) / std::sqrt(2.0);
    result[1] = (temp[1] + temp[3]) / std::sqrt(2.0);
    result[2] = (temp[0] - temp[2]) / std::sqrt(2.0);
    result[3] = (temp[1] - temp[3]) / std::sqrt(2.0);
    return result;
  } else if (basis == "GHZ" && numQubits == 3) {
    // GHZ Inverse: CX(1,2) -> CX(0,1) -> H(0)
    QVector<std::complex<double>> temp = result;

    // 1. CX(1,2) (q1 ctrl, q2 target) -> Swap (010 2, 011 3) and (110 6, 111 7)
    std::swap(temp[2], temp[3]);
    std::swap(temp[6], temp[7]);

    // 2. CX(0,1) (q0 ctrl, q1 target) -> Swap (100 4, 110 6) and (101 5, 111 7)
    std::swap(temp[4], temp[6]);
    std::swap(temp[5], temp[7]);

    // 3. H(0)
    QVector<std::complex<double>> res = temp;
    double invSqrt2 = 1.0 / std::sqrt(2.0);
    for (int i = 0; i < 4; ++i) {
      res[i] = (temp[i] + temp[i + 4]) * invSqrt2;
      res[i + 4] = (temp[i] - temp[i + 4]) * invSqrt2;
    }
    return res;
  } else if (basis == "W" && numQubits == 3) {
    // 3-Qubit W-Basis Transformation
    // We project the state onto an orthonormal basis set containing the W
    // state. Basis Definition: 0: |000⟩ 1: |W⟩       = (|001⟩ + |010⟩ + |100⟩)
    // / √3 2: |W⟂₁⟩     = (|010⟩ - |001⟩) / √2 3: |W⟂₂⟩     = (|001⟩ + |010⟩ -
    // 2|100⟩) / √6 4: |111⟩ 5: |W̅⟩       = (|011⟩ + |101⟩ + |110⟩) / √3 6:
    // |W̅⟂₁⟩     = (|101⟩ - |011⟩) / √2 7: |W̅⟂₂⟩     = (|011⟩ + |101⟩ - 2|110⟩)
    // / √6

    QVector<std::complex<double>> res(8, 0.0);
    const double r2 = 1.0 / std::sqrt(2.0);
    const double r3 = 1.0 / std::sqrt(3.0);
    const double r6 = 1.0 / std::sqrt(6.0);

    // Map indices to amplitudes for clarity
    auto &s = stateVector_; // alias for calculation (const)

    // |000⟩ part
    res[0] = s[0];

    // 1-excitation subspace (indices 1, 2, 4) -> (001, 010, 100)
    std::complex<double> c001 = s[1];
    std::complex<double> c010 = s[2];
    std::complex<double> c100 = s[4];

    res[1] = r3 * (c001 + c010 + c100);       // |W⟩
    res[2] = r2 * (c010 - c001);              // |W⟂₁⟩
    res[3] = r6 * (c001 + c010 - 2.0 * c100); // |W⟂₂⟩

    // |111⟩ part
    res[4] = s[7];

    // 2-excitation subspace (indices 3, 5, 6) -> (011, 101, 110)
    std::complex<double> c011 = s[3];
    std::complex<double> c101 = s[5];
    std::complex<double> c110 = s[6];

    res[5] = r3 * (c011 + c101 + c110);       // |W̅⟩
    res[6] = r2 * (c101 - c011);              // |W̅⟂₁⟩
    res[7] = r6 * (c011 + c101 - 2.0 * c110); // |W̅⟂₂⟩

    return res;
  }

  return result;
}

void QuantumStateViewer::onCalculateEntanglement() {
  // Simulate entanglement calculation
  QMap<QString, double> measures;
  measures["Concurrence"] = 0.5;
  measures["Negativity"] = 0.3;
  measures["Von Neumann"] = 0.7;
  measures["Tangle"] = 0.25;

  updateEntanglementMeasures(measures);
  entanglementBar_->setValue(static_cast<int>(measures["Concurrence"] * 100));

  // Update 3D Graph
  int numQubits = 0;
  if (!stateVector_.isEmpty()) {
    numQubits = static_cast<int>(std::log2(stateVector_.size()));
  } else if (!densityMatrix_.isEmpty()) {
    numQubits = static_cast<int>(std::log2(densityMatrix_.size()));
  }

  entanglementGraph_->setNumQubits(numQubits);

  // Create some dummy links for visualization based on the "calculated"
  // measures
  QVector<EntanglementLink> links;
  if (numQubits >= 2) {
    // Link qubit 0 and 1 strongly
    links.append({0, 1, measures["Concurrence"]});
    // Link others weakly if they exist
    for (int i = 1; i < numQubits - 1; ++i) {
      links.append({i, i + 1, 0.1});
    }
  }
  entanglementGraph_->setEntanglementData(links);
}

void QuantumStateViewer::onPerformTomography() {
  // Simulate tomography data
  QVector<double> tomographyData = {0.5, 0.5, 0.7, 0.3, 0.6, 0.4};
  updateTomographyData(tomographyData);
}

void QuantumStateViewer::onExportState() {
  QString fileName = QFileDialog::getSaveFileName(
      this, "Export Quantum State", "", "Text Files (*.txt);;All Files (*)");
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << "Quantum State Export\n";
      out << "===================\n\n";
      out << "State Vector:\n";
      out << currentStateString_ << "\n\n";
      out << "Properties:\n";
      out << "Purity: " << purityLabel_->text() << "\n";
      out << "Fidelity: " << fidelityLabel_->text() << "\n";
      out << "Von Neumann: " << vonNeumannLabel_->text() << "\n";
      file.close();
      QMessageBox::information(this, "Export", "State exported successfully!");
    }
  }
}

void QuantumStateViewer::onAnimateClicked() {
  bool active = animateButton_->isChecked();
  if (active) {
    animateButton_->setText("Stop Animation");
  } else {
    animateButton_->setText("Animate");
  }
  emit animateToggled(active);
}

void QuantumStateViewer::updateStateDisplay() {
  QString basis = basisSelector_->currentText();
  QVector<std::complex<double>> displayVector = getStateInBasis(basis);

  QString displayText = "State Vector (" + basis + " Basis):\n";
  for (int i = 0; i < displayVector.size(); ++i) {
    displayText += QString("|%1⟩: %2\n")
                       .arg(i, 0, 2)
                       .arg(formatComplexNumber(displayVector[i]));
  }
  stateTextEdit_->setText(displayText);
}

void QuantumStateViewer::updateEntanglementTable() {
  entanglementTable_->setRowCount(0);

  for (auto it = entanglementMeasures_.begin();
       it != entanglementMeasures_.end(); ++it) {
    int row = entanglementTable_->rowCount();
    entanglementTable_->insertRow(row);

    entanglementTable_->setItem(row, 0, new QTableWidgetItem(it.key()));
    entanglementTable_->setItem(
        row, 1, new QTableWidgetItem(QString::number(it.value(), 'f', 4)));

    QString description;
    if (it.key() == "Concurrence")
      description = "Two-qubit entanglement measure";
    else if (it.key() == "Negativity")
      description = "Entanglement monotone";
    else if (it.key() == "Von Neumann")
      description = "Entropy of reduced state";
    else if (it.key() == "Tangle")
      description = "Three-tangle measure";

    entanglementTable_->setItem(row, 2, new QTableWidgetItem(description));
  }
}

void QuantumStateViewer::updateTomographyTable() {
  tomographyTable_->setRowCount(0);

  QStringList measurements = {"⟨X⟩", "⟨Y⟩", "⟨Z⟩", "⟨XX⟩", "⟨YY⟩", "⟨ZZ⟩"};
  QVector<double> expected = {0.0, 0.0, 0.5, 0.25, 0.25, 0.5};

  for (int i = 0; i < tomographyData_.size() && i < measurements.size(); ++i) {
    int row = tomographyTable_->rowCount();
    tomographyTable_->insertRow(row);

    tomographyTable_->setItem(row, 0, new QTableWidgetItem(measurements[i]));
    tomographyTable_->setItem(
        row, 1, new QTableWidgetItem(QString::number(expected[i], 'f', 4)));
    tomographyTable_->setItem(
        row, 2,
        new QTableWidgetItem(QString::number(tomographyData_[i], 'f', 4)));

    double error = std::abs(tomographyData_[i] - expected[i]);
    tomographyTable_->setItem(
        row, 3, new QTableWidgetItem(QString::number(error, 'f', 4)));
  }
}

void QuantumStateViewer::calculateStateProperties() {
  if (stateVector_.isEmpty())
    return;

  // Calculate purity
  double purity = 0.0;
  for (const auto &amp : stateVector_) {
    purity += std::norm(amp);
  }
  purityLabel_->setText(QString("Purity: %1").arg(purity, 0, 'f', 3));

  // Calculate fidelity (assuming target is |0⟩)
  double fidelity = std::norm(stateVector_[0]);
  fidelityLabel_->setText(QString("Fidelity: %1").arg(fidelity, 0, 'f', 3));

  // Calculate von Neumann entropy
  double entropy = 0.0;
  for (const auto &amp : stateVector_) {
    double prob = std::norm(amp);
    if (prob > 0) {
      entropy -= prob * std::log2(prob);
    }
  }
  vonNeumannLabel_->setText(QString("S: %1").arg(entropy, 0, 'f', 3));

  // Calculate coherence
  double coherence = 0.0;
  for (int i = 0; i < stateVector_.size(); ++i) {
    for (int j = i + 1; j < stateVector_.size(); ++j) {
      coherence += std::abs(stateVector_[i] * std::conj(stateVector_[j]));
    }
  }
  coherenceBar_->setValue(static_cast<int>(coherence * 100));

  // Update properties table
  propertiesTable_->setRowCount(0);
  QStringList properties = {"Purity", "Fidelity", "Von Neumann Entropy",
                            "Coherence"};
  QVector<double> values = {purity, fidelity, entropy, coherence};

  for (int i = 0; i < properties.size(); ++i) {
    int row = propertiesTable_->rowCount();
    propertiesTable_->insertRow(row);

    propertiesTable_->setItem(row, 0, new QTableWidgetItem(properties[i]));
    propertiesTable_->setItem(
        row, 1, new QTableWidgetItem(QString::number(values[i], 'f', 4)));
  }
}

QString QuantumStateViewer::formatComplexNumber(const std::complex<double> &z) {
  double real = z.real();
  double imag = z.imag();

  if (std::abs(imag) < 1e-10) {
    return QString::number(real, 'f', 3);
  } else if (std::abs(real) < 1e-10) {
    return QString("%1i").arg(imag, 0, 'f', 3);
  } else {
    return QString("%1 %2 %3i")
        .arg(real, 0, 'f', 3)
        .arg(imag >= 0 ? "+" : "-")
        .arg(std::abs(imag), 0, 'f', 3);
  }
}

QLabel *QuantumStateViewer::createInfoIcon(const QString &tooltip) {
  ClickableLabel *infoIcon = new ClickableLabel("ⓘ", tooltip, this);
  return infoIcon;
}
