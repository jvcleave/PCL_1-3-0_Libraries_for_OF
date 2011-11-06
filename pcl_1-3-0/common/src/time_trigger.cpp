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
 * Date: 31. March 2011
 * Author: Suat Gedikli (gedikli@willowgarage.com)
 *
 */

#include "pcl/common/time_trigger.h"
#include "pcl/common/time.h"
#include <iostream>

using namespace boost;

pcl::TimeTrigger::TimeTrigger (double interval, const callback_type& callback)
: interval_ (interval)
, quit_ (false)
, running_ (false)
, timer_thread_ (boost::bind (&TimeTrigger::thread_function, this))
{
  registerCallback (callback);
}

pcl::TimeTrigger::TimeTrigger (double interval)
: interval_ (interval)
, quit_ (false)
, running_ (false)
, timer_thread_ (boost::bind (&TimeTrigger::thread_function, this))
{
}

pcl::TimeTrigger::~TimeTrigger ()
{
  unique_lock<mutex> lock (condition_mutex_);
  quit_ = true;
  condition_.notify_all ();
  lock.unlock ();
  
  timer_thread_.join ();
}

boost::signals2::connection pcl::TimeTrigger::registerCallback (const callback_type& callback)
{
  unique_lock<mutex> lock (condition_mutex_);
  return callbacks_.connect (callback);
}

void pcl::TimeTrigger::setInterval (double interval_seconds)
{
  unique_lock<mutex> lock (condition_mutex_);
  interval_ = interval_seconds;
  // notify, since we could switch from a large interval to a shorter one -> interrupt waiting for timeout!
  condition_.notify_all ();
}

void pcl::TimeTrigger::start ()
{
  unique_lock<mutex> lock (condition_mutex_);
  if (!running_)
  {
    running_ = true;
    condition_.notify_all ();
  }
}

void pcl::TimeTrigger::stop ()
{
  unique_lock<mutex> lock (condition_mutex_);
  if (running_)
  {
    running_ = false;
    condition_.notify_all ();
  }
}

void pcl::TimeTrigger::thread_function ()
{
  static double time = 0;
  while (!quit_)
  {
    time = getTime ();
    unique_lock<mutex> lock (condition_mutex_);
    if (!running_)
    {
      condition_.wait (lock); // wait util start is called or destructor is called
    }
    else
    {
      callbacks_();
      double rest = interval_ + time - getTime ();
      condition_.timed_wait (lock, posix_time::microseconds(rest * 1000000.0));
    }
  }
}

