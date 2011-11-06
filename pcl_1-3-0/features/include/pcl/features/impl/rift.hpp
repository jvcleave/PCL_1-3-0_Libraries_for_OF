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
 * $Id: rift.hpp 1370 2011-06-19 01:06:01Z jspricke $
 *
 */

#ifndef PCL_FEATURES_IMPL_RIFT_H_
#define PCL_FEATURES_IMPL_RIFT_H_

#include "pcl/features/rift.h"

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointInT, typename GradientT, typename PointOutT> void
pcl::RIFTEstimation<PointInT, GradientT, PointOutT>::computeRIFT (
      const PointCloudIn &cloud, const PointCloudGradient &gradient, 
      int p_idx, float radius, const std::vector<int> &indices, 
      const std::vector<float> &sqr_distances, Eigen::MatrixXf &rift_descriptor)
{
  if (indices.empty ())
  {
    PCL_ERROR ("[pcl::RIFTEstimation] Null indices points passed!\n");
    return;
  }

  // Determine the number of bins to use based on the size of rift_descriptor
  int nr_distance_bins = rift_descriptor.cols ();
  int nr_gradient_bins = rift_descriptor.rows ();

  // Get the center point
  pcl::Vector3fMapConst p0 = cloud.points[p_idx].getVector3fMap ();

  // Compute the RIFT descriptor
  rift_descriptor.setZero ();
  for (size_t idx = 0; idx < indices.size (); ++idx)
  {
    // Compute the gradient magnitude and orientation (relative to the center point)
    ///Eigen::Map<Eigen::Vector3f> point (& (cloud.points[indices[idx]].x));
    pcl::Vector3fMapConst point = cloud.points[indices[idx]].getVector3fMap ();
    Eigen::Map<const Eigen::Vector3f> gradient_vector (& (gradient.points[indices[idx]].gradient[0]));

    float gradient_magnitude = gradient_vector.norm ();
    float gradient_angle_from_center = acos (gradient_vector.dot ((point - p0).normalized ()) / gradient_magnitude);
    if (!pcl_isfinite (gradient_angle_from_center))
    {
      gradient_angle_from_center = 0.0;
    }

    // Normalize distance and angle values to: 0.0 <= d,g < nr_distances_bins,nr_gradient_bins
    const float eps = std::numeric_limits<float>::epsilon ();
    float d = nr_distance_bins * sqrt (sqr_distances[idx]) / (radius + eps);
    float g = nr_gradient_bins * gradient_angle_from_center / (M_PI + eps);

    // Compute the bin indices that need to be updated
    int d_idx_min = (std::max)((int) ceil (d - 1), 0);
    int d_idx_max = (std::min)((int) floor (d + 1), nr_distance_bins - 1);
    int g_idx_min = (int) ceil (g - 1);
    int g_idx_max = (int) floor (g + 1);

    // Update the appropriate bins of the histogram 
    for (int g_idx = g_idx_min; g_idx <= g_idx_max; ++g_idx)  
    {
      // Because gradient orientation is cyclical, out-of-bounds values must wrap around
      int g_idx_wrapped = ((g_idx + nr_gradient_bins) % nr_gradient_bins); 

      for (int d_idx = d_idx_min; d_idx <= d_idx_max; ++d_idx)
      {
        // To avoid boundary effects, use linear interpolation when updating each bin 
        float w = (1 - fabs (d - d_idx)) * (1 - fabs (g - g_idx));

        rift_descriptor (g_idx_wrapped * nr_distance_bins + d_idx) += w * gradient_magnitude;
      }
    }
  }

  // Normalize the RIFT descriptor to unit magnitude
  rift_descriptor.normalize ();
}


//////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointInT, typename GradientT, typename PointOutT> void
pcl::RIFTEstimation<PointInT, GradientT, PointOutT>::computeFeature (PointCloudOut &output)
{
  // Make sure a search radius is set
  if (search_radius_ == 0.0)
  {
    PCL_ERROR ("[pcl::%s::computeFeature] The search radius must be set before computing the feature!\n",
               getClassName ().c_str ());
    output.width = output.height = 0;
    output.points.clear ();
    return;
  }

  // Make sure the RIFT descriptor has valid dimensions
  if (nr_gradient_bins_ <= 0)
  {
    PCL_ERROR ("[pcl::%s::computeFeature] The number of gradient bins must be greater than zero!\n",
               getClassName ().c_str ());
    output.width = output.height = 0;
    output.points.clear ();
    return;
  }
  if (nr_distance_bins_ <= 0)
  {
    PCL_ERROR ("[pcl::%s::computeFeature] The number of distance bins must be greater than zero!\n",
               getClassName ().c_str ());
    output.width = output.height = 0;
    output.points.clear ();
    return;
  }

  // Check for valid input gradient
  if (!gradient_)
  {
    PCL_ERROR ("[pcl::%s::computeFeature] No input gradient was given!\n", getClassName ().c_str ());
    output.width = output.height = 0;
    output.points.clear ();
    return;
  }
  if (gradient_->points.size () != surface_->points.size ())
  {
    PCL_ERROR ("[pcl::%s::computeFeature] The number of points in the input dataset differs from the number of points in the gradient!\n", getClassName ().c_str ());
    output.width = output.height = 0;
    output.points.clear ();
    return;
  }

  Eigen::MatrixXf rift_descriptor (nr_gradient_bins_, nr_distance_bins_);
  std::vector<int> nn_indices;
  std::vector<float> nn_dist_sqr;
 
  // Iterating over the entire index vector
  for (size_t idx = 0; idx < indices_->size (); ++idx)
  {
    // Find neighbors within the search radius
    tree_->radiusSearch ((*indices_)[idx], search_radius_, nn_indices, nn_dist_sqr);

    // Compute the RIFT descriptor
    computeRIFT (*surface_, *gradient_, (*indices_)[idx], search_radius_, nn_indices, nn_dist_sqr, rift_descriptor);

    // Copy into the resultant cloud
    for (int bin = 0; bin < rift_descriptor.size (); ++bin)
      output.points[idx].histogram[bin] = rift_descriptor (bin);
  }
}

#define PCL_INSTANTIATE_RIFTEstimation(T,NT,OutT) template class PCL_EXPORTS pcl::RIFTEstimation<T,NT,OutT>;

#endif    // PCL_FEATURES_IMPL_RIFT_H_ 

