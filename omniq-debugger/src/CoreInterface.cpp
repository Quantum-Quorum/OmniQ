//
// Created by Goutham Arcot on 17/07/25.
//

#include "CoreInterface.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <cmath>
#include <random>

using omniq::VectorXcd;

CoreInterface::CoreInterface(QObject *parent)
    : QObject(parent), circuit_(nullptr), currentState_(nullptr),
      currentStep_(0), totalSteps_(0), isExecuting_(false) {}

CoreInterface::~CoreInterface() {}

bool CoreInterface::createCircuit(int numQubits, int numClassicalBits) {
  try {
    circuit_ = std::make_unique<omniq::QuantumCircuit>(numQubits);
    currentState_ = std::make_unique<omniq::QuantumState>(numQubits);

    circuit_->addGate(omniq::GateType::H, 0);
    circuit_->addGate(omniq::GateType::CNOT, 0, 1);
    circuit_->addGate(omniq::GateType::H, 1);

    totalSteps_ = circuit_->getTotalSteps();
    currentStep_ = 0;
    isExecuting_ = false;

    updateQubitStates();

    clearError();
    emit circuitChanged();
    return true;
  } catch (const std::exception &e) {
    lastError_ = QString("Failed to create circuit: %1").arg(e.what());
    emit errorOccurred(lastError_);
    return false;
  }
}

bool CoreInterface::loadCircuit(const QString &filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    lastError_ = QString("Cannot open file: %1").arg(filename);
    emit errorOccurred(lastError_);
    return false;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isNull() || !doc.isObject()) {
    lastError_ = "Invalid JSON circuit file";
    emit errorOccurred(lastError_);
    return false;
  }

  return importCircuit(doc.object());
}

bool CoreInterface::saveCircuit(const QString &filename) {
  QJsonObject circuitData = exportCircuit();
  QJsonDocument doc(circuitData);

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    lastError_ = QString("Cannot write to file: %1").arg(filename);
    emit errorOccurred(lastError_);
    return false;
  }

  file.write(doc.toJson());
  file.close();
  return true;
}

QString CoreInterface::getCircuitQASM() const {
  QString qasm = "OPENQASM 2.0;\n";
  qasm += "include \"qelib1.inc\";\n\n";
  qasm += "qreg q[2];\n";
  qasm += "creg c[2];\n\n";
  qasm += "h q[0];\n";
  qasm += "cx q[0], q[1];\n";
  qasm += "measure q[0] -> c[0];\n";
  qasm += "measure q[1] -> c[1];\n";

  return qasm;
}

bool CoreInterface::executeStep(int step) {
  if (!validateStepIndex(step) || !circuit_ || !currentState_) {
    return false;
  }

  try {
    while (currentStep_ < step && circuit_->executeStep(*currentState_)) {
      currentStep_++;
    }

    updateQubitStates();

    emit executionStepChanged(currentStep_);
    emit stateChanged();
    return true;
  } catch (const std::exception &e) {
    lastError_ =
        QString("Execution failed at step %1: %2").arg(step).arg(e.what());
    emit errorOccurred(lastError_);
    return false;
  }
}

bool CoreInterface::executeToStep(int step) {
  for (int i = currentStep_ + 1; i <= step; ++i) {
    if (!executeStep(i)) {
      return false;
    }
  }
  return true;
}

bool CoreInterface::executeFull() { return executeToStep(totalSteps_); }

void CoreInterface::resetExecution() {
  currentStep_ = 0;
  emit executionStepChanged(currentStep_);
  emit stateChanged();
}

QString CoreInterface::getCurrentStateString() const {
  QString state = "Current Quantum State:\n";
  for (auto it = qubitStates_.begin(); it != qubitStates_.end(); ++it) {
    int qubit = it.key();
    const QVector<double> &qstate = it.value();
    state += QString("Qubit %1: |ψ⟩ = %2|0⟩ + %3|1⟩\n")
                 .arg(qubit)
                 .arg(qstate[2], 0, 'f', 3)
                 .arg(qstate[3], 0, 'f', 3);
  }
  return state;
}

QVector<QComplex> CoreInterface::getCurrentStateVector() const {
  QVector<QComplex> result;

  if (!currentState_) {
    return result;
  }

  VectorXcd stateVector = currentState_->getStateVector();
  for (int i = 0; i < stateVector.size(); ++i) {
    result.append(QComplex::fromStdComplex(stateVector(i)));
  }

  return result;
}

