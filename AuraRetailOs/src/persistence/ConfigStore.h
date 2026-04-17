  #ifndef CONFIGSTORE_H
  #define CONFIGSTORE_H
  #include <string>
   
  class ConfigStore {
  private: std::string filePath;
  public:
      ConfigStore(const std::string& path);
      void load();  // reads JSON key-value pairs into CentralRegistry
  };
  #endif
