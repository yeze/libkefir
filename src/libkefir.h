/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2019 Netronome Systems, Inc. */

#ifndef LIBKEFIR_H
#define LIBKEFIR_H

#include <stddef.h>
#include <sys/types.h>

struct bpf_object;

typedef struct kefir_filter kefir_filter;

/*
 *
 * Filter management
 *
 */

enum kefir_rule_type {
	RULE_TYPE_ETHTOOL_NTUPLE,
	RULE_TYPE_LIBPCAP_EXPR,
	RULE_TYPE_TC_FLOWER,
	RULE_TYPE_IPTABLES,
	RULE_TYPE_OVS_FLOW,
};

/**
 * Create and initialize a new filter object.
 * @return a pointer to the filter (or NULL and sets errno if an error occurred)
 */
kefir_filter *kefir_init_filter(void);

/**
 * Destroy a filter object and free all associated memory.
 */
void kefir_destroy_filter(kefir_filter *filter);

/**
 * Copy a filter object.
 * @filter the filter to copy
 * @return a new filter object (the caller is responsible for its destruction)
 */
kefir_filter *kefir_clone_filter(const kefir_filter *filter);

/**
 * Count the number of rules present in the list of a filter.
 * @filter the filter for which to count the rules
 * @return the number of rules in that filter
 */
size_t kefir_sizeof_filter(const kefir_filter *filter);

/**
 * Add a rule to a filter.
 * @filter object to add the rule to
 * @rule_type type of the rule to add
 * @user_rule array of words defining the rule in the format for rule_type
 * @rule_size number of words in user_rule
 * @index index of the rule in the list (overwrite if pre-existing)
 * @return 0 on success, error code otherwise
 */
int kefir_load_rule(kefir_filter *filter,
		    enum kefir_rule_type rule_type,
		    const char **user_rule,
		    size_t rule_size,
		    ssize_t index);

/**
 * Add a rule to a filter.
 * @filter object to add the rule to
 * @rule_type type of the rule to add
 * @user_rule single string defining the rule in the format for rule_type
 * @index index of the rule in the list (overwrite if pre-existing)
 * @return 0 on success, error code otherwise
 */
int kefir_load_rule_l(kefir_filter *filter,
		      enum kefir_rule_type rule_type,
		      const char *user_rule,
		      ssize_t index);

/**
 * Delete a rule at given index from a filter.
 * @filter: object to remove the rule from
 * @index index of the rule to delete
 * @return 0 on success, error code otherwise
 */
int kefir_delete_rule_by_id(kefir_filter *filter,
			    ssize_t index);

/** Dump all rules of a filter to the console.
 * @filter: object to dump
 */
void kefir_dump_filter(const kefir_filter *filter);

/*
 *
 * Dump, save and restore filter
 *
 */

/**
 * Save a filter to a file
 * @filter filter to save
 * @filename name of the file where to save the filter (it will be created
 *           if necessary, overwritten overwise)
 * @return 0 on success, error code otherwise
 */
int kefir_save_filter_to_file(const kefir_filter *filter,
			      const char *filename);

/**
 * Load a filter from a backup
 * @filename name of the file to load the filter from
 * @return filter object on success (to be freed by the user on exit), NULL
 *         and sets errno otherwise
 */
kefir_filter *kefir_load_filter_from_file(const char *filename);

/*
 *
 * Back end: Conversion to C
 *
 */

typedef struct kefir_cprog kefir_cprog;

enum kefir_cprog_target {
	KEFIR_CPROG_TARGET_XDP,
	KEFIR_CPROG_TARGET_TC,
};

/**
 * Destroy and free allocated memory for a C program object.
 * @cprog C program object to destroy
 */
void kefir_destroy_cprog(kefir_cprog *cprog);

/**
 * Convert a filter into an eBPF-compatible C program.
 * @filter filter to convert
 * @target target for conversion (TC/XDP)
 * @return an object containing all parameters required to create an
 *         eBPF-compatible C program
 */
kefir_cprog *kefir_convert_filter_to_cprog(const kefir_filter *filter,
					   enum kefir_cprog_target target);

/**
 * Dump a C program generated by the library.
 * @cprog program to dump
 */
void kefir_dump_cprog(const kefir_cprog *cprog);

