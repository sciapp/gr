#include <unordered_map>
#include <iostream>

#include "vdr_int.hxx"
#include "plugin_int.hxx"
#include "plugin_loader_int.hxx"

std::unordered_map<std::string, std::string> Vdr::plugin_names{
    {"csv", "csv_plugin"}, {"dat", "csv_plugin"}, {"xsf", "xsf_plugin"}};

std::string getFileExtension(const std::string &file_name)
{
  if (auto dot_pos = file_name.find_last_of("."); dot_pos != std::string::npos) return file_name.substr(dot_pos + 1);
  return "";
}

Vdr::~Vdr()
{
  loaders.clear();
}

DataSource *Vdr::loadSourceForFile(const std::string &path)
{
  std::string extension = getFileExtension(path);
  std::string plugin_name = "csv_plugin"; // Use CSV as fallback
  if (path != "-")
    {
      try
        {
          plugin_name = Vdr::plugin_names.at(extension);
        }
      catch (std::out_of_range)
        {
          std::cerr << "The file extension \"" << extension
                    << "\" is not assigned to any input plugin, assuming CSV input." << std::endl;
        }
    }

  auto iter = loaders.find(plugin_name);
  if (iter != loaders.end()) return iter->second->getPlugin()->getDataSourceFromFile(path);
  Plugin *p = loadPlugin(plugin_name);
  if (!p) return nullptr;
  return p->getDataSourceFromFile(path);
}

Plugin *Vdr::loadPlugin(const std::string &name)
{
  std::unique_ptr<PluginLoader> loader = std::make_unique<PluginLoader>(name);
  Plugin *p = loader->getPlugin();
  if (p == NULL)
    {
      std::cerr << "Error in loading or allocating plugin \"" << name << "\"." << std::endl;
      return nullptr;
    }
  loaders.emplace(name, std::move(loader));
  return p;
}
void Vdr::unloadPlugin(const std::string &name)
{
  auto iter = loaders.find(name);
  if (iter == loaders.end())
    {
      std::cerr << "Trying to unload plugin \"" << name << "\" which has not been loaded." << std::endl;
      return;
    }
  loaders.erase(iter);
}
