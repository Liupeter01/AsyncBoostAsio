#pragma once
#ifndef _MSGNODE_H_
#define _MSGNODE_H_
#include"const.h"
#include<queue>
#include<iostream>
#include<memory>
#include<boost/asio.hpp>

namespace BoostAsioProject{

	struct MsgHead {
			MsgHead(short max_length)
						:_total_length(max_length), _cur_length(0), _msg(new char [max_length + 1] {0}) {
			}
			void clear() {
						std::memset(_msg, 0, _total_length);
						_cur_length = 0;
			}
			virtual ~MsgHead() { delete[]_msg; }

			char* _msg;
			short _total_length;
			short _cur_length;
	};
	class RecvNode :public MsgHead {
	public:
			RecvNode(short max_length, short msg_id);
			short getMsgID() const;
	private:
			short _msg_id;
	};

	class SendNode :public MsgHead {
	public:
			SendNode(const char* msg, short max_length, short msg_id);
			short getMsgID() const;
	private:
			short _msg_id;
	};
}

#endif