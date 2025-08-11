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
#include "../libomniq-core/include/omniq/Circuit.h"
#include "../libomniq-core/include/omniq/QuantumStates.h"

class CoreInterface : public QObject
{
    Q_OBJECT

public:
    explicit CoreInterface(QObject *parent = nullptr);
    ~CoreInterface();

    // Circuit management
    bool createCircuit(int numQubits, int numClassicalBits = 0);
    bool loadCircuit(const QString &filename);
    bool saveCircuit(const QString &filename);
    QString getCircuitQASM() const;
    
    // Circuit execution
    bool executeStep(int step);
    bool executeToStep(int step);
    bool executeFull();
    void resetExecution();
    
    // State inspection
    QString getCurrentStateString() const;
    QVector<QComplex> getCurrentStateVector() const;
    QVector<QVector<QComplex>> getCurrentDensityMatrix() const;
    double getQubitProbability(int qubit, int value) const;
    double getQubitExpectation(int qubit, const QString &observable) const;
    
    // Circuit analysis
    int getCircuitDepth() const;
    int getGateCount() const;
    int getTotalSteps() const;
    int getCurrentStep() const;
    QStringList getGateInfo(int step) const;
    
    // Circuit modification
    bool addGate(const QString &gateType, const QVector<int> &qubits, 
                 const QVector<double> &parameters = QVector<double>());
    bool removeGate(int step);
    bool insertGate(int step, const QString &gateType, const QVector<int> &qubits,
                   const QVector<double> &parameters = QVector<double>());
    
    // Optimization
    bool optimizeCircuit();
    QStringList getOptimizationSuggestions() const;
    
    // Export/Import
    QJsonObject exportCircuit() const;
    bool importCircuit(const QJsonObject &circuitData);
    
    // Error handling
    QString getLastError() const;
    void clearError();

signals:
    void circuitChanged();
    void executionStepChanged(int step);
    void stateChanged();
    void errorOccurred(const QString &error);

private:
    std::unique_ptr<omniq::Circuit> circuit_;
    std::unique_ptr<omniq::Statevector> currentState_;
    std::unique_ptr<omniq::DensityMatrix> currentDensityMatrix_;
    
    QVector<std::unique_ptr<omniq::Circuit>> stepCircuits_;
    QVector<std::unique_ptr<omniq::Statevector>> stepStates_;
    
    int currentStep_;
    int totalSteps_;
    QString lastError_;
    bool isExecuting_;
    
    // Helper methods
    void updateStepCircuits();
    void updateCurrentState();
    bool validateQubitIndex(int qubit) const;
    bool validateStepIndex(int step) const;
    QString gateTypeToString(omniq::GateType type) const;
    omniq::GateType stringToGateType(const QString &type) const;
};

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
    
private:
    double real_;
    double imag_;
};

#endif // COREINTERFACE_H
