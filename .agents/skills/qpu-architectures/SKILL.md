---
name: qpu-architectures
description: Architectural patterns, hardware interfaces, and kernel subsystems for Quantum Processing Units (QPUs), FPGA PCIe endpoints, and heterogeneous Tri-Compute (CPU+GPU+QPU) systems. Use when designing, debugging, or implementing quantum operating systems, low-level QPU drivers, or hybrid quantum-classical control planes.
---

# Quantum Processing Unit (QPU) Systems Architecture

## 1. The Core Physics & Control Stack

A Quantum Processing Unit (QPU) is not a von Neumann CPU. It cannot execute arbitrary instruction loops, store persistent memory across boots, or branch without state collapse. It functions as an **analog, phase-coherent microwave/optical accelerator**.

### Physical Modalities & Control Frequencies
* **Superconducting Transmons**: Operated via 4–8 GHz microwave pulses inside a dilution refrigerator at 10–20 mK. Qubit coherence lifetimes ($T_1, T_2$) are $\sim 50\text{--}200\,\mu\text{s}$.
* **Trapped Ions / Neutral Atoms**: Operated via optical laser pulses (UV/visible/IR) and AOM/EOM modulators. Coherence times can reach seconds, but 2-qubit gate times are slower ($\sim 10\text{--}100\,\mu\text{s}$).
* **Magnon Dual-Rail / Cavity Systems**: Hybrid microwave-magnonic architectures utilizing ferromagnetic spin-wave excitations.

---

## 2. Hardware Endpoint Architecture (PCIe & FPGA)

Physical QPUs connect to classical host servers via high-speed FPGA controllers:

```
+-------------------------------------------------------------------------+
|                  Classical Host Server (Linux PREEMPT_RT)               |
+-------------------------------------------------------------------------+
                                    |
                                    v (PCIe Gen4/Gen5 x16 Bus)
+-------------------------------------------------------------------------+
|                 FPGA Controller (e.g. AMD/Xilinx RFSoC)                 |
|   • BAR0/BAR1 MMIO Registers                                            |
|   • Scatter-Gather DMA Engines (XDMA / QDMA)                            |
|   • UltraRAM / BRAM Waveform Buffers                                    |
|   • Multi-Gigasample/sec DACs (Pulse Generation) & ADCs (Readout)       |
+-------------------------------------------------------------------------+
                                    |
                                    v (Semi-rigid Coaxial Cryogenic Lines)
+-------------------------------------------------------------------------+
|                 Cryogenic Refrigerator (Dilution Fridge)                |
|   • 50 K Stage -> 4 K Stage -> Still (800 mK) -> Mixing Chamber (14 mK) |
|   • Physical Qubit Chip                                                 |
+-------------------------------------------------------------------------+
```

### Key Principles for Kernel Driver Design (`drivers/accel/qpu/`):
1. **PCIe Probe & Enumeration**: Driver scans for the FPGA PCIe Device ID (e.g., `0x10ee` for Xilinx).
2. **Cryogenic Guard Rails**: Before driving physical microwave pulses, verify cryostat telemetry (mixing chamber temperature $< 50\,\text{mK}$). If warm, driver must enter maintenance mode.
3. **Graceful Degradation / Software Fallback**: If physical QPU hardware is absent (e.g. developer laptop or CI runner), the driver MUST NOT fail or panic. It falls back transparently to software co-processor emulation (`libomniq-core`).

---

## 3. Real-Time Determinism (`CONFIG_PREEMPT_RT`)

Standard Linux CFS/EEVDF introduces 10–50 ms of scheduling jitter. Because physical transmons dephase in microseconds, **hard real-time determinism is mandatory**:

* **Kernel Configuration**: Must run on a `PREEMPT_RT` patched kernel.
* **CPU Core Isolation**: Use kernel boot parameters `isolcpus=2-N` and `nohz_full=2-N` to dedicate specific CPU cores solely to QPU pulse sequencing and readout processing without OS interrupt interference.
* **Latency Invariant**: Worst-case interrupt response latency must remain $\le 5\,\mu\text{s}$.

---

## 4. The Heterogeneous Tri-Compute Engine (CPU + GPU + QPU)

High-performance quantum computing requires three specialized compute engines acting as one:

1. **CPU**: High-level control flow, POSIX system calls, process isolation.
2. **GPU**: Tensor Core neural-network decoders (for surface codes) and classical gradient optimizers (for VQE/QAOA).
3. **QPU**: Coherent quantum state evolution and measurement.

### GPUDirect-QPU Zero-Copy Bridge:
* Never bounce quantum syndrome data through CPU host memory.
* Use Linux `dma-buf` and PCIe Peer-to-Peer (P2P) BAR mapping to stream readout ADC bitstreams directly into GPU High-Bandwidth Memory (HBM).
* Enables active Quantum Error Correction (QEC) feedback loops to complete in **$< 600\,\text{ns}$**.

---

## 5. Quantum Memory Management (QMMU) & Multi-Tenancy

Because of the **Quantum No-Cloning Theorem**, quantum states cannot be swapped to disk or persisted in classical RAM:

1. **Spatial Multi-Tenancy**: Multiple tenant processes slice the physical chip graph simultaneously (e.g. Process A gets Q0–Q4, Process B gets Q12–Q19).
2. **Crosstalk Guard Bands**: The QMMU automatically detunes and reserves a buffer ring of unallocated physical qubits around each active partition to eliminate capacitive and AC Stark shift leakage.
3. **Dynamic Decoupling Daemon**: If a qubit sits idle during another partition's circuit execution, the driver automatically applies CPMG/XY-8 refocusing pulse trains to prevent environmental dephasing.

---

## 6. Standard User-Space Interfaces

* **Device Nodes**: `/dev/accel/qpuX` (character device) and `/dev/accel/qpuX_dma` (DMA ring).
* **System Telemetry**: `/sys/class/accel/qpuX/` exposing physical topology, $T_1/T_2$ times, and cryogenic temperatures.
* **Management Utility**: `qpu-smi` providing real-time ASCII dashboards, per-qubit health matrices, and partition inspection.
