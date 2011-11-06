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
  *
  */

#ifndef PCL_SHOT_OMP_H_
#define PCL_SHOT_OMP_H_

#include <pcl/point_types.h>
#include <pcl/features/feature.h>
#include <pcl/features/shot.h>

namespace pcl
{
  /** \brief @b SHOTEstimation estimates the Signature of Histograms of OrienTations (SHOT) descriptor for a given point cloud dataset
    * containing points and normals, in parallel, using the OpenMP standard.
    *
    * @note If you use this code in any academic work, please cite:
    *
    * <ul>
    * <li> F. Tombari, S. Salti, L. Di Stefano
    *      Unique Signatures of Histograms for Local Surface Description.
    *      In Proceedings of the 11th European Conference on Computer Vision (ECCV),
    *      Heraklion, Greece, September 5-11 2010.
    * </li>
    * <li> F. Tombari, S. Salti, L. Di Stefano
    *      A Combined Texture-Shape Descriptor For Enhanced 3D Feature Matching.
    *      In Proceedings of the 18th International Conference on Image Processing (ICIP),
    *      Brussels, Belgium, September 11-14 2011.
    * </li>
    * </ul>
    *
    * \author Samuele Salti
    * \ingroup features
    */

  template <typename PointInT, typename PointNT, typename PointOutT> 
  class SHOTEstimationOMP : public SHOTEstimation<PointInT, PointNT, PointOutT>
  {
    public:
      using Feature<PointInT, PointOutT>::feature_name_;
      using Feature<PointInT, PointOutT>::getClassName;
      using Feature<PointInT, PointOutT>::indices_;
      using Feature<PointInT, PointOutT>::k_;
      using Feature<PointInT, PointOutT>::search_parameter_;
      using Feature<PointInT, PointOutT>::search_radius_;
      using Feature<PointInT, PointOutT>::surface_;
      using FeatureFromNormals<PointInT, PointNT, PointOutT>::normals_;
      using SHOTEstimation<PointInT, PointNT, PointOutT>::descLength_;
      using SHOTEstimation<PointInT, PointNT, PointOutT>::nr_grid_sector_;
      using SHOTEstimation<PointInT, PointNT, PointOutT>::nr_shape_bins_;
      using SHOTEstimation<PointInT, PointNT, PointOutT>::sqradius_;
      using SHOTEstimation<PointInT, PointNT, PointOutT>::radius3_4_;
      using SHOTEstimation<PointInT, PointNT, PointOutT>::radius1_4_;
      using SHOTEstimation<PointInT, PointNT, PointOutT>::radius1_2_;
      using SHOTEstimation<PointInT, PointNT, PointOutT>::rf_;

      typedef typename Feature<PointInT, PointOutT>::PointCloudOut PointCloudOut;
      typedef typename Feature<PointInT, PointOutT>::PointCloudIn PointCloudIn;

      /** \brief Empty constructor. */
      SHOTEstimationOMP (unsigned int nr_threads = - 1) : SHOTEstimation<PointInT, PointNT, PointOutT> ()
      {
        setNumberOfThreads (nr_threads);
      }

      /** \brief Initialize the scheduler and set the number of threads to use.
        * \param nr_threads the number of hardware threads to use (-1 sets the value back to automatic)
        */
      inline void
      setNumberOfThreads (unsigned int nr_threads)
      {
        if (nr_threads == 0)
          nr_threads = 1;
        threads_ = nr_threads;
      }

    protected:

      /** \brief Estimate the Signatures of Histograms of OrienTations (SHOT) descriptors at a set of points given by
        * <setInputCloud (), setIndices ()> using the surface in setSearchSurface () and the spatial locator in
        * setSearchMethod ()
        * \param output the resultant point cloud model dataset that contains the SHOT feature estimates
        */
      void 
      computeFeature (PointCloudOut &output);

      /** \brief The number of threads the scheduler should use. */
      int threads_;
  };


  template <typename PointNT, typename PointOutT> 
  class SHOTEstimationOMP<pcl::PointXYZRGBA, PointNT, PointOutT> : public SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>
  {
    public:
      using Feature<pcl::PointXYZRGBA, PointOutT>::feature_name_;
      using Feature<pcl::PointXYZRGBA, PointOutT>::getClassName;
      using Feature<pcl::PointXYZRGBA, PointOutT>::indices_;
      using Feature<pcl::PointXYZRGBA, PointOutT>::k_;
      using Feature<pcl::PointXYZRGBA, PointOutT>::search_parameter_;
      using Feature<pcl::PointXYZRGBA, PointOutT>::search_radius_;
      using Feature<pcl::PointXYZRGBA, PointOutT>::surface_;
      using FeatureFromNormals<pcl::PointXYZRGBA, PointNT, PointOutT>::normals_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::descLength_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::nr_grid_sector_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::nr_shape_bins_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::sqradius_;
       using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::radius3_4_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::radius1_4_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::radius1_2_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::rf_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::b_describe_shape_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::b_describe_color_;
      using SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT>::nr_color_bins_;

      typedef typename Feature<pcl::PointXYZRGBA, PointOutT>::PointCloudOut PointCloudOut;
      typedef typename Feature<pcl::PointXYZRGBA, PointOutT>::PointCloudIn PointCloudIn;

      /** \brief Empty constructor. */
      SHOTEstimationOMP (bool describeShape = true, 
                         bool describeColor = false, 
                         unsigned int nr_threads = - 1) 
        : SHOTEstimation<pcl::PointXYZRGBA, PointNT, PointOutT> (describeShape, describeColor)
      {
        setNumberOfThreads (nr_threads);
      }

      /** \brief Initialize the scheduler and set the number of threads to use.
        * \param nr_threads the number of hardware threads to use (-1 sets the value back to automatic)
        */
      inline void
      setNumberOfThreads (unsigned int nr_threads)
      {
        if (nr_threads == 0)
          nr_threads = 1;
        threads_ = nr_threads;
      }

    private:

      /** \brief Estimate the Signatures of Histograms of OrienTations (SHOT) descriptors at a set of points given by
        * <setInputCloud (), setIndices ()> using the surface in setSearchSurface () and the spatial locator in
        * setSearchMethod ()
        * \param output the resultant point cloud model dataset that contains the SHOT feature estimates
        */
      void 
      computeFeature (PointCloudOut &output);

      /** \brief The number of threads the scheduler should use. */
      int threads_;
  };
}

#endif  //#ifndef PCL_SHOT_OMP_H_


