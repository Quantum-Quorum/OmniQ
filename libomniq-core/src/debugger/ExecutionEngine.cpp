//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/Debugger/ExecutionEngine.h"
#include <stdexcept>

namespace omniq {
namespace debugger {

ExecutionEngine::ExecutionEngine(const Circuit &circuit)
    : circuit_(circuit), currentState_(circuit.getNumQubits()), currentStep_(0),
      mode_(ExecutionMode::STEP) {
  // Save initial state
  stateHistory_.push_back(currentState_);
}

void ExecutionEngine::stepForward() {
  if (isFinished()) {
    throw std::runtime_error("Execution already finished");
  }

  // Apply next gate
  auto gates = circuit_.getGates();
  if (currentStep_ < static_cast<int>(gates.size())) {
    const auto &gate = gates[currentStep_];

    // Apply gate based on type
    switch (gate.type) {
    case GateType::H:
      currentState_.applyHadamard(gate.targetQubits[0]);
      break;
    case GateType::X:
      currentState_.applyPauliX(gate.targetQubits[0]);
      break;
    case GateType::Y:
      currentState_.applyPauliY(gate.targetQubits[0]);
      break;
    case GateType::Z:
      currentState_.applyPauliZ(gate.targetQubits[0]);
      break;
    case GateType::CNOT:
      currentState_.applyCNOT(gate.controlQubits[0], gate.targetQubits[0]);
      break;
    case GateType::PHASE:
      currentState_.applyPhaseShift(gate.targetQubits[0], gate.parameters[0]);
      break;
    case GateType::RX:
      currentState_.applyRotationX(gate.targetQubits[0], gate.parameters[0]);
      break;
    case GateType::RY:
      currentState_.applyRotationY(gate.targetQubits[0], gate.parameters[0]);
      break;
    case GateType::RZ:
      currentState_.applyRotationZ(gate.targetQubits[0], gate.parameters[0]);
      break;
    default:
      break;
    }

    currentStep_++;
    stateHistory_.push_back(currentState_);
  }
}

void ExecutionEngine::stepBackward() {
  if (currentStep_ <= 0) {
    throw std::runtime_error("Already at initial state");
  }

  currentStep_--;
  currentState_ = stateHistory_[currentStep_];
}

void ExecutionEngine::runToBreakpoint() {
  while (!isFinished() && !isAtBreakpoint()) {
    stepForward();
  }
}

void ExecutionEngine::runToEnd() {
  while (!isFinished()) {
    stepForward();
  }
}

void ExecutionEngine::reset() {
  currentStep_ = 0;
  currentState_ = Statevector(circuit_.getNumQubits());
  stateHistory_.clear();
  stateHistory_.push_back(currentState_);
}

void ExecutionEngine::addBreakpoint(int gateIndex) {
  Breakpoint bp;
  bp.gateIndex = gateIndex;
  bp.condition = nullptr;
  bp.description = "Gate " + std::to_string(gateIndex);
  breakpoints_.push_back(bp);
}

void ExecutionEngine::addConditionalBreakpoint(
    int gateIndex, std::function<bool(const Statevector &)> condition,
    const std::string &description) {
  Breakpoint bp;
  bp.gateIndex = gateIndex;
  bp.condition = condition;
  bp.description = description;
  breakpoints_.push_back(bp);
}

void ExecutionEngine::removeBreakpoint(int index) {
  if (index >= 0 && index < static_cast<int>(breakpoints_.size())) {
    breakpoints_.erase(breakpoints_.begin() + index);
  }
}

void ExecutionEngine::toggleBreakpoint(int index) {
  if (index >= 0 && index < static_cast<int>(breakpoints_.size())) {
    breakpoints_[index].enabled = !breakpoints_[index].enabled;
  }
}

void ExecutionEngine::clearBreakpoints() { breakpoints_.clear(); }

bool ExecutionEngine::isAtBreakpoint() const {
  return checkBreakpoint(currentStep_);
}

bool ExecutionEngine::checkBreakpoint(int step) const {
  for (const auto &bp : breakpoints_) {
    if (!bp.enabled)
      continue;

    if (bp.gateIndex == step) {
      if (bp.condition) {
        // Conditional breakpoint
        return bp.condition(currentState_);
      } else {
        // Unconditional breakpoint
        return true;
      }
    }
  }
  return false;
}

} // namespace debugger
} // namespace omniq
