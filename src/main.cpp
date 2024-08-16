#include "Bus.hpp"

#include <iostream>

int main() {
    try{
        Bus bus;
        bus.start();
    }
    catch(std::runtime_error e) {
        std::cerr << e.what() << std::endl;
    }
    catch(...) {
        std::cerr << "unknown exception" << std::endl;
    }
    return 0;
}
