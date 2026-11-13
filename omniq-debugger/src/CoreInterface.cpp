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

    totalSteps_ = 0;
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
  if (!circuit_) return "";
  int numQubits = currentState_ ? currentState_->getNumQubits() : 2;
  QString qasm = "OPENQASM 2.0;\n";
  qasm += "include \"qelib1.inc\";\n\n";
  qasm += QString("qreg q[%1];\n").arg(numQubits);
  qasm += QString("creg c[%1];\n\n").arg(numQubits);

  const auto &gates = circuit_->getGates();
  for (const auto &gate : gates) {
    omniq::GateType type = std::get<0>(gate);
    int q1 = std::get<1>(gate);
    int q2 = std::get<2>(gate);
    double param = std::get<3>(gate);

    switch (type) {
    case omniq::GateType::H:
      qasm += QString("h q[%1];\n").arg(q1);
      break;
    case omniq::GateType::X:
      qasm += QString("x q[%1];\n").arg(q1);
      break;
    case omniq::GateType::Y:
      qasm += QString("y q[%1];\n").arg(q1);
      break;
    case omniq::GateType::Z:
      qasm += QString("z q[%1];\n").arg(q1);
      break;
    case omniq::GateType::CNOT:
      qasm += QString("cx q[%1], q[%2];\n").arg(q1).arg(q2);
      break;
    case omniq::GateType::SWAP:
      qasm += QString("swap q[%1], q[%2];\n").arg(q1).arg(q2);
      break;
    case omniq::GateType::PHASE:
      qasm += QString("p(%1) q[%2];\n").arg(param).arg(q1);
      break;
    case omniq::GateType::RX:
      qasm += QString("rx(%1) q[%2];\n").arg(param).arg(q1);
      break;
    case omniq::GateType::RY:
      qasm += QString("ry(%1) q[%2];\n").arg(param).arg(q1);
      break;
    case omniq::GateType::RZ:
      qasm += QString("rz(%1) q[%2];\n").arg(param).arg(q1);
      break;
    case omniq::GateType::CP:
      qasm += QString("cp(%1) q[%2], q[%3];\n").arg(param).arg(q1).arg(q2);
      break;
    }
  }

  for (int q = 0; q < numQubits; ++q) {
    qasm += QString("measure q[%1] -> c[%1];\n").arg(q);
  }

  return qasm;
}

bool CoreInterface::executeStep(int step) {
  if (!circuit_ || !currentState_ || step < 0 || step > totalSteps_) {
    return false;
  }

  try {
    if (step < currentStep_) {
      // Step backward or rewind to previous step
      currentState_->reset();
      circuit_->reset();
      currentStep_ = 0;
    }

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
  return executeStep(step);
}

bool CoreInterface::executeFull() { return executeToStep(totalSteps_); }

void CoreInterface::resetExecution() {
  if (currentState_) {
    currentState_->reset();
  }
  if (circuit_) {
    circuit_->reset();
  }
  currentStep_ = 0;
  updateQubitStates();
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
  if (!currentState_) {
    return result;
  }

  int n = currentState_->getNumQubits();
  VectorXcd sv = currentState_->getStateVector();

  if (n <= 4) {
    int dim = 1 << n;
    result.resize(dim);
    for (int i = 0; i < dim; ++i) {
      result[i].resize(dim);
      for (int j = 0; j < dim; ++j) {
        std::complex<double> val = sv(i) * std::conj(sv(j));
        result[i][j] = QComplex::fromStdComplex(val);
      }
    }
  } else {
    // For n > 4, compute 4x4 reduced density matrix for qubits 0 and 1 by tracing out qubits >= 2
    int dim = 4;
    result.resize(dim);
    for (int i = 0; i < dim; ++i) {
      result[i].resize(dim);
      for (int j = 0; j < dim; ++j) {
        result[i][j] = QComplex(0.0, 0.0);
      }
    }
    int restDim = 1 << (n - 2);
    for (int r = 0; r < restDim; ++r) {
      int restBits = r << 2;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          std::complex<double> val = sv(restBits | i) * std::conj(sv(restBits | j));
          result[i][j] = QComplex(result[i][j].real() + val.real(), result[i][j].imag() + val.imag());
        }
      }
    }
  }
  return result;
}

