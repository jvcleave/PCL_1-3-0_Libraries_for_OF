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

#include "pcl/pcl_config.h"
#ifdef HAVE_OPENNI

#ifndef __PCL_IO_ONI_PLAYER__
#define __PCL_IO_ONI_PLAYER__

#include <Eigen/Core>
#include <pcl/io/grabber.h>
#include <pcl/io/openni_camera/openni_driver.h>
#include <pcl/io/openni_camera/openni_device_oni.h>
#include <pcl/io/openni_camera/openni_image.h>
#include <pcl/io/openni_camera/openni_depth_image.h>
#include <pcl/io/openni_camera/openni_ir_image.h>
#include <string>
#include <deque>
#include <boost/thread/mutex.hpp>
#include <pcl/common/synchronizer.h>


namespace pcl
{
  struct PointXYZ;
  struct PointXYZRGB;
  struct PointXYZI;
  template <typename T> class PointCloud;

  class PCL_EXPORTS ONIGrabber : public Grabber
  {
    public:
      //define callback signature typedefs
      typedef void (sig_cb_openni_image) (const boost::shared_ptr<openni_wrapper::Image>&);
      typedef void (sig_cb_openni_depth_image) (const boost::shared_ptr<openni_wrapper::DepthImage>&);
      typedef void (sig_cb_openni_ir_image) (const boost::shared_ptr<openni_wrapper::IRImage>&);
      typedef void (sig_cb_openni_image_depth_image) (const boost::shared_ptr<openni_wrapper::Image>&, const boost::shared_ptr<openni_wrapper::DepthImage>&, float constant) ;
      typedef void (sig_cb_openni_ir_depth_image) (const boost::shared_ptr<openni_wrapper::IRImage>&, const boost::shared_ptr<openni_wrapper::DepthImage>&, float constant) ;
      typedef void (sig_cb_openni_point_cloud) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZ> >&);
      typedef void (sig_cb_openni_point_cloud_rgb) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZRGB> >&);
      typedef void (sig_cb_openni_point_cloud_i) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZI> >&);

      ONIGrabber (const std::string& file_name, bool repeat, bool stream);

      virtual ~ONIGrabber () throw ();

     /**
     * @brief For devices that are streaming, the streams are started by calling this method.
     *        Trigger-based devices, just trigger the device once for each call of start.
     * @author Suat Gedikli
     */
    virtual void start ();

    /**
     * @brief For devices that are streaming, the streams are stopped.
     *        This method has no effect for triggered devices.
     * @author Suat Gedikli
     */
    virtual void stop ();

    /**
     * @brief returns the name of the concrete subclass.
     * @return the name of the concrete driver.
     * @author Suat Gedikli
     */
    virtual std::string getName () const;

    /**
     * @brief Indicates whether the grabber is streaming or not. This value is not defined for triggered devices.
     * @return true if grabber is running / streaming. False otherwise.
     */
    virtual bool isRunning () const;

  protected:
    void
    imageCallback (boost::shared_ptr<openni_wrapper::Image> image, void* cookie);

    /** \brief ... */
    void
    depthCallback (boost::shared_ptr<openni_wrapper::DepthImage> depth_image, void* cookie);

    /** \brief ... */
    void
    irCallback (boost::shared_ptr<openni_wrapper::IRImage> ir_image, void* cookie);

    /** \brief ... */
    void
    imageDepthImageCallback (const boost::shared_ptr<openni_wrapper::Image> &image,
                             const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image);

    /** \brief ... */
    void
    irDepthImageCallback (const boost::shared_ptr<openni_wrapper::IRImage> &image,
                          const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image);

    /** \brief ... */
    boost::shared_ptr<pcl::PointCloud<pcl::PointXYZ> >
    convertToXYZPointCloud (const boost::shared_ptr<openni_wrapper::DepthImage> &depth) const;

    /** \brief ... */
    boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB> >
    convertToXYZRGBPointCloud (const boost::shared_ptr<openni_wrapper::Image> &image,
                               const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image) const;
    /** \brief ... */
    boost::shared_ptr<pcl::PointCloud<pcl::PointXYZI> >
    convertToXYZIPointCloud (const boost::shared_ptr<openni_wrapper::IRImage> &image,
                             const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image) const;
    Synchronizer<boost::shared_ptr<openni_wrapper::Image>, boost::shared_ptr<openni_wrapper::DepthImage> > rgb_sync_;
    Synchronizer<boost::shared_ptr<openni_wrapper::IRImage>, boost::shared_ptr<openni_wrapper::DepthImage> > ir_sync_;

    /** \brief the actual openni device*/
    boost::shared_ptr<openni_wrapper::DeviceONI> device_;
    std::string rgb_frame_id_;
    std::string depth_frame_id_;
    bool running_;
    unsigned image_width_;
    unsigned image_height_;
    unsigned depth_width_;
    unsigned depth_height_;
    openni_wrapper::OpenNIDevice::CallbackHandle depth_callback_handle;
    openni_wrapper::OpenNIDevice::CallbackHandle image_callback_handle;
    openni_wrapper::OpenNIDevice::CallbackHandle ir_callback_handle;
    boost::signals2::signal<sig_cb_openni_image >*            image_signal_;
    boost::signals2::signal<sig_cb_openni_depth_image >*      depth_image_signal_;
    boost::signals2::signal<sig_cb_openni_ir_image >*         ir_image_signal_;
    boost::signals2::signal<sig_cb_openni_image_depth_image>* image_depth_image_signal_;
    boost::signals2::signal<sig_cb_openni_ir_depth_image>*    ir_depth_image_signal_;
    boost::signals2::signal<sig_cb_openni_point_cloud >*      point_cloud_signal_;
    boost::signals2::signal<sig_cb_openni_point_cloud_i >*    point_cloud_i_signal_;
    boost::signals2::signal<sig_cb_openni_point_cloud_rgb >*  point_cloud_rgb_signal_;

  public:
      EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  };

} // namespace

#endif // __PCL_IO_ONI_PLAYER__
#endif // HAVE_OPENNI

