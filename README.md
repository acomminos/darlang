# darlang

`darlang` is a purely functional, statically typed language that compiles to LLVM IR.

This project exists as a playground to experiment with complex type systems. The current implementation is quite rough around the edges, and lacks critical primitives that are required to construct meaningful software. In lieu of these, however, are elegant typing semantics with a fun syntax.

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