QVector<QVector<QComplex>> CoreInterface::getCurrentDensityMatrix() const {
  QVector<QVector<QComplex>> result;
  // Simulate a 4x4 density matrix
  for (int i = 0; i < 4; ++i) {
    QVector<QComplex> row;
    for (int j = 0; j < 4; ++j) {
      if (i == j) {
        row.append(QComplex(0.25, 0.0));
      } else {
        row.append(QComplex(0.0, 0.0));
      }
    }
    result.append(row);
  }
  return result;
}

double CoreInterface::getQubitProbability(int qubit, int value) const {
  if (!validateQubitIndex(qubit) || !qubitStates_.contains(qubit)) {
    return 0.0;
  }

  const QVector<double> &state = qubitStates_[qubit];
  if (value == 0) {
    return state[0]; // P(|0⟩)
  } else {
    return state[1]; // P(|1⟩)
  }
}

double CoreInterface::getQubitExpectation(int qubit,
                                          const QString &observable) const {
  if (!validateQubitIndex(qubit) || !qubitStates_.contains(qubit)) {
    return 0.0;
  }

  const QVector<double> &state = qubitStates_[qubit];
  if (observable == "Z") {
    return state[0] - state[1]; // ⟨Z⟩ = P(|0⟩) - P(|1⟩)
  } else if (observable == "X") {
    return 2 * sqrt(state[0] * state[1]) * cos(state[5]); // ⟨X⟩
  } else if (observable == "Y") {
    return 2 * sqrt(state[0] * state[1]) * sin(state[5]); // ⟨Y⟩
  }

  return 0.0;
}

// Enhanced qubit debugging methods
QVector<double> CoreInterface::getQubitState(int qubit) const {
  if (qubitStates_.contains(qubit)) {
    return qubitStates_[qubit];
  }
  return QVector<double>();
}

QVector<int> CoreInterface::measureQubit(int qubit, int count,
                                         const QString &basis) const {
  QVector<int> results;
  if (!validateQubitIndex(qubit) || !qubitStates_.contains(qubit)) {
    return results;
  }

  const QVector<double> &state = qubitStates_[qubit];
  double prob0 = state[0];

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 1.0);

  for (int i = 0; i < count; ++i) {
    double rand_val = dis(gen);
    results.append(rand_val < prob0 ? 0 : 1);
  }

  return results;
}

QVector<double> CoreInterface::getQubitExpectationValues(int qubit) const {
  QVector<double> expectations;
  expectations.append(getQubitExpectation(qubit, "X"));
  expectations.append(getQubitExpectation(qubit, "Y"));
  expectations.append(getQubitExpectation(qubit, "Z"));
  return expectations;
}

double CoreInterface::getQubitPurity(int qubit) const {
  if (!validateQubitIndex(qubit) || !qubitStates_.contains(qubit)) {
    return 0.0;
  }

  const QVector<double> &state = qubitStates_[qubit];
  return state[0] * state[0] + state[1] * state[1];
}

double CoreInterface::getQubitCoherence(int qubit) const {
  if (!validateQubitIndex(qubit) || !qubitStates_.contains(qubit)) {
    return 0.0;
  }

  const QVector<double> &state = qubitStates_[qubit];
  return 2 * sqrt(state[0] * state[1]);
}

QMap<QString, double>
CoreInterface::getQubitEntanglementMeasures(int qubit) const {
  QMap<QString, double> measures;
  // Simplified entanglement measures for single qubit
  measures["Purity"] = getQubitPurity(qubit);
  measures["Coherence"] = getQubitCoherence(qubit);
  return measures;
}

// Enhanced quantum state debugging methods
QVector<std::complex<double>> CoreInterface::getStateVectorComplex() const {
  QVector<std::complex<double>> result;
  auto qcomplex_vector = getCurrentStateVector();
  for (const auto &qc : qcomplex_vector) {
    result.append(qc.toStdComplex());
  }
  return result;
}

QVector<QVector<std::complex<double>>>
CoreInterface::getDensityMatrixComplex() const {
  QVector<QVector<std::complex<double>>> result;
  auto qcomplex_matrix = getCurrentDensityMatrix();
  for (const auto &row : qcomplex_matrix) {
    QVector<std::complex<double>> complex_row;
    for (const auto &qc : row) {
      complex_row.append(qc.toStdComplex());
    }
    result.append(complex_row);
  }
  return result;
}

double CoreInterface::getStatePurity() const { return calculatePurity(); }

double CoreInterface::getStateFidelity(
    const QVector<std::complex<double>> &targetState) const {
  return calculateFidelity(targetState);
}

double CoreInterface::getVonNeumannEntropy() const {
  return calculateVonNeumannEntropy();
}

double CoreInterface::getStateCoherence() const { return calculateCoherence(); }

