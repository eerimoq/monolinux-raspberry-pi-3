/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, Erik Moqvist
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file is part of the Monolinux RPi3 project.
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/gpio.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "ml/ml.h"

struct folder_t {
    const char *path_p;
    int mode;
};

static void create_folders(void)
{
    static const struct folder_t folders[] = {
        { "/proc", 0644 },
        { "/sys", 0444 },
        { "/etc", 0644 }
    };
    int i;
    int res;

    for (i = 0; i < membersof(folders); i++) {
        res = mkdir(folders[i].path_p, folders[i].mode);

        if (res != 0) {
            fprintf(stderr, "Failed to create '%s'", folders[i].path_p);
        }
    }
}

static void create_files(void)
{
    ml_mount("none", "/proc", "proc", 0, NULL);
    ml_mount("none", "/sys", "sysfs", 0, NULL);

    ml_mknod("/dev/null", S_IFCHR | 0644, makedev(1, 3));
    ml_mknod("/dev/zero", S_IFCHR | 0644, makedev(1, 5));
    ml_mknod("/dev/urandom", S_IFCHR | 0644, makedev(1, 9));
    ml_mknod("/dev/kmsg", S_IFCHR | 0644, makedev(1, 11));
    ml_mknod("/dev/mmcblk0", S_IFBLK | 0644, makedev(179, 0));
    ml_mknod("/dev/mmcblk0p1", S_IFBLK | 0644, makedev(179, 1));
    ml_mknod("/dev/mmcblk0p2", S_IFBLK | 0644, makedev(179, 2));

    ml_file_write_string("/etc/resolv.conf", "nameserver 8.8.4.4\n");
}

/**
 * This should probably be moved to the DHCP client (or Monlinux C
 * Library network module).
 *
 * Right now this function waits for any new network link, not only
 * eth0.
 */
static void wait_for_eth0_up(void)
{
    struct sockaddr_nl addr;
    int sockfd;
    char buf[4096];
    struct nlmsghdr *nlh_p;
    int res;

    sockfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

    if (sockfd == -1) {
        perror("couldn't open NETLINK_ROUTE socket");

        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK;

    res = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    if (res == -1) {
        perror("couldn't bind");

        return;
    }

    nlh_p = (struct nlmsghdr *)buf;

    while (true) {
        res = read(sockfd, nlh_p, 4096);

        if (res <= 0) {
            break;
        }

        while ((NLMSG_OK(nlh_p, res)) && (nlh_p->nlmsg_type != NLMSG_DONE)) {
            if (nlh_p->nlmsg_type == RTM_NEWLINK) {
                close(sockfd);
            }

            nlh_p = NLMSG_NEXT(nlh_p, res);
        }
    }
}

int main()
{
    create_folders();
    create_files();
    ml_init();
    ml_print_uptime();
    ml_shell_init();
    ml_network_init();
    ml_shell_start();

# if 0
    ml_network_interface_configure("eth0",
                                   "192.168.0.100",
                                   "255.255.255.0",
                                   1500);
    ml_network_interface_add_route("eth0", "192.168.0.1");
#else
    struct ml_dhcp_client_t dhcp_client;

    wait_for_eth0_up();
    ml_network_interface_up("eth0");
    ml_dhcp_client_init(&dhcp_client, "eth0", ML_LOG_INFO);
    ml_dhcp_client_start(&dhcp_client);
#endif

    while (true) {
        sleep(10);
    }

    return (0);
}
