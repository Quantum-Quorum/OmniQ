#include "omniq/Circuit.h"
#include "omniq/DensityMatrix.h"
#include "omniq/Grovers.h"
#include "omniq/QPE.h"
#include "omniq/QuantumStates.h"
#include "omniq/Statevector.h"
#include <pybind11/eigen.h>
#include <pybind11/functional.h> // For std::function bindings
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace omniq;

PYBIND11_MODULE(_omniq_core, m) {
  m.doc() = "OmniQ Core C++ extension";

  // GateType Enum
  py::enum_<GateType>(m, "GateType")
      .value("H", GateType::H)
      .value("X", GateType::X)
      .value("Y", GateType::Y)
      .value("Z", GateType::Z)
      .value("CNOT", GateType::CNOT)
      .value("SWAP", GateType::SWAP)
      .value("PHASE", GateType::PHASE)
      .value("RX", GateType::RX)
      .value("RY", GateType::RY)
      .value("RZ", GateType::RZ)
      .value("CP", GateType::CP)
      .value("MEASURE", GateType::MEASURE)
      .value("CUSTOM", GateType::CUSTOM)
      .export_values();

  // Gate Struct
  py::class_<Gate>(m, "Gate")
      .def(py::init<>())
      .def_readwrite("type", &Gate::type)
      .def_readwrite("control_qubits", &Gate::controlQubits)
      .def_readwrite("target_qubits", &Gate::targetQubits)
      .def_readwrite("parameters", &Gate::parameters);

  // Circuit Class
  py::class_<Circuit>(m, "Circuit")
      .def(py::init<int, int>(), py::arg("num_qubits"),
           py::arg("num_classical_bits") = 0)
      .def("add_gate",
           static_cast<void (Circuit::*)(GateType, int, double)>(
               &Circuit::addGate),
           py::arg("type"), py::arg("target_qubit"), py::arg("parameter") = 0.0)
      .def("add_gate",
           static_cast<void (Circuit::*)(GateType, int, int, double)>(
               &Circuit::addGate),
           py::arg("type"), py::arg("control_qubit"), py::arg("target_qubit"),
           py::arg("parameter") = 0.0)
      .def("add_gate",
           static_cast<void (Circuit::*)(GateType, const std::vector<int> &,
                                         const std::vector<double> &)>(
               &Circuit::addGate),
           py::arg("type"), py::arg("target_qubits"),
           py::arg("parameters") = std::vector<double>{})
      .def("remove_gate", &Circuit::removeGate)
      .def("clear", &Circuit::clear)
      .def("reset", &Circuit::reset)
      .def("execute_step", &Circuit::executeStep)
      .def("execute_all", &Circuit::executeAll)
      .def("get_state_vector",
           static_cast<const VectorXcd &(Circuit::*)() const>(
               &Circuit::getStateVector),
           py::return_value_policy::reference_internal)
      .def("get_classical_bits",
           static_cast<const std::vector<int> &(Circuit::*)() const>(
               &Circuit::getClassicalBits))
      .def("num_qubits", &Circuit::getNumQubits)
      .def("num_classical_bits", &Circuit::getNumClassicalBits)
      .def("to_qasm", &Circuit::toQASM)
      .def("get_probability", &Circuit::getQubitProbability)
      .def("get_density_matrix",
           [](const Circuit &c) { return c.getDensityMatrix(); });

  // Statevector Class
  py::class_<Statevector>(m, "Statevector")
      .def(py::init<int>())
      .def(py::init<const VectorXcd &>())
      .def("apply_hadamard", &Statevector::applyHadamard)
      .def("apply_cnot", &Statevector::applyCNOT)
      .def("apply_pauli_x", &Statevector::applyPauliX)
      .def("apply_pauli_y", &Statevector::applyPauliY)
      .def("apply_pauli_z", &Statevector::applyPauliZ)
      .def("apply_phase_shift", &Statevector::applyPhaseShift)
      .def("apply_rotation_x", &Statevector::applyRotationX)
      .def("apply_rotation_y", &Statevector::applyRotationY)
      .def("apply_rotation_z", &Statevector::applyRotationZ)
      .def("measure", &Statevector::measure)
      .def("measure_expectation", &Statevector::measureExpectation)
      .def("get_state_vector",
           static_cast<const VectorXcd &(Statevector::*)() const>(
               &Statevector::getStateVector),
           py::return_value_policy::reference_internal)
      .def("get_num_qubits", &Statevector::getNumQubits)
      .def("normalize", &Statevector::normalize)
      .def("to_string", &Statevector::toString)
      .def("tensor_product", &Statevector::tensorProduct)
      .def("partial_trace", &Statevector::partialTrace)
      .def("get_qubit_probability", &Statevector::getQubitProbability);

  // DensityMatrix Class
  py::class_<DensityMatrix>(m, "DensityMatrix")
      .def(py::init<int>())
      .def(py::init<const MatrixXcd &>())
      .def(py::init<const Statevector &>())
      .def("get_matrix",
           static_cast<const MatrixXcd &(DensityMatrix::*)() const>(
               &DensityMatrix::getDensityMatrix),
           py::return_value_policy::reference_internal)
      .def("get_num_qubits", &DensityMatrix::getNumQubits)
      // .def("apply_gate", &DensityMatrix::applyGate) // Not in header
      // .def("apply_channel", &DensityMatrix::applyChannel) // Not in header
      .def("partial_trace", &DensityMatrix::partialTrace)
      .def("is_pure", &DensityMatrix::isPure)
      .def("purity", &DensityMatrix::getPurity)
      // .def("fidelity", &DensityMatrix::fidelity) // Not in header
      .def("von_neumann_entropy", &DensityMatrix::getVonNeumannEntropy);

  // Algorithms - QPE
  py::class_<QPE>(m, "QPE")
      .def(py::init<int, int, UnitaryOperator>())
      .def("execute", &QPE::execute, py::arg("initial_state") = Statevector(0))
      .def("execute_with_measurements", &QPE::execute_with_measurements,
           py::arg("num_shots") = 1000);

  // Algorithms - Grovers
  py::class_<GroversAlgorithm>(m, "GroversAlgorithm")
      .def(py::init<int, OracleFunction, int>(), py::arg("num_qubits"),
           py::arg("oracle"), py::arg("num_solutions") = 1)
      .def("execute", &GroversAlgorithm::execute,
           py::arg("initial_state") = Statevector(0))
      .def("execute_with_measurements",
           &GroversAlgorithm::execute_with_measurements,
           py::arg("num_shots") = 1000)
      .def("get_success_probability",
           &GroversAlgorithm::get_success_probability);

  // Quantum States Math Functions
  m.def("calculate_purity", &calculatePurity,
        "Calculate purity of a density matrix");
  m.def("calculate_fidelity",
        static_cast<double (*)(const MatrixXcd &, const MatrixXcd &)>(
            &calculateFidelity),
        "Calculate fidelity between two density matrices");
  m.def("calculate_von_neumann_entropy", &calculateVonNeumannEntropy,
        "Calculate Von Neumann entropy");
  m.def("calculate_concurrence", &calculateConcurrence,
        "Calculate concurrence for 2-qubit state");
  m.def("calculate_negativity", &calculateNegativity,
        "Calculate negativity metric");
}
