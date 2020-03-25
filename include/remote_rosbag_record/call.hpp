#ifndef REMOTE_ROSBAG_RECORD_CALL
#define REMOTE_ROSBAG_RECORD_CALL

#include <string>

#include <ros/console.h>
#include <ros/master.h>
#include <ros/service.h>
#include <ros/this_node.h>
#include <std_srvs/Empty.h>
#include <xmlrpcpp/XmlRpcException.h>
#include <xmlrpcpp/XmlRpcValue.h>

#include <boost/regex.hpp>

namespace remote_rosbag_record {

// try calling services matching the given expression
// and return number of successfull calls
inline static std::size_t call(const boost::regex &expression, const bool verbose = true) {
  std::size_t n_success(0);
  try {
    // call ros master api to get services info
    XmlRpc::XmlRpcValue args, result, payload;
    args[0] = ros::this_node::getName();
    if (!ros::master::execute("getSystemState", args, result, payload,
                              /* wait_for_master = */ false)) {
      throw XmlRpc::XmlRpcException("getSystemState");
    }

    // call services which match the given expression
    XmlRpc::XmlRpcValue services(payload[2]);
    for (int i = 0; i < services.size(); ++i) {
      const std::string name(services[i][0]);
      if (!boost::regex_match(name, expression)) {
        continue;
      }

      std_srvs::Empty srv;
      if (!ros::service::call(name, srv)) {
        ROS_ERROR_STREAM("Failed to call '" << name << "'");
        continue;
      }

      if (verbose) {
        ROS_INFO_STREAM("Called '" << name << "'");
      }
      ++n_success;
    }
  } catch (const XmlRpc::XmlRpcException &error) {
    ROS_ERROR_STREAM("Error caught on calling services: " << error.getMessage());
  }
  return n_success;
}

} // namespace remote_rosbag_record

#endif