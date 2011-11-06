#include <pcl/pcl_config.h>
#ifdef HAVE_OPENNI

#include <pcl/io/oni_grabber.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/time.h>
#include <pcl/io/pcl_io_exception.h>
#include <boost/shared_array.hpp>
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


ONIGrabber::ONIGrabber (const std::string& file_name, bool repeat, bool stream)
: running_(false)
{
  rgb_frame_id_ = "/openni_rgb_optical_frame";
  depth_frame_id_ = "/openni_depth_optical_frame";

  openni_wrapper::OpenNIDriver& driver = openni_wrapper::OpenNIDriver::getInstance();
  device_ = boost::dynamic_pointer_cast< openni_wrapper::DeviceONI> (driver.createVirtualDevice(file_name, repeat, stream));

  if (!device_->hasDepthStream())
    THROW_PCL_IO_EXCEPTION("Device does not provide 3D information.");

  XnMapOutputMode depth_mode = device_->getDepthOutputMode();
  depth_width_ = depth_mode.nXRes;
  depth_height_ = depth_mode.nYRes;

  depth_image_signal_ = createSignal <sig_cb_openni_depth_image > ();
  point_cloud_signal_ = createSignal <sig_cb_openni_point_cloud > ();

  if (device_->hasIRStream())
  {
    ir_image_signal_        = createSignal <sig_cb_openni_ir_image > ();
    point_cloud_i_signal_   = createSignal <sig_cb_openni_point_cloud_i > ();
    ir_depth_image_signal_  = createSignal <sig_cb_openni_ir_depth_image > ();
  }

  if (device_->hasImageStream())
  {
    XnMapOutputMode depth_mode = device_->getImageOutputMode();
    image_width_ = depth_mode.nXRes;
    image_height_ = depth_mode.nYRes;

    image_signal_             = createSignal <sig_cb_openni_image > ();
    image_depth_image_signal_ = createSignal <sig_cb_openni_image_depth_image > ();
    point_cloud_rgb_signal_   = createSignal <sig_cb_openni_point_cloud_rgb > ();
    rgb_sync_.addCallback(boost::bind(&ONIGrabber::imageDepthImageCallback, this, _1, _2));
  }

  image_callback_handle = device_->registerImageCallback(&ONIGrabber::imageCallback, *this);
  depth_callback_handle = device_->registerDepthCallback(&ONIGrabber::depthCallback, *this);
  ir_callback_handle    = device_->registerIRCallback(&ONIGrabber::irCallback, *this);

  // if in trigger mode -> publish these topics
  if (!stream)
  {
    // check if we need to start/stop any stream
    if (device_->hasImageStream() && !device_->isImageStreamRunning())
      device_->startImageStream();

    if (device_->hasDepthStream() && !device_->isDepthStreamRunning())
      device_->startDepthStream();

    if (device_->hasIRStream() && !device_->isIRStreamRunning())
      device_->startIRStream();
  }
}

ONIGrabber::~ONIGrabber() throw ()
{
  try
  {
    stop();
    // unregister callbacks
    device_->unregisterDepthCallback(depth_callback_handle);
    device_->unregisterImageCallback(image_callback_handle);
    device_->unregisterIRCallback(image_callback_handle);

    // disconnect all listeners
    disconnect_all_slots <sig_cb_openni_image > ();
    disconnect_all_slots <sig_cb_openni_depth_image > ();
    disconnect_all_slots <sig_cb_openni_ir_image > ();
    disconnect_all_slots <sig_cb_openni_image_depth_image > ();
    disconnect_all_slots <sig_cb_openni_point_cloud > ();
    disconnect_all_slots <sig_cb_openni_point_cloud_rgb > ();
    disconnect_all_slots <sig_cb_openni_point_cloud_i > ();
  }
  catch (...)
  {
    // destructor never throws
  }
}

void ONIGrabber::start()
{
  if (device_->isStreaming())
  {
    try
    {
      // check if we need to start/stop any stream
      if (device_->hasImageStream() && !device_->isImageStreamRunning())
        device_->startImageStream();

      if (device_->hasDepthStream() && !device_->isDepthStreamRunning())
        device_->startDepthStream();

      if (device_->hasIRStream() && !device_->isIRStreamRunning())
        device_->startIRStream();

      running_ = true;
    }
    catch (openni_wrapper::OpenNIException& ex)
    {
      THROW_PCL_IO_EXCEPTION("Could not start streams. Reason: %s", ex.what());
    }
  }
  else
  {
    if (device_->hasImageStream())
      device_->trigger();

    if (device_->hasDepthStream())
      device_->trigger();

    if (device_->hasIRStream())
      device_->trigger();
  }
}

void ONIGrabber::stop()
{
  if (device_->isStreaming())
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
}

bool ONIGrabber::isRunning() const
{
  return running_;
}

std::string ONIGrabber::getName() const
{
  return std::string("ONIGrabber");
}

void ONIGrabber::imageCallback(boost::shared_ptr<openni_wrapper::Image> image, void* cookie)
{
  if (num_slots<sig_cb_openni_point_cloud_rgb > () > 0 ||
      num_slots<sig_cb_openni_image_depth_image > () > 0)
    rgb_sync_.add0(image, image->getTimeStamp());

  if (image_signal_->num_slots() > 0)
    image_signal_->operator()(image);

  return;
}

void ONIGrabber::depthCallback(boost::shared_ptr<openni_wrapper::DepthImage> depth_image, void* cookie)
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

void ONIGrabber::irCallback(boost::shared_ptr<openni_wrapper::IRImage> ir_image, void* cookie)
{
  if (num_slots<sig_cb_openni_point_cloud_i > () > 0 ||
      num_slots<sig_cb_openni_ir_depth_image > () > 0)
    ir_sync_.add0(ir_image, ir_image->getTimeStamp());

  if (ir_image_signal_->num_slots() > 0)
    ir_image_signal_->operator()(ir_image);

  return;
}

void ONIGrabber::imageDepthImageCallback(const boost::shared_ptr<openni_wrapper::Image> &image, const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image)
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

void ONIGrabber::irDepthImageCallback(const boost::shared_ptr<openni_wrapper::IRImage> &ir_image, const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image)
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

pcl::PointCloud<pcl::PointXYZ>::Ptr ONIGrabber::convertToXYZPointCloud(const boost::shared_ptr<openni_wrapper::DepthImage>& depth_image) const
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

pcl::PointCloud<pcl::PointXYZRGB>::Ptr ONIGrabber::convertToXYZRGBPointCloud(const boost::shared_ptr<openni_wrapper::Image> &image,
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

pcl::PointCloud<pcl::PointXYZI>::Ptr ONIGrabber::convertToXYZIPointCloud(const boost::shared_ptr<openni_wrapper::IRImage> &ir_image,
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

}
#endif // HAVE_OPENNI

