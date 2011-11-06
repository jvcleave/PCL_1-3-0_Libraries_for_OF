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
 * Author: Nico Blodow (blodow@cs.tum.edu), Suat Gedikli (gedikli@willowgarage.com)
 */

#include <pcl/pcl_config.h>
#ifdef HAVE_OPENNI

#include <pcl/io/openni_grabber.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/time.h>
#include <pcl/io/pcl_io_exception.h>
#include <boost/shared_array.hpp>
#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem.hpp>
#include <iostream>
namespace pcl
{

typedef union
{

  struct /*anonymous*/
  {
    unsigned char Blue;
    unsigned char Green;
    unsigned char Red;
    unsigned char Alpha;
  };
  float float_value;
  long long_value;
} RGBValue;

OpenNIGrabber::OpenNIGrabber(const std::string& device_id, const Mode& depth_mode, const Mode& image_mode)
: image_required_(false)
, depth_required_(false)
, sync_required_(false)
, running_(false)
{
  // initialize driver
  onInit(device_id, depth_mode, image_mode);

  if (!device_->hasDepthStream())
    THROW_PCL_IO_EXCEPTION("Device does not provide 3D information.");

  depth_image_signal_ = createSignal <sig_cb_openni_depth_image > ();
  ir_image_signal_ = createSignal <sig_cb_openni_ir_image > ();
  point_cloud_signal_ = createSignal <sig_cb_openni_point_cloud > ();
  point_cloud_i_signal_ = createSignal <sig_cb_openni_point_cloud_i > ();
  ir_depth_image_signal_ = createSignal <sig_cb_openni_ir_depth_image > ();

  ir_sync_.addCallback(boost::bind(&OpenNIGrabber::irDepthImageCallback, this, _1, _2));
  if (device_->hasImageStream())
  {
    // create callback signals
    image_signal_ = createSignal <sig_cb_openni_image > ();
    image_depth_image_signal_ = createSignal <sig_cb_openni_image_depth_image > ();
    point_cloud_rgb_signal_ = createSignal <sig_cb_openni_point_cloud_rgb > ();
    rgb_sync_.addCallback(boost::bind(&OpenNIGrabber::imageDepthImageCallback, this, _1, _2));
    openni_wrapper::DeviceKinect* kinect = dynamic_cast<openni_wrapper::DeviceKinect*> (device_.get());
    if (kinect)
      kinect->setDebayeringMethod(openni_wrapper::ImageBayerGRBG::EdgeAware);
  }

  image_callback_handle = device_->registerImageCallback(&OpenNIGrabber::imageCallback, *this);
  depth_callback_handle = device_->registerDepthCallback(&OpenNIGrabber::depthCallback, *this);
  ir_callback_handle    = device_->registerIRCallback(&OpenNIGrabber::irCallback, *this);
}

OpenNIGrabber::~OpenNIGrabber() throw ()
{
  try
  {
    stop();

    // unregister callbacks
    device_->unregisterDepthCallback(depth_callback_handle);

    if (device_->hasImageStream ())
      device_->unregisterImageCallback(image_callback_handle);

    if (device_->hasIRStream ())
      device_->unregisterIRCallback(image_callback_handle);

    // disconnect all listeners
    disconnect_all_slots <sig_cb_openni_image > ();
    disconnect_all_slots <sig_cb_openni_depth_image > ();
    disconnect_all_slots <sig_cb_openni_ir_image > ();
    disconnect_all_slots <sig_cb_openni_image_depth_image > ();
    disconnect_all_slots <sig_cb_openni_point_cloud > ();
    disconnect_all_slots <sig_cb_openni_point_cloud_rgb > ();
    disconnect_all_slots <sig_cb_openni_point_cloud_i > ();

    openni_wrapper::OpenNIDriver& driver = openni_wrapper::OpenNIDriver::getInstance();
    driver.stopAll();
  }
  catch (...)
  {
    // destructor never throws
  }
}

void OpenNIGrabber::checkImageAndDepthSynchronizationRequired()
{
  // do we have anyone listening to images or color point clouds?
  if (num_slots<sig_cb_openni_point_cloud_rgb > () > 0 ||
      num_slots<sig_cb_openni_image_depth_image > () > 0)
    sync_required_ = true;
  else
    sync_required_ = false;
}

void OpenNIGrabber::checkImageStreamRequired()
{
  // do we have anyone listening to images or color point clouds?
  if (num_slots<sig_cb_openni_image > () > 0 ||
      num_slots<sig_cb_openni_image_depth_image > () > 0 ||
      num_slots<sig_cb_openni_point_cloud_rgb > () > 0)
    image_required_ = true;
  else
    image_required_ = false;
}

void OpenNIGrabber::checkDepthStreamRequired()
{
  // do we have anyone listening to depth images or (color) point clouds?
  if (num_slots<sig_cb_openni_depth_image > () > 0 ||
      num_slots<sig_cb_openni_image_depth_image > () > 0 ||
      num_slots<sig_cb_openni_ir_depth_image > () > 0 ||
      num_slots<sig_cb_openni_point_cloud_rgb > () > 0 ||
      num_slots<sig_cb_openni_point_cloud > () > 0 ||
      num_slots<sig_cb_openni_point_cloud_i > () > 0 )
    depth_required_ = true;
  else
    depth_required_ = false;
}

void OpenNIGrabber::checkIRStreamRequired()
{
  if (num_slots<sig_cb_openni_ir_image > () > 0 ||
      num_slots<sig_cb_openni_point_cloud_i > () > 0 ||
      num_slots<sig_cb_openni_ir_depth_image > () > 0)
    ir_required_ = true;
  else
    ir_required_ = false;
}

void OpenNIGrabber::start()
{
  try
  {
    // check if we need to start/stop any stream
    if (image_required_ && !device_->isImageStreamRunning())
    {
      block_signals();
      device_->startImageStream();
      //startSynchronization ();
    }

    if (depth_required_ && !device_->isDepthStreamRunning())
    {
      block_signals();
      if (device_->hasImageStream() && !device_->isDepthRegistered() && device_->isDepthRegistrationSupported())
      {
        device_->setDepthRegistration(true);
      }
      device_->startDepthStream();
      //startSynchronization ();
    }

    if (ir_required_ && !device_->isIRStreamRunning())
    {
      block_signals();
      device_->startIRStream();
    }
    running_ = true;
  }
  catch (openni_wrapper::OpenNIException& ex)
  {
    THROW_PCL_IO_EXCEPTION("Could not start streams. Reason: %s", ex.what());
  }
  // workaround, since the first frame is corrupted
  boost::this_thread::sleep (boost::posix_time::seconds (1));
  unblock_signals();
}

void OpenNIGrabber::stop()
{
  try
  {
    if (device_->hasDepthStream() && device_->isDepthStreamRunning())
      device_->stopDepthStream();

    if (device_->hasImageStream() && device_->isImageStreamRunning())
      device_->stopImageStream();

    if (device_->hasIRStream() && device_->isIRStreamRunning())
      device_->stopIRStream();

    running_ = false;
  }
  catch (openni_wrapper::OpenNIException& ex)
  {
    THROW_PCL_IO_EXCEPTION("Could not stop streams. Reason: %s", ex.what());
  }
}

bool OpenNIGrabber::isRunning() const
{
  return running_;
}

void OpenNIGrabber::onInit(const std::string& device_id, const Mode& depth_mode, const Mode& image_mode)
{
  updateModeMaps(); // registering mapping from config modes to XnModes and vice versa
  setupDevice(device_id, depth_mode, image_mode);

  rgb_frame_id_ = "/openni_rgb_optical_frame";

  depth_frame_id_ = "/openni_depth_optical_frame";
}

void OpenNIGrabber::signalsChanged()
{
  // reevaluate which streams are required
  checkImageStreamRequired();
  checkDepthStreamRequired();
  checkIRStreamRequired();
  if (ir_required_ && image_required_)
    THROW_PCL_IO_EXCEPTION("Can not provide IR stream and RGB stream at the same time.");

  checkImageAndDepthSynchronizationRequired();
  if (running_)
    start();
}

std::string OpenNIGrabber::getName() const
{
  return std::string("OpenNIGrabber");
}

void OpenNIGrabber::setupDevice(const std::string& device_id, const Mode& depth_mode, const Mode& image_mode)
{
  // Initialize the openni device
  openni_wrapper::OpenNIDriver& driver = openni_wrapper::OpenNIDriver::getInstance();

  try
  {
    if (boost::filesystem::exists(device_id))
    {
      device_ = driver.createVirtualDevice(device_id, true, true);
    }
    else if (driver.getNumberDevices() == 0)
    {
      THROW_PCL_IO_EXCEPTION("No devices connected.");
    }
    else if (device_id[0] == '#')
    {
      unsigned index = atoi(device_id.c_str() + 1);
      //printf("[%s] searching for device with index = %d\n", getName().c_str(), index);
      device_ = driver.getDeviceByIndex(index - 1);
    }
#ifndef _WIN32
    else if (device_id.find('@') != std::string::npos)
    {
      size_t pos = device_id.find('@');
      unsigned bus = atoi(device_id.substr(0, pos).c_str());
      unsigned address = atoi(device_id.substr(pos + 1, device_id.length() - pos - 1).c_str());
      //printf("[%s] searching for device with bus@address = %d@%d\n", getName().c_str(), bus, address);
      device_ = driver.getDeviceByAddress(bus, address);
    }
    else if (!device_id.empty())
    {
      //printf("[%s] searching for device with serial number = %s\n", getName().c_str(), device_id.c_str());
      device_ = driver.getDeviceBySerialNumber(device_id);
    }
#endif
    else
    {
      device_ = driver.getDeviceByIndex(0);
    }
  }
  catch (const openni_wrapper::OpenNIException& exception)
  {
    if (!device_)
      THROW_PCL_IO_EXCEPTION("No matching device found. %s", exception.what());
    else
      THROW_PCL_IO_EXCEPTION("could not retrieve device. Reason %s", exception.what());
  }
  catch (...)
  {
    THROW_PCL_IO_EXCEPTION("unknown error occured");
  }
  //printf("[%s] Opened '%s' on bus %d:%d with serial number '%s'\n", getName().c_str(),
  //device_->getProductName(), device_->getBus(), device_->getAddress(), device_->getSerialNumber());

  // Set the selected output mode
  if (depth_mode != OpenNI_Default_Mode)
  {
    XnMapOutputMode depth_md, actual_depth_md;
    if (!mapConfigMode2XnMode(depth_mode, depth_md) || !device_->findCompatibleDepthMode(depth_md, actual_depth_md))
      THROW_PCL_IO_EXCEPTION("could not find compatible depth stream mode %d", (int) depth_mode);

    XnMapOutputMode current_depth_md =  device_->getDepthOutputMode();
    if (current_depth_md.nXRes != actual_depth_md.nXRes || current_depth_md.nYRes != actual_depth_md.nYRes )
      device_->setDepthOutputMode(actual_depth_md);

    depth_width_ = depth_md.nXRes;
    depth_height_ = depth_md.nYRes;
  }
  else
  {
    XnMapOutputMode depth_md = device_->getDefaultDepthMode();
    depth_width_ = depth_md.nXRes;
    depth_height_ = depth_md.nYRes;
  }

  if (device_->hasImageStream())
  {
    XnMapOutputMode actual_image_md;
    if (image_mode != OpenNI_Default_Mode)
    {
      XnMapOutputMode image_md;
      if (!mapConfigMode2XnMode(image_mode, image_md) || !device_->findCompatibleImageMode(image_md, actual_image_md))
        THROW_PCL_IO_EXCEPTION("could not find compatible image stream mode %d", (int) image_mode);

      XnMapOutputMode current_image_md =  device_->getImageOutputMode();
      if (current_image_md.nXRes != actual_image_md.nXRes || current_image_md.nYRes != actual_image_md.nYRes )
        device_->setImageOutputMode(actual_image_md);
    }
    else
    {
      actual_image_md = device_->getDefaultImageMode();
    }

    if (actual_image_md.nXRes < depth_width_ || actual_image_md.nYRes < depth_height_)
      THROW_PCL_IO_EXCEPTION("image size may not be smaller than depth image size (%dx%d) < (%dx%d)", actual_image_md.nXRes, actual_image_md.nYRes, depth_width_, depth_height_);

    // get smallest possible image that is at least as big as depth
    // e.g. 1280x1024 -> 640x512 > 640x480
    unsigned scaleX = actual_image_md.nXRes / depth_width_;
    unsigned scaleY = actual_image_md.nYRes / depth_height_;

    image_width_ = actual_image_md.nXRes / scaleX;
    image_height_ = actual_image_md.nYRes / scaleY;
  }
}

void OpenNIGrabber::startSynchronization()
{
  try
  {
    if (device_->isSynchronizationSupported() && !device_->isSynchronized() &&
        device_->getImageOutputMode().nFPS == device_->getDepthOutputMode().nFPS &&
        device_->isImageStreamRunning() && device_->isDepthStreamRunning())
        device_->setSynchronization(true);
  }
  catch (const openni_wrapper::OpenNIException& exception)
  {
    THROW_PCL_IO_EXCEPTION("Could not start synchronization %s", exception.what());
  }
}

void OpenNIGrabber::stopSynchronization()
{
  try
  {
    if (device_->isSynchronizationSupported() && device_->isSynchronized())
      device_->setSynchronization(false);
  }
  catch (const openni_wrapper::OpenNIException& exception)
  {
    THROW_PCL_IO_EXCEPTION("Could not start synchronization %s", exception.what());
  }
}

void OpenNIGrabber::imageCallback(boost::shared_ptr<openni_wrapper::Image> image, void* cookie)
{
  if (num_slots<sig_cb_openni_point_cloud_rgb > () > 0 ||
      num_slots<sig_cb_openni_image_depth_image > () > 0)
    rgb_sync_.add0(image, image->getTimeStamp());

  if (image_signal_->num_slots() > 0)
    image_signal_->operator()(image);

  return;
}

void OpenNIGrabber::depthCallback(boost::shared_ptr<openni_wrapper::DepthImage> depth_image, void* cookie)
{
  if (num_slots<sig_cb_openni_point_cloud_rgb > () > 0 ||
      num_slots<sig_cb_openni_image_depth_image > () > 0)
    rgb_sync_.add1(depth_image, depth_image->getTimeStamp());

  if (num_slots<sig_cb_openni_point_cloud_i > () > 0 ||
      num_slots<sig_cb_openni_ir_depth_image > () > 0)
    ir_sync_.add1(depth_image, depth_image->getTimeStamp());

  if (depth_image_signal_->num_slots() > 0)
    depth_image_signal_->operator()(depth_image);

  if (point_cloud_signal_->num_slots() > 0)
    point_cloud_signal_->operator()(convertToXYZPointCloud(depth_image));

  return;
}

void OpenNIGrabber::irCallback(boost::shared_ptr<openni_wrapper::IRImage> ir_image, void* cookie)
{
  if (num_slots<sig_cb_openni_point_cloud_i > () > 0 ||
      num_slots<sig_cb_openni_ir_depth_image > () > 0)
    ir_sync_.add0(ir_image, ir_image->getTimeStamp());

  if (ir_image_signal_->num_slots() > 0)
    ir_image_signal_->operator()(ir_image);

  return;
}

void OpenNIGrabber::imageDepthImageCallback(const boost::shared_ptr<openni_wrapper::Image> &image, const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image)
{
  // check if we have color point cloud slots
  if (point_cloud_rgb_signal_->num_slots() > 0)
    point_cloud_rgb_signal_->operator()(convertToXYZRGBPointCloud(image, depth_image));

  if (image_depth_image_signal_->num_slots() > 0)
  {
    float constant = 1.0f / device_->getDepthFocalLength(depth_width_);
    image_depth_image_signal_->operator()(image, depth_image, constant);
  }
}

void OpenNIGrabber::irDepthImageCallback(const boost::shared_ptr<openni_wrapper::IRImage> &ir_image, const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image)
{
  // check if we have color point cloud slots
  if (point_cloud_i_signal_->num_slots() > 0)
    point_cloud_i_signal_->operator()(convertToXYZIPointCloud(ir_image, depth_image));

  if (ir_depth_image_signal_->num_slots() > 0)
  {
    float constant = 1.0f / device_->getDepthFocalLength(depth_width_);
    ir_depth_image_signal_->operator()(ir_image, depth_image, constant);
  }
}

pcl::PointCloud<pcl::PointXYZ>::Ptr OpenNIGrabber::convertToXYZPointCloud(const boost::shared_ptr<openni_wrapper::DepthImage>& depth_image) const
{
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud <pcl::PointXYZ>);