QMap<QString, double> CoreInterface::getEntanglementMeasures() const {
  return calculateEntanglementMeasures();
}

QVector<double> CoreInterface::performStateTomography() const {
  return performTomography();
}

QVector<double>
CoreInterface::getStateExpectationValues(const QString &observable) const {
  QVector<double> expectations;
  if (observable == "X") {
    expectations = {0.5, 0.5, 0.0, 0.0};
  } else if (observable == "Y") {
    expectations = {0.0, 0.0, 0.5, 0.5};
  } else if (observable == "Z") {
    expectations = {0.5, 0.0, 0.0, 0.5};
  }
  return expectations;
}

int CoreInterface::getCircuitDepth() const { return totalSteps_; }

int CoreInterface::getGateCount() const { return totalSteps_; }

int CoreInterface::getTotalSteps() const { return totalSteps_; }

int CoreInterface::getCurrentStep() const { return currentStep_; }

QStringList CoreInterface::getGateInfo(int step) const {
  QStringList info;
  if (!validateStepIndex(step)) {
    return info;
  }

  if (step == 1) {
    info << "Gate type: H";
    info << "Qubits: 0";
    info << "Parameters: []";
  } else if (step == 2) {
    info << "Gate type: CNOT";
    info << "Qubits: 0, 1";
    info << "Parameters: []";
  }

  return info;
}

bool CoreInterface::addGate(const QString &gateType, const QVector<int> &qubits,
                            const QVector<double> &parameters) {
  totalSteps_++;
  emit circuitChanged();
  return true;
}

bool CoreInterface::removeGate(int step) {
  if (!validateStepIndex(step)) {
    return false;
  }

  totalSteps_--;
  emit circuitChanged();
  return true;
}

bool CoreInterface::insertGate(int step, const QString &gateType,
                               const QVector<int> &qubits,
                               const QVector<double> &parameters) {
  if (!validateStepIndex(step)) {
    return false;
  }

  totalSteps_++;
  emit circuitChanged();
  return true;
}

bool CoreInterface::optimizeCircuit() {
  // Simulate optimization
  totalSteps_ = qMax(1, totalSteps_ - 1);
  emit circuitChanged();
  return true;
}

QStringList CoreInterface::getOptimizationSuggestions() const {
  QStringList suggestions;
  suggestions << "Consider combining adjacent Hadamard gates";
  suggestions << "Remove redundant CNOT gates";
  return suggestions;
}

QJsonObject CoreInterface::exportCircuit() const {
  QJsonObject circuitData;
  circuitData["num_qubits"] = qubitStates_.size();
  circuitData["num_classical_bits"] = 0;
  circuitData["depth"] = getCircuitDepth();
  circuitData["gate_count"] = getGateCount();

  QJsonArray gates;
  circuitData["gates"] = gates;

  return circuitData;
}

bool CoreInterface::importCircuit(const QJsonObject &circuitData) {
  if (!circuitData.contains("num_qubits")) {
    lastError_ = "Invalid circuit data: missing num_qubits";
    emit errorOccurred(lastError_);
    return false;
  }

  int numQubits = circuitData["num_qubits"].toInt();
  if (!createCircuit(numQubits)) {
    return false;
  }

  // Clear the dummy gates from createCircuit
  circuit_->reset();

  if (circuitData.contains("initial_view")) {
    recommendedViewMode_ = circuitData["initial_view"].toString();
  } else {
    recommendedViewMode_.clear();
  }

  if (circuitData.contains("gates")) {
    QJsonArray gates = circuitData["gates"].toArray();
    for (const auto &gateVal : gates) {
      QJsonObject gateObj = gateVal.toObject();
      QString typeStr = gateObj["type"].toString();
      int qubit = gateObj["qubit"].toInt();
      double param = gateObj["parameter"].toDouble(0.0);

      try {
        omniq::GateType type = stringToGateType(typeStr);
        if (type == omniq::GateType::CNOT || type == omniq::GateType::SWAP) {
          int target = gateObj["target"].toInt(qubit + 1);
          circuit_->addGate(type, qubit, target);
        } else {
          circuit_->addGate(type, qubit, param);
        }
      } catch (...) {
        // Skip unknown gates for now
      }
    }
  }

  totalSteps_ = circuit_->getTotalSteps();
  currentStep_ = 0;

  // Initial execution to step 0 (identity)
  updateQubitStates();

  emit circuitChanged();
  emit stateChanged();
  return true;
}

QString CoreInterface::getLastError() const { return lastError_; }

void CoreInterface::clearError() { lastError_.clear(); }

void CoreInterface::updateStepCircuits() {
  // Simplified implementation
}

void CoreInterface::updateCurrentState() {
  // Simplified implementation
}

