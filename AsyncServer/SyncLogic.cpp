#include "SyncLogic.h"
#include "Session.h"

SyncLogic::SyncLogic() :_b_stop(false) 
{
          this->registerCallBack();               //register callback function in mapping structure
          this->_thread = std::thread(&SyncLogic::processLogicPair, this);
}

SyncLogic::~SyncLogic() 
{
          _b_stop = true;
          _queue_cond.notify_all();
          this->_thread.join();
}

void  SyncLogic::commitPairToQueue(std::shared_ptr<LogicPair> logicpair)
{
          std::lock_guard<std::mutex> _lckg(this->_queue_mutex);
          this->_msg_queue.push(logicpair);

          if (!_msg_queue.empty()) {
                    _queue_cond.notify_one();
          }
}

void SyncLogic::deployCallBack(std::shared_ptr<LogicPair> session)
{
          short msg_id = session->_node->getMsgID();
          auto callback_iter = _func_callback.find(msg_id);
          if (callback_iter != _func_callback.end()) {     /*we find it, deploy callback function*/
                    callback_iter->second(session->_session, msg_id,
                              std::string(session->_node->_msg, session->_node->_cur_length)
                    );
          }
}

void SyncLogic::processLogicPair()      //multi-thread
{
          for (;;) {
                    std::unique_lock<std::mutex> _lckg(_queue_mutex);
                    _queue_cond.wait(_lckg, [this]() {
                              return _b_stop || !_msg_queue.empty();
                    });

                    //We have to distinguish which condition trigger condition variable
                    /*SyncLogic could be shutdown, handle all the data and break the loop*/
                    if (_b_stop) {     
                              while (!_msg_queue.empty()) {
                                        auto& msg_pair = _msg_queue.front();
                                        deployCallBack(msg_pair);
                                        _msg_queue.pop();
                              }
                              break;
                    }
                    if (!_msg_queue.empty()) {
                              auto& msg_pair = _msg_queue.front();
                              deployCallBack(msg_pair);
                              _msg_queue.pop();
                    }
          }
}

void SyncLogic::registerCallBack()      //register basic callback
{
          _func_callback[MSG_EXAMPLE] = std::bind(&SyncLogic::callBackExample, this,
                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
          );
}

void  SyncLogic::callBackExample(std::shared_ptr<Session>session, const short& msg_id, const std::string& msg)
{
          Json::Value root;
          Json::Reader reader;
          reader.parse(msg, root);

          std::cout << "id = " << (root["id"] = msg_id) << ", data = " << (root["data"] = msg) << std::endl;
          root["data"] = "server has received msg = " + root["data"].asString();

          session->Send(root.toStyledString(), root["id"].asInt());
}

