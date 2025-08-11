//
// Created by Goutham Arcot on 17/07/25.
//

#include "CoreInterface.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <cmath>
#include <algorithm>

CoreInterface::CoreInterface(QObject *parent)
    : QObject(parent)
    , currentStep_(0)
    , totalSteps_(0)
    , isExecuting_(false)
{
}

CoreInterface::~CoreInterface()
{
}

bool CoreInterface::createCircuit(int numQubits, int numClassicalBits)
{
    try {
        circuit_ = std::make_unique<omniq::Circuit>(numQubits, numClassicalBits);
        currentState_ = std::make_unique<omniq::Statevector>(numQubits);
        currentDensityMatrix_ = nullptr;
        
        stepCircuits_.clear();
        stepStates_.clear();
        currentStep_ = 0;
        totalSteps_ = 0;
        isExecuting_ = false;
        
        clearError();
        emit circuitChanged();
        return true;
    } catch (const std::exception &e) {
        lastError_ = QString("Failed to create circuit: %1").arg(e.what());
        emit errorOccurred(lastError_);
        return false;
    }
}

bool CoreInterface::loadCircuit(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        lastError_ = QString("Cannot open file: %1").arg(filename);
        emit errorOccurred(lastError_);
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // TODO: Implement QASM parsing
    // For now, create a simple circuit
    if (content.contains("OPENQASM")) {
        // Parse QASM format
        return createCircuit(2); // Default to 2 qubits
    } else {
        // Try to parse as JSON
        QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8());
        if (doc.isObject()) {
            return importCircuit(doc.object());
        }
    }
    
    lastError_ = "Unsupported file format";
    emit errorOccurred(lastError_);
    return false;
}

