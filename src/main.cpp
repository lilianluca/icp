//
// ICP: ztrátová komprese videa (JPEG)
//

#include <iostream>

#include "App.h"

int main()
{
    try {
        App app;
        if (!app.init())
            return EXIT_FAILURE;
        return app.run();
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
