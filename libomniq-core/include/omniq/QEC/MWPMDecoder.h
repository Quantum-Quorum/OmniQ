//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_QEC_MWPMDECODER_H
#define OMNIQ_QEC_MWPMDECODER_H

#include "omniq/QEC/Decoder.h"
#include <map>
#include <vector>

namespace omniq {
namespace qec {

/**
 * @brief Minimum Weight Perfect Matching decoder
 *
 * Decodes surface code syndromes by finding minimum-weight matching
 * of violated stabilizers. This version uses a greedy approximation
 * for simplicity. For optimal results, integrate Blossom V library.
 */
class MWPMDecoder : public Decoder {
public:
  MWPMDecoder();

  std::vector<int> decode(const Syndrome &syndrome) override;
  std::string getName() const override { return "MWPM"; }

  std::string getDescription() const override {
    return "Minimum Weight Perfect Matching (Greedy)";
  }

private:
  struct Edge {
    int node1;
    int node2;
    double weight;

    bool operator<(const Edge &other) const { return weight < other.weight; }
  };

  // Greedy matching algorithm
  std::vector<std::pair<int, int>>
  findMatching(const std::vector<int> &violations);

  // Manhattan distance for weight calculation
  double calculateWeight(int idx1, int idx2) const;
};

} // namespace qec
} // namespace omniq

#endif // OMNIQ_QEC_MWPMDECODER_H
