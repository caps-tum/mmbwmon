FRUCTOSE - FRamework for Unit testing C++ for Test driven development Of SoftwarE

Version 1.3.0

This is a simple unit test framework to facilitate writing standalone
programs, one per class implementation file, that allow the developer
to evolve the class using Test Driven Development (TDD).
The library is implemented completely in header files.
Look to the examples to see how it is used. Doxygen output is also provided.

I still want fructose to work with Microsoft Windows but I no longer have
any such systems, so I can no longer easily ensure that changes to fructose
still work there. I welcome any bug/issue reports though, and will do
my best to resolve them.

Changes in version 1.3.0
========================

Syntax errors in the loop macros were corrected.
The loop macros were also corrected such that if
a test consists entirely of these you no get longer
a warning that no assertions were made.
A test example, ex16, was added to test this.

Changed the python script so it is compatible with python 3.
Generated code includes comment at start that says it
was generated using fructose_gen.py.

Added _USE_MATH_DEFINES to example 3 when compiling using
Microsoft Visual Studio so that it picks up the
definition of M_PI.

Added Studio files for VC10 (Visual Studio 2010) for the
example programs.


Changes in version 1.2.0
========================

Tests can now be invoked with extra command line parameters. 
This is done by appending "--args <arguments>" to the test name. 
The arguments are per test and so can be different for each test.
In order to implement this functionality, the use of TCLAP had to
be dropped. Therefore fructose no longer depends on TCLAP.
There is nothing wrong with TCLAP. The reason is simply because 
fructose now supports a command line argument format that does
not fit with the standard command lines that TCLAP can parse.

An example has been added, ex15, which shows how to use this new feature.

A minor bug was fixed in fructose_gen.py where it didn't handle class and
test class names containing digits.

fructose_gen.py was extended to recognise the macro names FRUCTOSE_CLASS,
FRUCTOSE_STRUCT and FRUCTOSE_TEST even when the --generator option is not used. 
This made the generator program redundant so it was removed.
The --generator option was retained for backwards compatibility.

Changes since 1.0.0
===================
Added fructose_gen.py, a python version of the code generator.
This contribution is from Brian Neal (thanks, Brian!).

The documentation has been split two; one is a rationale that explains 
the design and various decisions made during the development of FRUCTOSE;
the other is a user guide.

For (brief) documentation on the python code generator, see the user guide.

Also added the macro fructose_assert_same_data for binary data comparisons.
The macro fructose_assert_fail was also added.

Any and all feedback is welcome to: andrew@andrewpetermarlow.co.uk

