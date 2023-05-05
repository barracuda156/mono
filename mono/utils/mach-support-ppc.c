/**
 * \file
 * mach support for PPC
 *
 * Authors:
 *   Sergey Fedorov (vital.had@gmail.com)
 *
 * (C) 2023 Xamarin, Inc.
 */

#include <config.h>

#if defined(__MACH__)
#include <stdint.h>
#include <glib.h>
#include <pthread.h>
#include "utils/mono-sigcontext.h"
#include "utils/mono-compiler.h"
#include "mach-support.h"

int
mono_mach_arch_get_mcontext_size ()
{
	return sizeof (struct __darwin_mcontext);
}

void
mono_mach_arch_thread_states_to_mcontext (thread_state_t state, thread_state_t fpstate, void *context)
{
	ppc_thread_state_t *arch_state = (ppc_thread_state_t *) state;
	ppc_float_state_t *arch_fpstate = (ppc_float_state_t *) fpstate;
	struct __darwin_mcontext *ctx = (struct __darwin_mcontext *) context;

	ctx->__ss = *arch_state;
	ctx->__fs = *arch_fpstate;
}

void
mono_mach_arch_mcontext_to_thread_states (void *context, thread_state_t state, thread_state_t fpstate)
{
	ppc_thread_state_t *arch_state = (ppc_thread_state_t *) state;
	ppc_float_state_t *arch_fpstate = (ppc_float_state_t *) fpstate;
	struct __darwin_mcontext *ctx = (struct __darwin_mcontext *) context;

	*arch_state = ctx->__ss;
	*arch_fpstate = ctx->__fs;
}

void
mono_mach_arch_thread_states_to_mono_context (thread_state_t state, thread_state_t fpstate, MonoContext *context)
{
	int i;
	ppc_thread_state_t *arch_state = (ppc_thread_state_t *) state;
	for (i = 0; i < 32; ++i)
		context->regs [i] = arch_state->__r[i];

	context->pc = arch_state->__srr0;	/* Instruction address register (PC) */
	context->lr = arch_state->__lr;	/* Link register */
	/* Do we need the rest? */
}

int
mono_mach_arch_get_thread_state_size ()
{
	return sizeof (ppc_thread_state_t);
}

int
mono_mach_arch_get_thread_fpstate_size ()
{
	return sizeof (ppc_float_state_t);
}

kern_return_t
mono_mach_arch_get_thread_states (thread_port_t thread, thread_state_t state, mach_msg_type_number_t *count, thread_state_t fpstate, mach_msg_type_number_t *fpcount)
{
	ppc_thread_state_t *arch_state = (ppc_thread_state_t *) state;
	ppc_float_state_t *arch_fpstate = (ppc_float_state_t *) fpstate;
	kern_return_t ret;

	*count = PPC_THREAD_STATE_COUNT;
	*fpcount = PPC_FLOAT_STATE_COUNT;

	ret = thread_get_state (thread, PPC_THREAD_STATE, (thread_state_t) arch_state, count);
	if (ret != KERN_SUCCESS)
		return ret;
	ret = thread_get_state (thread, PPC_FLOAT_STATE, (thread_state_t) arch_fpstate, fpcount);
	return ret;
}

kern_return_t
mono_mach_arch_set_thread_states (thread_port_t thread, thread_state_t state, mach_msg_type_number_t count, thread_state_t fpstate, mach_msg_type_number_t fpcount)
{
	kern_return_t ret;
	return thread_set_state (thread, PPC_THREAD_STATE, state, count);
		if (ret != KERN_SUCCESS)
		return ret;
	ret = thread_set_state (thread, PPC_FLOAT_STATE, fpstate, fpcount);
	return ret;
}

#endif
