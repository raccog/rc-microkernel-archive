#pragma once

/**
 * Initialize architecture-dependent character output devices.
 *
 * This is done right away so that logging can be done as early in the boot
 * process as possible.
 */
void arch_early_init();

/**
 * Initialize architecture-dependent features that are necessary for kernel
 * initalization.
 */
void arch_init();
