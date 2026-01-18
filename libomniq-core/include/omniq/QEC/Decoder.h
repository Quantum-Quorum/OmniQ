//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_QEC_DECODER_H
#define OMNIQ_QEC_DECODER_H

#include "omniq/QEC/Syndrome.h"
#include <memory>
#include <string>
#include <vector>

namespace omniq {
namespace qec {

/**
 * @brief Abstract base class for error correction decoders
 */
class Decoder {
public:
  virtual ~Decoder() = default;

  /**
   * @brief Decode syndrome and return error chain
   * @param syndrome The measured syndrome
   * @return Vector of qubit indices indicating where to apply corrections
   */
  virtual std::vector<int> decode(const Syndrome &syndrome) = 0;

  /**
   * @brief Get decoder name for identification
   */
  virtual std::string getName() const = 0;

  /**
   * @brief Get decoder description
   */
  virtual std::string getDescription() const { return getName(); }

  /**
   * @brief Set code distance for decoder
   */
  virtual void setCodeDistance(int distance) { codeDistance_ = distance; }

  int getCodeDistance() const { return codeDistance_; }

protected:
  int codeDistance_ = 3;
};

} // namespace qec
} // namespace omniq

#endif // OMNIQ_QEC_DECODER_H
