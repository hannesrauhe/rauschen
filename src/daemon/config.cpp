#include "config.hpp"

RauschenConfig::RauschenConfig()
{
  if( !boost::filesystem::is_directory( getRauschenDir() ) )
  {
    boost::filesystem::create_directory( getRauschenDir() );
  }
}

boost::filesystem::path RauschenConfig::get_home_dir() const
{
  boost::filesystem::path res;

  // if HOME is set, use HOME
  char *home = getenv("HOME");
  if ( home )
  {
    res = home;
  }
  else
  {
    // if HOME is not set, try HOMEDRIVE + HOMEPATH
    char *home_drive = getenv("HOMEDRIVE");
    char *home_path = getenv("HOMEPATH");
    if ( home_drive && home_path )
    {
      res = home_drive;
      res /= getenv("HOMEPATH");
    }
  }
  return res;
}
