// Test Entry Point

#include "all_src.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include "doctest.h"


TEST_CASE("String empty") {
    String s;

    CHECK(s.isEmpty());
    CHECK(s.length() == 0);
    CHECK(s.c_str() == "");
    CHECK( (s == "") );
    CHECK( (s == String()) );
    CHECK_FALSE( (s != "") );
    CHECK_FALSE( (s != String()) );

    CHECK( ( s += "foo" ) == "foo" );
}

TEST_CASE("String empty") {
    String s;
    s = "foo";
    CHECK(s == "foo");
}

TEST_CASE("String empty") {
    String s;
    s = String("foo");
    CHECK(s == "foo");
}

TEST_CASE("String empty") {
    String s;
    s += String("foo");
    CHECK(s == "foo");
}

TEST_CASE("String") {
    String s("foo");

    CHECK(s.length() == 3);
    CHECK(s == "foo");
    CHECK(s.c_str() == "foo");
    CHECK( (s == "foo") );
    CHECK( (s == String("foo")) );
    CHECK( (s != "blah") );
    CHECK( (s != String("blah")) );

    s += "_blah";
    CHECK(s == "foo_blah");

    CHECK(s.charAt(4) == 'b');
}

TEST_CASE("String trim") {
    String s;

    s.trim();
    CHECK(s == "");

    s = " foo bar 1";
    s.trim();
    CHECK(s == "foo bar 1");

    s = "foo bar 2  ";
    s.trim();
    CHECK(s == "foo bar 2");

    s = "   \t\n  foo bar 3  \t\r  ";
    s.trim();
    CHECK(s == "foo bar 3");
}

TEST_CASE("String number") {
    String s1(42);
    CHECK(s1 == "42");
    CHECK(s1.toInt() == 42);

    String s2(65535, HEX);
    CHECK(s2 == "ffff");
}
