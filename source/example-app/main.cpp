#include <iostream>

import tektonik;

int main(int argc, char* argv[])
{
    tektonik::Runtime runtime = tektonik::Runtime(tektonik::Runtime::RunOptions{.argc = argc, .argv = argv});
    runtime.Test();
    runtime.Init();

    std::cout << "### END OF MAIN ###" << std::endl;
    return 0;
}