double CoreInterface::getQubitProbability(int qubit, int value) const {
  if (!validateQubitIndex(qubit) || !currentState_) {
    return 0.0;
  }
  return currentState_->getQubitProbability(qubit, value);
}

double CoreInterface::getQubitExpectation(int qubit,
                                          const QString &observable) const {
  if (!currentState_ || qubit < 0 || qubit >= currentState_->getNumQubits()) {
    return 0.0;
  }

  int n = currentState_->getNumQubits();
  int dim = 1 << n;
  VectorXcd sv = currentState_->getStateVector();

  if (observable == "Z") {
    double expZ = 0.0;
    for (int i = 0; i < dim; ++i) {
      double p = std::norm(sv(i));
      if (((i >> qubit) & 1) == 0) {
        expZ += p;
      } else {
        expZ -= p;
      }
    }
    return expZ;
  }

  std::complex<double> offDiag = 0.0;
  for (int i = 0; i < dim; ++i) {
    if (((i >> qubit) & 1) == 0) {
      int j = i | (1 << qubit);
      offDiag += std::conj(sv(i)) * sv(j);
    }
  }

  if (observable == "X") {
    return 2.0 * offDiag.real();
  } else if (observable == "Y") {
    return -2.0 * offDiag.imag();
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

QVector<CoreInterface::QubitStepTrajectory>
CoreInterface::getQubitTrajectory(int qubit) const {
  QVector<QubitStepTrajectory> trajectory;
  if (!circuit_ || !currentState_)
    return trajectory;

  int numQubits = circuit_->getNumQubits();
  if (qubit < 0 || qubit >= numQubits)
    return trajectory;

  // Clone circuit and state to simulate trajectory without disturbing current execution
  omniq::QuantumCircuit simCircuit(*circuit_);
  omniq::QuantumState simState(numQubits);
  simCircuit.reset();
  simState.reset();

  auto computeParams =
      [&](const omniq::QuantumState &st) -> std::tuple<double, double, double> {
    int dim = 1 << numQubits;
    VectorXcd sv = st.getStateVector();
    double prob0 = 0.0;
    double prob1 = 0.0;
    std::complex<double> rho01 = 0.0;
    for (int i = 0; i < dim; ++i) {
      if (((i >> qubit) & 1) == 0) {
        int partner = i | (1 << qubit);
        prob0 += std::norm(sv(i));
        rho01 += sv(i) * std::conj(sv(partner));
      } else {
        prob1 += std::norm(sv(i));
      }
    }
    double rx = 2.0 * rho01.real();
    double ry = -2.0 * rho01.imag();
    double rz = prob0 - prob1;
    double r = std::sqrt(rx * rx + ry * ry + rz * rz);
    double theta = 0.0;
    double phi = 0.0;
    if (r > 1e-12) {
      theta = std::acos(std::clamp(rz / r, -1.0, 1.0));
      phi = std::atan2(ry, rx);
    }
    return {theta, phi, r};
  };

  // Step 0: Initial state |0⟩
  auto [t0, p0, r0] = computeParams(simState);
  trajectory.append({0, "Init", t0, p0, r0});

  // Steps 1..totalSteps
  const auto &gates = simCircuit.getGates();
  int stepIdx = 1;
  for (const auto &g : gates) {
    simCircuit.executeStep(simState);
    auto [th, ph, r_val] = computeParams(simState);
    QString gateName = gateTypeToString(std::get<0>(g));
    int q1 = std::get<1>(g);
    int q2 = std::get<2>(g);
    QString label = gateName;
    if (q1 != qubit && q2 != qubit) {
      label = QString("(%1)").arg(gateName);
    }
    trajectory.append({stepIdx++, label, th, ph, r_val});
  }

  return trajectory;
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
  if (!currentState_ || qubit < 0 || qubit >= currentState_->getNumQubits()) {
    return 0.0;
  }

  int n = currentState_->getNumQubits();
  int dim = 1 << n;
  VectorXcd sv = currentState_->getStateVector();
  double rho00 = 0.0, rho11 = 0.0;
  std::complex<double> rho01 = 0.0;

  for (int i = 0; i < dim; ++i) {
    if (((i >> qubit) & 1) == 0) {
      int partner = i | (1 << qubit);
      rho00 += std::norm(sv(i));
      rho01 += sv(i) * std::conj(sv(partner));
    } else {
      rho11 += std::norm(sv(i));
    }
  }

  double purity = (rho00 * rho00) + (rho11 * rho11) + 2.0 * std::norm(rho01);
  return std::clamp(purity, 0.5, 1.0);
}

double CoreInterface::getQubitCoherence(int qubit) const {
  if (!currentState_ || qubit < 0 || qubit >= currentState_->getNumQubits()) {
    return 0.0;
  }

  int n = currentState_->getNumQubits();
  int dim = 1 << n;
  VectorXcd sv = currentState_->getStateVector();
  std::complex<double> rho01 = 0.0;

  for (int i = 0; i < dim; ++i) {
    if (((i >> qubit) & 1) == 0) {
      int partner = i | (1 << qubit);
      rho01 += sv(i) * std::conj(sv(partner));
    }
  }

  return 2.0 * std::abs(rho01);
}

QMap<QString, double>
CoreInterface::getQubitEntanglementMeasures(int qubit) const {
  QMap<QString, double> measures;
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
  if (!currentState_) return expectations;
  int numQubits = currentState_->getNumQubits();
  for (int q = 0; q < numQubits; ++q) {
    expectations.append(getQubitExpectation(q, observable));
  }
  return expectations;
}

QVector<CoreInterface::GateItem> CoreInterface::getCircuitGates() const {
  QVector<GateItem> result;
  if (!circuit_) return result;
  const auto &gates = circuit_->getGates();
  for (size_t i = 0; i < gates.size(); ++i) {
    GateItem item;
    item.step = static_cast<int>(i);
    item.type = gateTypeToString(std::get<0>(gates[i]));
    item.qubit1 = std::get<1>(gates[i]);
    item.qubit2 = std::get<2>(gates[i]);
    item.parameter = std::get<3>(gates[i]);
    result.append(item);
  }
  return result;
}

int CoreInterface::getCircuitDepth() const { return totalSteps_; }

int CoreInterface::getGateCount() const { return totalSteps_; }

int CoreInterface::getTotalSteps() const { return totalSteps_; }

int CoreInterface::getCurrentStep() const { return currentStep_; }

QStringList CoreInterface::getGateInfo(int step) const {
  QStringList info;
  if (!circuit_ || step < 0 || step >= circuit_->getTotalSteps()) {
    return info;
  }

  const auto &gates = circuit_->getGates();
  const auto &gate = gates[step];
  omniq::GateType type = std::get<0>(gate);
  int q1 = std::get<1>(gate);
  int q2 = std::get<2>(gate);
  double param = std::get<3>(gate);

  info << QString("Gate type: %1").arg(gateTypeToString(type));
  if (q2 == -1) {
    info << QString("Qubits: %1").arg(q1);
  } else {
    info << QString("Qubits: %1, %2").arg(q1).arg(q2);
  }
  if (param != 0.0) {
    info << QString("Parameters: [%1]").arg(param, 0, 'f', 4);
  } else {
    info << "Parameters: []";
  }

  return info;
}

bool CoreInterface::addGate(const QString &gateType, const QVector<int> &qubits,
                            const QVector<double> &parameters) {
  if (!circuit_ || qubits.isEmpty()) {
    return false;
  }

  try {
    omniq::GateType type = stringToGateType(gateType);
    double param = parameters.isEmpty() ? 0.0 : parameters[0];

    if (type == omniq::GateType::CNOT || type == omniq::GateType::SWAP || type == omniq::GateType::CP) {
      if (qubits.size() < 2) return false;
      circuit_->addGate(type, qubits[0], qubits[1], param);
    } else {
      circuit_->addGate(type, qubits[0], param);
    }

    totalSteps_ = circuit_->getTotalSteps();
    emit circuitChanged();
    return true;
  } catch (const std::exception &e) {
    lastError_ = QString("Failed to add gate: %1").arg(e.what());
    emit errorOccurred(lastError_);
    return false;
  }
}

bool CoreInterface::removeGate(int step) {
  if (!circuit_ || step < 0 || step >= circuit_->getTotalSteps()) {
    return false;
  }

  circuit_->removeGate(step);
  totalSteps_ = circuit_->getTotalSteps();
  if (currentStep_ > totalSteps_) {
    currentStep_ = totalSteps_;
  }
  emit circuitChanged();
  return true;
}

bool CoreInterface::insertGate(int step, const QString &gateType,
                               const QVector<int> &qubits,
                               const QVector<double> &parameters) {
  Q_UNUSED(step);
  return addGate(gateType, qubits, parameters);
}

bool CoreInterface::optimizeCircuit() {
  // Circuit optimization placeholder
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
  int numQubits = currentState_ ? currentState_->getNumQubits() : 2;
  circuitData["num_qubits"] = numQubits;
  circuitData["num_classical_bits"] = 0;
  circuitData["depth"] = getCircuitDepth();
  circuitData["gate_count"] = getGateCount();

  QJsonArray gates;
  if (circuit_) {
    for (const auto &gate : circuit_->getGates()) {
      QJsonObject gateObj;
      gateObj["type"] = gateTypeToString(std::get<0>(gate));
      gateObj["qubit"] = std::get<1>(gate);
      if (std::get<2>(gate) != -1) {
        gateObj["target"] = std::get<2>(gate);
      }
      gateObj["parameter"] = std::get<3>(gate);
      gates.append(gateObj);
    }
  }
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
  int dim = 1 << numQubits;
  VectorXcd sv = currentState_->getStateVector();
  qubitStates_.clear();

  for (int qubit = 0; qubit < numQubits; ++qubit) {
    double prob0 = 0.0;
    double prob1 = 0.0;
    std::complex<double> rho01 = 0.0;

    for (int i = 0; i < dim; ++i) {
      if (((i >> qubit) & 1) == 0) {
        int partner = i | (1 << qubit);
        prob0 += std::norm(sv(i));
        rho01 += sv(i) * std::conj(sv(partner));
      } else {
        prob1 += std::norm(sv(i));
      }
    }

    // Single qubit Bloch coordinates
    double rx = 2.0 * rho01.real();
    double ry = -2.0 * rho01.imag();
    double rz = prob0 - prob1;
    double r = std::sqrt(rx * rx + ry * ry + rz * rz);

    double theta = 0.0;
    double phi = 0.0;
    if (r > 1e-12) {
      theta = std::acos(std::clamp(rz / r, -1.0, 1.0));
      phi = std::atan2(ry, rx);
    }

    std::complex<double> amp0 = currentState_->getQubitAmplitude(qubit, 0);
    std::complex<double> amp1 = currentState_->getQubitAmplitude(qubit, 1);

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
  entanglementMeasures_ = calculateEntanglementMeasures();
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
  return 1.0;
}

double CoreInterface::calculateFidelity(
    const QVector<std::complex<double>> &targetState) const {
  if (!currentState_) return 0.0;
  VectorXcd sv = currentState_->getStateVector();
  if (targetState.size() != sv.size()) return 0.0;

  std::complex<double> dot = 0.0;
  for (int i = 0; i < sv.size(); ++i) {
    dot += std::conj(targetState[i]) * sv(i);
  }
  return std::clamp(std::norm(dot), 0.0, 1.0);
}

double CoreInterface::calculateVonNeumannEntropy() const {
  if (!currentState_) return 0.0;
  int n = currentState_->getNumQubits();
  if (n < 2) return 0.0;

  int dim = 1 << n;
  VectorXcd sv = currentState_->getStateVector();
  double rho00 = 0.0;
  std::complex<double> rho01 = 0.0;
  for (int i = 0; i < dim; ++i) {
    if ((i & 1) == 0) {
      rho00 += std::norm(sv(i));
      rho01 += sv(i) * std::conj(sv(i | 1));
    }
  }
  double rho11 = 1.0 - rho00;
  double det = rho00 * rho11 - std::norm(rho01);
  det = std::max(0.0, det);
  double disc = std::sqrt(std::max(0.0, 1.0 - 4.0 * det));
  double lambda1 = std::clamp((1.0 + disc) / 2.0, 0.0, 1.0);
  double lambda2 = std::clamp((1.0 - disc) / 2.0, 0.0, 1.0);

  double entropy = 0.0;
  if (lambda1 > 1e-12) entropy -= lambda1 * std::log2(lambda1);
  if (lambda2 > 1e-12) entropy -= lambda2 * std::log2(lambda2);
  return entropy;
}

double CoreInterface::calculateCoherence() const {
  if (!currentState_) return 0.0;
  VectorXcd sv = currentState_->getStateVector();
  double l1Coherence = 0.0;
  for (int i = 0; i < sv.size(); ++i) {
    for (int j = 0; j < sv.size(); ++j) {
      if (i != j) {
        l1Coherence += std::abs(sv(i) * std::conj(sv(j)));
      }
    }
  }
  return l1Coherence;
}

QMap<QString, double> CoreInterface::calculateEntanglementMeasures() const {
  QMap<QString, double> measures;
  if (!currentState_) {
    measures["Concurrence"] = 0.0;
    measures["Negativity"] = 0.0;
    measures["Von Neumann"] = 0.0;
    measures["Tangle"] = 0.0;
    return measures;
  }

  int n = currentState_->getNumQubits();
  if (n < 2) {
    measures["Concurrence"] = 0.0;
    measures["Negativity"] = 0.0;
    measures["Von Neumann"] = 0.0;
    measures["Tangle"] = 0.0;
    return measures;
  }

  int dim = 1 << n;
  VectorXcd sv = currentState_->getStateVector();
  double rho00 = 0.0;
  std::complex<double> rho01 = 0.0;
  for (int i = 0; i < dim; ++i) {
    if ((i & 1) == 0) {
      rho00 += std::norm(sv(i));
      rho01 += sv(i) * std::conj(sv(i | 1));
    }
  }
  double rho11 = 1.0 - rho00;
  double det = std::max(0.0, rho00 * rho11 - std::norm(rho01));
  double concurrence = std::clamp(2.0 * std::sqrt(det), 0.0, 1.0);
  double tangle = concurrence * concurrence;
  double negativity = concurrence / 2.0;
  double entropy = calculateVonNeumannEntropy();

  measures["Concurrence"] = concurrence;
  measures["Negativity"] = negativity;
  measures["Von Neumann"] = entropy;
  measures["Tangle"] = tangle;
  return measures;
}

QVector<double> CoreInterface::performTomography() const {
  QVector<double> tomographyData;
  if (!currentState_) return tomographyData;
  int numQubits = currentState_->getNumQubits();
  for (int q = 0; q < numQubits; ++q) {
    tomographyData.append(getQubitExpectation(q, "X"));
    tomographyData.append(getQubitExpectation(q, "Y"));
    tomographyData.append(getQubitExpectation(q, "Z"));
  }
  return tomographyData;
}
