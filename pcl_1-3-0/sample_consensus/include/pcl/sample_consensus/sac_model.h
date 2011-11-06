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
 * $Id: sac_model.h 3027 2011-11-01 04:04:27Z rusu $
 *
 */

#ifndef PCL_SAMPLE_CONSENSUS_MODEL_H_
#define PCL_SAMPLE_CONSENSUS_MODEL_H_

#include <cfloat>
#include <limits.h>
#include <set>

#include <pcl/console/print.h>
#include <pcl/point_cloud.h>
#include "pcl/sample_consensus/model_types.h"

namespace pcl
{
  // Forward declaration
#ifndef __ANDROID__
  template<class T> class ProgressiveSampleConsensus;
#endif

  /** \brief @b SampleConsensusModel represents the base model class. All sample consensus models must inherit 
    * from this class.
    * \author Radu Bogdan Rusu
    * \ingroup sample_consensus
    */
  template <typename PointT>
  class SampleConsensusModel
  {
    public:
      typedef typename pcl::PointCloud<PointT> PointCloud;
      typedef typename pcl::PointCloud<PointT>::ConstPtr PointCloudConstPtr;
      typedef typename pcl::PointCloud<PointT>::Ptr PointCloudPtr;

      typedef boost::shared_ptr<SampleConsensusModel> Ptr;
      typedef boost::shared_ptr<const SampleConsensusModel> ConstPtr;

    private:
      /** \brief Empty constructor for base SampleConsensusModel. */
      SampleConsensusModel () : radius_min_ (-DBL_MAX), radius_max_ (DBL_MAX) {};

    public:
      /** \brief Constructor for base SampleConsensusModel.
        * \param[in] cloud the input point cloud dataset
        */
      SampleConsensusModel (const PointCloudConstPtr &cloud) : 
        radius_min_ (-DBL_MAX), radius_max_ (DBL_MAX)
      {
        // Sets the input cloud and creates a vector of "fake" indices
        setInputCloud (cloud);
      }

      /** \brief Constructor for base SampleConsensusModel.
        * \param[in] cloud the input point cloud dataset
        * \param[in] indices a vector of point indices to be used from \a cloud
        */
      SampleConsensusModel (const PointCloudConstPtr &cloud, const std::vector<int> &indices) :
                            input_ (cloud),
                            radius_min_ (-DBL_MAX), radius_max_ (DBL_MAX) 
    
      {
        indices_.reset (new std::vector<int> (indices));
        if (indices_->size () > input_->points.size ())
        {
          PCL_ERROR ("[pcl::SampleConsensusModel] Invalid index vector given with size %lu while the input PointCloud has size %lu!\n", (unsigned long)indices_->size (), (unsigned long)input_->points.size ());
          indices_->clear ();
        }
        shuffled_indices_ = *indices_;
      };

      /** \brief Destructor for base SampleConsensusModel. */
      virtual ~SampleConsensusModel () {};

      /** \brief Get a set of random data samples and return them as point
        * indices. Pure virtual.  
        * \param[out] iterations the internal number of iterations used by SAC methods
        * \param[out] samples the resultant model samples
        */
      void 
      getSamples (int &iterations, std::vector<int> &samples)
      {
        // We're assuming that indices_ have already been set in the constructor
        if (indices_->size () < getSampleSize ())
        {
          PCL_ERROR ("[pcl::SampleConsensusModel::getSamples] Can not select %lu unique points out of %lu!\n",
                     (unsigned long)samples.size (), (unsigned long)indices_->size ());
          // one of these will make it stop :)
          samples.clear ();
          iterations = INT_MAX - 1;
          return;
        }

        // Get a second point which is different than the first
        samples.resize (getSampleSize());
        for (unsigned int iter = 0; iter < max_sample_checks_; ++iter)
        {
          // Choose the random indices
          SampleConsensusModel<PointT>::drawIndexSample (samples);

          // If it's a good sample, stop here
          if (isSampleGood (samples))
            return;
        }
        PCL_DEBUG ("[pcl::SampleConsensusModel::getSamples] WARNING: Could not select %d sample points in %d iterations!\n", getSampleSize (), max_sample_checks_);
        samples.clear ();
      }

