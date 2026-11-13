# OmniQ-OS: Real-Time Quantum Linux Architecture
## A Deterministic Operating System for Heterogeneous Tri-Compute (CPU + GPU + QPU)

---

## 1. Project Vision & Identity

* **Project Name**: `OmniQ-OS`
* **Sub-title**: Real-Time Quantum Linux
* **Core Paradigm**: The world's first open-source, deterministic, hard real-time Linux operating system built for heterogeneous **Tri-Compute (CPU + GPU + QPU)** systems.
* **Target Audience**: Quantum hardware research labs, national laboratories (ORNL, Fermilab, CERN), quantum cloud providers, and quantum software developers.
* **Official Brand Identity**: **Option A** (Minimalist Concentric Orbital "Q" in Electric Cyan `#00e5ff` on Matte Obsidian Black).

---

## 2. The Core Problem: Why Classical Operating Systems Fail Quantum Hardware

Today's quantum computers operate as primitive batch-queued appliances:
1. A developer writes a quantum circuit in Python.
2. The circuit is serialized into JSON and sent over HTTP to a cloud queue.
3. The server sits in a queue for minutes or hours.
4. The quantum hardware runs all measurement shots in a single static burst and returns counts.

### The Physics Wall
This workflow cannot support the next decade of quantum computing:
* **The Latency Trap**: Mid-circuit measurement, feed-forward branching, and active Quantum Error Correction (QEC) require real-time feedback within the qubit coherence window ($< 1\,\mu\text{s}$). An HTTP/TCP round trip ($> 10\,\text{ms}$) is fatal.
* **Scheduler Jitter**: Standard Linux schedulers (CFS / EEVDF) prioritize overall system throughput, introducing scheduling jitter of $10\text{--}50\,\text{ms}$. A background systemd daemon or cron interrupt can preempt pulse delivery, causing the quantum state to dephase into random thermal noise.
* **The No-Cloning Barrier**: In classical OS, preemption saves CPU register states to RAM. By the **Quantum No-Cloning Theorem**, an arbitrary unknown quantum state $|\psi\rangle$ cannot be copied or persisted to classical RAM. Quantum processes cannot be preemptively context-switched in time; they must be managed **spatially** across the chip.
* **The Tri-Processor Disconnect**: Modern quantum algorithms (VQE, QAOA, QML, tensor networks) require classical CPUs, GPUs (for gradient computation and neural decoding), and QPUs (for quantum state evolution). Today, communication between them is completely uncoordinated.

---

## 3. The Tri-Compute Architecture (CPU + GPU + QPU)

```
+---------------------------------------------------------------------------------------+
|                                    USER SPACE                                         |
|                                                                                       |
|   +-----------------------+   +------------------------+   +----------------------+   |
|   |       qpu-smi         |   |      libomniq-qpu      |   |  Hybrid AI / VQE App |   |
|   |  (System Management)  |   |     (POSIX C API)      |   | (PyTorch + cuQuantum)|   |
|   +-----------------------+   +------------------------+   +----------------------+   |
|                                            |                                          |
|                                            v                                          |
+---------------------------------------------------------------------------------------+
|                       OMNIQ-OS REAL-TIME KERNEL (PREEMPT_RT)                          |
|                                                                                       |
|   +-------------------------------------------------------------------------------+   |
|   |                     drivers/accel/qpu/ (Unified QPU Subsystem)                |   |
|   |                                                                               |   |
|   |   +--------------------------+             +------------------------------+   |   |
|   |   | Hardware Mode (PCIe/AXI) |             | Fallback Mode: Virt-QPU Sim  |   |   |
|   |   | • Xilinx RFSoC DMA       |   <=====>   | • Transparent emulation      |   |   |
|   |   | • Cryo-telemetry parser  |             | • Zero-config dev machine    |   |   |
|   |   +--------------------------+             +------------------------------+   |   |
|   |                                                                               |   |
|   |   +---------------------------------------+   +---------------------------+   |   |
|   |   |  GPUDirect-QPU Zero-Copy DMA Bridge   |   |  QMMU Spatial Allocator   |   |   |
|   |   |  (Direct PCIe P2P: GPU VRAM <-> QPU)  |   |  with Guard-Band Shields  |   |   |
|   |   +---------------------------------------+   +---------------------------+   |   |
|   +-------------------------------------------------------------------------------+   |
+---------------------------------------------------------------------------------------+
           |                                   |                              |
           v (PCIe Gen5)                       v (NVLink / PCIe)              v (PCIe / AXI)
     [ Classical CPU ]                  [ NVIDIA/AMD GPU ]             [ QPU Controller ]
```

