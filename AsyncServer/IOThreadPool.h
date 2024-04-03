#pragma once
#ifndef _IOTHREADPOOL_H_
#define _IOTHREADPOOL_H_
#include<boost/asio.hpp>
#include"Singleton.hpp"

class IOThreadPool:public Singleton<IOThreadPool>
{
          friend class Singleton<IOThreadPool>;
public:
          IOThreadPool(std::size_t poolSize = std::thread::hardware_concurrency());
          virtual ~IOThreadPool();

public:
          boost::asio::io_context& GetIOContext();
          void stopThreadPool();

private:
          using WORK = boost::asio::io_context::work;
          using UNIQUE_WORK = std::unique_ptr<WORK>;
          boost::asio::io_context _io_context;
          UNIQUE_WORK _uni_work;
          std::vector<std::thread> _threads;
};

#endif