/**
 * Nikolaus Mayer, 2018 (mayern@cs.uni-freiburg.de)
 */

#ifndef QUEUEPROCESSOR_H__
#define QUEUEPROCESSOR_H__

/// System/STL
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace QueueProcessor
{

  template <class T>
  class QueueProcessor
  {
    typedef std::shared_ptr<T> item_t;

    public:

      QueueProcessor(
            std::function<int(item_t&)> func,
            unsigned int number_of_threads,
            bool start_immediately);

      ~QueueProcessor();

      void Start();

      void Stop();

      void BlockUntilDone() const;

      bool IsDone() const;

      void Give(
            item_t item_ptr);
      void Give(
            T raw_item);

    private:

      void _WatchQueue();

      int _ProcessItem(
            item_t item_ptr);

      void _PushItem(
            item_t item_ptr);

      bool m_running;
      unsigned int m_threads;
      bool m_print_information;
      std::function<int(item_t&)> m_item_processor_handle;

      std::queue<item_t> m_queue;

      std::vector<std::thread*> m_worker_threads;
      std::mutex m_queue_lock;
  };



  /**
   * Implementation
   */

  /// Not pretty but readable
  #define QPT QueueProcessor<T>

  template <class T>
  QPT::QueueProcessor(std::function<int(item_t&)> func,
                      unsigned int number_of_threads,
                      bool start_immediately)
    : m_running(start_immediately),
      m_threads(number_of_threads),
      m_item_processor_handle(func)
  {
    if (start_immediately) {
      m_worker_threads.resize(m_threads);
      for (size_t i = 0; i < m_worker_threads.size(); ++i) {
        m_worker_threads[i] = new std::thread(&QPT::_WatchQueue, this);
      }
    }
  }

  
  template <class T>
  QPT::~QueueProcessor()
  {
    m_running = false;

    /// Stop and delete worker threads
    for (size_t i = 0; i < m_worker_threads.size(); ++i) {
      if (m_worker_threads[i]) {
        if (m_worker_threads[i]->joinable()) {
          m_worker_threads[i]->join();
        }
        delete m_worker_threads[i];
      }
    }
    m_worker_threads.clear();
  }


  template <class T>
  void QPT::Start()
  {
    if (m_running)
      return;

    m_running = true;
    m_worker_threads.resize(m_threads);
    for (size_t i = 0; i < m_worker_threads.size(); ++i) {
      m_worker_threads[i] = new std::thread(&QPT::_WatchQueue, this);
    }
  }


  template <class T>
  void QPT::Stop()
  {
    if (not m_running)
      return;

    m_running = false;
    /// Deplete queue (will NOT wait until items are processed)
    m_queue_lock.lock();
    while (m_queue.size() > 0) {
      m_queue.pop();
    }
    m_queue_lock.unlock();
    /// Stop and delete worker threads
    for (size_t i = 0; i < m_worker_threads.size(); ++i) {
      if (m_worker_threads[i]) {
        if (m_worker_threads[i]->joinable()) {
          m_worker_threads[i]->join();
        }
        delete m_worker_threads[i];
      }
    }
    m_worker_threads.clear();
  }


  template <class T>
  void QPT::BlockUntilDone() const
  {
    while (not IsDone()) {
      std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
  }


  template <class T>
  bool QPT::IsDone() const
  {
    return (m_queue.size() == 0);
  }
  

  template <class T>
  void QPT::Give(item_t item_ptr)
  {
    std::lock_guard<std::mutex> lock(m_queue_lock);
    m_queue.push(item_ptr);
  }

  template <class T>
  void QPT::Give(T raw_item)
  {
    std::lock_guard<std::mutex> lock(m_queue_lock);
    m_queue.push(std::make_shared<T>(raw_item));
  }

  
  template <class T>
  void QPT::_WatchQueue()
  {
    while (m_running or m_queue.size() > 0) {
      if (m_queue.size() > 0) {
        /// Read queue
        item_t item;
        m_queue_lock.lock();
        try {
          item = m_queue.front();
        } catch(...) {
          m_queue_lock.unlock();
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
        
        /// Pop from queue
        try {
          m_queue.pop();
        } catch(...) {
          m_queue_lock.unlock();
          continue;
        }
        m_queue_lock.unlock();

        /// Process item
        _ProcessItem(item);
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }


  template <class T>
  int QPT::_ProcessItem(item_t item)
  {
    return m_item_processor_handle(item);
  }


  #undef QPT


}  // namespace QueueProcessor


#endif  // QUEUEPROCESSOR_H__

