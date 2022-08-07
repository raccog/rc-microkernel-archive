# C++ Guidelines

This document includes some guidelines for writing C++ kernel code.

## No Namespaces

### Background

Namespaces allow entities with the same identifier to exist within separate
namespaces. C++ namespaces can be emulated in C by prefixing identifiers with
a namespace and underscore.

```c
// Example prefixed with 'kernel' and 'user' namespaces
int kernel_printf();
int user_printf();
```

The same example in C++:

```c++
namespace kernel {
    int printf();
}
namespace user {
    int printf();
}
```

### Pros and Cons

Not sure what pros namespaces bring that C does not have.

As shown in the namespace example above, 2 functions with the same name can
exist in separate namespaces. This prevents name collisions, but it also causes
some problems:

* It can be difficult to differentiate entities with the same exact identifier.
* Searching for identifiers can be difficult when the same identifier is used
for multiple entities.
* When looking at the entity declaration alone, it can be difficult to
understand the full context behind the entity without also looking at the
namespace. This requires looking for the namespace declaration in a separate
part of the code.
my experience).

### Decision

For now, namespaces should not be used. Instead, prefix public identifiers with
a "namespace" like the following example:

```c
int kernel_printf();
int user_printf();
```

If there are better alternatives to searching for namespace identifiers, then
I may choose to allow namespaces in the future. This would require:

* Easy search using grep
* Ctags access

