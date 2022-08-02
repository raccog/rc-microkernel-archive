# Development Rules for Kernel

These rules should be followed for all kernel code.

* C17 language standard
* Functions should be defined early on and structs even earlier to minimize changes
* Implementations can be more fluid than definitions (can change more often without breaking as much)
* All functions/files/structs/macros should be documented with doxygen
* Clang format should always be run before committing (this is done automatically with make)

## C Style Conventions

These style conventions are followed for all C code.

The following names should be snake-case (lower).

Ex: `kernel_init_interrupts`, `foo_bar_foo`

* Variables (not compile-time constant)
* Structs
* Struct members
* Functions
* Files

The following names should be snake-case (uppercase).

Ex: `MAX_NUM`, `FOO_BAR_FOO`

* Compile-time constants
* Macro definitions

### Structs

Structs should not be changed to a typedef.

Ex:

```c
struct interrupt_descriptor_table {
    // ...
};
```

### Namespaces

Almost all functions and structs should be prefixed with a short namespace.

Ex:

```c
void kernel_init_interrupts();
u64 ext4_file_size();
```


