//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/QEC/UnionFindDecoder.h"
#include <algorithm>
#include <set>

namespace omniq {
namespace qec {

// Union-Find implementation
UnionFindDecoder::UnionFind::UnionFind(int n) : parent_(n), rank_(n, 0) {
  for (int i = 0; i < n; ++i) {
    parent_[i] = i;
  }
}

int UnionFindDecoder::UnionFind::find(int x) {
  if (parent_[x] != x) {
    parent_[x] = find(parent_[x]); // Path compression
  }
  return parent_[x];
}

void UnionFindDecoder::UnionFind::unite(int x, int y) {
  int rootX = find(x);
  int rootY = find(y);

  if (rootX == rootY)
    return;

  // Union by rank
  if (rank_[rootX] < rank_[rootY]) {
    parent_[rootX] = rootY;
  } else if (rank_[rootX] > rank_[rootY]) {
    parent_[rootY] = rootX;
  } else {
    parent_[rootY] = rootX;
    rank_[rootX]++;
  }
}

bool UnionFindDecoder::UnionFind::connected(int x, int y) {
  return find(x) == find(y);
}

// Decoder implementation
UnionFindDecoder::UnionFindDecoder() {
  codeDistance_ = 3; // Default
}

std::vector<int> UnionFindDecoder::decode(const Syndrome &syndrome) {
  auto violations = syndrome.getViolatedStabilizers();

  if (violations.empty()) {
    return {}; // No errors
  }

  // Create Union-Find structure
  int n = syndrome.size();
  UnionFind uf(n);

  // Grow clusters by uniting nearby violations
  for (size_t i = 0; i < violations.size(); ++i) {
    for (size_t j = i + 1; j < violations.size(); ++j) {
      int dist = std::abs(violations[i] - violations[j]);
      // Unite if close enough (simplified heuristic)
      if (dist <= codeDistance_) {
        uf.unite(violations[i], violations[j]);
      }
    }
  }

  // Extract correction from clusters
  return extractCorrection(uf, violations);
}

std::vector<int>
UnionFindDecoder::extractCorrection(UnionFind &uf,
                                    const std::vector<int> &violations) {
  // Simplified: return cluster representatives
  std::vector<int> correction;
  std::set<int> roots;

  for (int v : violations) {
    int root = uf.find(v);
    if (roots.find(root) == roots.end()) {
      roots.insert(root);
      correction.push_back(root);
    }
  }

  return correction;
}

} // namespace qec
} // namespace omniq
