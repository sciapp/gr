#include <iostream>
#include <vector>

#include <grm/utilcpp_int.hxx>
#include "test.h"


const std::vector<std::string> test_strings = {
    "--", "---", "----", "-----", "-\\-", "-\\--", "-\\--\\-", "-\\-\\\\-\\-", "a -\\\\\\-\\\\--\\-- b",
};

void test()
{
  for (const auto &test_string : test_strings)
    {
      std::cout << "original input: " << test_string << std::endl;
      auto escaped_minuses = escapeDoubleMinus(test_string);
      std::cout << "escaped:        " << escaped_minuses << std::endl;
      auto unescaped_double_minuses = unescapeDoubleMinus(escaped_minuses);
      std::cout << "unescaped:      " << unescaped_double_minuses << std::endl << std::endl;
      assert(test_string == unescaped_double_minuses);
    }

  std::cout << std::endl;

  for (auto it = std::begin(test_strings); it != std::begin(test_strings) + 4; ++it)
    {
      std::cout << "original input: " << *it << std::endl;
      auto unescaped_double_minuses = unescapeDoubleMinus(*it);
      std::cout << "unescaped:      " << unescaped_double_minuses << std::endl << std::endl;
      assert(unescaped_double_minuses == *it);
    }
}

DEFINE_TEST_MAIN
