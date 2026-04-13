#ifndef CONFIGSTORE_H
  #define CONFIGSTORE_H
  
  #include <string>
  
  class ConfigStore {
  private:
      std::string filePath;
  
  public:
      ConfigStore(const std::string& path);
      void load();
  };
  
  #endif
 
