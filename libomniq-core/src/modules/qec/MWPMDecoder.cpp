//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/QEC/MWPMDecoder.h"
#include <algorithm>
#include <cmath>

namespace omniq {
namespace qec {

MWPMDecoder::MWPMDecoder() {
  codeDistance_ = 3; // Default
}

std::vector<int> MWPMDecoder::decode(const Syndrome &syndrome) {
  auto violations = syndrome.getViolatedStabilizers();

  if (violations.empty()) {
    return {}; // No errors
  }

  // Find matching pairs
  auto matching = findMatching(violations);

  // Extract correction path (simplified - returns qubit indices)
  std::vector<int> correction;
  for (const auto &pair : matching) {
    // In a real implementation, trace path between matched stabilizers
    // For now, return the indices themselves as a placeholder
    correction.push_back(pair.first);
    correction.push_back(pair.second);
  }

  return correction;
}

std::vector<std::pair<int, int>>
MWPMDecoder::findMatching(const std::vector<int> &violations) {
  std::vector<std::pair<int, int>> matching;
  std::vector<bool> matched(violations.size(), false);

  // Greedy matching: pair each violation with nearest unmatched partner
  for (size_t i = 0; i < violations.size(); ++i) {
    if (matched[i])
      continue;

    int bestPartner = -1;
    double bestWeight = 1e9;

    for (size_t j = i + 1; j < violations.size(); ++j) {
      if (matched[j])
        continue;

      double weight = calculateWeight(violations[i], violations[j]);
      if (weight < bestWeight) {
        bestWeight = weight;
        bestPartner = j;
      }
    }

    if (bestPartner != -1) {
      matching.push_back({violations[i], violations[bestPartner]});
      matched[i] = true;
      matched[bestPartner] = true;
    }
  }

  return matching;
}

double MWPMDecoder::calculateWeight(int idx1, int idx2) const {
  // Manhattan distance (simplified - assumes 1D layout)
  // In real surface code, use 2D lattice positions
  return std::abs(idx1 - idx2);
}

} // namespace qec
} // namespace omniq
