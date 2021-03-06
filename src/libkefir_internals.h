/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2019 Netronome Systems, Inc. */

#ifndef LIBKEFIR_INTERNALS_H
#define LIBKEFIR_INTERNALS_H

#include <stdint.h>
#include <sys/types.h>

#include <linux/bpf.h>
#include <linux/const.h>

#include "libkefir.h"
#include "libkefir_error.h"
#include "list.h"

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef array_size
#define array_size(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef sizeof_member
#define sizeof_member(type, member) sizeof(*(&((type *)0)->member))
#endif

#ifndef __printf
#define __printf(a, b) __attribute__((format(printf, a, b)))
#endif

#define KEFIR_CPROG_INIT_BUFLEN		8192

enum value_format {
	KEFIR_VAL_FMT_BIT,	/* MPLS BoS */
	KEFIR_VAL_FMT_UINT3,	/* VLAN prio, MPLS TC */
	KEFIR_VAL_FMT_UINT6,	/* IPv4 ToS */
	KEFIR_VAL_FMT_UINT8,
	KEFIR_VAL_FMT_UINT12,	/* VLAN ID, TCP flags */
	KEFIR_VAL_FMT_UINT16,
	KEFIR_VAL_FMT_UINT20,	/* MPLS label */
	KEFIR_VAL_FMT_UINT32,
	KEFIR_VAL_FMT_MAC_ADDR,
	KEFIR_VAL_FMT_IPV4_ADDR,
	KEFIR_VAL_FMT_IPV6_ADDR,
};

static const size_t format_size[] = {
	[KEFIR_VAL_FMT_BIT]		= 1,
	[KEFIR_VAL_FMT_UINT3]		= 3,
	[KEFIR_VAL_FMT_UINT6]		= 6,
	[KEFIR_VAL_FMT_UINT8]		= 8,
	[KEFIR_VAL_FMT_UINT12]		= 12,
	[KEFIR_VAL_FMT_UINT16]		= 16,
	[KEFIR_VAL_FMT_UINT20]		= 20,
	[KEFIR_VAL_FMT_UINT32]		= 32,
	[KEFIR_VAL_FMT_IPV4_ADDR]	= 32,
	[KEFIR_VAL_FMT_MAC_ADDR]	= 48,
	[KEFIR_VAL_FMT_IPV6_ADDR]	= 128,
};

static const enum value_format type_format[] = {
	[KEFIR_MATCH_TYPE_ETHER_SRC]		= KEFIR_VAL_FMT_MAC_ADDR,
	[KEFIR_MATCH_TYPE_ETHER_DST]		= KEFIR_VAL_FMT_MAC_ADDR,
	[KEFIR_MATCH_TYPE_ETHER_ANY]		= KEFIR_VAL_FMT_MAC_ADDR,
	[KEFIR_MATCH_TYPE_ETHER_PROTO]		= KEFIR_VAL_FMT_UINT16,

	[KEFIR_MATCH_TYPE_IP_4_SRC]		= KEFIR_VAL_FMT_IPV4_ADDR,
	[KEFIR_MATCH_TYPE_IP_4_DST]		= KEFIR_VAL_FMT_IPV4_ADDR,
	[KEFIR_MATCH_TYPE_IP_4_ANY]		= KEFIR_VAL_FMT_IPV4_ADDR,
	[KEFIR_MATCH_TYPE_IP_4_TOS]		= KEFIR_VAL_FMT_UINT6,
	[KEFIR_MATCH_TYPE_IP_4_TTL]		= KEFIR_VAL_FMT_UINT8,
	[KEFIR_MATCH_TYPE_IP_4_L4PROTO]		= KEFIR_VAL_FMT_UINT8,
	[KEFIR_MATCH_TYPE_IP_4_L4DATA]		= KEFIR_VAL_FMT_UINT32,
	[KEFIR_MATCH_TYPE_IP_4_L4PORT_SRC]	= KEFIR_VAL_FMT_UINT16,
	[KEFIR_MATCH_TYPE_IP_4_L4PORT_DST]	= KEFIR_VAL_FMT_UINT16,
	[KEFIR_MATCH_TYPE_IP_4_L4PORT_ANY]	= KEFIR_VAL_FMT_UINT16,

	[KEFIR_MATCH_TYPE_IP_6_SRC]		= KEFIR_VAL_FMT_IPV6_ADDR,
	[KEFIR_MATCH_TYPE_IP_6_DST]		= KEFIR_VAL_FMT_IPV6_ADDR,
	[KEFIR_MATCH_TYPE_IP_6_ANY]		= KEFIR_VAL_FMT_IPV6_ADDR,
	[KEFIR_MATCH_TYPE_IP_6_TOS]		= KEFIR_VAL_FMT_UINT8,
	[KEFIR_MATCH_TYPE_IP_6_TTL]		= KEFIR_VAL_FMT_UINT8,
	[KEFIR_MATCH_TYPE_IP_6_L4PROTO]		= KEFIR_VAL_FMT_UINT8,
	[KEFIR_MATCH_TYPE_IP_6_L4DATA]		= KEFIR_VAL_FMT_UINT32,
	[KEFIR_MATCH_TYPE_IP_6_L4PORT_SRC]	= KEFIR_VAL_FMT_UINT16,
	[KEFIR_MATCH_TYPE_IP_6_L4PORT_DST]	= KEFIR_VAL_FMT_UINT16,
	[KEFIR_MATCH_TYPE_IP_6_L4PORT_ANY]	= KEFIR_VAL_FMT_UINT16,

	[KEFIR_MATCH_TYPE_IP_ANY_TOS]		= KEFIR_VAL_FMT_UINT8,
	[KEFIR_MATCH_TYPE_IP_ANY_TTL]		= KEFIR_VAL_FMT_UINT8,
	[KEFIR_MATCH_TYPE_IP_ANY_L4PROTO]	= KEFIR_VAL_FMT_UINT8,
	[KEFIR_MATCH_TYPE_IP_ANY_L4DATA]	= KEFIR_VAL_FMT_UINT32,
	[KEFIR_MATCH_TYPE_IP_ANY_L4PORT_SRC]	= KEFIR_VAL_FMT_UINT16,
	[KEFIR_MATCH_TYPE_IP_ANY_L4PORT_DST]	= KEFIR_VAL_FMT_UINT16,
	[KEFIR_MATCH_TYPE_IP_ANY_L4PORT_ANY]	= KEFIR_VAL_FMT_UINT16,

	[KEFIR_MATCH_TYPE_VLAN_ID]		= KEFIR_VAL_FMT_UINT12,
	[KEFIR_MATCH_TYPE_VLAN_PRIO]		= KEFIR_VAL_FMT_UINT3,
	[KEFIR_MATCH_TYPE_VLAN_ETHERTYPE]	= KEFIR_VAL_FMT_UINT16,
	[KEFIR_MATCH_TYPE_CVLAN_ID]		= KEFIR_VAL_FMT_UINT12,
	[KEFIR_MATCH_TYPE_CVLAN_PRIO]		= KEFIR_VAL_FMT_UINT3,
	[KEFIR_MATCH_TYPE_CVLAN_ETHERTYPE]	= KEFIR_VAL_FMT_UINT16,
	[KEFIR_MATCH_TYPE_SVLAN_ID]		= KEFIR_VAL_FMT_UINT12,
	[KEFIR_MATCH_TYPE_SVLAN_PRIO]		= KEFIR_VAL_FMT_UINT3,
	[KEFIR_MATCH_TYPE_SVLAN_ETHERTYPE]	= KEFIR_VAL_FMT_UINT16,
};

