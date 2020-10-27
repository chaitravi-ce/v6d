/** Copyright 2020 Alibaba Group Holding Limited.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef SRC_SERVER_ASYNC_SOCKET_SERVER_H_
#define SRC_SERVER_ASYNC_SOCKET_SERVER_H_

#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "boost/asio.hpp"

#include "common/util/protocols.h"
#include "server/async/socket_server.h"
#include "server/server/vineyard_server.h"

namespace vineyard {

namespace asio = boost::asio;
using boost::asio::generic::stream_protocol;

class SocketServer;

using socket_message_queue_t = std::deque<std::string>;

/**
 * @brief SocketConnection handles the socket connection in vineyard
 *
 */
class SocketConnection : public std::enable_shared_from_this<SocketConnection> {
 public:
  SocketConnection(stream_protocol::socket socket, vs_ptr_t server_ptr,
                   SocketServer* socket_server_ptr, int conn_id);

  void Start();

  /**
   * Invoke internal doStop, and set the running status to false.
   */
  void Stop();

 private:
  int nativeHandle() { return socket_.native_handle(); }

  void doReadHeader();

  void doReadBody();

  /**
   * Return should be exit after this message.
   */
  bool processMessage(const std::string& message_in);

  void doWrite(const std::string& buf);

  void doWrite(std::string&& buf);

  void doWrite(const std::string& buf, callback_t<> callback);

  /**
   * Being called when the encounter a socket error (in read/write), or by
   * external "conn->Stop()".
   *
   * Just do some clean up and won't remove connecion from parent's pool.
   */
  void doStop();

  void doAsyncWrite();

  void doAsyncWrite(callback_t<> callback);

  stream_protocol::socket socket_;
  vs_ptr_t server_ptr_;
  SocketServer* socket_server_ptr_;
  int conn_id_;
  bool running_;

  asio::streambuf buf_;
  socket_message_queue_t write_msgs_;

  std::unordered_set<int> used_fds_;
  // the associated reader of the stream
  std::unordered_set<ObjectID> associated_streams_;

  size_t read_msg_header_;
  std::string read_msg_body_;
};

/**
 * @brief SocketServer is the base class of IPCServer and RPCServer.
 *
 */
class SocketServer {
 public:
  explicit SocketServer(vs_ptr_t vs_ptr);
  virtual ~SocketServer() {}

  virtual void Start();

  /**
   * Call "Stop" on all connections, then clear the connection pool.
   */
  void Stop();

  /**
   * Check if @conn_id@ exists in the connection pool.
   */
  bool ExistsConnection(int conn_id) const;

  /**
   * Remove @conn_id@ from connection pool, before removing, the "Stop"
   * on the connection has already been called.
   */
  void RemoveConnection(int conn_id);

  /**
   * Invoke the "Stop" on the connection, and then remove it from the
   * connection pool.
   */
  void CloseConnection(int conn_id);

  /**
   * Inspect the size of current alive connections.
   */
  size_t AliveConnections() const;

 protected:
  vs_ptr_t vs_ptr_;
  int next_conn_id_;
  std::unordered_map<int, std::shared_ptr<SocketConnection>> connections_;
  mutable std::mutex connections_mutx_;  // protect connections_ in removing

 private:
  virtual void doAccept() = 0;
};

}  // namespace vineyard

#endif  // SRC_SERVER_ASYNC_SOCKET_SERVER_H_
