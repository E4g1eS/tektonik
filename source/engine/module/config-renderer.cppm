module;
#include "std.hpp"
export module config_renderer;

namespace tektonik::config
{

export class Renderer
{
  public:
    Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Init();

  private:
};

}  // namespace tektonik::config
