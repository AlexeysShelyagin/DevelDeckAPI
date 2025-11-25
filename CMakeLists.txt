cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/DevelDeckAPI.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES arduino-esp32 TFT_eSPI@^2.5.43)

project(Gamepad-API)