### 3.1 Role of Each Processor
1. **CPU (Host Core)**:
   * Runs the Linux kernel, systemdaemons, POSIX filesystem, and high-level control-flow logic.
   * Manages user permissions, security rings, and quantum process tickets.
2. **GPU (NVIDIA / AMD)**:
   * Runs classical neural-network decoders (e.g. AI-driven Minimum-Weight Perfect Matching or Union-Find decoders for surface codes).
   * Calculates tensor-network contractions and gradient updates for hybrid algorithms (VQE / QAOA).
3. **QPU (Quantum Accelerator)**:
   * Synthesizes and delivers high-precision analog microwave pulses (4--8 GHz) or optical pulses to physical qubits.
   * Collects dispersive readout resonator voltages via ultra-high-speed ADCs.

### 3.2 GPUDirect-QPU: Sub-Microsecond Zero-Copy Bridge
* In current systems, quantum syndrome data passes:
  $$\text{QPU} \to \text{FPGA} \to \text{PCIe} \to \text{CPU Host Memory} \to \text{Userspace} \to \text{CUDA Driver} \to \text{GPU HBM}$$
  Latency: **$5\text{--}20\,\text{ms}$** (Physical qubits decohere in $\sim 100\,\mu\text{s}$).
* **OmniQ-OS Solution**:
  * Implement **GPUDirect-QPU DMA** using Linux `dma-buf` and PCIe Peer-to-Peer (P2P) BAR mapping.
  * The FPGA controller writes syndrome measurement bitstreams **directly into GPU High-Bandwidth Memory (HBM)** over the PCIe bus without touching CPU RAM.
  * The GPU Tensor Cores evaluate the error syndrome in **$< 200\,\text{ns}$**.
  * The GPU streams Pauli frame corrections directly back to the FPGA pulse controller.
  * **Total loop latency: $< 600\,\text{ns}$**, completing well within the physical qubit coherence window!

---

## 4. Hardware Discovery, Boot Sequence & Graceful Fallback

OmniQ-OS must boot on **any** machine without crashing or panicking if a cryogenic refrigerator is not attached.

```
+------------------------------------------------------------------------+
|                          System Power-On / Boot                        |
+------------------------------------------------------------------------+
                                    |
                                    v
+------------------------------------------------------------------------+
|                 drivers/accel/qpu: PCIe Bus Enumeration                 |
+------------------------------------------------------------------------+
                                    |
                 +------------------+------------------+
                 |                                     |
                 v                                     v
       [ Physical QPU Found ]                [ No QPU Hardware Found ]
       (e.g., Xilinx RFSoC 0x10ee)                     |
                 |                                     |
        +--------+--------+                            |
        |                 |                            |
        v                 v                            v
[ Cryo Temp < 50 mK ] [ Cryo Temp > 50 mK ]   [ Developer Workstation ]
[ Normal Operation  ] [ Maintenance Mode  ]   [ Cloud Virtual Machine ]
        |                 |                            |
        v                 v                            v
  /dev/accel/qpu0   /dev/accel/qpu0             /dev/accel/qpu0
 [HARDWARE ONLINE] [OFFLINE - WARM]           [SIMULATOR / EMULATION]
```

### 4.1 The Three Operating States
1. **State A: Physical Hardware Online**:
   * FPGA detected on PCIe bus.
   * Cryogenic sensor telemetry confirms mixing chamber temperature $< 50\,\text{mK}$ (e.g. $14.2\,\text{mK}$).
   * QMMU registers physical qubit count and coupling map.
   * Node: `/dev/accel/qpu0` in `HARDWARE` mode.
