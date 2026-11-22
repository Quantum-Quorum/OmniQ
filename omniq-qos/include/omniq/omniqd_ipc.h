/*
 * OmniQ-QOS: IPC Protocol & Header Framing for QPU Daemon
 * include/omniq/omniqd_ipc.h
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#ifndef _OMNIQ_OMNIQD_IPC_H
#define _OMNIQ_OMNIQD_IPC_H

#include "../uapi/linux/qpu.h"
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#define OMNIQ_DEFAULT_SOCKET_PATH "/tmp/omniq_qpu0.sock"
#define OMNIQ_IPC_MAGIC           0x51495043 /* "QIPC" */

enum omniq_ipc_msg_type : uint32_t {
    OMNIQ_MSG_QUERY_TOPOLOGY    = 0x01,
    OMNIQ_MSG_QUERY_CALIBRATION = 0x02,
    OMNIQ_MSG_ALLOC_QUBITS      = 0x03,
    OMNIQ_MSG_FREE_QUBITS       = 0x04,
    OMNIQ_MSG_SUBMIT_JOB        = 0x05,
    OMNIQ_MSG_READ_RESULTS      = 0x06,
    OMNIQ_MSG_QUERY_PARTITIONS  = 0x07,
    OMNIQ_MSG_QUERY_CRYO        = 0x08,
    OMNIQ_MSG_RESET_QUBIT       = 0x09,
    OMNIQ_MSG_PING              = 0x0A,
};

#pragma pack(push, 1)
struct omniq_ipc_msg_hdr {
    uint32_t magic;         /* OMNIQ_IPC_MAGIC */
    uint32_t msg_type;      /* enum omniq_ipc_msg_type */
    int32_t  status;        /* 0 on success, negative errno on error */
    uint32_t client_pid;    /* PID of client process */
    uint32_t payload_len;   /* Length of binary payload following header */
    uint32_t reserved;      /* Alignment padding */
};
#pragma pack(pop)

inline bool omniq_ipc_send_all(int fd, const void *buf, size_t len) {
    if (len == 0) return true;
    const uint8_t *ptr = static_cast<const uint8_t*>(buf);
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t n = write(fd, ptr, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        ptr += n;
        remaining -= n;
    }
    return true;
}

inline bool omniq_ipc_recv_all(int fd, void *buf, size_t len) {
    if (len == 0) return true;
    uint8_t *ptr = static_cast<uint8_t*>(buf);
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t n = read(fd, ptr, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) return false; // EOF: peer closed
        ptr += n;
        remaining -= n;
    }
    return true;
}

#endif /* _OMNIQ_OMNIQD_IPC_H */
