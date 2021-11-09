#include <iostream>
#include "layout.hpp"

Layout::Layout() : test_(42) {}

void Layout::print_test() const
{
  std::cout << "Test: " << test_ << std::endl;
}
