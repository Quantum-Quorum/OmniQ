//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_QEC_SYNDROME_H
#define OMNIQ_QEC_SYNDROME_H

#include <string>
#include <vector>

namespace omniq {
namespace qec {

/**
 * @brief Represents syndrome measurement outcomes from stabilizer measurements
 */
class Syndrome {
public:
  Syndrome();
  explicit Syndrome(int numStabilizers);

  // Accessors
  int size() const { return measurements_.size(); }
  int getMeasurement(int index) const;
  const std::vector<int> &getMeasurements() const { return measurements_; }

  // Mutators
  void setMeasurement(int index, int value); // value should be +1 or -1
  void setMeasurements(const std::vector<int> &measurements);

  // Analysis
  int countViolations() const; // Count of -1 measurements
  std::vector<int>
  getViolatedStabilizers() const; // Indices where measurement = -1
  bool isAllZero() const;         // Check if all stabilizers are satisfied (+1)

  // Metadata
  int getCodeDistance() const { return codeDistance_; }
  void setCodeDistance(int distance) { codeDistance_ = distance; }

  std::string toString() const;

private:
  std::vector<int> measurements_; // +1 (satisfied) or -1 (violated)
  int codeDistance_;
};

} // namespace qec
} // namespace omniq

#endif // OMNIQ_QEC_SYNDROME_H
