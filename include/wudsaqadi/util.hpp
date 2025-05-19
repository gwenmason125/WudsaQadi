#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#define ANSI_CLEAR_SCREEN           "\033[2J"
#define ANSI_HIDE_CURSOR            "\033[?25l"
#define ANSI_CURSOR_GOTO(row, col)  "\033[" << row << ";" << col << "H"
#define ANSI_BLUE                   "\033[34m"
#define ANSI_RED                    "\033[31m"

namespace wudsaqadi {

std::vector<std::string> readLinesFromFile(const std::string& path);
void printColoredDots(size_t leftWidth, size_t rightWidth);

}