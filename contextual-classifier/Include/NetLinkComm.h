#ifndef NETLINK_COMM_H
#define NETLINK_COMM_H

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
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

class NetLinkComm {
public:
    NetLinkComm();
    ~NetLinkComm();
    int connect();
    int set_listen(bool enable);
    int get_socket() const;
    void close_socket();

private:
    int nl_sock;
};

#endif // NETLINK_COMM_H
