import tektonik;

int main(int argc, char* argv[])
{
    tektonik::Runtime runtime = tektonik::Runtime(tektonik::Runtime::RunOptions{.argc = argc, .argv = argv});
    runtime.Test();
    return 0;
}
