//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef OMNIQ_CIRCUIT_H
#define OMNIQ_CIRCUIT_H

#include <Eigen/Dense>
#include <vector>
#include <string>

namespace omniq {

using Matrix2cd = Eigen::Matrix2cd;
using Matrix4cd = Eigen::Matrix4cd;
using MatrixXcd = Eigen::MatrixXcd;
using VectorXcd = Eigen::VectorXcd;

enum class GateType {
    H, X, Y, Z, CNOT, SWAP, PHASE, RX, RY, RZ, MEASURE
};

struct Gate {
    GateType type;
    std::vector<int> controlQubits;
    std::vector<int> targetQubits;
    std::vector<double> parameters;
};

class Circuit {
private:
    int numQubits_;
    int numClassicalBits_;
    std::vector<Gate> gates_;
    VectorXcd stateVector_;
    std::vector<int> classicalBits_;
    int currentStep_;

    // Helper methods
    void validateGate(const Gate& gate);
    void validateQubitIndex(int qubit);
    void validateClassicalBitIndex(int bit);
    MatrixXcd createSingleQubitGate(const Matrix2cd& gate, int qubit);
    MatrixXcd createTwoQubitGate(const Matrix4cd& gate, int qubit1, int qubit2);
    std::string gateToString(const Gate& gate) const;

public:
    // Constructors
    explicit Circuit(int numQubits, int numClassicalBits = 0);
    Circuit(const Circuit& other);
    Circuit& operator=(const Circuit& other);

    // Gate management
    void addGate(const Gate& gate);
    void addGate(GateType type, int targetQubit, double parameter = 0.0);
    void addGate(GateType type, int controlQubit, int targetQubit, double parameter = 0.0);
    void addGate(GateType type, const std::vector<int>& targetQubits, const std::vector<double>& parameters = {});
    void removeGate(int index);
    void insertGate(int index, const Gate& gate);
    void clear();

    // Circuit execution
    void reset();
    bool executeStep();
    void executeAll();
    void executeToStep(int step);

    // Gate application methods
    void applyGate(const Gate& gate);
    void applyHadamard(int qubit);
    void applyPauliX(int qubit);
    void applyPauliY(int qubit);
    void applyPauliZ(int qubit);
    void applyCNOT(int controlQubit, int targetQubit);
    void applySWAP(int qubit1, int qubit2);
    void applyPhaseShift(int qubit, double angle);
    void applyRotationX(int qubit, double angle);
    void applyRotationY(int qubit, double angle);
    void applyRotationZ(int qubit, double angle);
    void performMeasurement(int qubit, int classicalBit);

    // State access
    const VectorXcd& getStateVector() const { return stateVector_; }
    VectorXcd& getStateVector() { return stateVector_; }
    const std::vector<int>& getClassicalBits() const { return classicalBits_; }
    std::vector<int>& getClassicalBits() { return classicalBits_; }

    // Circuit information
    int getNumQubits() const { return numQubits_; }
    int getNumClassicalBits() const { return numClassicalBits_; }
    int getCurrentStep() const { return currentStep_; }
    int getTotalSteps() const { return static_cast<int>(gates_.size()); }
    const std::vector<Gate>& getGates() const { return gates_; }

    // Output
    std::string toQASM() const;

    // Utility methods
    double getQubitProbability(int qubit, int value) const;
    double getQubitExpectation(int qubit, const std::string& observable) const;
    MatrixXcd getDensityMatrix() const;
};

} // namespace omniq

#endif // OMNIQ_CIRCUIT_H 