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
 * $Id: rmsac.hpp 1370 2011-06-19 01:06:01Z jspricke $
 *
 */

#ifndef PCL_SAMPLE_CONSENSUS_IMPL_RMSAC_H_
#define PCL_SAMPLE_CONSENSUS_IMPL_RMSAC_H_

#include "pcl/sample_consensus/rmsac.h"

//////////////////////////////////////////////////////////////////////////
template <typename PointT> bool
pcl::RandomizedMEstimatorSampleConsensus<PointT>::computeModel (int debug_verbosity_level)
{
  // Warn and exit if no threshold was set
  if (threshold_ == DBL_MAX)
  {
    PCL_ERROR ("[pcl::RandomizedMEstimatorSampleConsensus::computeModel] No threshold set!\n");
    return (false);
  }

  iterations_ = 0;
  double d_best_penalty = DBL_MAX;
  double k = 1.0;

  std::vector<int> best_model;
  std::vector<int> selection;
  Eigen::VectorXf model_coefficients;
  std::vector<double> distances;
  std::set<int> indices_subset;

  int n_inliers_count = 0;

  // Number of samples to try randomly
  size_t fraction_nr_points = pcl_lrint (sac_model_->getIndices ()->size () * fraction_nr_pretest_ / 100.0);

  // Iterate
  while (iterations_ < k)
  {
    // Get X samples which satisfy the model criteria
    sac_model_->getSamples (iterations_, selection);

    if (selection.empty ()) break;

    // Search for inliers in the point cloud for the current plane model M
    if (!sac_model_->computeModelCoefficients (selection, model_coefficients))
    {
      //iterations_++;
      continue;
    }

    // RMSAC addon: verify a random fraction of the data
    // Get X random samples which satisfy the model criterion
    this->getRandomSamples (sac_model_->getIndices (), fraction_nr_points, indices_subset);

    if (!sac_model_->doSamplesVerifyModel (indices_subset, model_coefficients, threshold_))
    {
      // Unfortunately we cannot "continue" after the first iteration, because k might not be set, while iterations gets incremented
      if (k != 1.0)
      {
        ++iterations_;
        continue;
      }
    }

    double d_cur_penalty = 0;
    // Iterate through the 3d points and calculate the distances from them to the model
    sac_model_->getDistancesToModel (model_coefficients, distances);

    if (distances.empty () && k > 1.0)
      continue;

    for (size_t i = 0; i < distances.size (); ++i)
      d_cur_penalty += (std::min) (distances[i], threshold_);

    // Better match ?
    if (d_cur_penalty < d_best_penalty)
    {
      d_best_penalty = d_cur_penalty;

      // Save the current model/coefficients selection as being the best so far
      model_              = selection;
      model_coefficients_ = model_coefficients;

      n_inliers_count = 0;
      // Need to compute the number of inliers for this model to adapt k
      for (size_t i = 0; i < distances.size (); ++i)
        if (distances[i] <= threshold_)
          n_inliers_count++;

      // Compute the k parameter (k=log(z)/log(1-w^n))
      double w = (double)((double)n_inliers_count / (double)sac_model_->getIndices ()->size ());
      double p_no_outliers = 1 - pow (w, (double)selection.size ());
      p_no_outliers = (std::max) (std::numeric_limits<double>::epsilon (), p_no_outliers);       // Avoid division by -Inf
      p_no_outliers = (std::min) (1 - std::numeric_limits<double>::epsilon (), p_no_outliers);   // Avoid division by 0.
      k = log (1 - probability_) / log (p_no_outliers);
    }

    ++iterations_;
    if (debug_verbosity_level > 1)
      PCL_DEBUG ("[pcl::RandomizedMEstimatorSampleConsensus::computeModel] Trial %d out of %d. Best penalty is %f.\n", iterations_, (int)ceil (k), d_best_penalty);
    if (iterations_ > max_iterations_)
    {
      if (debug_verbosity_level > 0)
        PCL_DEBUG ("[pcl::RandomizedMEstimatorSampleConsensus::computeModel] MSAC reached the maximum number of trials.\n");
      break;
    }
  }

  if (model_.empty ())
  {
    if (debug_verbosity_level > 0)
      PCL_DEBUG ("[pcl::RandomizedMEstimatorSampleConsensus::computeModel] Unable to find a solution!\n");
    return (false);
  }

  // Iterate through the 3d points and calculate the distances from them to the model again
  sac_model_->getDistancesToModel (model_coefficients_, distances);
  std::vector<int> &indices = *sac_model_->getIndices ();
  if (distances.size () != indices.size ())
  {
    PCL_ERROR ("[pcl::RandomizedMEstimatorSampleConsensus::computeModel] Estimated distances (%lu) differs than the normal of indices (%lu).\n", (unsigned long)distances.size (), (unsigned long)indices.size ());
    return (false);
  }

  inliers_.resize (distances.size ());
  // Get the inliers for the best model found
  n_inliers_count = 0;
  for (size_t i = 0; i < distances.size (); ++i)
    if (distances[i] <= threshold_)
      inliers_[n_inliers_count++] = indices[i];

  // Resize the inliers vector
  inliers_.resize (n_inliers_count);

  if (debug_verbosity_level > 0)
    PCL_DEBUG ("[pcl::RandomizedMEstimatorSampleConsensus::computeModel] Model: %lu size, %d inliers.\n", (unsigned long)model_.size (), n_inliers_count);

  return (true);
}

#define PCL_INSTANTIATE_RandomizedMEstimatorSampleConsensus(T) template class PCL_EXPORTS pcl::RandomizedMEstimatorSampleConsensus<T>;

#endif    // PCL_SAMPLE_CONSENSUS_IMPL_RMSAC_H_

