#pragma once

struct Aggregate final {
  int number = 0;
  bool condition = false;
};

bool operator==(const Aggregate& l, const Aggregate& r);

// function to test user data as parameter and return value
Aggregate function_test_aggregate(Aggregate arg);
