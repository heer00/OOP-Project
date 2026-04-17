  #ifndef TRANSACTIONLOG_H
  #define TRANSACTIONLOG_H
  #include <string>
  #include <vector>
   
  class TransactionLog {
  private: std::string filePath;
  public:
      TransactionLog(const std::string& path);
      void append(const std::string& logLine);         // appends one line to CSV
      std::vector<std::string> readAll();              // returns all log entries
  };
  #endif
 
