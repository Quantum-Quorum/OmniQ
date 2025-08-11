//
// Created by Goutham Arcot on 17/07/25.
//

#include "QubitViewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QProgressBar>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QApplication>
#include <QStyle>
#include <cmath>
#include <complex>

// Bloch Sphere Widget Implementation
BlochSphereWidget::BlochSphereWidget(QWidget *parent)
    : QWidget(parent)
    , theta_(0.0)
    , phi_(0.0)
    , alpha_(1.0)
    , beta_(0.0)
    , isDragging_(false)
{
    setMinimumSize(200, 200);
    setMouseTracking(true);
}

void BlochSphereWidget::setQubitState(double theta, double phi, double alpha, double beta)
{
    theta_ = theta;
    phi_ = phi;
    alpha_ = alpha;
    beta_ = beta;
    update();
}

void BlochSphereWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Calculate center and radius
    center_ = rect().center();
    radius_ = qMin(width(), height()) / 2 - 20;
    
    // Draw Bloch sphere
    drawBlochSphere(painter);
    drawAxes(painter);
    drawQubitState(painter);
    drawMeasurementBasis(painter);
}

void BlochSphereWidget::drawBlochSphere(QPainter &painter)
{
    // Draw sphere outline
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center_, radius_, radius_);
    
    // Draw grid lines
    painter.setPen(QPen(Qt::lightGray, 1));
    for (int i = 0; i < 8; ++i) {
        double angle = i * M_PI / 4;
        QPoint p1(center_.x() + radius_ * cos(angle), center_.y() + radius_ * sin(angle));
        QPoint p2(center_.x() - radius_ * cos(angle), center_.y() - radius_ * sin(angle));
        painter.drawLine(p1, p2);
    }
}

void BlochSphereWidget::drawAxes(QPainter &painter)
{
    painter.setPen(QPen(Qt::red, 2));
    // X-axis
    painter.drawLine(center_.x() - radius_, center_.y(), center_.x() + radius_, center_.y());
    painter.drawText(center_.x() + radius_ + 5, center_.y(), "X");
    
    painter.setPen(QPen(Qt::green, 2));
    // Y-axis
    painter.drawLine(center_.x(), center_.y() - radius_, center_.x(), center_.y() + radius_);
    painter.drawText(center_.x(), center_.y() - radius_ - 5, "Y");
    
    painter.setPen(QPen(Qt::blue, 2));
    // Z-axis (vertical)
    painter.drawLine(center_.x(), center_.y() - radius_, center_.x(), center_.y() + radius_);
    painter.drawText(center_.x() - 15, center_.y() - radius_ - 5, "Z");
}

void BlochSphereWidget::drawQubitState(QPainter &painter)
{
    // Calculate qubit position on Bloch sphere
    double x = sin(theta_) * cos(phi_);
    double y = sin(theta_) * sin(phi_);
    double z = cos(theta_);
    
    QPoint qubitPos(center_.x() + radius_ * x, center_.y() - radius_ * z);
    
    // Draw qubit state
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::yellow));
    painter.drawEllipse(qubitPos, 8, 8);
    
    // Draw state vector
    painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
    painter.drawLine(center_, qubitPos);
    
    // Draw state label
    QString stateText = QString("|ψ⟩ = %1|0⟩ + %2|1⟩")
                       .arg(alpha_, 0, 'f', 3)
                       .arg(beta_, 0, 'f', 3);
    painter.drawText(qubitPos.x() + 10, qubitPos.y(), stateText);
}

void BlochSphereWidget::drawMeasurementBasis(QPainter &painter)
{
    // Draw measurement basis indicators
    painter.setPen(QPen(Qt::magenta, 1, Qt::DotLine));
    
    // Computational basis (Z-axis)
    painter.drawEllipse(center_.x() - 5, center_.y() - radius_ - 5, 10, 10);
    painter.drawEllipse(center_.x() - 5, center_.y() + radius_ - 5, 10, 10);
    
    painter.drawText(center_.x() - 10, center_.y() - radius_ - 10, "|0⟩");
    painter.drawText(center_.x() - 10, center_.y() + radius_ + 15, "|1⟩");
}

void BlochSphereWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging_ = true;
        lastMousePos_ = event->pos();
    }
}

void BlochSphereWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging_) {
        QPoint delta = event->pos() - lastMousePos_;
        
        // Update angles based on mouse movement
        phi_ += delta.x() * 0.01;
        theta_ += delta.y() * 0.01;
        
        // Clamp theta to [0, π]
        theta_ = qBound(0.0, theta_, M_PI);
        
        // Update amplitudes
        alpha_ = cos(theta_ / 2);
        beta_ = sin(theta_ / 2) * std::complex<double>(cos(phi_), sin(phi_)).real();
        
        lastMousePos_ = event->pos();
        update();
    }
}

// QubitViewer Implementation
QubitViewer::QubitViewer(QWidget *parent)
    : QWidget(parent)
    , currentQubit_(0)
    , animationPhase_(0.0)
{
    setupUI();
    
    // Setup animation timer
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &QubitViewer::onAnimateState);
    animationTimer_->start(50); // 20 FPS
}

QubitViewer::~QubitViewer()
{
}

void QubitViewer::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    titleLabel_ = new QLabel("Qubit Debugger", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(titleLabel_);
    
    // Qubit selector
    QHBoxLayout *selectorLayout = new QHBoxLayout();
    selectorLayout->addWidget(new QLabel("Qubit:"));
    qubitSelector_ = new QComboBox(this);
    qubitSelector_->addItems({"0", "1", "2", "3", "4", "5", "6", "7"});
    connect(qubitSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QubitViewer::onQubitSelected);
    selectorLayout->addWidget(qubitSelector_);
    selectorLayout->addStretch();
    mainLayout->addLayout(selectorLayout);
    
    // Bloch sphere group
    blochGroup_ = new QGroupBox("Bloch Sphere Visualization", this);
    QVBoxLayout *blochLayout = new QVBoxLayout(blochGroup_);
    blochSphere_ = new BlochSphereWidget(this);
    blochLayout->addWidget(blochSphere_);
    mainLayout->addWidget(blochGroup_);
    
    // Measurement group
    measurementGroup_ = new QGroupBox("Measurement & Statistics", this);
    QVBoxLayout *measureLayout = new QVBoxLayout(measurementGroup_);
    
    // Measurement controls
    QHBoxLayout *measureControlsLayout = new QHBoxLayout();
    measureControlsLayout->addWidget(new QLabel("Basis:"));
    measurementBasis_ = new QComboBox(this);
    measurementBasis_->addItems({"Computational (Z)", "X-basis", "Y-basis", "Custom"});
    connect(measurementBasis_, &QComboBox::currentTextChanged,
            this, &QubitViewer::onMeasurementBasisChanged);
    measureControlsLayout->addWidget(measurementBasis_);
    
    measureControlsLayout->addWidget(new QLabel("Count:"));
    measurementCount_ = new QSpinBox(this);
    measurementCount_->setRange(1, 10000);
    measurementCount_->setValue(1000);
    measureControlsLayout->addWidget(measurementCount_);
    
    measureButton_ = new QPushButton("Measure", this);
    connect(measureButton_, &QPushButton::clicked, this, &QubitViewer::onMeasureButtonClicked);
    measureControlsLayout->addWidget(measureButton_);
    
    resetButton_ = new QPushButton("Reset", this);
    connect(resetButton_, &QPushButton::clicked, this, &QubitViewer::onResetButtonClicked);
    measureControlsLayout->addWidget(resetButton_);
    
    measureLayout->addLayout(measureControlsLayout);
    
    // Measurement results table
    measurementTable_ = new QTableWidget(this);
    measurementTable_->setColumnCount(4);
    measurementTable_->setHorizontalHeaderLabels({"Basis", "|0⟩ Count", "|1⟩ Count", "Probability"});
    measurementTable_->horizontalHeader()->setStretchLastSection(true);
    measureLayout->addWidget(measurementTable_);
    
    mainLayout->addWidget(measurementGroup_);
    
    // Statistics group
    statisticsGroup_ = new QGroupBox("Qubit Properties", this);
    QVBoxLayout *statsLayout = new QVBoxLayout(statisticsGroup_);
    
    // Basic qubit table
    qubitTable_ = new QTableWidget(this);
    qubitTable_->setColumnCount(4);
    qubitTable_->setHorizontalHeaderLabels({"Qubit", "P(|0⟩)", "P(|1⟩)", "Phase"});
    qubitTable_->horizontalHeader()->setStretchLastSection(true);
    statsLayout->addWidget(qubitTable_);
    
    // Expectation values table
    expectationTable_ = new QTableWidget(this);
    expectationTable_->setColumnCount(4);
    expectationTable_->setHorizontalHeaderLabels({"Qubit", "⟨X⟩", "⟨Y⟩", "⟨Z⟩"});
    expectationTable_->horizontalHeader()->setStretchLastSection(true);
    statsLayout->addWidget(expectationTable_);
    
    // Additional properties
    QHBoxLayout *propsLayout = new QHBoxLayout();
    purityLabel_ = new QLabel("Purity: 1.000", this);
    entanglementLabel_ = new QLabel("Entanglement: None", this);
    propsLayout->addWidget(purityLabel_);
    propsLayout->addWidget(entanglementLabel_);
    propsLayout->addStretch();
    
    coherenceBar_ = new QProgressBar(this);
    coherenceBar_->setRange(0, 100);
    coherenceBar_->setValue(100);
    coherenceBar_->setFormat("Coherence: %p%");
    propsLayout->addWidget(coherenceBar_);
    
    statsLayout->addLayout(propsLayout);
    mainLayout->addWidget(statisticsGroup_);
    
    setLayout(mainLayout);
}

