# Text Output Devices

The kernel will have 2 lines of text output:

* `printk()`
* `debugk()`

Each line of output is connected to a character device from the following list:

* Serial line
* Screen console

## Usage

While `printk()` will be to print text for the user, `debugk()` will be to print
text for the developer.

## Devices

By default, `printk()` will output to the currently connected screen using a
limine terminal and `debugk()` will output to the serial line.

These defaults could be changed with macros such as:

* `NO_SERIAL` (serial output is disabled)
* `NO_SCREEN` (screen output is disabled)

#### Note

When one text output device is disabled, both `printk()` and `debugk()` will
print to the same non-disabled device.

#### Warning

If one text output device is disabled (ex: `NO_SERIAL`) and the other device
has failed to initialize, there will be no output at all.

## Boot-time Device Detection

During the boot sequence, both character devices are attempted to be
initialized (unless the defaults are changed with macros). If one device failed
to initialize, the other will be used for both lines of text output.
