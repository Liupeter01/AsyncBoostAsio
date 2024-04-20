#include"MsgNode.h"

BoostAsioProject::RecvNode::RecvNode(short max_length, short msg_id)
          : MsgHead(max_length), _msg_id(msg_id)
{}

short BoostAsioProject::RecvNode::getMsgID() const
{
          return this->_msg_id;
}

BoostAsioProject::SendNode::SendNode(const char* msg, short max_length, short msg_id) 
          : MsgHead(max_length+HEAD_TOTAL_LENGTH), _msg_id(msg_id)
{
          *(short*)this->_msg = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
          *(short*)(this->_msg + HEAD_ID_LENGTH) = boost::asio::detail::socket_ops::host_to_network_short(max_length);
          std::memcpy(this->_msg + HEAD_TOTAL_LENGTH, msg, max_length);
}

short BoostAsioProject::SendNode::getMsgID() const
{
          return this->_msg_id;
}