      /** \brief Check whether the given index samples can form a valid model,
        * compute the model coefficients from these samples and store them
        * in model_coefficients. Pure virtual.
        * \param[in] samples the point indices found as possible good candidates
        * for creating a valid model 
        * \param[out] model_coefficients the computed model coefficients
        */
      virtual bool 
      computeModelCoefficients (const std::vector<int> &samples, 
                                Eigen::VectorXf &model_coefficients) = 0;

      /** \brief Recompute the model coefficients using the given inlier set
        * and return them to the user. Pure virtual.
        *
        * @note: these are the coefficients of the model after refinement
        * (e.g., after a least-squares optimization)
        *
        * \param[in] inliers the data inliers supporting the model
        * \param[in] model_coefficients the initial guess for the model coefficients
        * \param[out] optimized_coefficients the resultant recomputed coefficients after non-linear optimization
        */
      virtual void 
      optimizeModelCoefficients (const std::vector<int> &inliers, 
                                 const Eigen::VectorXf &model_coefficients,
                                 Eigen::VectorXf &optimized_coefficients) = 0;

      /** \brief Compute all distances from the cloud data to a given model. Pure virtual.
        * 
        * \param[in] model_coefficients the coefficients of a model that we need to compute distances to 
        * \param[out] distances the resultant estimated distances
        */
      virtual void 
      getDistancesToModel (const Eigen::VectorXf &model_coefficients, 
                           std::vector<double> &distances) = 0;

      /** \brief Select all the points which respect the given model
        * coefficients as inliers. Pure virtual.
        * 
        * \param[in] model_coefficients the coefficients of a model that we need to compute distances to
        * \param[in] threshold a maximum admissible distance threshold for determining the inliers from 
        * the outliers
        * \param[out] inliers the resultant model inliers
        */
      virtual void 
      selectWithinDistance (const Eigen::VectorXf &model_coefficients, 
                            const double threshold,
                            std::vector<int> &inliers) = 0;

      /** \brief Count all the points which respect the given model
        * coefficients as inliers. Pure virtual.
        * 
        * \param[in] model_coefficients the coefficients of a model that we need to
        * compute distances to
        * \param[in] threshold a maximum admissible distance threshold for
        * determining the inliers from the outliers
        * \return the resultant number of inliers
        */
      virtual int
      countWithinDistance (const Eigen::VectorXf &model_coefficients, 
                           const double threshold) = 0;

      /** \brief Create a new point cloud with inliers projected onto the model. Pure virtual.
        * \param[in] inliers the data inliers that we want to project on the model
        * \param[in] model_coefficients the coefficients of a model
        * \param[out] projected_points the resultant projected points
        * \param[in] copy_data_fields set to true (default) if we want the \a
        * projected_points cloud to be an exact copy of the input dataset minus
        * the point projections on the plane model
        */
      virtual void 
      projectPoints (const std::vector<int> &inliers, 
                     const Eigen::VectorXf &model_coefficients,
                     PointCloud &projected_points, 
                     bool copy_data_fields = true) = 0;

      /** \brief Verify whether a subset of indices verifies a given set of
        * model coefficients. Pure virtual.
        *
        * \param[in] indices the data indices that need to be tested against the model
        * \param[in] model_coefficients the set of model coefficients
        * \param[in] threshold a maximum admissible distance threshold for
        * determining the inliers from the outliers
        */
      virtual bool 
      doSamplesVerifyModel (const std::set<int> &indices, 
                            const Eigen::VectorXf &model_coefficients, 
                            const double threshold) = 0;