void QubitViewer::updateQubitInfo(int qubit, double prob0, double prob1, 
                                 double alpha, double beta, double theta, double phi)
{
    // Store qubit state
    QVector<double> state = {prob0, prob1, alpha, beta, theta, phi};
    qubitStates_[qubit] = state;
    
    // Update table
    if (qubit >= qubitTable_->rowCount()) {
        qubitTable_->setRowCount(qubit + 1);
    }
    
    qubitTable_->setItem(qubit, 0, new QTableWidgetItem(QString::number(qubit)));
    qubitTable_->setItem(qubit, 1, new QTableWidgetItem(QString::number(prob0, 'f', 4)));
    qubitTable_->setItem(qubit, 2, new QTableWidgetItem(QString::number(prob1, 'f', 4)));
    qubitTable_->setItem(qubit, 3, new QTableWidgetItem(QString::number(phi, 'f', 3)));
    
    // Update Bloch sphere if this is the selected qubit
    if (qubit == currentQubit_) {
        updateBlochSphere();
    }
    
    calculateQubitProperties();
}

void QubitViewer::updateMeasurementResults(int qubit, const QVector<int> &results)
{
    measurementResults_[qubit] = results;
    updateMeasurementStats();
}

void QubitViewer::updateExpectationValues(int qubit, double x, double y, double z)
{
    QVector<double> expectations = {x, y, z};
    expectationValues_[qubit] = expectations;
    updateExpectationTable();
}

void QubitViewer::resetQubitData()
{
    qubitStates_.clear();
    measurementResults_.clear();
    expectationValues_.clear();
    
    qubitTable_->setRowCount(0);
    measurementTable_->setRowCount(0);
    expectationTable_->setRowCount(0);
    
    blochSphere_->setQubitState(0.0, 0.0, 1.0, 0.0);
}

void QubitViewer::onQubitSelected(int qubit)
{
    currentQubit_ = qubit;
    updateBlochSphere();
}

