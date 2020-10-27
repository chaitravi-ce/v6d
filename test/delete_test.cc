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

#if defined(__APPLE__) && defined(__clang__)
#define private public
#endif

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include "arrow/status.h"
#include "arrow/util/io_util.h"
#include "arrow/util/logging.h"
#include "glog/logging.h"

#include "basic/ds/array.h"
#include "client/client.h"
#include "client/ds/object_meta.h"

using namespace vineyard;  // NOLINT(build/namespaces)

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("usage ./delete_test <ipc_socket>");
    return 1;
  }
  std::string ipc_socket = std::string(argv[1]);

  Client client;
  VINEYARD_CHECK_OK(client.Connect(ipc_socket));
  LOG(INFO) << "Connected to IPCServer: " << ipc_socket;

  ObjectID id = InvalidObjectID(), blob_id = InvalidObjectID();
  bool exists;

  {
    // prepare data
    std::vector<double> double_array = {1.0, 7.0, 3.0, 4.0, 2.0};
    ArrayBuilder<double> builder(client, double_array);
    auto sealed_double_array =
        std::dynamic_pointer_cast<Array<double>>(builder.Seal(client));
    id = sealed_double_array->id();
    blob_id = VYObjectIDFromString(sealed_double_array->meta()
                                       .MetaData()
                                       .get_child("buffer_")
                                       .get<std::string>("id"));
    CHECK(blob_id != InvalidObjectID());
  }

  {
    std::unordered_map<ObjectID, Payload> objects;
    auto s = client.GetBuffers({blob_id}, objects);
    CHECK(s.ok() && objects.size() == 1);
  }

  // delete transient object
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.DelData(id, false, true));
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(!exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(!exists);

  {
    std::unordered_map<ObjectID, Payload> objects;
    auto s = client.GetBuffers({blob_id}, objects);
    CHECK(s.ok() && objects.size() == 0);
  }

  {
    // prepare data
    std::vector<double> double_array = {1.0, 7.0, 3.0, 4.0, 2.0};
    ArrayBuilder<double> builder(client, double_array);
    auto sealed_double_array =
        std::dynamic_pointer_cast<Array<double>>(builder.Seal(client));
    VINEYARD_CHECK_OK(client.Persist(sealed_double_array->id()));
    id = sealed_double_array->id();
    blob_id = VYObjectIDFromString(sealed_double_array->meta()
                                       .MetaData()
                                       .get_child("buffer_")
                                       .get<std::string>("id"));
    CHECK(blob_id != InvalidObjectID());
  }

  {
    std::unordered_map<ObjectID, Payload> objects;
    auto s = client.GetBuffers({blob_id}, objects);
    CHECK(s.ok() && objects.size() == 1);
  }

  // deep deletion
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.DelData(id, false, true));
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(!exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(!exists);

  // the blob should have been removed
  {
    std::unordered_map<ObjectID, Payload> objects;
    auto s = client.GetBuffers({blob_id}, objects);
    CHECK(s.ok() && objects.size() == 0);
  }

  {
    // prepare data
    std::vector<double> double_array = {1.0, 7.0, 3.0, 4.0, 2.0};
    ArrayBuilder<double> builder(client, double_array);
    auto sealed_double_array =
        std::dynamic_pointer_cast<Array<double>>(builder.Seal(client));
    VINEYARD_CHECK_OK(client.Persist(sealed_double_array->id()));
    id = sealed_double_array->id();
    blob_id = VYObjectIDFromString(sealed_double_array->meta()
                                       .MetaData()
                                       .get_child("buffer_")
                                       .get<std::string>("id"));
    CHECK(blob_id != InvalidObjectID());
  }

  {
    std::unordered_map<ObjectID, Payload> objects;
    auto s = client.GetBuffers({blob_id}, objects);
    CHECK(s.ok() && objects.size() == 1);
  }

  // shallow deletion
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.DelData(id, false, false));
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(!exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(exists);

  {
    std::unordered_map<ObjectID, Payload> objects;
    auto s = client.GetBuffers({blob_id}, objects);
    CHECK(s.ok() && objects.size() == 1);  // the deletion is shallow
  }

  {
    // prepare data
    std::vector<double> double_array = {1.0, 7.0, 3.0, 4.0, 2.0};
    ArrayBuilder<double> builder(client, double_array);
    auto sealed_double_array =
        std::dynamic_pointer_cast<Array<double>>(builder.Seal(client));
    VINEYARD_CHECK_OK(client.Persist(sealed_double_array->id()));
    id = sealed_double_array->id();
    blob_id = sealed_double_array->meta().GetMemberMeta("buffer_").GetId();
    CHECK(blob_id != InvalidObjectID());
  }

  // force deletion
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.DelData(blob_id, true, false));
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(!exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(!exists);

  // the blob should have been removed
  {
    std::unordered_map<ObjectID, Payload> objects;
    auto s = client.GetBuffers({blob_id}, objects);
    CHECK(s.ok() && objects.size() == 0);
  }

  {
    // prepare data
    std::vector<double> double_array = {1.0, 7.0, 3.0, 4.0, 2.0};
    ArrayBuilder<double> builder(client, double_array);
    auto sealed_double_array =
        std::dynamic_pointer_cast<Array<double>>(builder.Seal(client));
    VINEYARD_CHECK_OK(client.Persist(sealed_double_array->id()));
    id = sealed_double_array->id();
    blob_id = sealed_double_array->meta().GetMemberMeta("buffer_").GetId();
    CHECK(blob_id != InvalidObjectID());
  }

  // shallow delete multiple objects
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(exists);
  VINEYARD_CHECK_OK(client.DelData({id, blob_id}, false, false));
  VINEYARD_CHECK_OK(client.Exists(id, exists));
  CHECK(!exists);
  VINEYARD_CHECK_OK(client.Exists(blob_id, exists));
  CHECK(!exists);

  // the blob should have been removed
  {
    std::unordered_map<ObjectID, Payload> objects;
    auto s = client.GetBuffers({blob_id}, objects);
    CHECK(s.ok() && objects.size() == 0);
  }

  LOG(INFO) << "Passed delete tests...";

  client.Disconnect();

  return 0;
}
