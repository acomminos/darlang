Basics
======

*This document is very incomplete, perhaps some meaning can be derived from it.*

Declarations
------------

.. code-block:: darlang

  func(arg_1, arg_2, ..., arg_n) -> expr

All functions in darlang are implicitly typed. Both the argument types and the return value are derived through unification by the callee and implementation, respectively.

Binding
-------

.. code-block:: darlang

  id | expr; ...

Binding is the process of assigning an expression to a new identifier in the current scope. All references to the id on the left hand side of the expression will be substituted with the associated expression.

Conditionals
------------

.. code-block:: darlang

  {
    case_1 : expr_1;
    case_2 : expr_2;
    ...
    case_n : expr_n;
         * : expr_wildcard;
  }

Darlang uses a singular guard-like construct to provide control flow.

Cases are evaluated from top to bottom- the first true case expression will cause the associated expression to be returned.

All branching constructs must have a "wildcard" case, to be executed when no cases are satisfied.

Example
-------

Let's get started with a program that computes the classic `Euclidean algorithm`.

.. _Euclidean algorithm: https://en.wikipedia.org/wiki/Euclidean_algorithm

.. code-block:: darlang

  euclid(a, b) ->
    {
      is(b, 0) : a;
             * : euclid(b, mod(a, b))
    }

  main() -> euclid(15, 10)
