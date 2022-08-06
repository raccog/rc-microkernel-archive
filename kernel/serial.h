/** \file
 * Standardized COM port driver.
 */

/**
 * Initializes the COM port.
 */
int kinit_serial();

/**
 * Sends a character to the COM port.
 */
void kputc_serial(char c);
