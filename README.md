# darlang

`darlang` is a purely functional, statically typed language that compiles to LLVM IR.

    # A simple implementation of the euclidean algorithm.
    euclid(a, b) -> {
        is(b, 0) : a;
               * : euclid(b, mod(a, b));
    }

## Interesting features

- Type inference (module-level)
- Implicit parameter polymorphism (generics)
- Static typechecking
- Algebraic types (e.g. disjoint unions, product types)
- Structural typing
- Recursive typing

## Examples

See the `examples` folder.
