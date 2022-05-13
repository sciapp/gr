#include <catch2/catch.hpp>
#include "Value.hxx"

using GR::Value;

TEST_CASE("Value::Value()", "[value]")
{
  Value value;
  REQUIRE(value.isUndefined());
  REQUIRE(!value.isInt());
  REQUIRE(!value.isDouble());
  REQUIRE(!value.isString());
  REQUIRE((int)value == 0);
  REQUIRE((double)value == 0);
  REQUIRE((std::string)value == "");
}

TEST_CASE("Value::Value(int)", "[value]")
{
  Value value(1);
  REQUIRE((int)value == 1);
  REQUIRE((double)value == 1);
  REQUIRE((std::string)value == "1");
  REQUIRE(!value.isUndefined());
  REQUIRE(value.isInt());
  REQUIRE(!value.isDouble());
  REQUIRE(!value.isString());
}

TEST_CASE("Value::Value(double)", "[value]")
{
  Value value(1.0);
  REQUIRE((int)value == 1);
  REQUIRE((double)value == 1);
  REQUIRE((std::string)value == "1.000000");
  REQUIRE(!value.isUndefined());
  REQUIRE(!value.isInt());
  REQUIRE(value.isDouble());
  REQUIRE(!value.isString());
}

TEST_CASE("Value::Value(std::string)", "[value]")
{
  Value value1("test");
  REQUIRE((std::string)value1 == "test");
  REQUIRE((double)value1 == 0);
  REQUIRE(!value1.isUndefined());
  REQUIRE(!value1.isInt());
  REQUIRE(!value1.isDouble());
  REQUIRE(value1.isString());
  Value value2("1.0");
  REQUIRE((std::string)value2 == "1.0");
  REQUIRE((int)value2 == 0);
  REQUIRE((double)value2 == 1.0);
  REQUIRE(!value2.isUndefined());
  REQUIRE(!value2.isInt());
  REQUIRE(!value2.isDouble());
  REQUIRE(value2.isString());
}

TEST_CASE("Value::type", "[value]")
{
  REQUIRE(Value().type() == Value::Type::UNDEFINED);
  REQUIRE(Value(1).type() == Value::Type::INT);
  REQUIRE(Value(1.0).type() == Value::Type::DOUBLE);
  REQUIRE(Value("a").type() == Value::Type::STRING);
}

TEST_CASE("Value::operator==", "[value]")
{
  REQUIRE(Value() == Value());
  REQUIRE(Value(1) != Value());
  REQUIRE(Value(1) == Value(1));
  REQUIRE(Value(1) != Value(2));
  REQUIRE(Value(1.0) != Value());
  REQUIRE(Value(1.0) == Value(1.0));
  REQUIRE(Value(1.0) != Value(2.0));
  REQUIRE(Value("a") != Value());
  REQUIRE(Value("a") == Value("a"));
  REQUIRE(Value("a") != Value("b"));
}
