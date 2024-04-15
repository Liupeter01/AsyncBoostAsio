#include "ConnectionMgr.h"

void ConnectionMgr::addConnection(std::shared_ptr<Connection> conn_ptr)
{
          this->_map_conn[conn_ptr->GetUuid()] = conn_ptr;
}

void ConnectionMgr::removeConnection(std::string _uuid)
{
          this->_map_conn.erase(_uuid);
}

ConnectionMgr& ConnectionMgr::getInstance() {
		  static ConnectionMgr instance;
		  return instance;
}