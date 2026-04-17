  #ifndef STOCKOBSERVER_H
  #define STOCKOBSERVER_H
  #include <string>
   
  // Observer pattern interface -- receives alerts when stock drops low
  class StockObserver {
  public:
      virtual void onLowStock(const std::string& productId, int remaining) = 0;
      virtual ~StockObserver() {}
  };
  #endif
 
