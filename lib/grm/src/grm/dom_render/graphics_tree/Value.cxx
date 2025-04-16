#include <grm/dom_render/graphics_tree/Value.hxx>
#include <stdexcept>
#include <limits>

GRM::Value::Value() : m_type(Type::UNDEFINED), m_int_value(0), m_double_value(0), m_string_value() {}

GRM::Value::Value(int value) : m_type(Type::INT), m_int_value(value), m_double_value(0), m_string_value() {}

GRM::Value::Value(double value) : m_type(Type::DOUBLE), m_int_value(0), m_double_value(value), m_string_value() {}

GRM::Value::Value(std::string value)
    : m_type(Type::STRING), m_int_value(0), m_double_value(0), m_string_value(std::move(value))
{
}

bool GRM::Value::isType(Type type) const
{
  return m_type == type;
}

bool GRM::Value::isUndefined() const
{
  return isType(Type::UNDEFINED);
}

bool GRM::Value::isInt() const
{
  return isType(Type::INT);
}

bool GRM::Value::isDouble() const
{
  return isType(Type::DOUBLE);
}

bool GRM::Value::isString() const
{
  return isType(Type::STRING);
}

GRM::Value::Type GRM::Value::type() const
{
  return m_type;
}

bool GRM::Value::operator==(const GRM::Value &other) const
{
  if (m_type != other.m_type) return false;
  switch (m_type)
    {
    case Type::UNDEFINED:
      return true;
    case Type::INT:
      return m_int_value == other.m_int_value;
    case Type::DOUBLE:
      return m_double_value == other.m_double_value;
    case Type::STRING:
      return m_string_value == other.m_string_value;
    }
  return false;
}

GRM::Value::operator int() const
{
  switch (m_type)
    {
    case Type::INT:
      return m_int_value;
    case Type::DOUBLE:
      return static_cast<int>(m_double_value);
    case Type::STRING:
      {
        char *end = nullptr;
        long result = std::strtol(m_string_value.c_str(), &end, 10);
        if (end != m_string_value.c_str() + m_string_value.size()) return 0;
        if (result > std::numeric_limits<int>::max() || result < std::numeric_limits<int>::min()) return 0;
        return static_cast<int>(result);
      }
    default:
      return 0;
    }
}

GRM::Value::operator double() const
{
  switch (m_type)
    {
    case Type::INT:
      if (static_cast<int>(static_cast<double>(m_int_value)) != m_int_value) return 0;
      return m_int_value;
    case Type::DOUBLE:
      return m_double_value;
    case Type::STRING:
      {
        char *end = nullptr;
        double result = std::strtod(m_string_value.c_str(), &end);
        if (end == m_string_value.c_str() + m_string_value.size()) return result;
        return 0;
      }
    default:
      return 0;
    }
}

GRM::Value::operator std::string() const
{
  switch (m_type)
    {
    case Type::INT:
      return std::to_string(m_int_value);
    case Type::DOUBLE:
      return std::to_string(m_double_value);
    case Type::STRING:
      return m_string_value;
    default:
      return "";
    }
}

bool GRM::Value::operator!=(const GRM::Value &other) const
{
  return !(*this == other);
}
