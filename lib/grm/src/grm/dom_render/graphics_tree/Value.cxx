#include <grm/dom_render/graphics_tree/Value.hxx>
#include <stdexcept>
#include <limits>

GR::Value::Value() : m_type(Type::UNDEFINED), m_int_value(0), m_double_value(0), m_string_value() {}

GR::Value::Value(int value) : m_type(Type::INT), m_int_value(value), m_double_value(0), m_string_value() {}

GR::Value::Value(double value) : m_type(Type::DOUBLE), m_int_value(0), m_double_value(value), m_string_value() {}

GR::Value::Value(std::string value)
    : m_type(Type::STRING), m_int_value(0), m_double_value(0), m_string_value(std::move(value))
{
}

bool GR::Value::isType(Type type) const
{
  return m_type == type;
}

bool GR::Value::isUndefined() const
{
  return isType(Type::UNDEFINED);
}

bool GR::Value::isInt() const
{
  return isType(Type::INT);
}

bool GR::Value::isDouble() const
{
  return isType(Type::DOUBLE);
}

bool GR::Value::isString() const
{
  return isType(Type::STRING);
}

GR::Value::Type GR::Value::type() const
{
  return m_type;
}

bool GR::Value::operator==(const GR::Value &other) const
{
  if (m_type != other.m_type)
    {
      return false;
    }
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

GR::Value::operator int() const
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
        if (end != m_string_value.c_str() + m_string_value.size())
          {
            return 0;
          }
        if (result > std::numeric_limits<int>::max() || result < std::numeric_limits<int>::min())
          {
            return 0;
          }
        return static_cast<int>(result);
      }
    default:
      return 0;
    }
}

GR::Value::operator double() const
{
  switch (m_type)
    {
    case Type::INT:
      if (static_cast<int>(static_cast<double>(m_int_value)) != m_int_value)
        {
          return 0;
        }
      return m_int_value;
    case Type::DOUBLE:
      return m_double_value;
    case Type::STRING:
      {
        char *end = nullptr;
        double result = std::strtod(m_string_value.c_str(), &end);
        if (end == m_string_value.c_str() + m_string_value.size())
          {
            return result;
          }
        return 0;
      }
    default:
      return 0;
    }
}

GR::Value::operator std::string() const
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

bool GR::Value::operator!=(const GR::Value &other) const
{
  return !(*this == other);
}