  // TODO cloud->header.stamp = time;
  cloud->height = depth_height_;
  cloud->width = depth_width_;
  cloud->is_dense = false;

  cloud->points.resize(cloud->height * cloud->width);

  register float constant = 1.0f / device_->getDepthFocalLength(depth_width_);

  if (device_->isDepthRegistered())
    cloud->header.frame_id = rgb_frame_id_;
  else
    cloud->header.frame_id = depth_frame_id_;

  register int centerX = (cloud->width >> 1);
  int centerY = (cloud->height >> 1);

  float bad_point = std::numeric_limits<float>::quiet_NaN();

  // we have to use Data, since operator[] uses assert -> Debug-mode very slow!
  register const unsigned short* depth_map = depth_image->getDepthMetaData().Data();
  if (depth_image->getWidth() != depth_width_ || depth_image->getHeight() != depth_height_)
  {
    static unsigned buffer_size = 0;
    static boost::shared_array<unsigned short> depth_buffer(0);

    if (buffer_size < depth_width_ * depth_height_)
    {
      buffer_size = depth_width_ * depth_height_;
      depth_buffer.reset(new unsigned short [buffer_size]);
    }
    depth_image->fillDepthImageRaw(depth_width_, depth_height_, depth_buffer.get());
    depth_map = depth_buffer.get();
  }

