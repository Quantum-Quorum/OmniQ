#include "omniq/Circuit.h"
#include "omniq/DensityMatrix.h"
#include "omniq/Grovers.h"
#include "omniq/QPE.h"
#include "omniq/QuantumStates.h"
#include "omniq/Statevector.h"
#include "omniq/QuditStatevector.h"
#include "omniq/Spectral.h"
#include "omniq/Simulators/CliffordSimulator.h"
#include "omniq/QEC/SurfaceCode.h"
#include "omniq/QEC/Syndrome.h"
#include "omniq/QEC/Decoder.h"
#include "omniq/QEC/MWPMDecoder.h"
#include "omniq/QEC/UnionFindDecoder.h"
#include "omniq/Noise/NoiseChannel.h"
#include "omniq/Noise/DepolarizingChannel.h"
#include "omniq/Noise/AmplitudeDampingChannel.h"
#include "omniq/Noise/PhaseDampingChannel.h"
#include "omniq/Noise/DualRailDampingChannel.h"
#include "omniq/Noise/NoiseModel.h"

#include <pybind11/eigen.h>
#include <pybind11/functional.h>
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
           static_cast<void (Circuit::*)(GateType, int, double)>(&Circuit::addGate),
           py::arg("type"), py::arg("target_qubit"), py::arg("parameter") = 0.0)
      .def("add_gate",
           static_cast<void (Circuit::*)(GateType, int, int, double)>(&Circuit::addGate),
           py::arg("type"), py::arg("control_qubit"), py::arg("target_qubit"),
           py::arg("parameter") = 0.0)
      .def("add_gate",
           static_cast<void (Circuit::*)(GateType, const std::vector<int> &,
                                         const std::vector<double> &)>(&Circuit::addGate),
           py::arg("type"), py::arg("target_qubits"),
           py::arg("parameters") = std::vector<double>{})
      // Compatibility camelCase aliases
      .def("addGate",
           static_cast<void (Circuit::*)(GateType, int, double)>(&Circuit::addGate),
           py::arg("type"), py::arg("target_qubit"), py::arg("parameter") = 0.0)
      .def("addGate",
           static_cast<void (Circuit::*)(GateType, int, int, double)>(&Circuit::addGate),
           py::arg("type"), py::arg("control_qubit"), py::arg("target_qubit"),
           py::arg("parameter") = 0.0)
      .def("addGate",
           [](Circuit &c, GateType type, const std::vector<int> &controls,
              const std::vector<int> &targets, const std::vector<double> &params) {
             double p = params.empty() ? 0.0 : params[0];
             if (!controls.empty() && !targets.empty()) {
               c.addGate(type, controls[0], targets[0], p);
             } else if (!targets.empty()) {
               c.addGate(type, targets[0], p);
             }
           },
           py::arg("type"), py::arg("controls"), py::arg("targets"),
           py::arg("parameters") = std::vector<double>{})
      .def("remove_gate", &Circuit::removeGate)
      .def("clear", &Circuit::clear)
      .def("reset", &Circuit::reset)
      .def("execute_step", &Circuit::executeStep)
      .def("execute_all", &Circuit::executeAll)
      .def("execute", [](Circuit &c) {
        c.executeAll();
        return Statevector(c.getStateVector());
      })
      .def("executeCircuit", [](Circuit &c) {
        c.executeAll();
        return Statevector(c.getStateVector());
      })
      .def("get_state_vector",
           static_cast<const VectorXcd &(Circuit::*)() const>(&Circuit::getStateVector),
           py::return_value_policy::reference_internal)
      .def("getStateVector",
           static_cast<const VectorXcd &(Circuit::*)() const>(&Circuit::getStateVector),
           py::return_value_policy::reference_internal)
      .def("get_classical_bits",
           static_cast<const std::vector<int> &(Circuit::*)() const>(
               &Circuit::getClassicalBits))
      .def("num_qubits", &Circuit::getNumQubits)
      .def("getNumQubits", &Circuit::getNumQubits)
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
      .def("applyH", &Statevector::applyHadamard)
      .def("applyCNOT", &Statevector::applyCNOT)
      .def("measure", &Statevector::measure)
      .def("measure_expectation", &Statevector::measureExpectation)
      .def("get_state_vector",
           static_cast<const VectorXcd &(Statevector::*)() const>(
               &Statevector::getStateVector),
           py::return_value_policy::reference_internal)
      .def("getStateVector",
           static_cast<const VectorXcd &(Statevector::*)() const>(
               &Statevector::getStateVector),
           py::return_value_policy::reference_internal)
      .def("get_num_qubits", &Statevector::getNumQubits)
      .def("getNumQubits", &Statevector::getNumQubits)
      .def("normalize", &Statevector::normalize)
      .def("to_string", &Statevector::toString)
      .def("tensor_product", &Statevector::tensorProduct)
      .def("partial_trace", &Statevector::partialTrace)
      .def("get_qubit_probability", &Statevector::getQubitProbability);

  // QuditStatevector Class
  py::class_<QuditStatevector>(m, "QuditStatevector")
      .def(py::init<int, int>(), py::arg("num_qudits"), py::arg("dimension"))
      .def(py::init<int, int, const VectorXcd &>(), py::arg("num_qudits"),
           py::arg("dimension"), py::arg("amplitudes"))
      .def("apply_aharonov_casher_rz", &QuditStatevector::applyAharonovCasherRZ,
           py::arg("qudit"), py::arg("theta"))
      .def("apply_csum", &QuditStatevector::applyCSUM, py::arg("control_qudit"),
           py::arg("target_qudit"))
      .def("apply_controlled_phase", &QuditStatevector::applyControlledPhase,
           py::arg("control_qudit"), py::arg("target_qudit"), py::arg("phase"))
      .def("apply_custom_single_op", &QuditStatevector::applyCustomSingleQuditOp,
           py::arg("qudit"), py::arg("op"))
      .def("get_qudit_state_probability", &QuditStatevector::getQuditStateProbability,
           py::arg("qudit"), py::arg("state_level"))
      .def("measure", &QuditStatevector::measure, py::arg("qudit"))
      .def("normalize", &QuditStatevector::normalize)
      .def("get_norm", &QuditStatevector::getNorm)
      .def("to_string", &QuditStatevector::toString)
      .def("get_num_qudits", &QuditStatevector::getNumQudits)
      .def("get_dimension", &QuditStatevector::getDimension)
      .def("get_total_dimension", &QuditStatevector::getTotalDimension)
      .def("get_state_vector",
           static_cast<const VectorXcd &(QuditStatevector::*)() const>(
               &QuditStatevector::getStateVector),
           py::return_value_policy::reference_internal);

  // DualRailDampingChannel Class
  py::class_<omniq::noise::DualRailDampingChannel>(m, "DualRailDampingChannel")
      .def(py::init<double>(), py::arg("gamma"))
      .def_static("from_t1", &omniq::noise::DualRailDampingChannel::fromT1,
                  py::arg("t1"), py::arg("gate_time"))
      .def("apply", &omniq::noise::DualRailDampingChannel::apply)
      .def("get_kraus_operators", &omniq::noise::DualRailDampingChannel::getKrausOperators)
      .def("get_name", &omniq::noise::DualRailDampingChannel::getName)
      .def("get_description", &omniq::noise::DualRailDampingChannel::getDescription)
      .def("get_gamma", &omniq::noise::DualRailDampingChannel::getGamma)
      .def("set_gamma", &omniq::noise::DualRailDampingChannel::setGamma);

  // DensityMatrix Class
  py::class_<DensityMatrix>(m, "DensityMatrix")
      .def(py::init<int>())
      .def(py::init<const MatrixXcd &>())
      .def(py::init<const Statevector &>())
      .def("get_matrix",
           static_cast<const MatrixXcd &(DensityMatrix::*)() const>(
               &DensityMatrix::getDensityMatrix),
           py::return_value_policy::reference_internal)
      .def("getDensityMatrix",
           static_cast<const MatrixXcd &(DensityMatrix::*)() const>(
               &DensityMatrix::getDensityMatrix),
           py::return_value_policy::reference_internal)
      .def("get_num_qubits", &DensityMatrix::getNumQubits)
      .def("partial_trace", &DensityMatrix::partialTrace)
      .def("is_pure", &DensityMatrix::isPure)
      .def("purity", &DensityMatrix::getPurity)
      .def("von_neumann_entropy", &DensityMatrix::getVonNeumannEntropy);

  // CliffordSimulator
  py::class_<omniq::simulators::CliffordSimulator>(m, "CliffordSimulator")
      .def(py::init<int>(), py::arg("num_qubits"))
      .def("applyH", &omniq::simulators::CliffordSimulator::applyH, py::arg("qubit"))
      .def("applyS", &omniq::simulators::CliffordSimulator::applyS, py::arg("qubit"))
      .def("applySdag", &omniq::simulators::CliffordSimulator::applySdag, py::arg("qubit"))
      .def("applyCNOT", &omniq::simulators::CliffordSimulator::applyCNOT,
           py::arg("control"), py::arg("target"))
      .def("applyCZ", &omniq::simulators::CliffordSimulator::applyCZ,
           py::arg("q1"), py::arg("q2"))
      .def("applyX", &omniq::simulators::CliffordSimulator::applyX, py::arg("qubit"))
      .def("applyY", &omniq::simulators::CliffordSimulator::applyY, py::arg("qubit"))
      .def("applyZ", &omniq::simulators::CliffordSimulator::applyZ, py::arg("qubit"))
      .def("measure", &omniq::simulators::CliffordSimulator::measure, py::arg("qubit"))
      .def("getNumQubits", &omniq::simulators::CliffordSimulator::getNumQubits)
      .def("isPureState", &omniq::simulators::CliffordSimulator::isPureState)
      .def("reset", &omniq::simulators::CliffordSimulator::reset);

  // SurfaceCode & QEC
  py::class_<omniq::qec::SurfaceCode>(m, "SurfaceCode")
      .def(py::init<int>(), py::arg("distance"))
      .def("getDistance", &omniq::qec::SurfaceCode::getDistance)
      .def("getNumDataQubits", &omniq::qec::SurfaceCode::getNumDataQubits)
      .def("getNumAncillaQubits", &omniq::qec::SurfaceCode::getNumAncillaQubits)
      .def("getTotalQubits", &omniq::qec::SurfaceCode::getTotalQubits)
      .def("prepareLogicalZero", &omniq::qec::SurfaceCode::prepareLogicalZero);

  py::class_<omniq::qec::Syndrome>(m, "Syndrome")
      .def(py::init<>())
      .def(py::init<int>(), py::arg("numStabilizers"))
      .def("size", &omniq::qec::Syndrome::size)
      .def("getMeasurement", &omniq::qec::Syndrome::getMeasurement)
      .def("setMeasurement", &omniq::qec::Syndrome::setMeasurement)
      .def("countViolations", &omniq::qec::Syndrome::countViolations)
      .def("isAllZero", &omniq::qec::Syndrome::isAllZero);

  py::class_<omniq::qec::Decoder, std::shared_ptr<omniq::qec::Decoder>>(m, "Decoder")
      .def("getName", &omniq::qec::Decoder::getName)
      .def("getDescription", &omniq::qec::Decoder::getDescription);

  py::class_<omniq::qec::MWPMDecoder, omniq::qec::Decoder,
             std::shared_ptr<omniq::qec::MWPMDecoder>>(m, "MWPMDecoder")
      .def(py::init<>())
      .def("getName", &omniq::qec::MWPMDecoder::getName)
      .def("getDescription", &omniq::qec::MWPMDecoder::getDescription);

  py::class_<omniq::qec::UnionFindDecoder, omniq::qec::Decoder,
             std::shared_ptr<omniq::qec::UnionFindDecoder>>(m, "UnionFindDecoder")
      .def(py::init<>())
      .def("getName", &omniq::qec::UnionFindDecoder::getName)
      .def("getDescription", &omniq::qec::UnionFindDecoder::getDescription);

  // Noise Channels & NoiseModel
  py::class_<omniq::noise::NoiseChannel, std::shared_ptr<omniq::noise::NoiseChannel>>(
      m, "NoiseChannel")
      .def("getName", &omniq::noise::NoiseChannel::getName)
      .def("getDescription", &omniq::noise::NoiseChannel::getDescription);

  py::class_<omniq::noise::DepolarizingChannel, omniq::noise::NoiseChannel,
             std::shared_ptr<omniq::noise::DepolarizingChannel>>(m, "DepolarizingChannel")
      .def(py::init<double>(), py::arg("p"))
      .def("getName", &omniq::noise::DepolarizingChannel::getName)
      .def("getDescription", &omniq::noise::DepolarizingChannel::getDescription)
      .def("getErrorProbability",
           &omniq::noise::DepolarizingChannel::getErrorProbability);

  py::class_<omniq::noise::AmplitudeDampingChannel, omniq::noise::NoiseChannel,
             std::shared_ptr<omniq::noise::AmplitudeDampingChannel>>(
      m, "AmplitudeDampingChannel")
      .def(py::init<double>(), py::arg("gamma"))
      .def_static("fromT1", &omniq::noise::AmplitudeDampingChannel::fromT1,
                  py::arg("t1"), py::arg("gate_time"))
      .def("getName", &omniq::noise::AmplitudeDampingChannel::getName)
      .def("getDescription", &omniq::noise::AmplitudeDampingChannel::getDescription)
      .def("getGamma", &omniq::noise::AmplitudeDampingChannel::getGamma);

  py::class_<omniq::noise::PhaseDampingChannel, omniq::noise::NoiseChannel,
             std::shared_ptr<omniq::noise::PhaseDampingChannel>>(m, "PhaseDampingChannel")
      .def(py::init<double>(), py::arg("lambda"))
      .def_static("fromT2", &omniq::noise::PhaseDampingChannel::fromT2,
                  py::arg("t2"), py::arg("gate_time"))
      .def("getName", &omniq::noise::PhaseDampingChannel::getName)
      .def("getDescription", &omniq::noise::PhaseDampingChannel::getDescription)
      .def("getLambda", &omniq::noise::PhaseDampingChannel::getLambda);

  py::class_<omniq::noise::NoiseModel>(m, "NoiseModel")
      .def(py::init<>())
      .def_static("createTypicalModel", &omniq::noise::NoiseModel::createTypicalModel)
      .def_static("createIdealModel", &omniq::noise::NoiseModel::createIdealModel)
      .def_static("createNoisyModel", &omniq::noise::NoiseModel::createNoisyModel)
      .def("toString", &omniq::noise::NoiseModel::toString)
      .def("isEnabled", &omniq::noise::NoiseModel::isEnabled)
      .def("setEnabled", &omniq::noise::NoiseModel::setEnabled);

  // SpectralAnalysis
  py::class_<omniq::SpectralAnalysis::EigenDecomposition>(m, "EigenDecomposition")
      .def(py::init<>())
      .def_property_readonly("eigenvalues",
          [](const omniq::SpectralAnalysis::EigenDecomposition &ed) -> Eigen::VectorXd {
            return ed.eigenvalues;
          })
      .def_property_readonly("eigenvectors",
          [](const omniq::SpectralAnalysis::EigenDecomposition &ed) -> Eigen::MatrixXcd {
            return ed.eigenvectors;
          });

  py::class_<omniq::SpectralAnalysis>(m, "SpectralAnalysis")
      .def_static("computeEigendecomposition",
                  &omniq::SpectralAnalysis::computeEigendecomposition,
                  py::arg("matrix"))
      .def_static("calculateNegativity",
                  &omniq::SpectralAnalysis::calculateNegativity,
                  py::arg("rho"), py::arg("systemADim"), py::arg("systemBDim"));

  // Algorithms - QPE
  py::class_<QPE>(m, "QPE")
      .def(py::init<int, int, UnitaryOperator>())
      .def("execute", &QPE::execute, py::arg("initial_state") = Statevector(1))
      .def("execute_with_measurements", &QPE::execute_with_measurements,
           py::arg("num_shots") = 1000);

  // Algorithms - Grovers
  py::class_<GroversAlgorithm>(m, "GroversAlgorithm")
      .def(py::init<int, OracleFunction, int>(), py::arg("num_qubits"),
           py::arg("oracle"), py::arg("num_solutions") = 1)
      .def("execute", &GroversAlgorithm::execute,
           py::arg("initial_state") = Statevector(1))
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
  m.def("calculateVonNeumannEntropy",
        [](const DensityMatrix &dm) { return dm.getVonNeumannEntropy(); },
        "Calculate Von Neumann entropy of DensityMatrix");
  m.def("calculateVonNeumannEntropy",
        [](const MatrixXcd &mat) { return calculateVonNeumannEntropy(mat); },
        "Calculate Von Neumann entropy of MatrixXcd");
  m.def("calculate_concurrence", &calculateConcurrence,
        "Calculate concurrence for 2-qubit state");
  m.def("calculateConcurrence",
        [](const Statevector &sv) {
          DensityMatrix dm(sv);
          return calculateConcurrence(dm.getDensityMatrix());
        },
        "Calculate concurrence for 2-qubit statevector");
  m.def("calculateConcurrence",
        [](const DensityMatrix &dm) {
          return calculateConcurrence(dm.getDensityMatrix());
        },
        "Calculate concurrence for 2-qubit density matrix");
  m.def("calculateConcurrence",
        [](const MatrixXcd &mat) { return calculateConcurrence(mat); },
        "Calculate concurrence for 2-qubit matrix");
  m.def("calculate_negativity", &calculateNegativity,
        "Calculate negativity metric");
}