bool CoreInterface::saveCircuit(const QString &filename)
{
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

QString CoreInterface::getCircuitQASM() const
{
    if (!circuit_) {
        return "";
    }
    
    QString qasm = "OPENQASM 2.0;\n";
    qasm += "include \"qelib1.inc\";\n\n";
    qasm += QString("qreg q[%1];\n").arg(circuit_->get_num_qubits());
    if (circuit_->get_num_classical_bits() > 0) {
        qasm += QString("creg c[%1];\n\n").arg(circuit_->get_num_classical_bits());
    }
    
    // TODO: Convert circuit gates to QASM format
    qasm += "// Circuit gates would be listed here\n";
    
    return qasm;
}

bool CoreInterface::executeStep(int step)
{
    if (!validateStepIndex(step)) {
        return false;
    }
    
    try {
        if (step >= stepCircuits_.size()) {
            updateStepCircuits();
        }
        
        if (step < stepCircuits_.size()) {
            currentState_ = std::make_unique<omniq::Statevector>(
                stepCircuits_[step]->execute_statevector());
            currentStep_ = step;
            updateCurrentState();
            emit executionStepChanged(currentStep_);
            emit stateChanged();
            return true;
        }
    } catch (const std::exception &e) {
        lastError_ = QString("Execution failed at step %1: %2").arg(step).arg(e.what());
        emit errorOccurred(lastError_);
    }
    
    return false;
}

bool CoreInterface::executeToStep(int step)
{
    for (int i = currentStep_ + 1; i <= step; ++i) {
        if (!executeStep(i)) {
            return false;
        }
    }
    return true;
}

bool CoreInterface::executeFull()
{
    if (!circuit_) {
        lastError_ = "No circuit to execute";
        emit errorOccurred(lastError_);
        return false;
    }
    
    try {
        currentState_ = std::make_unique<omniq::Statevector>(
            circuit_->execute_statevector());
        currentStep_ = totalSteps_;
        updateCurrentState();
        emit executionStepChanged(currentStep_);
        emit stateChanged();
        return true;
    } catch (const std::exception &e) {
        lastError_ = QString("Full execution failed: %1").arg(e.what());
        emit errorOccurred(lastError_);
        return false;
    }
}

void CoreInterface::resetExecution()
{
    currentStep_ = 0;
    if (currentState_) {
        currentState_ = std::make_unique<omniq::Statevector>(
            circuit_->get_num_qubits());
    }
    emit executionStepChanged(currentStep_);
    emit stateChanged();
}

QString CoreInterface::getCurrentStateString() const
{
    if (!currentState_) {
        return "No state available";
    }
    
    return currentState_->to_string().c_str();
}

QVector<QComplex> CoreInterface::getCurrentStateVector() const
{
    QVector<QComplex> result;
    if (!currentState_) {
        return result;
    }
    
    auto amplitudes = currentState_->get_amplitudes();
    for (const auto &amp : amplitudes) {
        result.append(QComplex(amp.real(), amp.imag()));
    }
    
    return result;
}

QVector<QVector<QComplex>> CoreInterface::getCurrentDensityMatrix() const
{
    QVector<QVector<QComplex>> result;
    if (!currentDensityMatrix_) {
        return result;
    }
    
    auto matrix = currentDensityMatrix_->get_matrix();
    int size = matrix.size();
    result.resize(size);
    
    for (int i = 0; i < size; ++i) {
        result[i].resize(size);
        for (int j = 0; j < size; ++j) {
            result[i][j] = QComplex(matrix[i][j].real(), matrix[i][j].imag());
        }
    }
    
    return result;
}

double CoreInterface::getQubitProbability(int qubit, int value) const
{
    if (!validateQubitIndex(qubit) || !currentState_) {
        return 0.0;
    }
    
    try {
        // TODO: Implement qubit probability calculation
        return 0.5; // Placeholder
    } catch (const std::exception &e) {
        lastError_ = QString("Probability calculation failed: %1").arg(e.what());
        return 0.0;
    }
}

double CoreInterface::getQubitExpectation(int qubit, const QString &observable) const
{
    if (!validateQubitIndex(qubit) || !currentState_) {
        return 0.0;
    }
    
    try {
        // TODO: Implement expectation value calculation
        return 0.0; // Placeholder
    } catch (const std::exception &e) {
        lastError_ = QString("Expectation calculation failed: %1").arg(e.what());
        return 0.0;
    }
}

int CoreInterface::getCircuitDepth() const
{
    return circuit_ ? circuit_->get_depth() : 0;
}

int CoreInterface::getGateCount() const
{
    return circuit_ ? circuit_->get_gate_count() : 0;
}

int CoreInterface::getTotalSteps() const
{
    return totalSteps_;
}

int CoreInterface::getCurrentStep() const
{
    return currentStep_;
}

QStringList CoreInterface::getGateInfo(int step) const
{
    QStringList info;
    if (!validateStepIndex(step)) {
        return info;
    }
    
    // TODO: Extract gate information for the given step
    info << "Gate type: H";
    info << "Qubits: 0";
    info << "Parameters: []";
    
    return info;
}

bool CoreInterface::addGate(const QString &gateType, const QVector<int> &qubits, 
                           const QVector<double> &parameters)
{
    if (!circuit_) {
        lastError_ = "No circuit available";
        emit errorOccurred(lastError_);
        return false;
    }
    
    try {
        omniq::GateType type = stringToGateType(gateType);
        // TODO: Implement gate addition with parameters
        emit circuitChanged();
        return true;
    } catch (const std::exception &e) {
        lastError_ = QString("Failed to add gate: %1").arg(e.what());
        emit errorOccurred(lastError_);
        return false;
    }
}

bool CoreInterface::removeGate(int step)
{
    if (!validateStepIndex(step)) {
        return false;
    }
    
    // TODO: Implement gate removal
    emit circuitChanged();
    return true;
}

bool CoreInterface::insertGate(int step, const QString &gateType, 
                              const QVector<int> &qubits,
                              const QVector<double> &parameters)
{
    if (!validateStepIndex(step)) {
        return false;
    }
    
    // TODO: Implement gate insertion
    emit circuitChanged();
    return true;
}

bool CoreInterface::optimizeCircuit()
{
    if (!circuit_) {
        lastError_ = "No circuit to optimize";
        emit errorOccurred(lastError_);
        return false;
    }
    
    // TODO: Implement circuit optimization
    emit circuitChanged();
    return true;
}

QStringList CoreInterface::getOptimizationSuggestions() const
{
    QStringList suggestions;
    if (!circuit_) {
        return suggestions;
    }
    
    // TODO: Implement optimization analysis
    suggestions << "Consider combining adjacent Hadamard gates";
    suggestions << "Remove redundant CNOT gates";
    
    return suggestions;
}

QJsonObject CoreInterface::exportCircuit() const
{
    QJsonObject circuitData;
    
    if (!circuit_) {
        return circuitData;
    }
    
    circuitData["num_qubits"] = circuit_->get_num_qubits();
    circuitData["num_classical_bits"] = circuit_->get_num_classical_bits();
    circuitData["depth"] = circuit_->get_depth();
    circuitData["gate_count"] = circuit_->get_gate_count();
    
    // TODO: Export circuit gates as JSON
    QJsonArray gates;
    circuitData["gates"] = gates;
    
    return circuitData;
}

bool CoreInterface::importCircuit(const QJsonObject &circuitData)
{
    if (!circuitData.contains("num_qubits")) {
        lastError_ = "Invalid circuit data: missing num_qubits";
        emit errorOccurred(lastError_);
        return false;
    }
    
    int numQubits = circuitData["num_qubits"].toInt();
    int numClassicalBits = circuitData["num_classical_bits"].toInt(0);
    
    if (!createCircuit(numQubits, numClassicalBits)) {
        return false;
    }
    
    // TODO: Import circuit gates from JSON
    emit circuitChanged();
    return true;
}

QString CoreInterface::getLastError() const
{
    return lastError_;
}

void CoreInterface::clearError()
{
    lastError_.clear();
}

void CoreInterface::updateStepCircuits()
{
    if (!circuit_) {
        return;
    }
    
    // TODO: Implement step-by-step circuit generation
    totalSteps_ = circuit_->get_gate_count();
}

void CoreInterface::updateCurrentState()
{
    if (currentState_ && circuit_) {
        // Convert to density matrix if needed
        // TODO: Implement density matrix conversion
    }
}

bool CoreInterface::validateQubitIndex(int qubit) const
{
    if (!circuit_ || qubit < 0 || qubit >= circuit_->get_num_qubits()) {
        lastError_ = QString("Invalid qubit index: %1").arg(qubit);
        return false;
    }
    return true;
}

bool CoreInterface::validateStepIndex(int step) const
{
    if (step < 0 || step > totalSteps_) {
        lastError_ = QString("Invalid step index: %1").arg(step);
        return false;
    }
    return true;
}

QString CoreInterface::gateTypeToString(omniq::GateType type) const
{
    switch (type) {
        case omniq::GateType::H: return "H";
        case omniq::GateType::X: return "X";
        case omniq::GateType::Y: return "Y";
        case omniq::GateType::Z: return "Z";
        case omniq::GateType::CNOT: return "CNOT";
        case omniq::GateType::SWAP: return "SWAP";
        case omniq::GateType::PHASE: return "PHASE";
        case omniq::GateType::RX: return "RX";
        case omniq::GateType::RY: return "RY";
        case omniq::GateType::RZ: return "RZ";
        default: return "UNKNOWN";
    }
}

omniq::GateType CoreInterface::stringToGateType(const QString &type) const
{
    if (type == "H") return omniq::GateType::H;
    if (type == "X") return omniq::GateType::X;
    if (type == "Y") return omniq::GateType::Y;
    if (type == "Z") return omniq::GateType::Z;
    if (type == "CNOT") return omniq::GateType::CNOT;
    if (type == "SWAP") return omniq::GateType::SWAP;
    if (type == "PHASE") return omniq::GateType::PHASE;
    if (type == "RX") return omniq::GateType::RX;
    if (type == "RY") return omniq::GateType::RY;
    if (type == "RZ") return omniq::GateType::RZ;
    
    throw std::invalid_argument("Unknown gate type: " + type.toStdString());
}
