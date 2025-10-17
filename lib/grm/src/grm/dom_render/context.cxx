#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <grm/dom_render/not_found_error.hxx>
#include <grm/dom_render/type_error.hxx>
#include <grm/dom_render/context.hxx>


GRM::Context::Context() = default; /*! default constructor for GRM::Context*/

GRM::Context::Inner::Inner(Context &context, std::string key) : context(&context), key(std::move(key))
{
  /*!
   * The non const constructor for GRM::Context::Inner
   *
   * \param[in] context The GRM::Context it belongs to
   * \param[in] key The assigned key for Inner
   */
}
GRM::Context::Inner::Inner(const Context &context, std::string key)
    : context(&const_cast<Context &>(context)), key(std::move(key))
{
  /*!
   * The const constructor for GRM::Context::Inner
   *
   * \param[in] context A const GRM::Context it belongs to
   * \param[in] key The assigned key for Inner
   */
}

bool GRM::Context::Inner::intUsed()
{
  /*!
   * This function is used for checking if the table_int map of GRM::Context contains a value for GRM::Context::Inner's
   * key
   *
   * \returns a bool indicating the usage of GRM::Context::Inner::key by GRM::Context::table_int
   */
  return context->table_int.find(key) != context->table_int.end();
}

bool GRM::Context::Inner::doubleUsed()
{
  /*!
   * This function is used for checking if the table_double map of GRM::Context contains a value for
   * GRM::Context::Inner's key
   *
   * \returns a bool indicating the usage of GRM::Context::Inner::key by GRM::Context::table_double
   */
  return context->table_double.find(key) != context->table_double.end();
}

bool GRM::Context::Inner::stringUsed()
{
  /*!
   * This function is used for checking if the table_string map of GRM::Context contains a value for
   * GRM::Context::Inner's key
   *
   * \returns a bool indicating the usage of GRM::Context::Inner::key by GRM::Context::table_string
   */
  return context->table_string.find(key) != context->table_string.end();
}


GRM::Context::Inner &GRM::Context::Inner::operator=(std::vector<double> vec)
{
  /*!
   * Overloaded operator= for GRM::Context::Inner assigning std::vector<double>
   * Stores the vector in GRM::Context::table_double with GRM::Context::Inner's key
   * Throws a TypeError if the GRM::Context::Inner key is already used by other GRM::Context tableTYPES
   */
  if (intUsed() || stringUsed()) throw TypeError("Wrong Type: std::vector<double> expected\n");
  context->table_double[key] = std::move(vec);
  return *this;
}

GRM::Context::Inner &GRM::Context::Inner::operator=(std::vector<int> vec)
{
  /*!
   * Overloaded operator= for GRM::Context::Inner assigning std::vector<int>
   * Stores the vector in GRM::Context::table_int with GRM::Context::Inner's key
   * Throws a TypeError if the GRM::Context::Inner key is already used by other GRM::Context tableTYPES
   */
  if (doubleUsed() || stringUsed()) throw TypeError("Wrong type: std::vector<int> expected\n");
  context->table_int[key] = std::move(vec);
  return *this;
}


GRM::Context::Inner &GRM::Context::Inner::operator=(std::vector<std::string> vec)
{
  /*!
   * Overloaded operator= for GRM::Context::Inner assigning std::vector<std::string>
   * Stores the vector in GRM::Context::table_string with GRM::Context::Inner's key
   * Throws a TypeError if the GRM::Context::Inner is already used by other GRM::Context tableTYPES
   */
  if (intUsed() || doubleUsed()) throw TypeError("Wrong type: std::vector<std::string> expected\n");
  context->table_string[key] = std::move(vec);
  return *this;
}


