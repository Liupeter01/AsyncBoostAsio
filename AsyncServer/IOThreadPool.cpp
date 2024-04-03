#include "IOThreadPool.h"

IOThreadPool::IOThreadPool(std::size_t poolSize)
          : _threads(poolSize) ,_uni_work(new WORK(this->_io_context))
{
          for (std::size_t i = 0; i < poolSize; ++i) {
                    this->_threads.emplace_back([this]() {
                              _io_context.run();
                    });
          }
}


IOThreadPool::~IOThreadPool()
{
          this->stopThreadPool();
}

boost::asio::io_context& IOThreadPool::GetIOContext()
{
          return _io_context;
}

void IOThreadPool::stopThreadPool()
{
          _uni_work.reset();
          for (auto& thread : _threads) {
                    if (thread.joinable()) {
                              thread.join();
                    }
          }
}