      /** \brief Provide a pointer to the input dataset
        * \param[in] cloud the const boost shared pointer to a PointCloud message
        */
      inline virtual void
      setInputCloud (const PointCloudConstPtr &cloud)
      {
        input_ = cloud;
        if (!indices_)
          indices_.reset (new std::vector<int> ());
        if (indices_->empty ())
        {
          // Prepare a set of indices to be used (entire cloud)
          indices_->resize (cloud->points.size ());
          for (size_t i = 0; i < cloud->points.size (); ++i) 
            (*indices_)[i] = (int) i;
        }
        shuffled_indices_ = *indices_;
      }

      /** \brief Get a pointer to the input point cloud dataset. */
      inline PointCloudConstPtr 
      getInputCloud () const { return (input_); }

      /** \brief Provide a pointer to the vector of indices that represents the input data.
        * \param[in] indices a pointer to the vector of indices that represents the input data.
        */
      inline void 
      setIndices (const boost::shared_ptr <std::vector<int> > &indices) 
      { 
        indices_ = indices; 
        shuffled_indices_ = *indices_;
      }

      /** \brief Provide the vector of indices that represents the input data.
        * \param[out] indices the vector of indices that represents the input data.
        */
      inline void 
      setIndices (const std::vector<int> &indices) 
      { 
        indices_.reset (new std::vector<int> (indices));
        shuffled_indices_ = indices;
      }

      /** \brief Get a pointer to the vector of indices used. */
      inline boost::shared_ptr <std::vector<int> > 
      getIndices () const { return (indices_); }

      /** \brief Return an unique id for each type of model employed. */
      virtual SacModel 
      getModelType () const = 0;

      /** \brief Return the size of a sample from which a model is computed */
      inline unsigned int 
      getSampleSize () const 
      { 
        std::map<pcl::SacModel, unsigned int>::const_iterator it = SAC_SAMPLE_SIZE.find (getModelType ());
        if (it == SAC_SAMPLE_SIZE.end ())
          throw InvalidSACModelTypeException ("No sample size defined for given model type!\n");
        return (it->second);
      }

      /** \brief Set the minimum and maximum allowable radius limits for the
        * model (applicable to models that estimate a radius)
        * \param[in] min_radius the minimum radius model
        * \param[in] max_radius the maximum radius model
        * \todo change this to set limits on the entire model
        */
      inline void
      setRadiusLimits (const double &min_radius, const double &max_radius)
      {
        radius_min_ = min_radius;
        radius_max_ = max_radius;
      }

      /** \brief Get the minimum and maximum allowable radius limits for the
        * model as set by the user.
        *
        * \param[out] min_radius the resultant minimum radius model
        * \param[out] max_radius the resultant maximum radius model
        */
      inline void
      getRadiusLimits (double &min_radius, double &max_radius)
      {
        min_radius = radius_min_;
        max_radius = radius_max_;
      }
#ifndef __ANDROID__
      friend class ProgressiveSampleConsensus<PointT>;
#endif
    protected:
      /** \brief Fills a sample array with random samples from the indices_ vector
        * \param[out] sample the set of indices of target_ to analyze
        */
      inline void
      drawIndexSample (std::vector<int> &sample)
      {
        size_t sample_size = sample.size ();
        size_t index_size = shuffled_indices_.size ();
        for (unsigned int i = 0; i < sample_size; ++i)
          // The 1/(RAND_MAX+1.0) trick is when the random numbers are not uniformly distributed and for small modulo
          // elements, that does not matter (and nowadays, random number generators are good)
          std::swap (shuffled_indices_[i], shuffled_indices_[i + (rand () % (index_size - i))]);
        std::copy (shuffled_indices_.begin (), shuffled_indices_.begin () + sample_size, sample.begin ());
      }

      /** \brief Check whether a model is valid given the user constraints.
        * \param[in] model_coefficients the set of model coefficients
        */
      virtual inline bool
      isModelValid (const Eigen::VectorXf &model_coefficients) = 0;

