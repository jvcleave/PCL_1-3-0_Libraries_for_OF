/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2011, Willow Garage, Inc.
 *
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
 */

#ifndef PCL_INTEGRALIMAGE_BASED_NORMAL_ESTIMATOR_H_
#define PCL_INTEGRALIMAGE_BASED_NORMAL_ESTIMATOR_H_

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include "pcl/features/feature.h"
#include "pcl/features/integral_image_2d.h"
#include "pcl/features/integral_image2D.h"

namespace pcl
{
  /**
    * \brief Surface normal estimation on dense data using integral images.
    * \author Stefan Holzer
    */
  template <typename PointInT, typename PointOutT>
  class IntegralImageNormalEstimation: public Feature<PointInT, PointOutT>
  {
    using Feature<PointInT, PointOutT>::input_;
    using Feature<PointInT, PointOutT>::feature_name_;
    using Feature<PointInT, PointOutT>::tree_;
    using Feature<PointInT, PointOutT>::k_;

    public:

      enum NormalEstimationMethod
      {
        COVARIANCE_MATRIX,
        AVERAGE_3D_GRADIENT,
        AVERAGE_DEPTH_CHANGE
      };

      typedef typename Feature<PointInT, PointOutT>::PointCloudIn  PointCloudIn;
      typedef typename Feature<PointInT, PointOutT>::PointCloudOut PointCloudOut;

      /** \brief Constructor */
      IntegralImageNormalEstimation () : 
        normal_estimation_method_(AVERAGE_3D_GRADIENT),
        integral_image_x_(NULL), integral_image_y_(NULL), 
        integral_image_xyz_(NULL), integral_image_(NULL),
        integral_image_XYZ_ (true),
        diff_x_(NULL), diff_y_(NULL), depth_data_(NULL),
        use_depth_dependent_smoothing_(false),
        max_depth_change_factor_(20.0f*0.001f),
        normal_smoothing_size_(10.0f),
        init_covariance_matrix_(false), init_average_3d_gradient_(false), init_depth_change_(false)
      {
        feature_name_ = "IntegralImagesNormalEstimation";
        tree_.reset ();
        k_ = 1;
      }


      /** \brief Destructor **/
      virtual ~IntegralImageNormalEstimation ();

      /** \brief Set the regions size which is considered for normal estimation.
        * \param width the width of the search rectangle
        * \param height the height of the search rectangle
        */
      void 
      setRectSize (const int width, const int height);

      /** \brief Computes the normal at the specified position. 
        * \param pos_x x position (pixel)
        * \param pos_y y position (pixel)
        * \param normal the output estimated normal 
        */
      void
      computePointNormal (const int pos_x, const int pos_y, PointOutT &normal);

      /** \brief The depth change threshold for computing object borders
        * \param max_depth_change_factor the depth change threshold for computing object borders based on 
        * depth changes
        */
      void 
      setMaxDepthChangeFactor (float max_depth_change_factor)
      {
        max_depth_change_factor_ = max_depth_change_factor;
      }

      /** \brief Set the normal smoothing size
        * \param normal_smoothing_size factor which influences the size of the area used to smooth normals 
        * (depth dependent if useDepthDependentSmoothing is true)
        */
      void
      setNormalSmoothingSize (float normal_smoothing_size)
      {
        normal_smoothing_size_ = normal_smoothing_size;
      }

      /** \brief Set the normal estimation method. The current implemented algorithms are:
        * <ul>
        *   <li><b>COVARIANCE_MATRIX</b> - creates 9 integral images to compute the normal for a specific point 
        *   from the covariance matrix of its local neighborhood.</b>
        *   <li><b>AVERAGE_3D_GRADIENT</b> - creates 6 integral images to compute smoothed versions of 
        *   horizontal and vertical 3D gradients and computes the normals using the cross-product between these 
        *   two gradients.
        *   <li><b>AVERAGE_DEPTH_CHANGE</b> -  creates only a single integral image and computes the normals 
        *   from the average depth changes.
        * </ul>
        * \param normal_estimation_method the method used for normal estimation
        */
      void
      setNormalEstimationMethod (NormalEstimationMethod normal_estimation_method)
      {
        normal_estimation_method_ = normal_estimation_method;
      }

      /** \brief Set whether to use depth depending smoothing or not
        * \param use_depth_dependent_smoothing decides whether the smoothing is depth dependent
        */
      void
      setDepthDependentSmoothing (bool use_depth_dependent_smoothing)
      {
        use_depth_dependent_smoothing_ = use_depth_dependent_smoothing;
      }

       /** \brief Provide a pointer to the input dataset (overwrites the PCLBase::setInputCloud method)
        * \param cloud the const boost shared pointer to a PointCloud message
        */
      virtual inline void 
      setInputCloud (const typename PointCloudIn::ConstPtr &cloud) 
      { 
        input_ = cloud; 

        init_covariance_matrix_ = init_average_3d_gradient_ = init_depth_change_ = false;

        // Initialize the correct data structure based on the normal estimation method chosen 
        initData ();
      }

    protected:

      /** \brief Computes the normal for the complete cloud. 
        * \param output the resultant normals
        */
      void 
      computeFeature (PointCloudOut &output);

      /** \brief Initialize the data structures, based on the normal estimation method chosen.
        */
      void 
      initData ();
      
    private:
      /** \brief The normal estimation method to use. Currently, 3 implementations are provided:
        *
        * - COVARIANCE_MATRIX
        * - AVERAGE_3D_GRADIENT
        * - AVERAGE_DEPTH_CHANGE
        */
      NormalEstimationMethod normal_estimation_method_;
    
      /** The width of the neighborhood region used for computing the normal. */
      int rect_width_;
      /** The height of the neighborhood region used for computing the normal. */
      int rect_height_;

      /** the threshold used to detect depth discontinuities */
      float distance_threshold_;

      /** integral image in x-direction */
      IntegralImage2D<float, double> *integral_image_x_;
      /** integral image in y-direction */
      IntegralImage2D<float, double> *integral_image_y_;
      /** integral image xyz */
      IntegralImage2D<float, double> *integral_image_xyz_;
      /** integral image */
      IntegralImage2D<float, double> *integral_image_;
      /** integral image xyz */
      IntegralImage2Dim<float, 3> integral_image_XYZ_;

      /** derivatives in x-direction */
      float *diff_x_;
      /** derivatives in y-direction */
      float *diff_y_;

      /** depth data */
      float *depth_data_;

      /** \brief Smooth data based on depth (true/false). */
      bool use_depth_dependent_smoothing_;

      /** \brief Threshold for detecting depth discontinuities */
      float max_depth_change_factor_;

      /** \brief */
      float normal_smoothing_size_;

      /** \brief True when a dataset has been received and the covariance_matrix data has been initialized. */
      bool init_covariance_matrix_;
      
      /** \brief True when a dataset has been received and the average 3d gradient data has been initialized. */
      bool init_average_3d_gradient_;

      /** \brief True when a dataset has been received and the depth change data has been initialized. */
      bool init_depth_change_;

      /** \brief Internal initialization method for COVARIANCE_MATRIX estimation. */
      void
      initCovarianceMatrixMethod ();

      /** \brief Internal initialization method for AVERAGE_3D_GRADIENT estimation. */
      void
      initAverage3DGradientMethod ();

      /** \brief Internal initialization method for AVERAGE_DEPTH_CHANGE estimation. */
      void
      initAverageDepthChangeMethod ();
  };
}

#endif 

