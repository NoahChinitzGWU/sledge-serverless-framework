#pragma once

#include <assert.h>
#include <errno.h>
#include <panic.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "debuglog.h"
#include "runtime.h"
#include "worker_thread.h"

/************
 * Externs  *
 ***********/

extern __thread volatile sig_atomic_t         software_interrupt_is_disabled;
extern _Atomic __thread volatile sig_atomic_t software_interrupt_deferred_sigalrm;
extern _Atomic volatile sig_atomic_t          software_interrupt_deferred_sigalrm_max[RUNTIME_WORKER_THREAD_CORE_COUNT];

/*************************
 * Public Static Inlines *
 ************************/

static inline void
software_interrupt_disable(void)
{
	if (__sync_bool_compare_and_swap(&software_interrupt_is_disabled, 0, 1) == false) {
		panic("Recursive call to software_interrupt_disable\n");
	}
}

/**
 * Enables signals
 */
static inline void
software_interrupt_enable(void)
{
	if (__sync_bool_compare_and_swap(&software_interrupt_is_disabled, 1, 0) == false) {
		panic("Recursive call to software_interrupt_enable\n");
	}

	if (software_interrupt_deferred_sigalrm > 0) {
		// TODO: Atomic set?
		software_interrupt_deferred_sigalrm_max[worker_thread_idx] = software_interrupt_deferred_sigalrm;
		software_interrupt_deferred_sigalrm                        = 0;
		// TODO: REPLAY sigalrm;
	}
}

/**
 * @returns boolean if signals are enabled
 */
static inline int
software_interrupt_is_enabled(void)
{
	return (software_interrupt_is_disabled == 0);
}

/**
 * Masks a signal on the current thread
 * @param signal - the signal you want to mask
 * @return 0 on success. Exits program otherwise
 */
static inline int
software_interrupt_mask_signal(int signal)
{
	sigset_t set;
	int      return_code;

	assert(signal == SIGALRM || signal == SIGUSR1);
	/* all threads created by the calling thread will have signal blocked */
	sigemptyset(&set);
	sigaddset(&set, signal);
	return_code = pthread_sigmask(SIG_BLOCK, &set, NULL);
	if (return_code != 0) {
		debuglog("pthread_sigmask: %s", strerror(return_code));
		exit(-1);
	}

	return 0;
}

/**
 * Unmasks a signal on the current thread
 * @param signal - the signal you want to unmask
 * @return 0 on success. Exits program otherwise
 */
static inline int
software_interrupt_unmask_signal(int signal)
{
	sigset_t set;
	int      return_code;

	assert(signal == SIGALRM || signal == SIGUSR1);
	/* all threads created by the calling thread will have signal unblocked */
	sigemptyset(&set);
	sigaddset(&set, signal);
	return_code = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	if (return_code != 0) {
		errno = return_code;
		perror("pthread_sigmask");
		exit(-1);
	}

	return 0;
}

/*************************
 * Exports from module.c *
 ************************/

void software_interrupt_initialize(void);
void software_interrupt_arm_timer(void);
void software_interrupt_disarm_timer(void);
void software_interrupt_set_interval_duration(uint64_t cycles);
void software_interrupt_deferred_sigalrm_max_print(void);
