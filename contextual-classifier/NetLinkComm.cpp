// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <unistd.h>

#include "Logger.h"
#include "AuxRoutines.h"
#include "NetLinkComm.h"
#include "ContextualClassifier.h"

#define CLASSIFIER_TAG "NetLinkComm"

static std::string format_string(const char *fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

NetLinkComm::NetLinkComm() {
    this->mNlSock = -1;
}

NetLinkComm::~NetLinkComm() {
    this->closeSocket();
}

void NetLinkComm::closeSocket() {
    if(this->mNlSock != -1) {
        close(this->mNlSock);
        this->mNlSock = -1;
    }
}

int32_t NetLinkComm::getSocket() const {
    return this->mNlSock;
}

int32_t NetLinkComm::connect() {
    struct sockaddr_nl sa_nl;

    this->mNlSock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if(this->mNlSock == -1) {
        TYPELOGV(ERRNO_LOG, "socket", strerror(errno));
        return -1;
    }

    sa_nl.nl_family = AF_NETLINK;
    sa_nl.nl_groups = CN_IDX_PROC;
    sa_nl.nl_pid = getpid();

    if((bind(this->mNlSock, (struct sockaddr *)&sa_nl, sizeof(sa_nl))) == -1) {
        TYPELOGV(ERRNO_LOG, "bind", strerror(errno));
        this->closeSocket();
        return -1;
    }

    return this->mNlSock;
}

int32_t NetLinkComm::setListen(int8_t enable) {
    struct __attribute__((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__((__packed__)) {
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

    if(send(mNlSock, &nlcn_msg, sizeof(nlcn_msg), 0) == -1) {
        TYPELOGV(ERRNO_LOG, "netlink send", strerror(errno));
        return -1;
    }

    return 0;
}


static int8_t fetchPpid(pid_t tgid, pid_t& targetPpid) {
    std::ostringstream path;
    path << "/proc/" << tgid << "/status";
    std::ifstream f(path.str());
    if (!f.is_open()) return false;

    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("PPid:", 0) == 0) {
            std::istringstream iss(line.substr(5));
            iss >> targetPpid;
            return !iss.fail();
        }
    }
    return false;
}

int32_t NetLinkComm::recvEvent(ProcEvent &ev) {
    int32_t rc = 0;
    struct __attribute__((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__((__packed__)) {
            struct cn_msg_hdr cn_msg;
            struct proc_event proc_ev;
        };
    } nlcn_msg;

    rc = recv(mNlSock, &nlcn_msg, sizeof(nlcn_msg), 0);
    if(rc == 0) {
        // Socket shutdown or no more data.
        return 0;
    }
    if(rc == -1) {
        if (errno == EINTR) {
            // Caller (ContextualClassifier::HandleProcEv) will handle EINTR.
            return -1;
        }
        TYPELOGV(ERRNO_LOG, "netlink recv", strerror(errno));
        return -1;
    }

    ev.pid = -1;
    ev.tgid = -1;
    ev.type = CC_IGNORE;

    LOGE(CLASSIFIER_TAG, "Incoming proc event");

    pid_t ppid;
    switch(nlcn_msg.proc_ev.what) {
        case PROC_EVENT_NONE:
        case PROC_EVENT_FORK:
        case PROC_EVENT_UID:
        case PROC_EVENT_GID: {
            // No actionable item.
            break;
        }

        case PROC_EVENT_EXEC:
            ev.pid = nlcn_msg.proc_ev.event_data.exec.process_pid;
            ev.tgid = nlcn_msg.proc_ev.event_data.exec.process_tgid;
            ev.type = CC_APP_OPEN;

            // if(!fetchPpid(ev.tgid, ppid)) {
            //     return CC_IGNORE;
            // }

            // if(ppid <= 50) {
            //     return CC_IGNORE;
            // }

            rc = CC_APP_OPEN;

            if(AuxRoutines::fileExists(COMM(ev.pid))) {
                std::string procComm = AuxRoutines::readFromFile(COMM(ev.pid));
                LOGE(CLASSIFIER_TAG, "Incoming proc [exec] comm: " + procComm);
            } else {
                rc = ev.type = CC_IGNORE;
            }
            break;

        case PROC_EVENT_EXIT:
            ev.pid = nlcn_msg.proc_ev.event_data.exit.process_pid;
            ev.tgid = nlcn_msg.proc_ev.event_data.exit.process_tgid;
            ev.type = CC_APP_CLOSE;

            // if(!fetchPpid(ev.tgid, ppid)) {
            //     return CC_IGNORE;
            // }

            // if(ppid <= 50) {
            //     return CC_IGNORE;
            // }

            rc = CC_APP_CLOSE;

            if(AuxRoutines::fileExists(COMM(ev.pid))) {
                std::string procComm = AuxRoutines::readFromFile(COMM(ev.pid));
                LOGE(CLASSIFIER_TAG, "Incoming proc [exit] comm: " + procComm);
            } else {
                rc = ev.type = CC_IGNORE;
            }
            break;

        default:
            break;
    }

    return rc;
}
