#ifndef GRM_GRAPHICS_TREE_INTERFACE_VALUE_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_VALUE_HXX

#include <string>
#include <grm/util.h>

namespace GRM
{
class EXPORT Value
{
public:
  enum class Type
  {
    UNDEFINED = 0,
    INT,
    DOUBLE,
    STRING
  };

  Value();

  explicit Value(int value);

  explicit Value(double value);

  explicit Value(std::string value);

  bool isType(Type type) const;

  bool isUndefined() const;

  bool isInt() const;

  bool isDouble() const;

  bool isString() const;

  Type type() const;

  explicit operator int() const;

  explicit operator double() const;

  explicit operator std::string() const;

  bool operator==(const Value &other) const;

  bool operator!=(const Value &other) const;

private:
  Type m_type;
  int m_int_value;
  double m_double_value;
  std::string m_string_value;
};
} // namespace GRM

#endif
