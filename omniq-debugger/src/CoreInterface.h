//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef COREINTERFACE_H
#define COREINTERFACE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <memory>
#include <complex>

#include <Eigen/Dense>
#include <vector>
#include <complex>
#include <random>

namespace omniq {
    enum class GateType {
        H, X, Y, Z, CNOT, SWAP, PHASE, RX, RY, RZ
    };
    
    using Matrix2cd = Eigen::Matrix2cd;
    using Matrix4cd = Eigen::Matrix4cd;
    using MatrixXcd = Eigen::MatrixXcd;
    using VectorXcd = Eigen::VectorXcd;
    
    class QuantumState {
    private:
        VectorXcd stateVector_;
        int numQubits_;
        
        MatrixXcd createSingleQubitGate(GateType type, int qubit, double parameter);
        MatrixXcd createTwoQubitGate(GateType type, int qubit1, int qubit2, double parameter);
        
    public:
        explicit QuantumState(int numQubits);
        void applyGate(GateType type, int qubit, double parameter = 0.0);
        void applyGate(GateType type, int control, int target, double parameter = 0.0);
        double getQubitProbability(int qubit, int value) const;
        std::complex<double> getQubitAmplitude(int qubit, int value) const;
        VectorXcd getStateVector() const { return stateVector_; }
        int getNumQubits() const { return numQubits_; }
        void reset();
    };
    
    class QuantumCircuit {
    private:
        std::vector<std::tuple<GateType, int, int, double>> gates_; // type, qubit1, qubit2, parameter
        int numQubits_;
        int currentStep_;
        
    public:
        explicit QuantumCircuit(int numQubits);
        void addGate(GateType type, int qubit, double parameter = 0.0);
        void addGate(GateType type, int control, int target, double parameter = 0.0);
        bool executeStep(QuantumState& state);
        void reset();
        int getCurrentStep() const { return currentStep_; }
        int getTotalSteps() const { return static_cast<int>(gates_.size()); }
        std::string getGateDescription(int step) const;
    };
}

// Helper class for complex numbers in Qt
class QComplex
{
public:
    QComplex(double real = 0.0, double imag = 0.0) : real_(real), imag_(imag) {}
    
    double real() const { return real_; }
    double imag() const { return imag_; }
    double magnitude() const { return std::sqrt(real_ * real_ + imag_ * imag_); }
    double phase() const { return std::atan2(imag_, real_); }
    
    QString toString() const {
        return QString("(%1 + %2i)").arg(real_).arg(imag_);
    }
    
    std::complex<double> toStdComplex() const {
        return std::complex<double>(real_, imag_);
    }
    
    static QComplex fromStdComplex(const std::complex<double> &z) {
        return QComplex(z.real(), z.imag());
    }
    
private:
    double real_;
    double imag_;
};

class CoreInterface : public QObject
{
    Q_OBJECT

public:
    explicit CoreInterface(QObject *parent = nullptr);
    ~CoreInterface();

    bool createCircuit(int numQubits, int numClassicalBits = 0);
    bool loadCircuit(const QString &filename);
    bool saveCircuit(const QString &filename);
    QString getCircuitQASM() const;
    
    bool executeStep(int step);
    bool executeToStep(int step);
    bool executeFull();
    void resetExecution();
    
    QString getCurrentStateString() const;
    QVector<QComplex> getCurrentStateVector() const;
    QVector<QVector<QComplex>> getCurrentDensityMatrix() const;
    double getQubitProbability(int qubit, int value) const;
    double getQubitExpectation(int qubit, const QString &observable) const;
    
    QVector<double> getQubitState(int qubit) const;  // [prob0, prob1, alpha, beta, theta, phi]
    QVector<int> measureQubit(int qubit, int count, const QString &basis = "Z") const;
    QVector<double> getQubitExpectationValues(int qubit) const;  // [X, Y, Z]
    double getQubitPurity(int qubit) const;
    double getQubitCoherence(int qubit) const;
    QMap<QString, double> getQubitEntanglementMeasures(int qubit) const;
    
    QVector<std::complex<double>> getStateVectorComplex() const;
    QVector<QVector<std::complex<double>>> getDensityMatrixComplex() const;
    double getStatePurity() const;
    double getStateFidelity(const QVector<std::complex<double>> &targetState) const;
    double getVonNeumannEntropy() const;
    double getStateCoherence() const;
    QMap<QString, double> getEntanglementMeasures() const;
    QVector<double> performStateTomography() const;
    QVector<double> getStateExpectationValues(const QString &observable) const;
    
    int getCircuitDepth() const;
    int getGateCount() const;
    int getTotalSteps() const;
    int getCurrentStep() const;
    QStringList getGateInfo(int step) const;
    
    bool addGate(const QString &gateType, const QVector<int> &qubits, 
                 const QVector<double> &parameters = QVector<double>());
    bool removeGate(int step);
    bool insertGate(int step, const QString &gateType, const QVector<int> &qubits,
                   const QVector<double> &parameters = QVector<double>());
    
    bool optimizeCircuit();
    QStringList getOptimizationSuggestions() const;
    
    QJsonObject exportCircuit() const;
    bool importCircuit(const QJsonObject &circuitData);
    
    QString getLastError() const;
    void clearError();

signals:
    void circuitChanged();
    void executionStepChanged(int step);
    void stateChanged();
    void qubitStateChanged(int qubit);
    void errorOccurred(const QString &error);

private:
    std::unique_ptr<omniq::QuantumCircuit> circuit_;
    std::unique_ptr<omniq::QuantumState> currentState_;
    
    int currentStep_;
    int totalSteps_;
    QString lastError_;
    bool isExecuting_;
    
    QMap<int, QVector<double>> qubitStates_;  // qubit -> [prob0, prob1, alpha, beta, theta, phi]
    QMap<int, QVector<int>> measurementResults_;  // qubit -> measurement results
    QMap<int, QVector<double>> expectationValues_;  // qubit -> [X, Y, Z]    
    QMap<QString, double> entanglementMeasures_;  // measure name -> value
    
    void updateStepCircuits();
    void updateCurrentState();
    void updateQubitStates();
    void updateEntanglementMeasures();
    bool validateQubitIndex(int qubit) const;
    bool validateStepIndex(int step) const;
    QString gateTypeToString(omniq::GateType type) const;
    omniq::GateType stringToGateType(const QString &type) const;
    
    QVector<double> calculateQubitState(int qubit) const;
    QVector<double> calculateExpectationValues(int qubit) const;
    double calculatePurity() const;
    double calculateFidelity(const QVector<std::complex<double>> &targetState) const;
    double calculateVonNeumannEntropy() const;
    double calculateCoherence() const;
    QMap<QString, double> calculateEntanglementMeasures() const;
    QVector<double> performTomography() const;
};

#endif
