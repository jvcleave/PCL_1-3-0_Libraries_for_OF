/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Suat Gedikli (gedikli@willowgarage.com)
 */

#include <pcl/pcl_config.h>
#ifdef HAVE_OPENNI

#include <pcl/io/pcd_grabber.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>

//////////////////////// GrabberImplementation //////////////////////
struct pcl::PCDGrabberBase::PCDGrabberImpl
{
  PCDGrabberImpl (pcl::PCDGrabberBase& grabber, const std::string& pcd_path, float frames_per_second, bool repeat);
  PCDGrabberImpl (pcl::PCDGrabberBase& grabber, const std::vector<std::string>& pcd_files, float frames_per_second, bool repeat);
  void trigger ();
  void readAhead ();
  pcl::PCDGrabberBase& grabber_;
  float frames_per_second_;
  bool repeat_;
  bool running_;
  std::vector<std::string> pcd_files_;
  std::vector<std::string>::iterator pcd_iterator_;
  TimeTrigger time_trigger_;

  sensor_msgs::PointCloud2 next_cloud_;
  bool valid_;
};

pcl::PCDGrabberBase::PCDGrabberImpl::PCDGrabberImpl (pcl::PCDGrabberBase& grabber, const std::string& pcd_path, float frames_per_second, bool repeat)
: grabber_ (grabber)
, frames_per_second_ (frames_per_second)
, repeat_ (repeat)
, running_ (false)
, time_trigger_ (1.0 / (double) std::max(frames_per_second, 0.001f), boost::bind (&PCDGrabberImpl::trigger, this))
, valid_ (false)
{
  pcd_files_.push_back (pcd_path);
  pcd_iterator_ = pcd_files_.begin ();
}

pcl::PCDGrabberBase::PCDGrabberImpl::PCDGrabberImpl (pcl::PCDGrabberBase& grabber, const std::vector<std::string>& pcd_files, float frames_per_second, bool repeat)
: grabber_ (grabber)
, frames_per_second_ (frames_per_second)
, repeat_ (repeat)
, running_ (false)
, time_trigger_ (1.0 / (double) std::max(frames_per_second, 0.001f), boost::bind (&PCDGrabberImpl::trigger, this))
, valid_ (false)
{
  pcd_files_ = pcd_files;
  pcd_iterator_ = pcd_files_.begin ();
}

void pcl::PCDGrabberBase::PCDGrabberImpl::readAhead ()
{
  if (pcd_iterator_ != pcd_files_.end ())
  {
    PCDReader reader;
    int pcd_version;
    Eigen::Vector4f origin;
    Eigen::Quaternionf orientation;
    valid_ = (reader.read (*pcd_iterator_, next_cloud_, origin, orientation, pcd_version) == 0);

    if (++pcd_iterator_ == pcd_files_.end () && repeat_)
      pcd_iterator_ = pcd_files_.begin ();
  }
  else
    valid_ = false;
}

void pcl::PCDGrabberBase::PCDGrabberImpl::trigger ()
{
  if (valid_)
    grabber_.publish (next_cloud_);

  // use remaining time, if there is time left!
  readAhead ();
}

//////////////////////// GrabberBase //////////////////////
pcl::PCDGrabberBase::PCDGrabberBase (const std::string& pcd_path, float frames_per_second, bool repeat)
: impl_( new PCDGrabberImpl (*this, pcd_path, frames_per_second, repeat ) )
{
}

pcl::PCDGrabberBase::PCDGrabberBase (const std::vector<std::string>& pcd_files, float frames_per_second, bool repeat)
: impl_( new PCDGrabberImpl (*this, pcd_files, frames_per_second, repeat) )
{
}

pcl::PCDGrabberBase::~PCDGrabberBase () throw ()
{
  stop ();
  delete impl_;
}

void pcl::PCDGrabberBase::start ()
{
  if (impl_->frames_per_second_ > 0)
  {
    impl_->running_ = true;
    impl_->time_trigger_.start ();
  }
  else // manual trigger
  {
    impl_->trigger ();
  }
}

void pcl::PCDGrabberBase::stop ()
{
  if (impl_->frames_per_second_ > 0)
  {
    impl_->time_trigger_.stop ();
    impl_->running_ = false;
  }
}

bool pcl::PCDGrabberBase::isRunning () const
{
  return impl_->running_;
}

std::string pcl::PCDGrabberBase::getName () const
{
  return "PCDGrabber";
}

void pcl::PCDGrabberBase::rewind ()
{
  impl_->pcd_iterator_ = impl_->pcd_files_.begin ();
}

#endif
