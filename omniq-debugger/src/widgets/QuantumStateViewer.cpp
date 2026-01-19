//
// Created by Goutham Arcot on 17/07/25.
//

#include "QuantumStateViewer.h"
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
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
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
    int height = static_cast<int>((magnitude / maxMagnitude) * maxHeight);
    int y = height - height;

    QRect barRect(x - barWidth / 2, y, barWidth - 2, height);

    // Color based on magnitude
    QColor color = getMatrixColor(magnitude, maxMagnitude);
    painter.setBrush(QBrush(color));
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(barRect);

    // Draw basis label
    QString basis = QString("|%1⟩").arg(i, 0, 2);
    painter.setPen(Qt::black);
    painter.drawText(x - barWidth / 2, height + 20, barWidth, 20,
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
      painter.drawText(x, startY - 20, cellSize, 20, Qt::AlignCenter,
                       QString("|%1⟩").arg(j));
    }

    // Row labels
    for (int i = 0; i < size; ++i) {
      int y = startY + i * cellSize;
      painter.drawText(startX - 30, y, 30, cellSize, Qt::AlignCenter,
                       QString("⟨%1|").arg(i));
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
QuantumStateViewer::QuantumStateViewer(QWidget *parent)
    : QWidget(parent), animationPhase_(0.0), isAnimating_(false) {
  setupUI();

  // Setup animation timer
  animationTimer_ = new QTimer(this);
  connect(animationTimer_, &QTimer::timeout, this,
          &QuantumStateViewer::onUpdateAnimation);
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
  connect(animateButton_, &QPushButton::clicked, this,
          &QuantumStateViewer::onAnimateState);
  controlsLayout->addWidget(animateButton_);

  exportButton_ = new QPushButton("Export", this);
  connect(exportButton_, &QPushButton::clicked, this,
          &QuantumStateViewer::onExportState);
  controlsLayout->addWidget(exportButton_);

  mainLayout->addLayout(controlsLayout);

  // State display group
  stateGroup_ = new QGroupBox("State Representation", this);
  QHBoxLayout *stateLayout = new QHBoxLayout(stateGroup_);

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
  densityMatrixWidget_->setStateVector(stateVector);
  hilbertSpace_->setStateVector(stateVector);
  updateStateDisplay();
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

void QuantumStateViewer::onViewModeChanged(const QString &mode) {
  if (mode == "State Vector") {
    densityMatrixWidget_->setVisible(true);
    densityMatrixWidget_->setStateVector(stateVector_);
    entanglementGraph_->setVisible(false);
    hilbertSpace_->setVisible(false);
  } else if (mode == "Density Matrix") {
    densityMatrixWidget_->setVisible(true);
    densityMatrixWidget_->setDensityMatrix(densityMatrix_);
    entanglementGraph_->setVisible(false);
    hilbertSpace_->setVisible(false);
  } else if (mode == "Entanglement Graph") {
    densityMatrixWidget_->setVisible(false);
    entanglementGraph_->setVisible(true);
    hilbertSpace_->setVisible(false);
    // Trigger update to ensure geometry
    onCalculateEntanglement();
  } else if (mode == "Hilbert Space") {
    densityMatrixWidget_->setVisible(false);
    entanglementGraph_->setVisible(false);
    hilbertSpace_->setVisible(true);
    hilbertSpace_->setStateVector(stateVector_);
  }
  updateStateDisplay();
}

void QuantumStateViewer::onBasisChanged(const QString &basis) {
  // TODO: Implement basis transformation
  updateStateDisplay();
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

void QuantumStateViewer::onAnimateState() {
  isAnimating_ = !isAnimating_;
  if (isAnimating_) {
    animationTimer_->start(100);
    animateButton_->setText("Stop Animation");
  } else {
    animationTimer_->stop();
    animateButton_->setText("Animate");
  }
}

void QuantumStateViewer::onUpdateAnimation() {
  animationPhase_ += 0.1;
  // Shift state vector slightly for a "living" visualization effect
  if (!stateVector_.isEmpty() && isAnimating_) {
    QVector<std::complex<double>> animatedState = stateVector_;
    for (int i = 0; i < animatedState.size(); ++i) {
      double phase =
          std::arg(animatedState[i]) + 0.3 * std::sin(animationPhase_);
      double mag = std::abs(animatedState[i]);
      animatedState[i] =
          std::complex<double>(mag * std::cos(phase), mag * std::sin(phase));
    }
    densityMatrixWidget_->setStateVector(animatedState);
    hilbertSpace_->setStateVector(animatedState);
  }
}

void QuantumStateViewer::updateStateDisplay() {
  QString displayText = "State Vector:\n";
  for (int i = 0; i < stateVector_.size(); ++i) {
    displayText += QString("|%1⟩: %2\n")
                       .arg(i, 0, 2)
                       .arg(formatComplexNumber(stateVector_[i]));
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
