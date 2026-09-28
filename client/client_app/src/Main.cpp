#include "App.h"

#include <string_view>

int main(int argc, char** argv) {
    client_app::App app;
    if (argc == 2 && std::string_view(argv[1]) == "--smoke") {
        return app.runSmoke();
    }
    return app.run();
}
