
int netlink_init()
{

	struct rtnl_handle rth;
	struct {
		struct nlmsghdr n;
		struct rtmsh r;
		char buf[1024];
	} req;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
	req.n.nlmsg_type = RTM_NEWROUTE;

	req.r.rtm_family = AF_INET;
	req.r.rtm_table = RT_TABLE_MAIN;
	req.r.rtm_protocol = RTPROT_BOOT;
	req.r.rtm_scope = RT_SCOPE_UNIVERSE;
	req.r.rtm_type = RTN_UNICAST;

	u_int32_t dst;
	dst = inet_addr(destination);
	req.r.rtm_dst_len = bitmask;

	addattr_l(&req.n, sizeof(req), RTA_DST, &dst, 4);

	int idx;
	if ((idx = if_nametoindex(interface)) == 0) {
		/* XXX error */
		return -1;
	}

	addattr_l(&req.n, sizeof(req), RTA_OIF, &idx, 4);

	if (rtnl_open(&rth) < 0) {
		/* XXX error */
		return -1;
	}

	if (rtnl_talk(&rth, &req.n, 0, 0, NULL) < 0) {
		/* XXX error */
	}

	return 0;


}
