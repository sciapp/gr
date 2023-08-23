#include <utility>
#include <vector>

#include <grm/dom_render/NotFoundError.hxx>
#include <grm/dom_render/TypeError.hxx>
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
   * The const construtor for GRM::Context::Inner
   *
   * \param[in] context A const GRM::Context it belongs to
   * \param[in] key The assigned key for Inner
   */
}

bool GRM::Context::Inner::intUsed()
{
  /*!
   * This function is used for checking if the tableInt map of GRM::Context contains a value for GRM::Context::Inner's
   * key
   *
   * \returns a bool indicating the usage of GRM::Context::Inner::key by GRM::Context::tableInt
   */
  return context->tableInt.find(key) != context->tableInt.end();
}

bool GRM::Context::Inner::doubleUsed()
{
  /*!
   * This function is used for checking if the tableDouble map of GRM::Context contains a value for
   * GRM::Context::Inner's key
   *
   * \returns a bool indicating the usage of GRM::Context::Inner::key by GRM::Context::tableDouble
   */
  return context->tableDouble.find(key) != context->tableDouble.end();
}

bool GRM::Context::Inner::stringUsed()
{
  /*!
   * This function is used for checking if the tableString map of GRM::Context contains a value for
   * GRM::Context::Inner's key
   *
   * \returns a bool indicating the usage of GRM::Context::Inner::key by GRM::Context::tableString
   */
  return context->tableString.find(key) != context->tableString.end();
}


GRM::Context::Inner &GRM::Context::Inner::operator=(std::vector<double> vec)
{
  /*!
   * Overloaded operator= for GRM::Context::Inner assigning std::vector<double>
   * Stores the vector in GRM::Context::tableDouble with GRM::Context::Inner's key
   * Throws a TypeError if the GRM::Context::Inner key is already used by other GRM::Context tableTYPES
   */
  if (intUsed() || stringUsed())
    {
      throw TypeError("Wrong Type: std::vector<double> expected\n");
    }
  else
    {
      context->tableDouble[key] = std::move(vec);
      return *this;
    }
}

GRM::Context::Inner &GRM::Context::Inner::operator=(std::vector<int> vec)
{
  /*!
   * Overloaded operator= for GRM::Context::Inner assigning std::vector<int>
   * Stores the vector in GRM::Context::tableInt with GRM::Context::Inner's key
   * Throws a TypeError if the GRM::Context::Inner key is already used by other GRM::Context tableTYPES
   */
  if (doubleUsed() || stringUsed())
    {
      throw TypeError("Wrong type: std::vector<int> expected\n");
    }
  else
    {
      context->tableInt[key] = std::move(vec);
      return *this;
    }
}


GRM::Context::Inner &GRM::Context::Inner::operator=(std::vector<std::string> vec)
{
  /*!
   * Overloaded operator= for GRM::Context::Inner assigning std::vector<std::string>
   * Stores the vector in GRM::Context::tableString with GRM::Context::Inner's key
   * Throws a TypeError if the GRM::Context::Inner is already used by other GRM::Context tableTYPES
   */
  if (intUsed() || doubleUsed())
    {
      throw TypeError("Wrong type: std::vector<std::string> expected\n");
    }
  else
    {
      context->tableString[key] = std::move(vec);
      return *this;
    }
}


GRM::Context::Inner::operator std::vector<int> &()
{
  /*!
   * Overloaded operator std::vector<int>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in tableInt with Inner's key
   */
  if (context->tableInt.find(key) != context->tableInt.end())
    {
      return context->tableInt[key];
    }
  throw NotFoundError("No integer value found for given key");
}

GRM::Context::Inner::operator const std::vector<int> &() const
{
  /*!
   * The const overloaded operator std::vector<int>& used for converting GRM::Context::Inner to const std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in tableInt with Inner's key
   */
  if (context->tableInt.find(key) != context->tableInt.end())
    {
      return context->tableInt[key];
    }
  throw NotFoundError("No integer value found for given key");
}

GRM::Context::Inner::operator std::vector<double> &()
{
  /*!
   * Overloaded operator std::vector<double>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in tableDouble with Inner's key
   */
  if (context->tableDouble.find(key) != context->tableDouble.end())
    {
      return context->tableDouble[key];
    }
  throw NotFoundError("No double value found for given key");
}

GRM::Context::Inner::operator const std::vector<double> &() const
{
  /*!
   * The const overloaded operator std::vector<double>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in tableDouble with Inner's key
   */
  if (context->tableDouble.find(key) != context->tableDouble.end())
    {
      return context->tableDouble[key];
    }
  throw NotFoundError("No double value found for given key");
}

GRM::Context::Inner::operator std::vector<std::string> &()
{
  /*!
   * Overloaded operator std::vector<std::string>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in tableString with Inner's key
   */
  if (context->tableString.find(key) != context->tableString.end())
    {
      return context->tableString[key];
    }
  throw NotFoundError("No string value found for given key");
}

GRM::Context::Inner::operator const std::vector<std::string> &() const
{
  /*!
   * The const overloaded operator std::vector<std::string>& used for converting GRM::Context::Inner to std::vector
   * This operator is used in GRM::get
   *
   * Throws a NotFoundError if there is no vector found in tableString with Inner's key
   */
  if (context->tableString.find(key) != context->tableString.end())
    {
      return context->tableString[key];
    }
  throw NotFoundError("No string value found for given key");
}

GRM::Context::Inner::operator std::vector<int> *()
{
  /*!
   * Overloaded operator std::vector<int>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableInt with Inner's key
   */
  if (context->tableInt.find(key) != context->tableInt.end())
    {
      return &context->tableInt[key];
    }
  throw NotFoundError("No int value found for given key");
}

GRM::Context::Inner::operator const std::vector<int> *() const
{
  /*!
   * The const overloaded operator std::vector<int>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableInt with Inner's key
   */
  if (context->tableInt.find(key) != context->tableInt.end())
    {
      return &context->tableInt[key];
    }
  throw NotFoundError("No int value found for given key");
}


GRM::Context::Inner::operator std::vector<double> *()
{
  /*!
   * Overloaded operator std::vector<double>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableDouble with Inner's key
   */
  if (context->tableDouble.find(key) != context->tableDouble.end())
    {
      return &context->tableDouble[key];
    }
  throw NotFoundError("No double value found for given key");
}

GRM::Context::Inner::operator const std::vector<double> *() const
{
  /*!
   * The const overloaded operator std::vector<double>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableDouble with Inner's key
   */
  if (context->tableDouble.find(key) != context->tableDouble.end())
    {
      return &context->tableDouble[key];
    }
  throw NotFoundError("No double value found for given key");
}

GRM::Context::Inner::operator std::vector<std::string> *()
{
  /*!
   * Overloaded operator std::vector<std::string>* used for converting GRM::Context::Inner to a std::vector pointer
   * This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableString with Inner's key
   */
  if (context->tableString.find(key) != context->tableString.end())
    {
      return &context->tableString[key];
    }
  throw NotFoundError("No std::string value found for given key");
}


GRM::Context::Inner::operator const std::vector<std::string> *() const
{
  /*!
   * Const overloaded operator std::vector<std::string>* used for converting GRM::Context::Inner to a std::vector
   * pointer This operator is used in GRM::get_if
   *
   * Throws a NotFoundError if there is no vector found in tableString with Inner's key
   */
  if (context->tableString.find(key) != context->tableString.end())
    {
      return &context->tableString[key];
    }
  throw NotFoundError("No std::string value found for given key");
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
