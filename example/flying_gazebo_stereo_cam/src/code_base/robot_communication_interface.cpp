/* Copyright (c) 2016, Stefan Isler, islerstefan@bluewin.ch
 * (ETH Zurich / Robotics and Perception Group, University of Zurich, Switzerland)
 *
 * This file is part of ig_active_reconstruction, software for information gain based, active reconstruction.
 *
 * ig_active_reconstruction is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * ig_active_reconstruction is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * Please refer to the GNU Lesser General Public License for details on the license,
 * on <http://www.gnu.org/licenses/>.
*/

#include "flying_gazebo_stereo_cam/robot_communication_interface.hpp"

#include <thread>
#include <boost/thread/thread.hpp>
#include <boost/chrono/include.hpp>

namespace flying_gazebo_stereo_cam
{
  
  CommunicationInterface::CommunicationInterface( ros::NodeHandle nh, std::shared_ptr<Controller> controller, std::string in_name, std::string out_name )
  : cam_controller_(controller)
  , pcl_rerouter_(nh,in_name,out_name)
  {
    
  }
  
  CommunicationInterface::View CommunicationInterface::getCurrentView()
  {
    View current_view;
    
    try
    {
      current_view.pose() = cam_controller_->currentPose();
    }
    catch(...)
    {
      current_view.bad() = true;
    }
    current_view.nonViewSpace() = true;
    
    return current_view;
  }
  
  CommunicationInterface::ReceptionInfo CommunicationInterface::retrieveData()
  {
    if( pcl_rerouter_.rerouteOneToSrv() )
    {
      return ReceptionInfo::SUCCEEDED;
    }
    return ReceptionInfo::FAILED;
  }
  
  CommunicationInterface::MovementCost CommunicationInterface::movementCost( View& target_view )
  {
    MovementCost cost;
    
    movements::Pose current;
    auto target_pos = target_view.pose().position;
    
    try
    {
      current = cam_controller_->currentPose();
      auto distance = current.position - target_pos;
      cost.cost = distance.norm();
    }
    catch(...)
    {
      cost.exception = MovementCost::Exception::COST_UNKNOWN;
    }
    
    return cost;
  }
  
  CommunicationInterface::MovementCost CommunicationInterface::movementCost( View& start_view, View& target_view, bool fill_additional_information  )
  {
    MovementCost cost;
    
    auto distance = start_view.pose().position - target_view.pose().position;
    cost.cost = distance.norm();
    
    return cost;
  }
  
  bool CommunicationInterface::moveTo( View& target_view )
  {
    bool success = cam_controller_->moveTo(target_view.pose());
    
    boost::this_thread::sleep_for( boost::chrono::seconds(3) ); // give gazebo time to execute movement command (TODO test: maybe just wait until current pose has changed (if it is different from current)?)
    return success;
  }
  
  
}