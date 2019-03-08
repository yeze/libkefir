/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2019 Netronome Systems, Inc. */

#ifndef LIBKEFIR_PROGGEN_H
#define LIBKEFIR_PROGGEN_H

#include <stdlib.h>

#include "libkefir.h"

void proggen_cprog_destroy(kefir_cprog *cprog);
kefir_cprog *
proggen_make_cprog_from_filter(const kefir_filter *filter,
			       enum kefir_cprog_target target);
int proggen_cprog_to_buf(const kefir_cprog *cprog, char **buf, size_t *buf_len);

#endif /* LIBKEFIR_PROGGEN_H */
