#include <map>
#include <string>
#include <vector>

class Context
{

private:
  int idInt = 0;
  int idDouble = 0;

public:
  std::map<std::string, std::vector<double>> tableDouble;
  std::map<std::string, std::vector<int>> tableInt;

  std::string insertIntoDoubleTable(const std::string &keyName, const std::vector<double> &data);
  std::string insertIntoIntTable(const std::string &keyName, const std::vector<int> &data);

  void replaceInDoubleTable(const std::string &key, const std::vector<double> &data, bool force = false);
  void replaceInIntTable(const std::string &key, const std::vector<int> &data, bool force = false);
  void clearDoubleTable();
  void clearIntTable();

  void removeFromDoubleTable(const std::string &key);
  void removeFromIntTable(const std::string &key);

  bool doubleTableHasKey(const std::string &key);
  bool intTableHasKey(const std::string &key);

  std::vector<double> &getFromDoubleTable(const std::string &key);
  std::vector<int> &getFromIntTable(const std::string &key);
  // or just return primitive pointers?
  int *getFromIntTablePointer(const std::string &key);

  // export function?
};
