/*
 * Project: Software Defined Blocks
 * Copyright (C) 2024 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Test mock WString

#include "all_src.h"
#include "doctest.h"


TEST_CASE("String empty") {
    String s;

    CHECK(s.isEmpty());
    CHECK_EQ(s.length(), 0);
    CHECK_EQ(s.c_str(), "");
    CHECK( (s == "") );
    CHECK( (s == String()) );
    CHECK_FALSE( (s != "") );
    CHECK_FALSE( (s != String()) );

    CHECK_EQ( ( s += "foo" ), "foo" );
}

TEST_CASE("String empty") {
    String s;
    s = "foo";
    CHECK_EQ(s, "foo");
}

TEST_CASE("String empty") {
    String s;
    s = String("foo");
    CHECK_EQ(s, "foo");
}

TEST_CASE("String empty") {
    String s;
    s += String("foo");
    CHECK_EQ(s, "foo");
}

TEST_CASE("String") {
    String s("foo");

    CHECK_EQ(s.length(), 3);
    CHECK_EQ(s, "foo");
    CHECK_EQ(s.c_str(), "foo");
    CHECK( (s == "foo") );
    CHECK( (s == String("foo")) );
    CHECK( (s != "blah") );
    CHECK( (s != String("blah")) );

    s += "_blah";
    CHECK_EQ(s, "foo_blah");

    CHECK_EQ(s.charAt(4), 'b');
}

TEST_CASE("String trim") {
    String s;

    s.trim();
    CHECK_EQ(s, "");

    s = " foo bar 1";
    s.trim();
    CHECK_EQ(s, "foo bar 1");

    s = "foo bar 2  ";
    s.trim();
    CHECK_EQ(s, "foo bar 2");

    s = "   \t\n  foo bar 3  \t\r  ";
    s.trim();
    CHECK_EQ(s, "foo bar 3");
}

TEST_CASE("String number") {
    String s1(42);
    CHECK_EQ(s1, "42");
    CHECK_EQ(s1.toInt(), 42);

    String s2(65535, HEX);
    CHECK_EQ(s2, "ffff");
    CHECK_NE(s2.toInt(), 65535); // can't read "ffff" in base 10.
}
