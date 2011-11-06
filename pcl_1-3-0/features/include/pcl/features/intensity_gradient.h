/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Willow Garage, Inc.
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
 * $Id: intensity_gradient.h 1370 2011-06-19 01:06:01Z jspricke $
 *
 */
#ifndef PCL_INTENSITY_GRADIENT_H_
#define PCL_INTENSITY_GRADIENT_H_

#include <pcl/features/feature.h>

namespace pcl
{
  /** \brief @b IntensityGradientEstimation estimates the intensity gradient for a point cloud that contains position
    * and intensity values.  The intensity gradient at a given point will be a vector orthogonal to the surface
    * normal and pointing in the direction of the greatest increase in local intensity; the vector's magnitude
    * indicates the rate of intensity change.
    * \author Michael Dixon
    * \ingroup features
    */
  template <typename PointInT, typename PointNT, typename PointOutT>
  class IntensityGradientEstimation : public FeatureFromNormals<PointInT, PointNT, PointOutT>
  {
    public:
      using Feature<PointInT, PointOutT>::feature_name_;
      using Feature<PointInT, PointOutT>::getClassName;
      using Feature<PointInT, PointOutT>::indices_;
      using Feature<PointInT, PointOutT>::surface_;
      using Feature<PointInT, PointOutT>::k_;
      using Feature<PointInT, PointOutT>::search_parameter_;
      using FeatureFromNormals<PointInT, PointNT, PointOutT>::normals_;

      typedef typename Feature<PointInT, PointOutT>::PointCloudOut PointCloudOut;

      /** \brief Empty constructor. */
      IntensityGradientEstimation ()
      {
        feature_name_ = "IntensityGradientEstimation";
      };

    protected:
      /** \brief Estimate the intensity gradients for a set of points given in <setInputCloud (), setIndices ()> using 
        *  the surface in setSearchSurface () and the spatial locator in setSearchMethod ().
        *  \param output the resultant point cloud that contains the intensity gradient vectors
        */
      void 
      computeFeature (PointCloudOut &output);

    private:
      /** \brief Estimate the intensity gradient around a given point based on its spatial neighborhood of points
        * \param cloud a point cloud dataset containing XYZI coordinates (Cartesian coordinates + intensity)
        * \param indices the indices of the neighoring points in the dataset
        * \param point the 3D Cartesian coordinates of the point at which to estimate the gradient 
        * \param normal the 3D surface normal of the given point
        * \param gradient the resultant 3D gradient vector
        */
      void 
      computePointIntensityGradient (const pcl::PointCloud<PointInT> &cloud, 
                                     const std::vector<int> &indices, 
                                     const Eigen::Vector3f &point, const Eigen::Vector3f &normal, 
                                     Eigen::Vector3f &gradient);
                                      
  };
}

#endif // #ifndef PCL_INTENSITY_GRADIENT_H_
