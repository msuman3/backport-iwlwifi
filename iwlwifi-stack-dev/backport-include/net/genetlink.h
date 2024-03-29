#ifndef __BACKPORT_NET_GENETLINK_H
#define __BACKPORT_NET_GENETLINK_H
#include_next <net/genetlink.h>
#include <linux/version.h>

static inline void __bp_genl_info_userhdr_set(struct genl_info *info,
					      void *userhdr)
{
	info->userhdr = userhdr;
}

static inline void *__bp_genl_info_userhdr(struct genl_info *info)
{
	return info->userhdr;
}

#if LINUX_VERSION_IS_LESS(4,12,0)
#define GENL_SET_ERR_MSG(info, msg) NL_SET_ERR_MSG(genl_info_extack(info), msg)

static inline int genl_err_attr(struct genl_info *info, int err,
				struct nlattr *attr)
{
	return err;
}
#endif /* < 4.12 */

/* this is for patches we apply */
static inline struct netlink_ext_ack *genl_info_extack(struct genl_info *info)
{
#if LINUX_VERSION_IS_GEQ(4,12,0) && \
	RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(7,6)
	return info->extack;
#else
	return info->userhdr;
#endif
}

/* this gets put in place of info->userhdr, since we use that above */
static inline void *genl_info_userhdr(struct genl_info *info)
{
	return (u8 *)info->genlhdr + GENL_HDRLEN;
}

/* this is for patches we apply */
#if LINUX_VERSION_IS_LESS(3,7,0)
#define genl_info_snd_portid(__genl_info) (__genl_info->snd_pid)
#else
#define genl_info_snd_portid(__genl_info) (__genl_info->snd_portid)
#endif

#if LINUX_VERSION_IS_LESS(3,13,0) && \
	RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(7,6)
#define __genl_const
#else /* < 3.13 */
#define __genl_const const
#endif /* < 3.13 */

#ifndef GENLMSG_DEFAULT_SIZE
#define GENLMSG_DEFAULT_SIZE (NLMSG_DEFAULT_SIZE - GENL_HDRLEN)
#endif

#if LINUX_VERSION_IS_LESS(3,1,0)
#define genl_dump_check_consistent(cb, user_hdr) do { } while (0)
#endif

#if LINUX_VERSION_IS_LESS(4,10,0)
#define __genl_ro_after_init
#else
#define __genl_ro_after_init __ro_after_init
#endif

#if LINUX_VERSION_IS_LESS(4,15,0)
#define genlmsg_nlhdr LINUX_BACKPORT(genlmsg_nlhdr)
static inline struct nlmsghdr *genlmsg_nlhdr(void *user_hdr)
{
	return (struct nlmsghdr *)((char *)user_hdr -
				   GENL_HDRLEN -
				   NLMSG_HDRLEN);
}

#ifndef genl_dump_check_consistent
static inline
void backport_genl_dump_check_consistent(struct netlink_callback *cb,
					 void *user_hdr)
{
	struct genl_family dummy_family = {
		.hdrsize = 0,
	};

	genl_dump_check_consistent(cb, user_hdr, &dummy_family);
}
#define genl_dump_check_consistent LINUX_BACKPORT(genl_dump_check_consistent)
#endif
#endif /* LINUX_VERSION_IS_LESS(4,15,0) */

#if LINUX_VERSION_IS_LESS(5,2,0)
static inline int
__real_backport_genl_register_family(struct genl_family *family)
{
	return genl_register_family(family);
}
static inline int
__real_backport_genl_unregister_family(struct genl_family *family)
{
	return genl_unregister_family(family);
}

struct backport_genl_family {
	struct genl_family	family;
	const struct genl_ops *	copy_ops;

	/* copied */
	int			id;		/* private */
	unsigned int		hdrsize;
	char			name[GENL_NAMSIZ];
	unsigned int		version;
	unsigned int		maxattr;
	bool			netnsok;
	bool			parallel_ops;
	const struct nla_policy *policy;
	int			(*pre_doit)(__genl_const struct genl_ops *ops,
					    struct sk_buff *skb,
					    struct genl_info *info);
	void			(*post_doit)(__genl_const struct genl_ops *ops,
					     struct sk_buff *skb,
					     struct genl_info *info);
/*
 * unsupported!
	int			(*mcast_bind)(struct net *net, int group);
	void			(*mcast_unbind)(struct net *net, int group);
 */
	struct nlattr **	attrbuf;	/* private */
	__genl_const struct genl_ops *	ops;
	__genl_const struct genl_multicast_group *mcgrps;
	unsigned int		n_ops;
	unsigned int		n_mcgrps;
	struct module		*module;
};
#undef genl_family
#define genl_family backport_genl_family

#define genl_register_family backport_genl_register_family
int genl_register_family(struct genl_family *family);

#define genl_unregister_family backport_genl_unregister_family
int backport_genl_unregister_family(struct genl_family *family);

#define genl_notify LINUX_BACKPORT(genl_notify)
void genl_notify(const struct genl_family *family, struct sk_buff *skb,
		 struct genl_info *info, u32 group, gfp_t flags);

#define genlmsg_put LINUX_BACKPORT(genlmsg_put)
void *genlmsg_put(struct sk_buff *skb, u32 portid, u32 seq,
		  const struct genl_family *family, int flags, u8 cmd);

#define genlmsg_put_reply LINUX_BACKPORT(genlmsg_put_reply)
void *genlmsg_put_reply(struct sk_buff *skb,
			struct genl_info *info,
			const struct genl_family *family,
			int flags, u8 cmd);

#define genlmsg_multicast_netns LINUX_BACKPORT(genlmsg_multicast_netns)
int genlmsg_multicast_netns(const struct genl_family *family,
			    struct net *net, struct sk_buff *skb,
			    u32 portid, unsigned int group,
			    gfp_t flags);

#define genlmsg_multicast LINUX_BACKPORT(genlmsg_multicast)
int genlmsg_multicast(const struct genl_family *family,
		      struct sk_buff *skb, u32 portid,
		      unsigned int group, gfp_t flags);

#define genlmsg_multicast_allns LINUX_BACKPORT(genlmsg_multicast_allns)
int backport_genlmsg_multicast_allns(const struct genl_family *family,
				     struct sk_buff *skb, u32 portid,
				     unsigned int group, gfp_t flags);

#define genl_family_attrbuf LINUX_BACKPORT(genl_family_attrbuf)
static inline struct nlattr **genl_family_attrbuf(struct genl_family *family)
{
	WARN_ON(family->parallel_ops);

	return family->attrbuf;
}
#endif /* LINUX_VERSION_IS_LESS(4,20,0) */

#endif /* __BACKPORT_NET_GENETLINK_H */