#define MATCH_FLAGS_USE_MASK	_BITUL(0)

struct kefir_filter {
	struct list *rules;
};

/*
 * kefir_cprog
 */

#define OPT_FLAGS_NEED_ETHER	_BITUL(0)
#define OPT_FLAGS_NEED_IPV4	_BITUL(1)
#define OPT_FLAGS_NEED_IPV6	_BITUL(2)
#define OPT_FLAGS_NEED_L4_4B	_BITUL(3)
#define OPT_FLAGS_NEED_UDP	_BITUL(4)
#define OPT_FLAGS_NEED_TCP	_BITUL(5)
#define OPT_FLAGS_NEED_SCTP	_BITUL(6)
#define OPT_FLAGS_NEED_L4_HDR	\
	(OPT_FLAGS_NEED_UDP | OPT_FLAGS_NEED_TCP | OPT_FLAGS_NEED_SCTP)
#define OPT_FLAGS_NEED_CVLAN	_BITUL(7)
#define OPT_FLAGS_NEED_SVLAN	_BITUL(8)
#define OPT_FLAGS_NEED_VLAN	\
	(OPT_FLAGS_NEED_CVLAN | OPT_FLAGS_NEED_SVLAN)
#define OPT_FLAGS_USE_MASKS	_BITUL(9)
#define OPT_FLAGS_INLINE_FUNC	_BITUL(10)
#define OPT_FLAGS_NO_LOOPS	_BITUL(11)
#define OPT_FLAGS_CLONE_FILTER	_BITUL(12)
#define OPT_FLAGS_NO_VLAN	_BITUL(13)
#define OPT_FLAGS_USE_PRINTK	_BITUL(14)

struct kefir_cprog_options {
	uint64_t		flags;
	unsigned int		nb_matches;
	enum kefir_cprog_target	target;
	const char *		license;
	uint8_t	req_helpers[__BPF_FUNC_MAX_ID / 8 + 1];
};

struct kefir_cprog {
	const struct kefir_filter	*filter;
	struct kefir_cprog_options	options;
};

/*
 * Shared functions
 */

static inline size_t bytes_for_format(enum value_format format)
{
	return (format_size[format] + 7) / 8;
}

static inline size_t bits_for_type(enum kefir_match_type type)
{
	return format_size[type_format[type]];
}

#endif /* LIBKEFIR_INTERNALS_H */
