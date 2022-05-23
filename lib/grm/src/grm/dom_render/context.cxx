#include <utility>
#include <vector>

#include <grm/dom_render/NotFoundError.hxx>
#include <grm/dom_render/TypeError.hxx>
#include <grm/dom_render/context.hxx>


GR::Context::Context() = default; /*! default constructor for GR::Context*/

GR::Context::Inner::Inner(Context &context, std::string key) : context(&context), key(std::move(key))
{
  /*!
   * The non const constructor for GR::Context::Inner
   *
   * \param[in] context The GR::Context it belongs to
   * \param[in] key The assigned key for Inner
   */
}
GR::Context::Inner::Inner(const Context &context, std::string key)
    : context(&const_cast<Context &>(context)), key(std::move(key))
{
  /*!
   * The const construtor for GR::Context::Inner
   *
   * \param[in] context A const GR::Context it belongs to
   * \param[in] key The assigned key for Inner
   */
}

bool GR::Context::Inner::intUsed()
{
  /*!
   * This function is used for checking if the tableInt map of GR::Context contains a value for GR::Context::Inner's key
   *
   * \returns a bool indicating the usage of GR::Context::Inner::key by GR::Context::tableInt
   */
  return context->tableInt.find(key) != context->tableInt.end();
}

bool GR::Context::Inner::doubleUsed()
{
  /*!
   * This function is used for checking if the tableDouble map of GR::Context contains a value for GR::Context::Inner's
   * key
   *
   * \returns a bool indicating the usage of GR::Context::Inner::key by GR::Context::tableDouble
   */
  return context->tableDouble.find(key) != context->tableDouble.end();
}


GR::Context::Inner &GR::Context::Inner::operator=(std::vector<double> vec)
{
  /*!
   * Overloaded operator= for GR::Context::Inner assigning std::vector<double>
   * Stores the vector in GR::Context::tableDouble with GR::Context::Inner's key
   * Throws a TypeError if the GR::Context::Inner key is already used by GR::Context::tableInt
   */
  if (intUsed())
    {
      throw TypeError("Wrong Type: std::vector<int> expected\n");
    }
  else
    {
      context->tableDouble[key] = std::move(vec);
      return *this;
    }
}

GR::Context::Inner &GR::Context::Inner::operator=(std::vector<int> vec)
{
  /*!
   * Overloaded operator= for GR::Context::Inner assigning std::vector<int>
   * Stores the vector in GR::Context::tableInt with GR::Context::Inner's key
   * Throws a TypeError if the GR::Context::Inner key is already used by GR::Context::tableDouble
   */
  if (doubleUsed())
    {
      throw TypeError("Wrong type: std::vector<double> expected\n");
    }
  else
    {
      context->tableInt[key] = std::move(vec);
      return *this;
    }
}


GR::Context::Inner::operator std::vector<int> &()
{
  /*!
   * Overloaded operator std::vector<int>& used for converting GR::Context::Inner to std::vector
   * This operator is used in GR::get
   *
   * Throws a NotFoundError if there is no vector found in tableInt with Inner's key
   */
  if (context->tableInt.find(key) != context->tableInt.end())
    {
      return context->tableInt[key];
    }
  throw NotFoundError("No integer value found for given key");
}

GR::Context::Inner::operator const std::vector<int> &() const
{
  /*!
   * The const overloaded operator std::vector<int>& used for converting GR::Context::Inner to const std::vector
   * This operator is used in GR::get
   *
   * Throws a NotFoundError if there is no vector found in tableInt with Inner's key
   */
  if (context->tableInt.find(key) != context->tableInt.end())
    {
      return context->tableInt[key];
    }
  throw NotFoundError("No integer value found for given key");
}

GR::Context::Inner::operator std::vector<double> &()
{
  /*!
   * Overloaded operator std::vector<double>& used for converting GR::Context::Inner to std::vector
   * This operator is used in GR::get
   *
   * Throws a NotFoundError if there is no vector found in tableDouble with Inner's key
   */
  if (context->tableDouble.find(key) != context->tableDouble.end())
    {
      return context->tableDouble[key];
    }
  throw NotFoundError("No double value found for given key");
}

GR::Context::Inner::operator const std::vector<double> &() const
{
  /*!
   * The const overloaded operator std::vector<double>& used for converting GR::Context::Inner to std::vector
   * This operator is used in GR::get
   *
   * Throws a NotFoundError if there is no vector found in tableDouble with Inner's key
   */
  if (context->tableDouble.find(key) != context->tableDouble.end())
    {
      return context->tableDouble[key];
    }
  throw NotFoundError("No double value found for given key");
}


GR::Context::Inner::operator std::vector<int> *()
{
  /*!
   * Overloaded operator std::vector<int>* used for converting GR::Context::Inner to a std::vector pointer
   * This operator is used in GR::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableInt with Inner's key
   */
  if (context->tableInt.find(key) != context->tableInt.end())
    {
      return &context->tableInt[key];
    }
  return nullptr;
}

GR::Context::Inner::operator const std::vector<int> *() const
{
  /*!
   * The const overloaded operator std::vector<int>* used for converting GR::Context::Inner to a std::vector pointer
   * This operator is used in GR::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableInt with Inner's key
   */
  if (context->tableInt.find(key) != context->tableInt.end())
    {
      return &context->tableInt[key];
    }
  return nullptr;
}


GR::Context::Inner::operator std::vector<double> *()
{
  /*!
   * Overloaded operator std::vector<double>* used for converting GR::Context::Inner to a std::vector pointer
   * This operator is used in GR::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableDouble with Inner's key
   */
  if (context->tableDouble.find(key) != context->tableDouble.end())
    {
      return &context->tableDouble[key];
    }
  return nullptr;
}

GR::Context::Inner::operator const std::vector<double> *() const
{
  /*!
   * The const overloaded operator std::vector<double>* used for converting GR::Context::Inner to a std::vector pointer
   * This operator is used in GR::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableDouble with Inner's key
   */
  if (context->tableDouble.find(key) != context->tableDouble.end())
    {
      return &context->tableDouble[key];
    }
  return nullptr;
}


GR::Context::Inner GR::Context::operator[](const std::string &str)
{
  /*!
   * Overloaded operator[] of GR::Context. This is used to mimic std::map's usage syntax
   *
   * \param[in] str A std::string used as the key for GR::Context::Inner
   * \returns GR::Context::Inner containing access to this GR::Context object and str as key
   */
  return Inner(*this, str);
}


const GR::Context::Inner GR::Context::operator[](const std::string &str) const
{
  /*!
   * The const overloaded operator[] of GR::Context. This is used to mimic std::map's usage syntax
   *
   * \param[in] str A std::string used as the key for GR::Context::Inner
   * \returns const GR::Context::Inner containing access to this GR::Context object and str as key
   */
  return Inner(*this, str);
}