      /** \brief Check if a sample of indices results in a good sample of points
        * indices. Pure virtual.
        * \param[in] samples the resultant index samples
        */
      virtual bool
      isSampleGood (const std::vector<int> &samples) const = 0;

      /** \brief A boost shared pointer to the point cloud data array. */
      PointCloudConstPtr input_;

      /** \brief A pointer to the vector of point indices to use. */
      boost::shared_ptr <std::vector<int> > indices_;

      /** The maximum number of samples to try until we get a good one */
      static const unsigned int max_sample_checks_ = 1000;

      /** \brief The minimum and maximum radius limits for the model.
        * Applicable to all models that estimate a radius. 
        */
      double radius_min_, radius_max_;

      /** Data containing a shuffled version of the indices. This is used and modified when drawing samples. */
      std::vector<int> shuffled_indices_;
  };

  /** \brief @b SampleConsensusModelFromNormals represents the base model class
    * for models that require the use of surface normals for estimation.
    */
  template <typename PointT, typename PointNT>
  class SampleConsensusModelFromNormals
  {
    public:
      typedef typename pcl::PointCloud<PointNT>::ConstPtr PointCloudNConstPtr;
      typedef typename pcl::PointCloud<PointNT>::Ptr PointCloudNPtr;

      typedef boost::shared_ptr<SampleConsensusModelFromNormals> Ptr;
      typedef boost::shared_ptr<const SampleConsensusModelFromNormals> ConstPtr;

      /** \brief Empty constructor for base SampleConsensusModelFromNormals. */
      SampleConsensusModelFromNormals () : normal_distance_weight_ (0.0) {};

      /** \brief Set the normal angular distance weight.
        * \param[in] w the relative weight (between 0 and 1) to give to the angular
        * distance (0 to pi/2) between point normals and the plane normal.
        * (The Euclidean distance will have weight 1-w.)
        */
      inline void 
      setNormalDistanceWeight (const double w) 
      { 
        normal_distance_weight_ = w; 
      }

      /** \brief Get the normal angular distance weight. */
      inline double 
      getNormalDistanceWeight () { return (normal_distance_weight_); }

      /** \brief Provide a pointer to the input dataset that contains the point
        * normals of the XYZ dataset.
        *
        * \param[in] normals the const boost shared pointer to a PointCloud message
        */
      inline void 
      setInputNormals (const PointCloudNConstPtr &normals) 
      { 
        normals_ = normals; 
      }

      /** \brief Get a pointer to the normals of the input XYZ point cloud dataset. */
      inline PointCloudNConstPtr 
      getInputNormals () { return (normals_); }

    protected:
      /** \brief The relative weight (between 0 and 1) to give to the angular
        * distance (0 to pi/2) between point normals and the plane normal. 
        */
      double normal_distance_weight_;

      /** \brief A pointer to the input dataset that contains the point normals
        * of the XYZ dataset. 
        */
      PointCloudNConstPtr normals_;
  };

  /** Base functor all the models that need non linear optimization must
    * define their own one and implement operator() (const Eigen::VectorXd& x, Eigen::VectorXd& fvec)
    * or operator() (const Eigen::VectorXf& x, Eigen::VectorXf& fvec) dependening on the choosen _Scalar
    */
  template<typename _Scalar, int NX=Eigen::Dynamic, int NY=Eigen::Dynamic>
  struct Functor
  {
    typedef _Scalar Scalar;
    enum {
      InputsAtCompileTime = NX,
      ValuesAtCompileTime = NY
    };
    typedef Eigen::Matrix<Scalar,InputsAtCompileTime,1> InputType;
    typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,1> ValueType;
    typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,InputsAtCompileTime> JacobianType;
    
    const int m_inputs, m_values;
    
    Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
    Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}
  
    int inputs() const { return m_inputs; }
    int values() const { return m_values; }
  };
}

#endif  //#ifndef PCL_SAMPLE_CONSENSUS_MODEL_H_
