#include "omniq/Circuit.h"
#include "omniq/Statevector.h"
#include <gtest/gtest.h>

TEST(CoreTest, CircuitCreation) {
  omniq::Circuit circuit(2);
  EXPECT_EQ(circuit.getNumQubits(), 2);
  EXPECT_EQ(circuit.getTotalSteps(), 0);
}

TEST(CoreTest, StatevectorNormalization) {
  omniq::Statevector sv(2);
  EXPECT_NEAR(sv.getNorm(), 1.0, 1e-6);
}