void CoreInterface::updateQubitStates() {
  if (!currentState_) {
    return;
  }

  int numQubits = currentState_->getNumQubits();
  qubitStates_.clear();

  for (int qubit = 0; qubit < numQubits; ++qubit) {
    // Get probabilities for |0⟩ and |1⟩ states
    double prob0 = currentState_->getQubitProbability(qubit, 0);
    double prob1 = currentState_->getQubitProbability(qubit, 1);

    // Get amplitudes
    std::complex<double> amp0 = currentState_->getQubitAmplitude(qubit, 0);
    std::complex<double> amp1 = currentState_->getQubitAmplitude(qubit, 1);

    // Calculate Bloch sphere parameters with safety check to avoid NaN
    double clamped_prob0 = std::clamp(prob0, 0.0, 1.0);
    double theta = 2.0 * std::acos(std::sqrt(clamped_prob0));

    double phase0 = std::arg(amp0);
    double phase1 = std::arg(amp1);
    double phi = phase1 - phase0;

    QVector<double> state = {
        prob0,       prob1,       // Probabilities
        amp0.real(), amp0.imag(), // |0⟩ amplitude
        amp1.real(), amp1.imag(), // |1⟩ amplitude
        theta,       phi          // Bloch sphere parameters
    };

    qubitStates_[qubit] = state;
  }
}

void CoreInterface::updateEntanglementMeasures() {
  // Simplified implementation
}

bool CoreInterface::validateQubitIndex(int qubit) const {
  if (qubit < 0 || qubit >= 8) { // Support up to 8 qubits
    return false;
  }
  return true;
}

bool CoreInterface::validateStepIndex(int step) const {
  if (step < 0 || step > totalSteps_) {
    return false;
  }
  return true;
}

QString CoreInterface::gateTypeToString(omniq::GateType type) const {
  switch (type) {
  case omniq::GateType::H:
    return "H";
  case omniq::GateType::X:
    return "X";
  case omniq::GateType::Y:
    return "Y";
  case omniq::GateType::Z:
    return "Z";
  case omniq::GateType::CNOT:
    return "CNOT";
  case omniq::GateType::SWAP:
    return "SWAP";
  case omniq::GateType::PHASE:
    return "PHASE";
  case omniq::GateType::RX:
    return "RX";
  case omniq::GateType::RY:
    return "RY";
  case omniq::GateType::RZ:
    return "RZ";
  case omniq::GateType::CP:
    return "CP";
  default:
    return "UNKNOWN";
  }
}

omniq::GateType CoreInterface::stringToGateType(const QString &type) const {
  if (type == "H")
    return omniq::GateType::H;
  if (type == "X")
    return omniq::GateType::X;
  if (type == "Y")
    return omniq::GateType::Y;
  if (type == "Z")
    return omniq::GateType::Z;
  if (type == "CNOT")
    return omniq::GateType::CNOT;
  if (type == "SWAP")
    return omniq::GateType::SWAP;
  if (type == "PHASE")
    return omniq::GateType::PHASE;
  if (type == "RX")
    return omniq::GateType::RX;
  if (type == "RY")
    return omniq::GateType::RY;
  if (type == "RZ")
    return omniq::GateType::RZ;
  if (type == "CP")
    return omniq::GateType::CP;

  throw std::invalid_argument("Unknown gate type: " + type.toStdString());
}

// Helper methods for quantum state analysis
QVector<double> CoreInterface::calculateQubitState(int qubit) const {
  return getQubitState(qubit);
}

QVector<double> CoreInterface::calculateExpectationValues(int qubit) const {
  return getQubitExpectationValues(qubit);
}

double CoreInterface::calculatePurity() const {
  return 1.0; // Simplified: assume pure state
}

double CoreInterface::calculateFidelity(
    const QVector<std::complex<double>> &targetState) const {
  return 0.95; // Simplified: assume high fidelity
}

double CoreInterface::calculateVonNeumannEntropy() const {
  return 0.0; // Simplified: assume pure state
}

double CoreInterface::calculateCoherence() const {
  return 0.8; // Simplified: assume some coherence
}

QMap<QString, double> CoreInterface::calculateEntanglementMeasures() const {
  QMap<QString, double> measures;
  measures["Concurrence"] = 0.5;
  measures["Negativity"] = 0.3;
  measures["Von Neumann"] = 0.7;
  measures["Tangle"] = 0.25;
  return measures;
}

QVector<double> CoreInterface::performTomography() const {
  QVector<double> tomographyData = {0.5, 0.5, 0.7, 0.3, 0.6, 0.4};
  return tomographyData;
}
