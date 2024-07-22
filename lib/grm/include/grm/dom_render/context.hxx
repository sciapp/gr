#ifndef CONTEXT_HXX
#define CONTEXT_HXX

#include <functional>
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

#include <grm/util.h>
#include <grm/dom_render/NotFoundError.hxx>
#include <grm/dom_render/TypeError.hxx>

namespace GRM
{


class EXPORT Context
{
  /*!
   * Context is used to store data that can not be easily stored inside the tree such as vectors.
   * The syntax to save or overwrite data in a context object is the same as std::map
   * >>> Context c;
   * >>> c["x"] = ...;
   *
   * To get data from a context object the same syntax can be used again although an object of type Context::Inner
   * will be returned instead of the stored type
   * To get data from that Context::Inner object the function `get` or `get_if` should be used
   * >>> GRM::get<...>(c["x"]);
   *
   */

public:
  class EXPORT Inner
  {
    /*!
     * The class Inner is used to return data from Context
     *
     * This class contains vector conversion methods that allow casting an Inner object to a std::vector that is
     * stored in one of Context's tables.
     * These conversion methods are used in `GRM::get` or `GRM::get_if`
     */

  private:
    Context *context;
    std::string key;
    bool intUsed();
    bool doubleUsed();
    bool stringUsed();
    void increment_key(const std::string &);

  public:
    Inner(Context &context, std::string key);
    Inner(const Context &context, std::string key);

    Inner &operator=(std::vector<int> vec);
    Inner &operator=(std::vector<double> vec);
    Inner &operator=(std::vector<std::string> vec);

    explicit operator std::vector<int> &();
    explicit operator const std::vector<int> &() const;

    explicit operator std::vector<double> &();
    explicit operator const std::vector<double> &() const;

    explicit operator std::vector<std::string> &();
    explicit operator const std::vector<std::string> &() const;

    explicit operator std::vector<int> *();
    explicit operator const std::vector<int> *() const;

    explicit operator std::vector<double> *();
    explicit operator const std::vector<double> *() const;

    explicit operator std::vector<std::string> *();
    explicit operator const std::vector<std::string> *() const;

    void delete_key(const std::string &);
    void use_context_key(const std::string &key, const std::string &old_key = "");
    void decrement_key(const std::string &);
  };

  Context();

  Inner operator[](const std::string &str);
  const Inner operator[](const std::string &str) const;

  /*!
   * \brief A forward iterator for the Context class.
   *
   * This iterator can be used to iterate over all key/value pairs in the context in lexicographical order.
   */
  class Iterator
  {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::variant<std::reference_wrapper<std::pair<const std::string, std::vector<int>>>,
                                    std::reference_wrapper<std::pair<const std::string, std::vector<double>>>,
                                    std::reference_wrapper<std::pair<const std::string, std::vector<std::string>>>>;
    using pointer = std::variant<std::pair<const std::string, std::vector<int>> *,
                                 std::pair<const std::string, std::vector<double>> *,
                                 std::pair<const std::string, std::vector<std::string>> *>;
    using reference = value_type;

    Iterator(Context &context, bool is_end_iterator = false);

    reference operator*();

    // Prefix increment
    Iterator &operator++();
    // Postfix increment
    Iterator operator++(int);

    friend bool operator==(const Iterator &a, const Iterator &b);
    friend bool operator!=(const Iterator &a, const Iterator &b);

  private:
    std::variant<std::reference_wrapper<std::map<std::string, std::vector<double>>::iterator>,
                 std::reference_wrapper<std::map<std::string, std::vector<int>>::iterator>,
                 std::reference_wrapper<std::map<std::string, std::vector<std::string>>::iterator>>
    next_iterator();

    Context &context_;
    std::map<std::string, std::vector<double>>::iterator table_double_it_;
    std::map<std::string, std::vector<int>>::iterator table_int_it_;
    std::map<std::string, std::vector<std::string>>::iterator table_string_it_;
    std::variant<std::reference_wrapper<std::map<std::string, std::vector<double>>::iterator>,
                 std::reference_wrapper<std::map<std::string, std::vector<int>>::iterator>,
                 std::reference_wrapper<std::map<std::string, std::vector<std::string>>::iterator>>
        current_it_;
  };

