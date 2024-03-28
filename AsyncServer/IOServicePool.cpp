#include "IOServicePool.h"

IOServicePool::IOServicePool(std::size_t poolSize)
          :_cur_io_servies(0),
          _threads(poolSize),
          _io_contexts(poolSize),
          _uni_works(poolSize)
{
          /*allocate unique_ptr memory*/
          for (std::size_t i = 0; i < poolSize; ++i) {
                    this->_uni_works[i] = std::unique_ptr<WORK>(new WORK(this->_io_contexts[i]));
          }
          /*start threads*/
          for (std::size_t i = 0; i < this->_threads.size(); ++i) {
                    this->_threads.emplace_back([this, i]() {_io_contexts[i].run(); });
          }
}

void IOServicePool::stopServicePool()
{
          /*deallocated all UNIQUE_WORK*/
          for (auto& work : this->_uni_works) {
                    work.reset();
          }
          /*wait for all threads to join*/
          for (auto& pool : this->_threads) {
                    if (pool.joinable()) {
                              pool.join();
                    }
          }
}

boost::asio::io_context& IOServicePool::getIOService()
{
          return this->_io_contexts[(++_cur_io_servies) % _io_contexts.size()];
}

IOServicePool::~IOServicePool()
{
          this->stopServicePool();
}