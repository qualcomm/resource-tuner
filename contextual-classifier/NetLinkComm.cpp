// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "NetLinkComm.h"
#include "Logger.h"
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdarg>

#define CLASSIFIER_TAG "NetLinkComm"

static std::string format_string(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

NetLinkComm::NetLinkComm() : nl_sock(-1) {}

NetLinkComm::~NetLinkComm() {
    close_socket();
}

void NetLinkComm::close_socket() {
    if (nl_sock != -1) {
        close(nl_sock);
        nl_sock = -1;
    }
}

int NetLinkComm::get_socket() const {
    return nl_sock;
}

int NetLinkComm::connect() {
    int rc;
    struct sockaddr_nl sa_nl;

    nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if (nl_sock == -1) {
        LOGE(CLASSIFIER_TAG, format_string("socket: %s", strerror(errno)));
        return -1;
    }

    sa_nl.nl_family = AF_NETLINK;
    sa_nl.nl_groups = CN_IDX_PROC;
    sa_nl.nl_pid = getpid();

    rc = bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl));
    if (rc == -1) {
        LOGE(CLASSIFIER_TAG, format_string("bind: %s", strerror(errno)));
        close_socket();
        return -1;
    }

    return nl_sock;
}

int NetLinkComm::set_listen(bool enable) {
    int rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg_hdr cn_msg;
            enum proc_cn_mcast_op cn_mcast;
        };
    } nlcn_msg;

    memset(&nlcn_msg, 0, sizeof(nlcn_msg));
    nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
    nlcn_msg.nl_hdr.nlmsg_pid = getpid();
    nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

    nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
    nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
    nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);

    nlcn_msg.cn_mcast = enable ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

    rc = send(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
    if (rc == -1) {
        LOGE(CLASSIFIER_TAG, format_string("netlink send: %s", strerror(errno)));
        return -1;
    }

    return 0;
}