  Iterator begin();
  Iterator end();

private:
  friend class Inner;
  std::map<std::string, std::vector<double>> tableDouble;
  std::map<std::string, std::vector<int>> tableInt;
  std::map<std::string, std::vector<std::string>> tableString;
  std::map<std::string, int> referenceNumberOfKeys;
};

bool operator==(const Context::Iterator &a, const Context::Iterator &b);
bool operator!=(const Context::Iterator &a, const Context::Iterator &b);

template <class T> static T &get(Context::Inner &&data)
{
  /*!
   * The GRM::get function is used to retrieve data from a GRM::Context::Inner object
   * Uses GRM::Context::Inner custom conversions
   *
   * \param[in] data rvalue GRM::Context::Inner
   * \returns T& casted data
   */
  return static_cast<T &>(data);
}

template <class T> static T &get(Context::Inner &data)
{
  /*!
   * The GRM::get function is used to retrieve data from a GRM::Context::Inner object
   * Uses GRM::Context::Inner custom conversions
   *
   * \param[in] data lvalue GRM::Context::Inner
   * \returns T& casted data
   */
  return static_cast<T &>(data);
}

template <class T> static const T &get(const Context::Inner &&data)
{
  /*!
   * The const GRM::get function is used to retrieve data from a GRM::Context::Inner object
   * Uses GRM::Context::Inner custom conversions
   *
   * \param[in] data const rvalue GRM::Context::Inner
   * \returns const T& casted data
   */
  return static_cast<const T &>(data);
}

template <class T> static const T &get(const Context::Inner &data)
{
  /*!
   * The const GRM::get function is used to retrieve data from a GRM::Context::Inner object
   * Uses GRM::Context::Inner custom conversions
   *
   * \param[in] data const lvalue GRM::Context::Inner
   * \returns const T& casted data
   */
  return static_cast<const T &>(data);
}

template <class T> static T *get_if(Context::Inner &&data)
{
  /*!
   * The GRM::get_if function can be used to retrieve a pointer to the data in GRM::Context::Inner without throwing
   * runtime exceptions. It can be used to check if a Context::Inner object contains a certain datatype and get a
   * pointer for that data. But it can not be used to check if it contains unsupported types (types that
   * Context::Inner can not be casted to). In those cases the code will not build.
   *
   * Uses Context::Inner custom conversions
   *
   * \param[in] data rvalue GRM::Context::Inner
   *
   * \returns a pointer to the vector if it exists else returns nullptr
   */
  try
    {
      return static_cast<T *>(data);
    }
  catch (const NotFoundError &e)
    {
      return nullptr;
    }
  catch (const TypeError &e)
    {
      return nullptr;
    }
}

template <class T> static T *get_if(Context::Inner &data)
{
  /*!
   * The GRM::get_if function can be used to retrieve a pointer to the data in GRM::Context::Inner without throwing
   * runtime exceptions. It can be used to check if a Context::Inner object contains a certain datatype and get a
   * pointer for that data. But it can not be used to check if it contains unsupported types (types that
   * Context::Inner can not be casted to). In those cases the code will not build.
   *
   * Uses Context::Inner custom conversions
   *
   * \param[in] data lvalue GRM::Context::Inner
   *
   * \returns a pointer to the vector if it exists else returns nullptr
   */
  try
    {
      return static_cast<T *>(data);
    }
  catch (const NotFoundError &e)
    {
      return nullptr;
    }
  catch (const TypeError &e)
    {
      return nullptr;
    }
}

template <class T> static const T *get_if(const Context::Inner &&data)
{
  /*!
   * The const GRM::get_if function can be used to retrieve a pointer to the data in GRM::Context::Inner without
   * throwing runtime exceptions. It can be used to check if a Context::Inner object contains a certain datatype and get
   * a pointer for that data. But it can not be used to check if it contains unsupported types (types that
   * Context::Inner can not be casted to). In those cases the code will not build.
   *
   * Uses Context::Inner custom conversions
   *
   * \param[in] data const rvalue GRM::Context::Inner
   *
   * \returns a const pointer to the vector if it exists else returns nullptr
   */
  try
    {
      return static_cast<const T *>(data);
    }
  catch (const NotFoundError &e)
    {
      return nullptr;
    }
  catch (const TypeError &e)
    {
      return nullptr;
    }
}

template <class T> static const T *get_if(const Context::Inner &data)
{
  /*!
   * The const GRM::get_if function can be used to retrieve a pointer to the data in GRM::Context::Inner without
   * throwing runtime exceptions. It can be used to check if a Context::Inner object contains a certain datatype and get
   * a pointer for that data. But it can not be used to check if it contains unsupported types (types that
   * Context::Inner can not be casted to). In those cases the code will not build.
   *
   * Uses Context::Inner custom conversions
   *
   * \param[in] data const lvalue GRM::Context::Inner
   *
   * \returns a const pointer to the vector if it exists else returns nullptr
   */
  try
    {
      return static_cast<const T *>(data);
    }
  catch (const NotFoundError &e)
    {
      return nullptr;
    }
  catch (const TypeError &e)
    {
      return nullptr;
    }
}

} // namespace GRM

#endif