2. **State B: Physical Hardware Present but Warm (Maintenance)**:
   * FPGA board detected, but cryostat is warm ($> 50\,\text{mK}$, e.g. room temperature or $4.2\,\text{K}$).
   * Kernel logs warning: `[WARNING] QPU detected but temperature exceeds threshold. Marked offline.`
   * Node created in read-only maintenance mode; execution falls back to simulation.
3. **State C: No Physical QPU Present (Developer Laptop / Workstation)**:
   * No FPGA board on PCIe bus.
   * Kernel does **not** fail boot.
   * Logs: `[INFO] No physical QPU detected. Initializing OmniQ Software Co-Processor.`
   * Node: `/dev/accel/qpu0` backed transparently by `libomniq-core` statevector / density matrix engine.
   * **Result**: Software written for OmniQ-OS runs identically in lab production or on a developer's laptop!

---

## 5. System Management Interface: `qpu-smi`

Just as `nvidia-smi` is the universal tool for GPU clusters, `qpu-smi` is the administrative interface for quantum systems.

### 5.1 Interactive Dashboard View
```text
$ qpu-smi
+-----------------------------------------------------------------------------+
| OmniQ QPU System Management Interface                     Driver: 0.1.0-rt |
| QPU 0: OmniQ-Transmon-127     Temp: 14.25 mK     Status: ONLINE (Hardware)  |
| Technology: Superconducting    Frequency: 4.8-5.3 GHz    Couplers: 144 Edges|
|=============================================================================|
| Physical Qubit Allocation Matrix:                                           |
| [AAAAA.......BBBBBBBBGGGG....................CCCCCCCC.....................] |
| • Allocated: 46 Qubits   • Guard Bands: 14 Qubits   • Free: 67 Qubits       |
| • Global Avg T1: 142.4 us    • Global Avg T2: 108.6 us                      |
|-----------------------------------------------------------------------------|
| Active Multi-Tenant Quantum Partitions:                                     |
| PID    User       Partition  Virtual Qubits  Phys Range  Fidelity   Status  |
| 1042   quantum1   #1         5 Qubits        p[0..4]     99.71%     RUNNING |
| 1088   alice      #2         8 Qubits        p[12..19]   99.42%     IDLE    |
| 1102   bob        #3         8 Qubits        p[35..42]   99.55%     RUNNING |
+-----------------------------------------------------------------------------+
```

### 5.2 Command Line Options
* `qpu-smi`: Live terminal dashboard.
* `qpu-smi --query-qubits`: Exports full calibration table ($T_1$, $T_2$, 1Q/2Q gate fidelities, frequencies, defect flags).
* `qpu-smi --query-cryo`: Reads cryostat temperature stages (50K, 4K, Still, Cold Plate, Mixing Chamber).
* `qpu-smi --partitions`: Shows spatial partition geometry and guard-band isolation boundaries.
* `qpu-smi --reset <qubit_id>`: Dispatches active microwave resonator reset pulses to return qubit to $|0\rangle$.
* `qpu-smi --format=json`: Machine-readable telemetry for Grafana / Prometheus monitoring.

---

## 6. Kernel Subsystem Specification (`drivers/accel/qpu/`)

### 6.1 UAPI Header (`include/uapi/linux/qpu.h`)
Standardized kernel IOCTL interface:
* `QPU_IOCTL_QUERY_TOPOLOGY`: Returns physical qubit count, coupler adjacency list, technology type.
* `QPU_IOCTL_QUERY_CALIBRATION`: Returns live per-qubit $T_1$, $T_2$, and gate error rates.
* `QPU_IOCTL_ALLOC_QUBITS`: QMMU virtual partition request with isolation and fidelity flags.
* `QPU_IOCTL_FREE_QUBITS`: Partition teardown, active state reset, and guard-band release.
* `QPU_IOCTL_SUBMIT_JOB`: Submits compiled OpenQASM 3.0 / QIR bytecode or pulse schedule.
* `QPU_IOCTL_READ_RESULTS`: Reads measurement bitstrings and count histograms.