  register int depth_idx = 0;
  for (int v = -centerY; v < centerY; ++v)
  {
    for (register int u = -centerX; u < centerX; ++u, ++depth_idx)
    {
      pcl::PointXYZ& pt = cloud->points[depth_idx];
      // Check for invalid measurements
      if (depth_map[depth_idx] == 0 ||
          depth_map[depth_idx] == depth_image->getNoSampleValue() ||
          depth_map[depth_idx] == depth_image->getShadowValue())
      {
        // not valid
        pt.x = pt.y = pt.z = bad_point;
        continue;
      }
      pt.z = depth_map[depth_idx] * 0.001;
      pt.x = u * pt.z * constant;
      pt.y = v * pt.z * constant;
    }
  }
  return cloud;
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr OpenNIGrabber::convertToXYZRGBPointCloud(const boost::shared_ptr<openni_wrapper::Image> &image,
  const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image) const
{
  static unsigned rgb_array_size = 0;
  static boost::shared_array<unsigned char> rgb_array(0);
  static unsigned char* rgb_buffer = 0;

  boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB> > cloud(new pcl::PointCloud<pcl::PointXYZRGB > ());

  cloud->header.frame_id = rgb_frame_id_;
  cloud->height = depth_height_;
  cloud->width = depth_width_;
  cloud->is_dense = false;

  cloud->points.resize(cloud->height * cloud->width);

  float constant = 1.0f / device_->getImageFocalLength(cloud->width);
  register int centerX = (cloud->width >> 1);
  int centerY = (cloud->height >> 1);

  register const XnDepthPixel* depth_map = depth_image->getDepthMetaData().Data();
  if (depth_image->getWidth() != depth_width_ || depth_image->getHeight() != depth_height_)
  {
    static unsigned buffer_size = 0;
    static boost::shared_array<unsigned short> depth_buffer(0);

    if (buffer_size < depth_width_ * depth_height_)
    {
      buffer_size = depth_width_ * depth_height_;
      depth_buffer.reset(new unsigned short [buffer_size]);
    }

    depth_image->fillDepthImageRaw(depth_width_, depth_height_, depth_buffer.get());
    depth_map = depth_buffer.get();
  }

  // here we need exact the size of the point cloud for a one-one correspondence!
  if (rgb_array_size < image_width_ * image_height_ * 3)
  {
    rgb_array_size = image_width_ * image_height_ * 3;
    rgb_array.reset(new unsigned char [rgb_array_size]);
    rgb_buffer = rgb_array.get();
  }
  image->fillRGB(image_width_, image_height_, rgb_buffer, image_width_ * 3);

  // depth_image already has the desired dimensions, but rgb_msg may be higher res.
  register int color_idx = 0, depth_idx = 0;
  RGBValue color;
  color.Alpha = 0;

  float bad_point = std::numeric_limits<float>::quiet_NaN();

  for (int v = -centerY; v < centerY; ++v)
  {
    for (register int u = -centerX; u < centerX; ++u, color_idx += 3, ++depth_idx)
    {
      pcl::PointXYZRGB& pt = cloud->points[depth_idx];
      /// @todo Different values for these cases
      // Check for invalid measurements
      if (depth_map[depth_idx] == 0 ||
          depth_map[depth_idx] == depth_image->getNoSampleValue() ||
          depth_map[depth_idx] == depth_image->getShadowValue())
      {
        pt.x = pt.y = pt.z = bad_point;
      }
      else
      {
        pt.z = depth_map[depth_idx] * 0.001f;
        pt.x = u * pt.z * constant;
        pt.y = v * pt.z * constant;
      }

      // Fill in color
      color.Red = rgb_buffer[color_idx];
      color.Green = rgb_buffer[color_idx + 1];
      color.Blue = rgb_buffer[color_idx + 2];
      pt.rgb = color.float_value;
    }
  }
  return (cloud);
}

pcl::PointCloud<pcl::PointXYZI>::Ptr OpenNIGrabber::convertToXYZIPointCloud(const boost::shared_ptr<openni_wrapper::IRImage> &ir_image,
  const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image) const
{
  boost::shared_ptr<pcl::PointCloud<pcl::PointXYZI> > cloud(new pcl::PointCloud<pcl::PointXYZI > ());

  cloud->header.frame_id = rgb_frame_id_;
  cloud->height = depth_height_;
  cloud->width = depth_width_;
  cloud->is_dense = false;

  cloud->points.resize(cloud->height * cloud->width);

  float constant = 1.0f / device_->getImageFocalLength(cloud->width);
  register int centerX = (cloud->width >> 1);
  int centerY = (cloud->height >> 1);

  register const XnDepthPixel* depth_map = depth_image->getDepthMetaData().Data();
  register const XnIRPixel* ir_map = ir_image->getMetaData().Data();

  if (depth_image->getWidth() != depth_width_ || depth_image->getHeight() != depth_height_)
  {
    static unsigned buffer_size = 0;
    static boost::shared_array<unsigned short> depth_buffer(0);
    static boost::shared_array<unsigned short> ir_buffer(0);

    if (buffer_size < depth_width_ * depth_height_)
    {
      buffer_size = depth_width_ * depth_height_;
      depth_buffer.reset(new unsigned short [buffer_size]);
      ir_buffer.reset(new unsigned short [buffer_size]);
    }

    depth_image->fillDepthImageRaw(depth_width_, depth_height_, depth_buffer.get());
    depth_map = depth_buffer.get();

    ir_image->fillRaw(depth_width_, depth_height_, ir_buffer.get());
    ir_map = ir_buffer.get ();
  }

  register int depth_idx = 0;
  float bad_point = std::numeric_limits<float>::quiet_NaN();

  for (int v = -centerY; v < centerY; ++v)
  {
    for (register int u = -centerX; u < centerX; ++u, ++depth_idx)
    {
      pcl::PointXYZI& pt = cloud->points[depth_idx];
      /// @todo Different values for these cases
      // Check for invalid measurements
      if (depth_map[depth_idx] == 0 ||
          depth_map[depth_idx] == depth_image->getNoSampleValue() ||
          depth_map[depth_idx] == depth_image->getShadowValue())
      {
        pt.x = pt.y = pt.z = bad_point;
      }
      else
      {
        pt.z = depth_map[depth_idx] * 0.001f;
        pt.x = u * pt.z * constant;
        pt.y = v * pt.z * constant;
      }

      pt.data_c[0] = pt.data_c[1] = pt.data_c[2] = pt.data_c[3] = 0;
      pt.intensity = (float) ir_map[depth_idx];
    }
  }
  return (cloud);
}
// TODO: delete me?

void OpenNIGrabber::updateModeMaps()
{
  XnMapOutputMode output_mode;

  output_mode.nXRes = XN_SXGA_X_RES;
  output_mode.nYRes = XN_SXGA_Y_RES;
  output_mode.nFPS = 15;
  config2xn_map_[OpenNI_SXGA_15Hz] = output_mode;

  output_mode.nXRes = XN_VGA_X_RES;
  output_mode.nYRes = XN_VGA_Y_RES;
  output_mode.nFPS = 25;
  config2xn_map_[OpenNI_VGA_25Hz] = output_mode;
  output_mode.nFPS = 30;
  config2xn_map_[OpenNI_VGA_30Hz] = output_mode;

  output_mode.nXRes = XN_QVGA_X_RES;
  output_mode.nYRes = XN_QVGA_Y_RES;
  output_mode.nFPS = 25;
  config2xn_map_[OpenNI_QVGA_25Hz] = output_mode;
  output_mode.nFPS = 30;
  config2xn_map_[OpenNI_QVGA_30Hz] = output_mode;
  output_mode.nFPS = 60;
  config2xn_map_[OpenNI_QVGA_60Hz] = output_mode;

  output_mode.nXRes = XN_QQVGA_X_RES;
  output_mode.nYRes = XN_QQVGA_Y_RES;
  output_mode.nFPS = 25;
  config2xn_map_[OpenNI_QQVGA_25Hz] = output_mode;
  output_mode.nFPS = 30;
  config2xn_map_[OpenNI_QQVGA_30Hz] = output_mode;
  output_mode.nFPS = 60;
  config2xn_map_[OpenNI_QQVGA_60Hz] = output_mode;
}

bool
OpenNIGrabber::mapConfigMode2XnMode(int mode, XnMapOutputMode &xnmode) const
{
  std::map<int, XnMapOutputMode>::const_iterator it = config2xn_map_.find(mode);
  if (it != config2xn_map_.end())
  {
    xnmode = it->second;
    return (true);
  }
  else
  {
    return (false);
  }
}

std::vector<std::pair<int, XnMapOutputMode> > OpenNIGrabber::getAvailableDepthModes() const
{
  XnMapOutputMode dummy;
  std::vector<std::pair<int, XnMapOutputMode> > result;
  for (std::map<int, XnMapOutputMode>::const_iterator it = config2xn_map_.begin(); it != config2xn_map_.end(); ++it)
  {
    if (device_->findCompatibleDepthMode(it->second, dummy))
      result.push_back(*it);
  }

  return result;
}

std::vector<std::pair<int, XnMapOutputMode> > OpenNIGrabber::getAvailableImageModes() const
{
  XnMapOutputMode dummy;
  std::vector<std::pair<int, XnMapOutputMode> > result;
  for (std::map<int, XnMapOutputMode>::const_iterator it = config2xn_map_.begin(); it != config2xn_map_.end(); ++it)
  {
    if (device_->findCompatibleImageMode(it->second, dummy))
      result.push_back(*it);
  }

  return result;
}

} // namespace

#endif
