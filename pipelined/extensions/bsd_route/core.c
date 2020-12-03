union sockunion {

        struct sockaddr sa;
        struct sockaddr_in sin;

} so_dst, so_mask, so_gate;


struct {
        struct rt_msghdr m_rtm;
        char m_space[512];
} m_rtmsg;


#define ROUNDUP(a) \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

#define NEXTADDR(w, u) \
        if (rtm_addrs & (w)) { \
                sz = ROUNDUP(u.sa.sa_len); memmove(cp, &(u), sz); cp += sz; \
        }

int add_route(u_int32_t subnet, u_int32_t subnetmask, u_int32_t gateway)
{

        char *cp = m_rtmsg.m_space;
        int nbytes = 0;
        int sz = 0;

        int rtm_addrs = RTA_DST | RTA_NETMASK | RTA_GATEWAY;

        int sock = 0;

        errno = 0;

        memset(&m_rtmsg, 0, sizeof(m_rtmsg));


        so_dst.sa.sa_len = sizeof(so_dst);
        so_dst.sa.sa_family = AF_INET;
        so_dst.sin.sin_addr.s_addr = subnet;

        syslog(LOG_ALERT, "subnet: %s\n", inet_ntoa(so_dst.sin.sin_addr));

        so_mask.sa.sa_len = sizeof(so_mask);
        so_mask.sa.sa_family = AF_INET;
        so_mask.sin.sin_addr.s_addr = subnetmask;

        syslog(LOG_ALERT, "subnetmask: %s\n", inet_ntoa(so_mask.sin.sin_addr));

        so_gate.sa.sa_len = sizeof(so_gate);
        so_gate.sa.sa_family = AF_INET;
        so_gate.sin.sin_addr.s_addr = gateway;

        syslog(LOG_ALERT, "gateway: %s\n", inet_ntoa(so_gate.sin.sin_addr));


#define rtm m_rtmsg.m_rtm
        rtm.rtm_type = RTM_ADD;
        rtm.rtm_flags = RTF_UP | RTF_GATEWAY;
        rtm.rtm_version = RTM_VERSION;
        rtm.rtm_addrs = rtm_addrs;

        NEXTADDR(RTA_DST, so_dst);
        NEXTADDR(RTA_GATEWAY, so_gate);
        NEXTADDR(RTA_NETMASK, so_mask);

        rtm.rtm_msglen = cp - (char *)&m_rtmsg;

        sock = socket(PF_ROUTE, SOCK_RAW, 0);
        if ((nbytes = write(sock, (char *)&m_rtmsg, rtm.rtm_msglen)) < 0) {
                syslog(LOG_ALERT, "writing to routing socket: %s", strerror(errno));
                return -1;
        }

        if (nbytes < 1) {
                syslog(LOG_ALERT, "write to routing socket, got %i for nbytes", nbytes);
                return -1;
        }

        close(sock);
        return 0;
}


