/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2009, Willow Garage, Inc.
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
 * $Id: pfh.hpp 1778 2011-07-15 00:02:55Z rusu $
 *
 */

#ifndef PCL_FEATURES_IMPL_PFH_H_
#define PCL_FEATURES_IMPL_PFH_H_

#include "pcl/features/pfh.h"

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointInT, typename PointNT, typename PointOutT> bool
pcl::PFHEstimation<PointInT, PointNT, PointOutT>::computePairFeatures (
      const pcl::PointCloud<PointInT> &cloud, const pcl::PointCloud<PointNT> &normals,
      int p_idx, int q_idx, float &f1, float &f2, float &f3, float &f4)
{
  pcl::computePairFeatures (cloud.points[p_idx].getVector4fMap (), normals.points[p_idx].getNormalVector4fMap (),
                            cloud.points[q_idx].getVector4fMap (), normals.points[q_idx].getNormalVector4fMap (),
                            f1, f2, f3, f4);
  return (true);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointInT, typename PointNT, typename PointOutT> void
pcl::PFHEstimation<PointInT, PointNT, PointOutT>::computePointPFHSignature (
      const pcl::PointCloud<PointInT> &cloud, const pcl::PointCloud<PointNT> &normals,
      const std::vector<int> &indices, int nr_split, Eigen::VectorXf &pfh_histogram)
{
  int h_index, h_p;

  // Clear the resultant point histogram
  pfh_histogram.setZero ();

  // Factorization constant
  float hist_incr = 100.0 / (indices.size () * indices.size () - 1);

  // Iterate over all the points in the neighborhood
  for (size_t i_idx = 0; i_idx < indices.size (); ++i_idx)
  {
    for (size_t j_idx = 0; j_idx < indices.size (); ++j_idx)
    {
      // Avoid unnecessary returns
      if (i_idx == j_idx)
        continue;

      // Compute the pair NNi to NNj
      if (!computePairFeatures (cloud, normals, indices[i_idx], indices[j_idx],
                                pfh_tuple_[0], pfh_tuple_[1], pfh_tuple_[2], pfh_tuple_[3]))
        continue;

      // Normalize the f1, f2, f3 features and push them in the histogram
      f_index_[0] = floor (nr_split * ((pfh_tuple_[0] + M_PI) * d_pi_));
      if (f_index_[0] < 0)         f_index_[0] = 0;
      if (f_index_[0] >= nr_split) f_index_[0] = nr_split - 1;

      f_index_[1] = floor (nr_split * ((pfh_tuple_[1] + 1.0) * 0.5));
      if (f_index_[1] < 0)         f_index_[1] = 0;
      if (f_index_[1] >= nr_split) f_index_[1] = nr_split - 1;

      f_index_[2] = floor (nr_split * ((pfh_tuple_[2] + 1.0) * 0.5));
      if (f_index_[2] < 0)         f_index_[2] = 0;
      if (f_index_[2] >= nr_split) f_index_[2] = nr_split - 1;

      // Copy into the histogram
      h_index = 0;
      h_p     = 1;
      for (int d = 0; d < 3; ++d)
      {
        h_index += h_p * f_index_[d];
        h_p     *= nr_split;
      }
      pfh_histogram[h_index] += hist_incr;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointInT, typename PointNT, typename PointOutT> void
pcl::PFHEstimation<PointInT, PointNT, PointOutT>::computeFeature (PointCloudOut &output)
{
  pfh_histogram_.setZero (nr_subdiv_ * nr_subdiv_ * nr_subdiv_);

  // Allocate enough space to hold the results
  // \note This resize is irrelevant for a radiusSearch ().
  std::vector<int> nn_indices (k_);
  std::vector<float> nn_dists (k_);

  // Iterating over the entire index vector
  for (size_t idx = 0; idx < indices_->size (); ++idx)
  {
    this->searchForNeighbors ((*indices_)[idx], search_parameter_, nn_indices, nn_dists);

    // Estimate the PFH signature at each patch
    computePointPFHSignature (*surface_, *normals_, nn_indices, nr_subdiv_, pfh_histogram_);

    // Copy into the resultant cloud
    for (int d = 0; d < pfh_histogram_.size (); ++d)
      output.points[idx].histogram[d] = pfh_histogram_[d];
  }
}

#define PCL_INSTANTIATE_PFHEstimation(T,NT,OutT) template class PCL_EXPORTS pcl::PFHEstimation<T,NT,OutT>;

#endif    // PCL_FEATURES_IMPL_PFH_H_ 

