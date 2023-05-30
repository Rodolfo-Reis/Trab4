/* auto-generated by gen_syscalls.py, don't edit */

#ifndef Z_INCLUDE_SYSCALLS_PWM_H
#define Z_INCLUDE_SYSCALLS_PWM_H


#include <tracing/tracing_syscall.h>

#ifndef _ASMLANGUAGE

#include <syscall_list.h>
#include <syscall.h>

#include <linker/sections.h>


#ifdef __cplusplus
extern "C" {
#endif

extern int z_impl_pwm_set_cycles(const struct device * dev, uint32_t channel, uint32_t period, uint32_t pulse, pwm_flags_t flags);

__pinned_func
static inline int pwm_set_cycles(const struct device * dev, uint32_t channel, uint32_t period, uint32_t pulse, pwm_flags_t flags)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = channel };
		union { uintptr_t x; uint32_t val; } parm2 = { .val = period };
		union { uintptr_t x; uint32_t val; } parm3 = { .val = pulse };
		union { uintptr_t x; pwm_flags_t val; } parm4 = { .val = flags };
		return (int) arch_syscall_invoke5(parm0.x, parm1.x, parm2.x, parm3.x, parm4.x, K_SYSCALL_PWM_SET_CYCLES);
	}
#endif
	compiler_barrier();
	return z_impl_pwm_set_cycles(dev, channel, period, pulse, flags);
}

#if (CONFIG_TRACING_SYSCALL == 1)
#ifndef DISABLE_SYSCALL_TRACING

#define pwm_set_cycles(dev, channel, period, pulse, flags) ({ 	int retval; 	sys_port_trace_syscall_enter(K_SYSCALL_PWM_SET_CYCLES, pwm_set_cycles, dev, channel, period, pulse, flags); 	retval = pwm_set_cycles(dev, channel, period, pulse, flags); 	sys_port_trace_syscall_exit(K_SYSCALL_PWM_SET_CYCLES, pwm_set_cycles, dev, channel, period, pulse, flags, retval); 	retval; })
#endif
#endif


extern int z_impl_pwm_get_cycles_per_sec(const struct device * dev, uint32_t channel, uint64_t * cycles);

__pinned_func
static inline int pwm_get_cycles_per_sec(const struct device * dev, uint32_t channel, uint64_t * cycles)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = channel };
		union { uintptr_t x; uint64_t * val; } parm2 = { .val = cycles };
		return (int) arch_syscall_invoke3(parm0.x, parm1.x, parm2.x, K_SYSCALL_PWM_GET_CYCLES_PER_SEC);
	}
#endif
	compiler_barrier();
	return z_impl_pwm_get_cycles_per_sec(dev, channel, cycles);
}

#if (CONFIG_TRACING_SYSCALL == 1)
#ifndef DISABLE_SYSCALL_TRACING

#define pwm_get_cycles_per_sec(dev, channel, cycles) ({ 	int retval; 	sys_port_trace_syscall_enter(K_SYSCALL_PWM_GET_CYCLES_PER_SEC, pwm_get_cycles_per_sec, dev, channel, cycles); 	retval = pwm_get_cycles_per_sec(dev, channel, cycles); 	sys_port_trace_syscall_exit(K_SYSCALL_PWM_GET_CYCLES_PER_SEC, pwm_get_cycles_per_sec, dev, channel, cycles, retval); 	retval; })
#endif
#endif


extern int z_impl_pwm_enable_capture(const struct device * dev, uint32_t channel);

__pinned_func
static inline int pwm_enable_capture(const struct device * dev, uint32_t channel)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = channel };
		return (int) arch_syscall_invoke2(parm0.x, parm1.x, K_SYSCALL_PWM_ENABLE_CAPTURE);
	}
#endif
	compiler_barrier();
	return z_impl_pwm_enable_capture(dev, channel);
}

#if (CONFIG_TRACING_SYSCALL == 1)
#ifndef DISABLE_SYSCALL_TRACING

#define pwm_enable_capture(dev, channel) ({ 	int retval; 	sys_port_trace_syscall_enter(K_SYSCALL_PWM_ENABLE_CAPTURE, pwm_enable_capture, dev, channel); 	retval = pwm_enable_capture(dev, channel); 	sys_port_trace_syscall_exit(K_SYSCALL_PWM_ENABLE_CAPTURE, pwm_enable_capture, dev, channel, retval); 	retval; })
#endif
#endif


extern int z_impl_pwm_disable_capture(const struct device * dev, uint32_t channel);

__pinned_func
static inline int pwm_disable_capture(const struct device * dev, uint32_t channel)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = channel };
		return (int) arch_syscall_invoke2(parm0.x, parm1.x, K_SYSCALL_PWM_DISABLE_CAPTURE);
	}
#endif
	compiler_barrier();
	return z_impl_pwm_disable_capture(dev, channel);
}

#if (CONFIG_TRACING_SYSCALL == 1)
#ifndef DISABLE_SYSCALL_TRACING

#define pwm_disable_capture(dev, channel) ({ 	int retval; 	sys_port_trace_syscall_enter(K_SYSCALL_PWM_DISABLE_CAPTURE, pwm_disable_capture, dev, channel); 	retval = pwm_disable_capture(dev, channel); 	sys_port_trace_syscall_exit(K_SYSCALL_PWM_DISABLE_CAPTURE, pwm_disable_capture, dev, channel, retval); 	retval; })
#endif
#endif


extern int z_impl_pwm_capture_cycles(const struct device * dev, uint32_t channel, pwm_flags_t flags, uint32_t * period, uint32_t * pulse, k_timeout_t timeout);

__pinned_func
static inline int pwm_capture_cycles(const struct device * dev, uint32_t channel, pwm_flags_t flags, uint32_t * period, uint32_t * pulse, k_timeout_t timeout)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = channel };
		union { uintptr_t x; pwm_flags_t val; } parm2 = { .val = flags };
		union { uintptr_t x; uint32_t * val; } parm3 = { .val = period };
		union { uintptr_t x; uint32_t * val; } parm4 = { .val = pulse };
		union { struct { uintptr_t lo, hi; } split; k_timeout_t val; } parm5 = { .val = timeout };
		uintptr_t more[] = {
			parm5.split.lo,
			parm5.split.hi
		};
		return (int) arch_syscall_invoke6(parm0.x, parm1.x, parm2.x, parm3.x, parm4.x, (uintptr_t) &more, K_SYSCALL_PWM_CAPTURE_CYCLES);
	}
#endif
	compiler_barrier();
	return z_impl_pwm_capture_cycles(dev, channel, flags, period, pulse, timeout);
}

#if (CONFIG_TRACING_SYSCALL == 1)
#ifndef DISABLE_SYSCALL_TRACING

#define pwm_capture_cycles(dev, channel, flags, period, pulse, timeout) ({ 	int retval; 	sys_port_trace_syscall_enter(K_SYSCALL_PWM_CAPTURE_CYCLES, pwm_capture_cycles, dev, channel, flags, period, pulse, timeout); 	retval = pwm_capture_cycles(dev, channel, flags, period, pulse, timeout); 	sys_port_trace_syscall_exit(K_SYSCALL_PWM_CAPTURE_CYCLES, pwm_capture_cycles, dev, channel, flags, period, pulse, timeout, retval); 	retval; })
#endif
#endif


#ifdef __cplusplus
}
#endif

#endif
#endif /* include guard */
