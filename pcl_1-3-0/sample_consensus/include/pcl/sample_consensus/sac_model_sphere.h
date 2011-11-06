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
 * $Id: sac_model_sphere.h 3027 2011-11-01 04:04:27Z rusu $
 *
 */

#ifndef PCL_SAMPLE_CONSENSUS_MODEL_SPHERE_H_
#define PCL_SAMPLE_CONSENSUS_MODEL_SPHERE_H_

#include <pcl/sample_consensus/sac_model.h>
#include <pcl/sample_consensus/model_types.h>
#include <boost/thread/mutex.hpp>

namespace pcl
{
  /** \brief @b SampleConsensusModelSphere defines a model for 3D sphere segmentation.
    * The model coefficients are defined as:
    * <ul>
    * <li><b>center.x</b> : the X coordinate of the sphere's center
    * <li><b>center.y</b> : the Y coordinate of the sphere's center
    * <li><b>center.z</b> : the Z coordinate of the sphere's center
    * <li><b>radius</b>   : the sphere's radius
    * </ul>
    * \author Radu Bogdan Rusu
    * \ingroup sample_consensus
    */
  template <typename PointT>
  class SampleConsensusModelSphere : public SampleConsensusModel<PointT>
  {
    using SampleConsensusModel<PointT>::input_;
    using SampleConsensusModel<PointT>::indices_;
    using SampleConsensusModel<PointT>::radius_min_;
    using SampleConsensusModel<PointT>::radius_max_;

    public:
      typedef typename SampleConsensusModel<PointT>::PointCloud PointCloud;
      typedef typename SampleConsensusModel<PointT>::PointCloudPtr PointCloudPtr;
      typedef typename SampleConsensusModel<PointT>::PointCloudConstPtr PointCloudConstPtr;

      typedef boost::shared_ptr<SampleConsensusModelSphere> Ptr;

      /** \brief Constructor for base SampleConsensusModelSphere.
        * \param[in] cloud the input point cloud dataset
        */
      SampleConsensusModelSphere (const PointCloudConstPtr &cloud) : SampleConsensusModel<PointT> (cloud) { }

      /** \brief Constructor for base SampleConsensusModelSphere.
        * \param[in] cloud the input point cloud dataset
        * \param[in] indices a vector of point indices to be used from \a cloud
        */
      SampleConsensusModelSphere (const PointCloudConstPtr &cloud, const std::vector<int> &indices) : SampleConsensusModel<PointT> (cloud, indices) { }

      /** \brief Get 4 random points (3 non-collinear) as data samples and return them as point indices.
        * \param[out] iterations the internal number of iterations used by SAC methods
        * \param[out] samples the resultant model samples
        * \note assumes unique points!
        * \note Two different points could be enough in theory, to infere some sort of a center and a radius,
        *       but in practice, we might end up with a lot of points which are just 'close' to one another.
        *       Therefore we have two options:
        *       a) use normal information (good but I wouldn't rely on it in extremely noisy point clouds, no matter what)
        *       b) get two more points and uniquely identify a sphere in space (3 unique points define a circle)
        */
      void 
      getSamples (int &iterations, std::vector<int> &samples);

      /** \brief Check whether the given index samples can form a valid sphere model, compute the model 
        * coefficients from these samples and store them internally in model_coefficients. 
        * The sphere coefficients are: x, y, z, R.
        * \param[in] samples the point indices found as possible good candidates for creating a valid model
        * \param[out] model_coefficients the resultant model coefficients
        */
      bool 
      computeModelCoefficients (const std::vector<int> &samples, 
                                Eigen::VectorXf &model_coefficients);

      /** \brief Compute all distances from the cloud data to a given sphere model.
        * \param[in] model_coefficients the coefficients of a sphere model that we need to compute distances to
        * \param[out] distances the resultant estimated distances
        */
      void 
      getDistancesToModel (const Eigen::VectorXf &model_coefficients, 
                           std::vector<double> &distances);

      /** \brief Select all the points which respect the given model coefficients as inliers.
        * \param[in] model_coefficients the coefficients of a sphere model that we need to compute distances to
        * \param[in] threshold a maximum admissible distance threshold for determining the inliers from the outliers
        * \param[out] inliers the resultant model inliers
        */
      void 
      selectWithinDistance (const Eigen::VectorXf &model_coefficients, 
                            const double threshold, 
                            std::vector<int> &inliers);

      /** \brief Count all the points which respect the given model coefficients as inliers. 
        * 
        * \param[in] model_coefficients the coefficients of a model that we need to compute distances to
        * \param[in] threshold maximum admissible distance threshold for determining the inliers from the outliers
        * \return the resultant number of inliers
        */
      virtual int
      countWithinDistance (const Eigen::VectorXf &model_coefficients, 
                           const double threshold);

