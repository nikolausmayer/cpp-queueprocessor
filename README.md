cpp-queueprocessor
==================
C++ tool: Use multiple threads to process custom queue items

Usage example
=============
See `main.cpp`:
* create a `struct` or `class` for your queue items
* write a function that takes a `std::shared_ptr<YOUR_ITEM_TYPE>&` and returns `int`
* create a `QueueProcessor::QueueProcessor`
* (now or later) start the QueueProcessor
* feed `std::shared_ptr` items to the QueueProcessor
* stop the QueueProcessor

