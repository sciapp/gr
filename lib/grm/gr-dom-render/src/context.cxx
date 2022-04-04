#include "context.hxx"
#include "string"


std::string Context::insertIntoDoubleTable(const std::string &keyName, const std::vector<double> &data)
{
  /*!
   * Inserts a key_name with a unique ID into the doubleTable as the key with data as its value
   */
  std::string newKey = keyName + std::to_string(this->idDouble);
  this->idDouble++;
  this->tableDouble.insert(std::make_pair(newKey, data));
  return newKey;
}

std::string Context::insertIntoIntTable(const std::string &keyName, const std::vector<int> &data)
{
  std::string newKey = keyName + std::to_string(this->idInt);
  this->idInt++;
  this->tableInt.insert(std::make_pair(newKey, data));
  return newKey;
}

void Context::clearDoubleTable()
{
  this->tableDouble.clear();
}

void Context::clearIntTable()
{
  this->tableInt.clear();
}

void Context::removeFromDoubleTable(const std::string &key)
{
  if (this->doubleTableHasKey(key))
    {
      this->tableDouble.erase(key);
    }
  else
    {
      // todo: Error?
    }
}

void Context::removeFromIntTable(const std::string &key)
{
  if (this->intTableHasKey(key))
    {
      this->tableInt.erase(key);
    }
  else
    {
      // todo: exception?
    }
}

void Context::replaceInDoubleTable(const std::string &key, const std::vector<double> &data, bool force)
{
  if (force || this->doubleTableHasKey(key))
    {
      this->tableDouble[key] = data;
    }
}

void Context::replaceInIntTable(const std::string &key, const std::vector<int> &data, bool force)
{
  if (force || this->intTableHasKey(key))
    {
      this->tableInt[key] = data;
    }
}

bool Context::doubleTableHasKey(const std::string &key)
{
  if (this->tableDouble.find(key) == this->tableDouble.end()) return false;
  return true;
}

bool Context::intTableHasKey(const std::string &key)
{
  if (this->tableInt.find(key) == this->tableInt.end()) return false;
  return true;
}

std::vector<double> &Context::getFromDoubleTable(const std::string &key)
{
  if (this->doubleTableHasKey(key)) return this->tableDouble[key];
  // todo: else exception?
}

std::vector<int> &Context::getFromIntTable(const std::string &key)
{
  if (this->intTableHasKey(key)) return this->tableInt[key];
}
// todo: or get int etc pointers

int *Context::getFromIntTablePointer(const std::string &key)
{
  if (this->intTableHasKey(key)) return &(this->tableInt[key][0]);
}