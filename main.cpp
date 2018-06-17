/**
 * Nikolaus Mayer, 2018 (mayern@cs.uni-freiburg.de)
 */

/// System/STL
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
/// Local files
#include "QueueProcessor.h"

struct Item
{
  Item(int i)
  : i{i} 
  { }

  ~Item() 
  { }

  int i;
};

int process_item(std::shared_ptr<Item>& item)
{
  std::cout << "item i = " << item->i << std::endl;
  return 0;
}

int main()
{
  QueueProcessor::QueueProcessor<Item> QP{process_item, 1, false};
  for (int i = 0; i < 1000; ++i) {
    std::shared_ptr<Item> item = std::make_shared<Item>(i);
    QP.Give(item);
  }
  QP.Start();
  QP.BlockUntilDone();
  QP.Stop();

  return EXIT_SUCCESS;
}