GRM::Context::Inner::operator std::vector<int> &()
{
  /*!
   * Overloaded operator std::vector<int>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in table_int with Inner's key
   */
  if (context->table_int.find(key) != context->table_int.end()) return context->table_int[key];
  std::string msg = "No integer value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator const std::vector<int> &() const
{
  /*!
   * The const overloaded operator std::vector<int>& used for converting GRM::Context::Inner to const std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in table_int with Inner's key
   */
  if (context->table_int.find(key) != context->table_int.end()) return context->table_int[key];
  std::string msg = "No integer value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator std::vector<double> &()
{
  /*!
   * Overloaded operator std::vector<double>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in table_double with Inner's key
   */
  if (context->table_double.find(key) != context->table_double.end()) return context->table_double[key];
  std::string msg = "No double value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator const std::vector<double> &() const
{
  /*!
   * The const overloaded operator std::vector<double>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in table_double with Inner's key
   */
  if (context->table_double.find(key) != context->table_double.end()) return context->table_double[key];
  std::string msg = "No double value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator std::vector<std::string> &()
{
  /*!
   * Overloaded operator std::vector<std::string>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in table_string with Inner's key
   */
  if (context->table_string.find(key) != context->table_string.end()) return context->table_string[key];
  std::string msg = "No string value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator const std::vector<std::string> &() const
{
  /*!
   * The const overloaded operator std::vector<std::string>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in table_string with Inner's key
   */
  if (context->table_string.find(key) != context->table_string.end()) return context->table_string[key];
  std::string msg = "No string value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator std::vector<int> *()
{
  /*!
   * Overloaded operator std::vector<int>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in table_int with Inner's key
   */
  if (context->table_int.find(key) != context->table_int.end()) return &context->table_int[key];
  std::string msg = "No integer value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator const std::vector<int> *() const
{
  /*!
   * The const overloaded operator std::vector<int>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in table_int with Inner's key
   */
  if (context->table_int.find(key) != context->table_int.end()) return &context->table_int[key];
  std::string msg = "No integer value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator std::vector<double> *()
{
  /*!
   * Overloaded operator std::vector<double>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in table_double with Inner's key
   */
  if (context->table_double.find(key) != context->table_double.end()) return &context->table_double[key];
  std::string msg = "No double value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator const std::vector<double> *() const
{
  /*!
   * The const overloaded operator std::vector<double>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in table_double with Inner's key
   */
  if (context->table_double.find(key) != context->table_double.end()) return &context->table_double[key];
  std::string msg = "No double value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator std::vector<std::string> *()
{
  /*!
   * Overloaded operator std::vector<std::string>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in table_string with Inner's key
   */
  if (context->table_string.find(key) != context->table_string.end()) return &context->table_string[key];
  std::string msg = "No string value found for given key: " + key;
  throw NotFoundError(msg);
}

GRM::Context::Inner::operator const std::vector<std::string> *() const
{
  /*!
   * Const overloaded operator std::vector<std::string>* used for converting GRM::Context::Inner to a std::vector
   * pointer This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in table_string with Inner's key
   */
  if (context->table_string.find(key) != context->table_string.end()) return &context->table_string[key];
  std::string msg = "No string value found for given key: " + key;
  throw NotFoundError(msg);
}

void GRM::Context::Inner::deleteKey(const std::string &context_key)
{
  bool erased = false;
  if (context->table_string.find(context_key) != context->table_string.end())
    {
      context->table_string.erase(context_key);
      erased = true;
    }
  if (context->table_double.find(context_key) != context->table_double.end())
    {
      context->table_double.erase(context_key);
      erased = true;
    }
  if (context->table_int.find(context_key) != context->table_int.end())
    {
      context->table_int.erase(context_key);
      erased = true;
    }
  if (erased) context->reference_number_of_keys.erase(context_key);
}

void GRM::Context::Inner::decrementKey(const std::string &context_key)
{
  context->reference_number_of_keys[context_key] -= 1;
  if (context->reference_number_of_keys[context_key] <= 0) deleteKey(context_key);
}

void GRM::Context::Inner::incrementKey(const std::string &context_key)
{
  context->reference_number_of_keys[context_key] += 1;
}

void GRM::Context::Inner::useContextKey(const std::string &context_key, const std::string &old_key)
{
  if (context_key != old_key)
    {
      if (!old_key.empty()) decrementKey(old_key);
      incrementKey(context_key);
    }
}

GRM::Context::Inner GRM::Context::operator[](const std::string &str)
{
  /*!
   * Overloaded operator[] of GRM::Context. This is used to mimic std::map's usage syntax
   *
   * \param[in] str A std::string used as the key for GRM::Context::Inner
   * \returns GRM::Context::Inner containing access to this GRM::Context object and str as key
   */
  return Inner(*this, str);
}

const GRM::Context::Inner GRM::Context::operator[](const std::string &str) const
{
  /*!
   * The const overloaded operator[] of GRM::Context. This is used to mimic std::map's usage syntax
   *
   * \param[in] str A std::string used as the key for GRM::Context::Inner
   * \returns const GRM::Context::Inner containing access to this GRM::Context object and str as key
   */
  return Inner(*this, str);
}

/*!
 * \brief Construct a new GRM::Context iterator.
 *
 * \param[in] context The context to iterate over.
 * \param[in] is_end_iterator Set this to true if an end iterator shall be created.
 */
GRM::Context::Iterator::Iterator(Context &context, bool is_end_iterator)
    : context_(context), table_double_it_(context.table_double.begin()), table_int_it_(context.table_int.begin()),
      table_string_it_(context.table_string.begin()), current_it_(table_double_it_)
{
  if (is_end_iterator)
    {
      table_double_it_ = context.table_double.end();
      table_int_it_ = context.table_int.end();
      table_string_it_ = context.table_string.end();
    }
  else
    {
      current_it_ = nextIterator();
    }
};

/*!
 * \brief Overload of the dereference operator.
 *
 * Please note, that this iterator has no arrow operator defined. The arrow operator must return a pointer which is not
 * possible with a variadic value type which is hold by this iterator (the pointer would be returned in a variant and
 * this couldn't be used for direct dereferencing; expressions like `it->first` wouldn't work).
 *
 * \return A reference to the value the iterator is currently pointing to
 */
GRM::Context::Iterator::reference GRM::Context::Iterator::operator*()
{
  return std::visit([](auto &&it_ref_wrapper) -> reference { return *it_ref_wrapper.get(); }, current_it_);
}

/*!
 * \brief Overload of the prefix increment operator.
 */
GRM::Context::Iterator &GRM::Context::Iterator::operator++()
{
  std::visit([](auto &&it) { ++it.get(); }, current_it_);
  current_it_ = nextIterator();
  return *this;
}

/*!
 * \brief Overload of the postfix increment operator.
 */
GRM::Context::Iterator GRM::Context::Iterator::operator++(int)
{
  Iterator tmp{*this};
  ++(*this);
  return tmp;
}

/*!
 * \brief Overload of the equality operator.
 *
 * \param[in] a First GRM::Context iterator to compare.
 * \param[in] b Second GRM::Context iterator to compare.
 * \return The equality of the two iterators.
 */
bool GRM::operator==(const GRM::Context::Iterator &a, const GRM::Context::Iterator &b)
{
  return a.table_double_it_ == b.table_double_it_ && a.table_int_it_ == b.table_int_it_ &&
         a.table_string_it_ == b.table_string_it_;
}

/*!
 * \brief Overload of the inequality operator.
 *
 * \param[in] a First GRM::Context iterator to compare.
 * \param[in] b Second GRM::Context iterator to compare.
 * \return The inequality of the two iterators.
 */
bool GRM::operator!=(const GRM::Context::Iterator &a, const GRM::Context::Iterator &b)
{
  return !(a == b);
}

/*! \brief Find the next iterator of the underlying data structures to process.
 *
 * A GRM::Context iterator internally stores iterators to all data structures within the GRM::Context object. This
 * function is used to find the next iterator to process. Since all iterators process sorted pairs of string keys and
 * data values, the next iterator will always be the one with the smallest string key. By repeatedly calling this
 * function, all items of all iterators will be processed in lexicographic order.
 *
 * \return The next internal iterator to process
 */
std::variant<std::reference_wrapper<std::map<std::string, std::vector<double>>::iterator>,
             std::reference_wrapper<std::map<std::string, std::vector<int>>::iterator>,
             std::reference_wrapper<std::map<std::string, std::vector<std::string>>::iterator>>
GRM::Context::Iterator::nextIterator()
{
  auto is_it_lt = [](const auto &it_a, const auto &it_b, const auto &it_a_end, const auto &it_b_end) {
    if (it_a != it_a_end && it_b != it_b_end) return it_a->first < it_b->first;
    return it_a != it_a_end && it_b == it_b_end;
  };

  if (is_it_lt(table_double_it_, table_int_it_, context_.table_double.end(), context_.table_int.end()) &&
      is_it_lt(table_double_it_, table_string_it_, context_.table_double.end(), context_.table_string.end()))
    return table_double_it_;
  if (is_it_lt(table_int_it_, table_string_it_, context_.table_int.end(), context_.table_string.end()))
    return table_int_it_;
  return table_string_it_;
}

/*!
 * \brief Create a iterator pointing to the first element of this context.
 */
GRM::Context::Iterator GRM::Context::begin()
{
  return Iterator(*this);
}

/*!
 * \brief Create a iterator pointing past the last element of this context.
 */
GRM::Context::Iterator GRM::Context::end()
{
  return Iterator(*this, true);
}