/**
 * Write a generated C program into a buffer.
 * @cprog C program to write
 * @buf buffer to write to, will be realloc()'ed if required
 * @buf_len length of the buffer, will be updated if realloc() occurs
 */
int kefir_cprog_to_buf(const kefir_cprog *cprog,
		       char **buf,
		       size_t *buf_len);

/**
 * Save a C program to a file on the disk.
 * @cprog C program to save
 * @filename name of file to write into (existing file will be overwritten)
 * @return 0 on success, error code otherwise
 */
int kefir_cprog_to_file(const kefir_cprog *cprog,
			const char *filename);

/*
 *
 * Compile to eBPF, load, attach programs
 *
 */

/**
 * Compile a C file into BPF bytecode as an ELF object file.
 * @c_file input C source code file
 * @opt_object_file optional name for the output file, if NULL will be derived
 *                  from c_file if possible (".c" extension will be replaced by
 *                  ".o")
 * @opt_ll_file optional name for intermediary ll file (LLVM IR), if NULL will
 *              be derived from c_file (".ll")
 * @opt_clang_bin optional path to clang executable, if NULL defaults to
 *                /usr/bin/clang
 * @opt_llc_bin optional path to llc executable, if NULL defaults to
 *              /usr/bin/llc
 * @return 0 on success, error code otherwise
 */
int kefir_compile_to_bpf(const char *c_file,
			 const char *opt_object_file,
			 const char *opt_ll_file,
			 const char *opt_clang_bin,
			 const char *opt_llc_bin);

/**
 * Struct containing attributes used when loading a BPF program from an object
 * file.
 * @ifindex: interface index, for indicating where the filter should be
 *           attached (or where the map should be allocated, for hardware
 *           offload, even if the program is simply loaded)
 * @log_level: TODO
 * @flags for XDP: passed to netlink to set XDP mode (socket buffer, driver,
 *        hardware) (see <linux/if_link.h>)
 *        for TC: TODO
 */
struct kefir_load_attr {
	int ifindex;
	int log_level;
	unsigned int flags;
};

/**
 * Unload and destroy a BPF object and free all associated memory.
 * @obj: pointer to the BPF object to destroy
 */
void kefir_destroy_bpf_object(struct bpf_object *obj);

/**
 * Retrieve the file descriptor of the filter program associated with a BPF
 * object.
 * @obj the BPF object resulting from a program load or attachment
 * @return a file descriptor related to that program
 */
int kefir_get_prog_fd(struct bpf_object *obj);

/**
 * Load the BPF program associated to a C program object into the kernel.
 * @cprog cprog used to generate the BPF program
 * @objfile name of ELF object file containing the BPF program generated from
 *          the filter
 * @attr object containing optional attributes to use when loading the program
 * @return a BPF object containing information related to the loaded program,
 *         NULL on error
 */
struct bpf_object *
kefir_load_cprog_from_objfile(const kefir_cprog *cprog,
			      const char *objfile,
			      struct kefir_load_attr *attr);

/**
 * Load the BPF program associated to a C program object into the kernel, then
 * immediately attach it to a given interface and fill the map with rules
 * associated to the filter.
 * @cprog cprog used to generate the BPF program
 * @objfile name of ELF object file containing the BPF program generated from
 *          the filter
 * @attr object containing optional attributes to use when loading the program
 * @return a BPF object containing information related to the loaded program,
 *         NULL on error
 */
struct bpf_object *
kefir_attach_cprog_from_objfile(const kefir_cprog *cprog,
				const char *objfile,
				struct kefir_load_attr *attr);

/**
 * Fill the map associated to a filter loaded in the kernel with the rules
 * associated with that filter.
 * @cprog: cprog used to generate the BPF program loaded on the system
 * @bpf_obj: BPF object resulting from program load
 * @return 0 on success, error code otherwise
 */
int kefir_fill_map(const kefir_cprog *cprog,
		   struct bpf_object *bpf_obj);

/*
 *
 * Other
 *
 */

/**
 * Return a pointer to the error messages produced by the library.
 * @return pointer to a buffer containing all error messages produced by the
 *         library
 */
const char *kefir_strerror();

/**
 * Reset the error string. This is useful to get rid of libbpf warnings that
 * may get incrementally appended to the string.
 */
void kefir_strerror_reset();

#endif /* LIBKEFIR_H */
