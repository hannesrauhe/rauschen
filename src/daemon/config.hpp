#pragma once

#include <boost/filesystem.hpp>

class RauschenConfig
{
public:
  RauschenConfig();

  boost::filesystem::path getRauschenDir() const { return (get_home_dir() / ".rauschen"); }

  std::string getHostsFile() const { return (getRauschenDir() / RAUSCHEN_HOSTS_FILE).string(); }
  std::string getKeyFile() const { return (getRauschenDir() / RAUSCHEN_KEY_FILE).string(); }

protected:
  boost::filesystem::path get_home_dir() const;

  const char* RAUSCHEN_HOSTS_FILE = "hosts.txt";
  const char* RAUSCHEN_KEY_FILE = "key";
};

static RauschenConfig rauschen_config;
