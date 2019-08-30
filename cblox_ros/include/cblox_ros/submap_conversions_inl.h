#ifndef CBLOX_ROS_SUBMAP_CONVERSIONS_INL_H
#define CBLOX_ROS_SUBMAP_CONVERSIONS_INL_H

#include <cblox_msgs/MapHeader.h>
#include <cblox_msgs/MapPoseEstimate.h>

namespace cblox {

template <typename SubmapType>
std_msgs::Header generateHeaderMsg(
    const typename SubmapType::Ptr& submap_ptr, const ros::Time &timestamp) {
  std_msgs::Header msg_header;
  msg_header.frame_id = "submap_" + std::to_string(submap_ptr->getID());
  msg_header.stamp = timestamp;
  return msg_header;
}

template <typename SubmapType>
cblox_msgs::MapHeader generateSubmapHeaderMsg(
    const typename SubmapType::Ptr& submap_ptr) {
    // Set the submap ID and type
    cblox_msgs::MapHeader submap_header;
    submap_header.id = submap_ptr->getID();
    submap_header.is_submap = true;

    // Set the submap's start and end time
    submap_header.start_time = submap_ptr->getMappingInterval().first;
    submap_header.end_time = submap_ptr->getMappingInterval().second;

    // Set the pose estimate and indicate what frame it's in
    // TODO(victorr): Get the world frame name from FrameNames once implemented
    submap_header.pose_estimate.frame_id =
        "submap_" + std::to_string(submap_ptr->getID());
    tf::poseKindrToMsg(submap_ptr->getPose().template cast<double>(),
                       &submap_header.pose_estimate.map_pose);

    return submap_header;
  }

template<typename SubmapType>
void serializePoseToMsg(typename SubmapType::Ptr submap_ptr,
    cblox_msgs::MapPoseUpdate* msg) {
  CHECK_NOTNULL(msg);
  CHECK_NOTNULL(submap_ptr);

  ros::Time timestamp = ros::Time::now();
  // fill in headers
  msg->header = generateHeaderMsg<SubmapType>(submap_ptr, timestamp);
  msg->map_header = generateSubmapHeaderMsg<SubmapType>(submap_ptr);
}

template<typename SubmapType>
SubmapID deserializeMsgToPose(const cblox_msgs::MapPoseUpdate* msg,
    typename SubmapCollection<SubmapType>::Ptr submap_collection_ptr) {

  CHECK_NOTNULL(submap_collection_ptr);
  if (!msg->map_header.is_submap) {
    return -1;
  }

  // read id
  SubmapID submap_id = msg->map_header.id;
  // read pose
  kindr::minimal::QuatTransformationTemplate<double> pose;
  tf::poseMsgToKindr(msg->map_header.pose_estimate.map_pose, &pose);
  Transformation submap_pose = pose.cast<float>();

  if (!submap_collection_ptr->exists(submap_id)) {
    // create new submap
    submap_collection_ptr->createNewSubmap(submap_pose, submap_id);
  }
  typename SubmapType::Ptr submap_ptr =
      submap_collection_ptr->getSubmapPtr(submap_id);
  submap_ptr->setPose(submap_pose);

  return submap_id;
}

template <typename SubmapType>
void serializeSubmapToMsg(typename SubmapType::Ptr submap_ptr,
    cblox_msgs::MapLayer* msg) {
  CHECK_NOTNULL(msg);
  CHECK_NOTNULL(submap_ptr);

  ros::Time timestamp = ros::Time::now();
  // fill in headers
  msg->header = generateHeaderMsg<SubmapType>(submap_ptr, timestamp);
  msg->map_header = generateSubmapHeaderMsg<SubmapType>(submap_ptr);

  submap_ptr->serializeToMsg(msg);
}

template <typename SubmapType>
SubmapID deserializeMsgToSubmap(cblox_msgs::MapLayer* msg_ptr,
    typename SubmapCollection<SubmapType>::Ptr submap_collection_ptr) {

  CHECK_NOTNULL(submap_collection_ptr);
  if (!msg_ptr->map_header.is_submap) {
    return -1;
  }

  // read id
  SubmapID submap_id = msg_ptr->map_header.id;
  // read pose
  kindr::minimal::QuatTransformationTemplate<double> pose;
  tf::poseMsgToKindr(msg_ptr->map_header.pose_estimate.map_pose, &pose);
  Transformation submap_pose = pose.cast<float>();

  if (!submap_collection_ptr->exists(submap_id)) {
    // create new submap
    submap_collection_ptr->createNewSubmap(submap_pose, submap_id);
  }
  typename SubmapType::Ptr submap_ptr =
      submap_collection_ptr->getSubmapPtr(submap_id);
  submap_ptr->setPose(submap_pose);

  // read mapping interval
  submap_ptr->startMappingTime(msg_ptr->map_header.start_time);
  submap_ptr->stopMappingTime(msg_ptr->map_header.end_time);

  submap_ptr->deserializeFromMsg(msg_ptr);

  return submap_id;
}

}
#endif //CBLOX_ROS_SUBMAP_CONVERSIONS_INL_H