void QubitViewer::onMeasurementBasisChanged(const QString &basis)
{
    // Update measurement basis visualization
    updateBlochSphere();
}

void QubitViewer::onMeasureButtonClicked()
{
    int qubit = currentQubit_;
    int count = measurementCount_->value();
    QString basis = measurementBasis_->currentText();
    
    // Simulate measurements
    QVector<int> results;
    if (qubitStates_.contains(qubit)) {
        double prob0 = qubitStates_[qubit][0];
        for (int i = 0; i < count; ++i) {
            double rand_val = static_cast<double>(std::rand()) / RAND_MAX;
            results.append(rand_val < prob0 ? 0 : 1);
        }
    }
    
    updateMeasurementResults(qubit, results);
}

void QubitViewer::onResetButtonClicked()
{
    resetQubitData();
}

void QubitViewer::onAnimateState()
{
    animationPhase_ += 0.1;
    if (qubitStates_.contains(currentQubit_)) {
        const QVector<double> &state = qubitStates_[currentQubit_];
        double theta = state[4];
        double phi = state[5] + 0.01 * sin(animationPhase_);
        
        blochSphere_->setQubitState(theta, phi, state[2], state[3]);
    }
}

void QubitViewer::updateBlochSphere()
{
    if (qubitStates_.contains(currentQubit_)) {
        const QVector<double> &state = qubitStates_[currentQubit_];
        blochSphere_->setQubitState(state[4], state[5], state[2], state[3]);
    }
}

void QubitViewer::updateMeasurementStats()
{
    measurementTable_->setRowCount(0);
    
    for (auto it = measurementResults_.begin(); it != measurementResults_.end(); ++it) {
        int qubit = it.key();
        const QVector<int> &results = it.value();
        
        int count0 = results.count(0);
        int count1 = results.count(1);
        double prob0 = static_cast<double>(count0) / results.size();
        
        int row = measurementTable_->rowCount();
        measurementTable_->insertRow(row);
        
        measurementTable_->setItem(row, 0, new QTableWidgetItem(QString("Qubit %1").arg(qubit)));
        measurementTable_->setItem(row, 1, new QTableWidgetItem(QString::number(count0)));
        measurementTable_->setItem(row, 2, new QTableWidgetItem(QString::number(count1)));
        measurementTable_->setItem(row, 3, new QTableWidgetItem(QString::number(prob0, 'f', 4)));
    }
}

void QubitViewer::updateExpectationTable()
{
    expectationTable_->setRowCount(0);
    
    for (auto it = expectationValues_.begin(); it != expectationValues_.end(); ++it) {
        int qubit = it.key();
        const QVector<double> &expectations = it.value();
        
        int row = expectationTable_->rowCount();
        expectationTable_->insertRow(row);
        
        expectationTable_->setItem(row, 0, new QTableWidgetItem(QString::number(qubit)));
        expectationTable_->setItem(row, 1, new QTableWidgetItem(QString::number(expectations[0], 'f', 4)));
        expectationTable_->setItem(row, 2, new QTableWidgetItem(QString::number(expectations[1], 'f', 4)));
        expectationTable_->setItem(row, 3, new QTableWidgetItem(QString::number(expectations[2], 'f', 4)));
    }
}

void QubitViewer::calculateQubitProperties()
{
    if (qubitStates_.contains(currentQubit_)) {
        const QVector<double> &state = qubitStates_[currentQubit_];
        double prob0 = state[0];
        double prob1 = state[1];
        
        // Calculate purity (for pure states, purity = 1)
        double purity = prob0 * prob0 + prob1 * prob1;
        purityLabel_->setText(QString("Purity: %1").arg(purity, 0, 'f', 3));
        
        // Calculate coherence
        double coherence = 2 * sqrt(prob0 * prob1);
        coherenceBar_->setValue(static_cast<int>(coherence * 100));
        
        // Entanglement (simplified - assumes single qubit)
        entanglementLabel_->setText("Entanglement: None (single qubit)");
    }
}
