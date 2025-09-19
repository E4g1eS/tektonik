module;

#include <iostream>
#include <vector>

export module app;

export namespace app
{

class App
{
  public:
    void Init() {
        std::cout << "App initialized." << std::endl;
    }
  private:
};

}