      /** \brief Recompute the sphere coefficients using the given inlier set and return them to the user.
        * @note: these are the coefficients of the sphere model after refinement (eg. after SVD)
        * \param[in] inliers the data inliers found as supporting the model
        * \param[in] model_coefficients the initial guess for the optimization
        * \param[out] optimized_coefficients the resultant recomputed coefficients after non-linear optimization
        */
      void 
      optimizeModelCoefficients (const std::vector<int> &inliers, 
                                 const Eigen::VectorXf &model_coefficients, 
                                 Eigen::VectorXf &optimized_coefficients);

      /** \brief Create a new point cloud with inliers projected onto the sphere model.
        * \param[in] inliers the data inliers that we want to project on the sphere model
        * \param[in] model_coefficients the coefficients of a sphere model
        * \param[out] projected_points the resultant projected points
        * \param[in] copy_data_fields set to true if we need to copy the other data fields
        * \todo implement this.
        */
      void 
      projectPoints (const std::vector<int> &inliers, 
                     const Eigen::VectorXf &model_coefficients, 
                     PointCloud &projected_points, 
                     bool copy_data_fields = true);

      /** \brief Verify whether a subset of indices verifies the given sphere model coefficients.
        * \param[in] indices the data indices that need to be tested against the sphere model
        * \param[in] model_coefficients the sphere model coefficients
        * \param[in] threshold a maximum admissible distance threshold for determining the inliers from the outliers
        */
      bool 
      doSamplesVerifyModel (const std::set<int> &indices, 
                            const Eigen::VectorXf &model_coefficients, 
                            const double threshold);

      /** \brief Return an unique id for this model (SACMODEL_SPHERE). */
      inline pcl::SacModel getModelType () const { return (SACMODEL_SPHERE); }

    protected:
      /** \brief Check whether a model is valid given the user constraints.
        * \param[in] model_coefficients the set of model coefficients
        */
      inline bool 
      isModelValid (const Eigen::VectorXf &model_coefficients)
      {
        // Needs a valid model coefficients
        if (model_coefficients.size () != 4)
        {
          PCL_ERROR ("[pcl::SampleConsensusModelSphere::isModelValid] Invalid number of model coefficients given (%lu)!\n", (unsigned long)model_coefficients.size ());
          return (false);
        }

        if (radius_min_ != -DBL_MAX && model_coefficients[3] < radius_min_)
          return (false);
        if (radius_max_ != DBL_MAX && model_coefficients[3] > radius_max_)
          return (false);

        return (true);
      }

      /** \brief Check if a sample of indices results in a good sample of points
        * indices.
        * \param[in] samples the resultant index samples
        */
      bool
      isSampleGood(const std::vector<int> &samples) const;

    private:
      /** \brief Temporary boost mutex for \a tmp_inliers_ */
      boost::mutex tmp_mutex_;

      /** \brief Temporary pointer to a list of given indices for optimizeModelCoefficients () */
      const std::vector<int> *tmp_inliers_;

      struct OptimizationFunctor : pcl::Functor<double>
      {
        /** Functor constructor
          * \param[in] n the number of variables
          * \param[in] m the number of functions   
          * \param[in] estimator pointer to the estimator object
          * \param[in] distance distance computation function pointer
          */
        OptimizationFunctor(int n, int m, pcl::SampleConsensusModelSphere<PointT> *model) : 
          pcl::Functor<double>(m,n), model_(model) {}
        /** Cost function to be minimized
          * \param[in] x the variables array
          * \param[out] fvec the resultant functions evaluations
          * \return 0
          */
        int operator() (const Eigen::VectorXd &x, Eigen::VectorXd &fvec) const
        {
          Eigen::Vector4d cen_t;
          cen_t[3] = 0;
          for (int i = 0; i < m_values; ++i)
          {
            // Compute the difference between the center of the sphere and the datapoint X_i
            cen_t[0] = model_->input_->points[(*model_->tmp_inliers_)[i]].x - x[0];
            cen_t[1] = model_->input_->points[(*model_->tmp_inliers_)[i]].y - x[1];
            cen_t[2] = model_->input_->points[(*model_->tmp_inliers_)[i]].z - x[2];
            
            // g = sqrt ((x-a)^2 + (y-b)^2 + (z-c)^2) - R
            fvec[i] = sqrt (cen_t.dot (cen_t)) - x[3];
          }
          return (0);
        }
        
        pcl::SampleConsensusModelSphere<PointT> *model_;
      };
  };
}

#endif  //#ifndef PCL_SAMPLE_CONSENSUS_MODEL_SPHERE_H_
