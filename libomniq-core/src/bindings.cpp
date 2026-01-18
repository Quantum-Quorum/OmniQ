#include "omniq/Circuit.h"
#include "omniq/Statevector.h"
#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace omniq;

PYBIND11_MODULE(_omniq_core, m) {
  m.doc() = "OmniQ Core C++ extension"; // optional module docstring

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
      .value("MEASURE", GateType::MEASURE)
      .export_values();

  py::class_<Gate>(m, "Gate")
      .def(py::init<>())
      .def_readwrite("type", &Gate::type)
      .def_readwrite("control_qubits", &Gate::controlQubits)
      .def_readwrite("target_qubits", &Gate::targetQubits)
      .def_readwrite("parameters", &Gate::parameters);

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
      .def("get_state_vector", &Circuit::getStateVector,
           py::return_value_policy::reference_internal)
      .def("get_classical_bits", &Circuit::getClassicalBits)
      .def("num_qubits", &Circuit::getNumQubits)
      .def("num_classical_bits", &Circuit::getNumClassicalBits)
      .def("to_qasm", &Circuit::toQASM)
      .def("get_probability", &Circuit::getQubitProbability);

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
      .def("get_state_vector", &Statevector::getStateVector,
           py::return_value_policy::reference_internal)
      .def("get_num_qubits", &Statevector::getNumQubits)
      .def("normalize", &Statevector::normalize)
      .def("to_string", &Statevector::toString)
      .def("tensor_product", &Statevector::tensorProduct)
      .def("partial_trace", &Statevector::partialTrace)
      .def("get_qubit_probability", &Statevector::getQubitProbability);
}
