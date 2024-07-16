#pragma once

#include <iostream>

void error(std::string msg) {
    std::cout << "\x1b[30;41;1m ✖ \x1b[0m " << msg << std::endl;
}

void success(std::string msg) {
    std::cout << "\x1b[30;42;1m ✔ \x1b[0m " << msg << std::endl;
}

void info(std::string msg) {
    std::cout << "\x1b[30;44;1m i \x1b[0m " << msg << std::endl;
}

void query(std::string msg, bool endl = true) {
    if (endl)
        std::cout << "\x1b[30;45;1m ? \x1b[0m " << msg << std::endl;
    else
        std::cout << "\x1b[30;45;1m ? \x1b[0m " << msg;
}