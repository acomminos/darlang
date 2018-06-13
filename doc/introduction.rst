Introduction
============

Let's get started with a classical example, the euclidean function.

.. code-block:: darlang

  euclid(a, b) ->
    rem | mod(a, b);
    {
      is(b, 0) : a;
             * : euclid(b, rem)
    }

.. TODO
