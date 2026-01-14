// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef NETLINK_COMM_H
#define NETLINK_COMM_H

#include <linux/cn_proc.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>

// Define a local version of cn_msg without the flexible array member 'data[]'
// to allow embedding it in other structures.
struct cn_msg_hdr {
    struct cb_id id;
    __u32 seq;
    __u32 ack;
    __u16 len;
    __u16 flags;
};

// Forward declaration; ProcEvent is defined in ContextualClassifier.h
struct ProcEvent;

class NetLinkComm {
private:
	int32_t mNlSock;

public:
	NetLinkComm();
    ~NetLinkComm();

    int32_t connect();
    int32_t setListen(int8_t enable);
    int32_t getSocket() const;
    void closeSocket();

    // Receive a single proc connector event and fill ProcEvent.
    // Returns:
    //   CC_APP_OPEN  on EXEC
    //   CC_APP_CLOSE on EXIT
    //   0            on non-actionable events
    //   -1           on error
    int32_t recvEvent(ProcEvent &ev);
};

#endif // NETLINK_COMM_H
