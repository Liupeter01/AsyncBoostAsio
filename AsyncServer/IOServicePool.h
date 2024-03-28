#pragma once
#ifndef  _IOSERVICEPOOL_H_
#define _IOSERVICEPOOL_H_
#include<vector>
#include<atomic>
#include<boost/asio.hpp>
#include"Singleton.hpp"

class IOServicePool :public Singleton<IOServicePool>
{
          friend class Singleton<IOServicePool>;
public:
          IOServicePool(std::size_t poolSize = std::thread::hardware_concurrency());
          virtual ~IOServicePool();

public:
          void stopServicePool();
          boost::asio::io_context& getIOService();

private:
          using IO_CONTEXT = boost::asio::io_context;
          using WORK = boost::asio::io_context::work;
          using UNIQUE_WORK = std::unique_ptr<WORK>;

          std::vector<std::thread> _threads;
          std::vector<IO_CONTEXT> _io_contexts;
          std::vector<UNIQUE_WORK> _uni_works;
          std::atomic<std::size_t> _cur_io_servies;
};

#endif