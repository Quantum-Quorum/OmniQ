//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_DEBUGGER_EXECUTIONENGINE_H
#define OMNIQ_DEBUGGER_EXECUTIONENGINE_H

#include "omniq/Circuit.h"
#include "omniq/Statevector.h"
#include <functional>
#include <memory>
#include <vector>

namespace omniq {
namespace debugger {

enum class ExecutionMode {
  RUN,  // Run to completion or next breakpoint
  STEP, // Execute one gate at a time
  PAUSE // Paused execution
};

struct Breakpoint {
  int gateIndex;
  std::function<bool(const Statevector &)> condition;
  std::string description;
  bool enabled = true;
};

/**
 * @brief Step-by-step circuit execution engine with breakpoint support
 */
class ExecutionEngine {
public:
  explicit ExecutionEngine(const Circuit &circuit);

  // Execution control
  void stepForward();
  void stepBackward();
  void runToBreakpoint();
  void runToEnd();
  void reset();

  // Breakpoints
  void addBreakpoint(int gateIndex);
  void
  addConditionalBreakpoint(int gateIndex,
                           std::function<bool(const Statevector &)> condition,
                           const std::string &description);
  void removeBreakpoint(int index);
  void toggleBreakpoint(int index);
  void clearBreakpoints();

  // State access
  const Statevector &getCurrentState() const { return currentState_; }
  const std::vector<Statevector> &getStateHistory() const {
    return stateHistory_;
  }
  int getCurrentStep() const { return currentStep_; }
  int getTotalSteps() const {
    return static_cast<int>(circuit_.getGates().size());
  }

  // Execution mode
  void setMode(ExecutionMode mode) { mode_ = mode; }
  ExecutionMode getMode() const { return mode_; }

  bool isAtBreakpoint() const;
  bool isFinished() const { return currentStep_ >= getTotalSteps(); }

  const std::vector<Breakpoint> &getBreakpoints() const { return breakpoints_; }

private:
  Circuit circuit_;
  Statevector currentState_;
  std::vector<Statevector> stateHistory_;
  int currentStep_;
  ExecutionMode mode_;
  std::vector<Breakpoint> breakpoints_;

  bool checkBreakpoint(int step) const;
};

} // namespace debugger
} // namespace omniq

#endif // OMNIQ_DEBUGGER_EXECUTIONENGINE_H
