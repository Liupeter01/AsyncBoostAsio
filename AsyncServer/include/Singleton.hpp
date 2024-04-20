#pragma once
#ifndef _SINGLETON_HPP_
#define _SINGLETON_HPP_
#include<iostream>
#include<memory>
#include<mutex>
#include<boost/noncopyable.hpp>

namespace BoostAsioProject{

    template<typename _Type>
    class Singleton : public boost::noncopyable_::noncopyable{
    public:
            virtual ~Singleton() {}
            static  std::shared_ptr<_Type> getInstance() {
                        static std::once_flag _flag;
                        std::call_once(_flag, [&]() {
                                _instance = std::make_shared<_Type>();
                        });
                        return _instance;
            }

    protected:
            Singleton() = default;
            static std::shared_ptr<_Type> _instance;
    };

    template<typename _Type>
    std::shared_ptr<_Type>  Singleton<_Type>::_instance = nullptr;
}
#endif