#!/usr/bin/env python
"""
Copyright (C) 2012 by Brian Neal <bgneal@gmail.com>

fructose_gen.py - A program to auto-generate the main() routine for the C++
testing framework Fructose.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
from __future__ import with_statement
import re
import optparse
import sys


USAGE = "usage: %prog [options] test1.h test2.h ... > main.cpp"
DESCRIPTION = "Generates the main() routine for the Fructose C++ testing framework"
VERSION = "%prog 0.3"
INDENT_COUNT = 4
INDENT = " " * INDENT_COUNT


def strip_comments(s):
    """
    A simple function to strip out C++ style // comments from a string.
    This function is really simple-minded; it doesn't know about string literals, etc.,
    but it should suit our purposes for finding commented out test classes and
    cases.

    """
    i = s.find('//')
    return s if i == -1 else s[:i]


class TestClass(object):
    """
    This class represents a Fructose test class.
    Each test class has a name attribute and a list of test case names
    (strings).

    """
    def __init__(self, name):
        self.name = name
        self.test_cases = []


class TestFile(object):
    """
    A class to represent a Fructose unit test file.
    Each test file has a filename attribute and a list of test classes.

    """
    def __init__(self, filename):
        self.filename = filename
        self.test_classes = []


class TestFileParser(object):
    """
    Base class for parsing Fructose unit test files.
    """
    def __init__(self, filename):
        self.test_file = TestFile(filename)

    def parse(self):
        """
        Parse the file by reading it line by line.
        Returns a TestFile object that contains the test classes found within.

        """
        with open(self.test_file.filename, 'r') as f:
            for line in f:
                s = strip_comments(line)
                if s:
                    self._parse_line(s)

        return self.test_file

    def _parse_line(self, line):
        """
        Parses each line of the test file, calling into derived classes to
        find test classes and test cases.

        """
        test_class = self._parse_test_class(line)
        if test_class:
            self.test_file.test_classes.append(test_class)
        else:
            test_case = self._parse_test_case(line)
            if len(self.test_file.test_classes) and test_case:
                self.test_file.test_classes[-1].test_cases.append(test_case)

    def _parse_test_class(self, line):
        """Derived classes override this"""
        raise NotImplementedError

    def _parse_test_case(self, line):
        """Derived classes override this"""
        raise NotImplementedError


class GeneratorFileParser(TestFileParser):
    """
    This class parses Fructose test files using the generator style of code
    generation.

    """
    CLASS_RE = re.compile(r'\bFRUCTOSE_(?:CLASS|STRUCT)\s*\(\s*([a-zA-Z0-9_]\w*)\s*\)')
    CASE_RE = re.compile(r'\bFRUCTOSE_TEST\s*\(\s*([a-zA-Z0-9_]\w*)\s*\)')

    def _parse_test_class(self, line):
        m = self.CLASS_RE.search(line)
        return TestClass(m.group(1)) if m else None

    def _parse_test_case(self, line):
        m = self.CASE_RE.search(line)
        return m.group(1) if m else None


class XunitFileParser(TestFileParser):
    """
    This class parses Fructose test files using the xUnit style of code
    generation.

    """
    CLASS_RE = re.compile(r'\b(?:struct|class)\s+([a-zA-Z_0-9]\w*)\s+:\s+public'
                            r'\s+(?:fructose\s*::\s*)?test_base\s*<\s*\1\s*>')
    CLASS_MACRO_RE = re.compile(r'\bFRUCTOSE_(?:CLASS|STRUCT)\s*\(([a-zA-Z0-9_]*)')

    CASE_RE = re.compile(r'\bvoid\s+(test\w+)\s*\(const\s+(?:std::)?string\s*&'
                            r'(?:\s+[a-zA-Z_]\w+)?\s*\)')

    CASE_MACRO_RE = re.compile(r'\bFRUCTOSE_TEST\s*\(([a-zA-Z0-9_]*)')

    def _parse_test_class(self, line):
        m = self.CLASS_RE.search(line)
        if m is None:
            m = self.CLASS_MACRO_RE.search(line)
            if m is None:
                return None
            else:
                return TestClass(m.group(1))
        else:
            return TestClass(m.group(1))

    def _parse_test_case(self, line):
        m = self.CASE_RE.search(line)
        if m is None:
            m = self.CASE_MACRO_RE.search(line)
            if m is None:
                return None
            else:
                return m.group(1)
        else:
            return m.group(1)

def generate_test_instance(test_class):
    """
    Generates the code to instantiate a test instance, register and run the
    tests.

    """
    type_name = test_class.name
    instance = type_name + '_instance'

    print ("%s{" % INDENT)
    block_indent = INDENT * 2
    print ("%s%s %s;" % (block_indent, type_name, instance))
    for test_case in test_class.test_cases:
        print ('%s%s.add_test("%s", &%s::%s);' % (
                block_indent,
                instance,
                test_case,
                type_name,
                test_case,
                ))
    print ("%sconst int r = %s.run(argc, argv);" % (block_indent, instance))
    print ("%sretval = (retval == EXIT_SUCCESS) ? r : EXIT_FAILURE;" % block_indent)
    print ("%s}" % INDENT)


def generate_main(test_files):
    """
    Generates the main() file given a list of TestFile objects.

    """
    print ("// Generated by fructose_gen.py\n")
    for test_file in test_files:
        print ('#include "%s"' % test_file.filename)

    print ('\n#include <stdlib.h>\n')
    print ('int main(int argc, char* argv[])\n{')
    print ('%sint retval = EXIT_SUCCESS;\n' % INDENT)

    for test_file in test_files:
        for test_class in test_file.test_classes:
            generate_test_instance(test_class)

    print ('\n%sreturn retval;\n}' % INDENT)


def main(argv=None):

    parser = optparse.OptionParser(usage=USAGE, description=DESCRIPTION,
                                   version=VERSION)
    parser.set_defaults(
        generator=False,
    )
    parser.add_option("-g", "--generator", action="store_true",
            help="use generator style code generation [default: %default]")

    opts, args = parser.parse_args(args=argv)

    xunit = not opts.generator

    parser_class = XunitFileParser if xunit else GeneratorFileParser

    if len(args) == 0:
        sys.exit("No input files")

    test_files = []
    for test_file in args:

        test_parser = parser_class(test_file)
        try:
            test_files.append(test_parser.parse())
        # except IOError, ex:
        except (IOError) as ex:
            sys.stderr.write("Error parsing %s: %s, skipping" % (test_file, ex))

    generate_main(test_files)


if __name__ == '__main__':
    try:
        main()
    except (IOError) as ex:
        sys.exit("IO Error: %s" % ex)
    except KeyboardInterrupt:
        sys.exit("Control-C interrupt")

