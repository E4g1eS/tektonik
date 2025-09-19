export module runtime;

import app;

export namespace runtime
{

class Runtime
{
  public:
    void Run() {
        auto app = app::App();
        app.Init();
    };
  private:
};

}
