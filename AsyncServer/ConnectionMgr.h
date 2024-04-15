#pragma once
#ifndef _CONNECTIONMGR_H_
#define _CONNECTIONMGR_H_

#include"Singleton.hpp"
#include"Connection.h"
#include<boost/unordered_map.hpp>

class ConnectionMgr:public Singleton<ConnectionMgr>
{
public:
		  ConnectionMgr(){}
		  virtual ~ConnectionMgr(){}

public:
		  static ConnectionMgr& getInstance();

public:
		  void addConnection(std::shared_ptr<Connection> conn_ptr);
		  void removeConnection(std::string _uuid);

private:
		  boost::unordered_map<std::string, std::shared_ptr<Connection>> _map_conn;
};
#endif // !#