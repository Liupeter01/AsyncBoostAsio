#pragma once
#ifndef _SYNCLOGIC_H_
#define _SYNCLOGIC_H_

#include<thread>
#include<map>
#include<functional>
#include<condition_variable>
#include<json/json.h>
#include<json/value.h>
#include<json/reader.h>
#include<memory>

#include"const.h"
#include"Session.h"
#include"Singleton.hpp"

namespace BoostAsioProject{
    /*declaration*/
    class LogicPair;   
    
    class SyncLogic :public Singleton<BoostAsioProject::SyncLogic>{
            friend class Singleton<BoostAsioProject::SyncLogic>;
    public:
            using CallBackFunc = std::function<void(std::shared_ptr<BoostAsioProject::Session>, const short& msg_id, const std::string& msg)>;
            SyncLogic();
            virtual ~SyncLogic();
            void commitPairToQueue(std::shared_ptr<BoostAsioProject::LogicPair> logicpair);

    private:
            void processLogicPair();      //multi-thread
            void deployCallBack(std::shared_ptr<BoostAsioProject::LogicPair> session);
            void registerCallBack();      //register basic callback
            void callBackExample(std::shared_ptr<BoostAsioProject::Session>session, const short& msg_id, const std::string& msg);

    private:
            bool _b_stop;
            std::thread _thread;
            std::mutex _queue_mutex;
            std::queue<std::shared_ptr<BoostAsioProject::LogicPair>>_msg_queue;
            std::map<short, CallBackFunc> _func_callback;
            std::condition_variable _queue_cond;
    };

    class LogicPair
    {
            friend class SyncLogic;
    public:
            LogicPair(std::shared_ptr<BoostAsioProject::Session> session, 
                      std::shared_ptr<BoostAsioProject::RecvNode> recvnode)
                        :_session(session), _node(recvnode) 
            {}

    private:
            std::shared_ptr<BoostAsioProject::Session> _session;
            std::shared_ptr<BoostAsioProject::RecvNode > _node;
    };
}

#endif
