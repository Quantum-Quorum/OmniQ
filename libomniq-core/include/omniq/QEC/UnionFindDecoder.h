//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_QEC_UNIONFINDDECODER_H
#define OMNIQ_QEC_UNIONFINDDECODER_H

#include "omniq/QEC/Decoder.h"
#include <vector>

namespace omniq {
namespace qec {

/**
 * @brief Union-Find decoder for surface codes
 *
 * Fast near-optimal decoder using Union-Find data structure.
 * O(n α(n)) complexity vs O(n³) for MWPM.
 */
class UnionFindDecoder : public Decoder {
public:
  UnionFindDecoder();

  std::vector<int> decode(const Syndrome &syndrome) override;
  std::string getName() const override { return "Union-Find"; }

  std::string getDescription() const override {
    return "Union-Find Decoder (Fast, Near-Optimal)";
  }

private:
  // Union-Find data structure
  class UnionFind {
  public:
    explicit UnionFind(int n);
    int find(int x);
    void unite(int x, int y);
    bool connected(int x, int y);

  private:
    std::vector<int> parent_;
    std::vector<int> rank_;
  };

  struct Cluster {
    std::vector<int> nodes;
    int boundary = 0;
  };

  std::vector<int> extractCorrection(UnionFind &uf,
                                     const std::vector<int> &violations);
};

} // namespace qec
} // namespace omniq

#endif // OMNIQ_QEC_UNIONFINDDECODER_H