### 6.2 Quantum Memory Management Unit (QMMU)
* **Virtual-to-Physical Translation**: Evaluates interaction graphs and maps logical qubits to the highest-fidelity contiguous physical subgraph.
* **Spatial Multi-Tenancy**: Slices the physical processor so multiple independent user processes run simultaneously without pulse cross-talk.
* **Crosstalk Guard Bands**: Automatically locks perimeter qubits around an active partition into a detuned idle state to prevent Stark shifts and capacitive leakage.
* **Bad-Qubit Page Faults**: Emits kernel interrupt if physical qubit coherence drops unexpectedly, triggering dynamic remapping to reserve qubits.

### 6.3 Dynamic Decoupling Engine (`q-keepalived`)
* Background kernel thread monitors scheduled pulse timing.
* If a qubit is scheduled to sit idle for $\Delta t > \tau_{\text{threshold}}$, the kernel automatically injects Carr-Purcell-Meiboom-Gill (CPMG) or XY-8 refocusing pulse trains.
* Extends effective $T_2$ coherence by up to $10\times$ transparently.

---

## 7. The Base Operating System & Distribution Plan

### 7.1 Foundation: Ubuntu 24.04 LTS + `PREEMPT_RT`
* Base: Ubuntu 24.04 LTS (Noble Numbat) for package stability and hardware compatibility.
* Kernel: Official Linux kernel patched with `CONFIG_PREEMPT_RT=y`:
  * All interrupt handlers run as schedulable threads.
  * Spinlocks converted to sleepable mutexes.
  * Real-time priority inheritance enabled.
  * Guaranteed worst-case interrupt response latency: **$< 5\,\mu\text{s}$**.
* Tuning kernel boot arguments:
  * `isolcpus=2-15`: Dedicates CPU cores exclusively to QPU pulse timing without OS interference.
  * `nohz_full=2-15`: Eliminates periodic timer ticks on isolated cores.
  * `intel_idle.max_cstate=0`: Disables CPU power saving states to prevent wake-up latency.

### 7.2 Bootloader & Distro Packaging
* Toolchain: Ubuntu `live-build` / `debootstrap`.
* GRUB bootloader with official OmniQ-OS Option A splash screen.
* Boot menu options:
  1. `OmniQ-OS (Real-Time Quantum Linux - Hardware Auto-Detect)`
  2. `OmniQ-OS (Force Simulator Fallback Mode)`
  3. `OmniQ-OS (Recovery & Cryo Diagnostics Mode)`

---

## 8. Project Separation & Repository Structure

To maintain clean separation between the simulator and the operating system:

```
OmniQ/ (Existing Repository)
├── libomniq-core/        # Pure C++ quantum simulator & mathematical engine
├── libomniq-c/           # C bindings
├── omniq-debugger/       # Desktop IDE, 3D Bloch sphere, Entanglement viewer
└── omniq-python/         # Python SDK (pip install omniq)

omniqos-linux/ (New Dedicated Repository)
├── kernel/
│   ├── drivers/accel/qpu/ # Native Linux QPU kernel driver
│   ├── patches/           # PREEMPT_RT and GPUDirect DMA patches
│   └── include/uapi/      # include/uapi/linux/qpu.h
├── tools/
│   ├── qpu-smi/           # C/Rust System Management Interface
│   └── omniqd/            # User-space quantum daemon & QMMU manager
├── runtime/
│   ├── libomniq-qpu/      # POSIX C client library
│   └── python/            # omniqos Python bindings
├── distro/
│   ├── live-build/        # Ubuntu 24.04 LTS ISO generator
│   └── branding/          # Option A logo, Plymouth boot splash, GRUB themes
└── docs/
    └── RFC_LKML_QPU.md    # Proposal for upstreaming drivers/accel/qpu to LKML
```
