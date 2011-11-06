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
 * $Id: filter.h 3028 2011-11-01 04:12:17Z rusu $
 *
 */

#ifndef PCL_FILTER_H_
#define PCL_FILTER_H_

#include "pcl/pcl_base.h"
#include "pcl/ros/conversions.h"
#include <boost/make_shared.hpp>
#include <cfloat>

namespace pcl
{
  /** \brief Removes points with x, y, or z equal to NaN
    * \param cloud_in the input point cloud
    * \param cloud_out the input point cloud
    * \param index the mapping (ordered): cloud_out.points[i] = cloud_in.points[index[i]]
    * \note The density of the point cloud is lost.
    * \note Can be called with cloud_in == cloud_out
    * \ingroup filters
    */
  template<typename PointT> void
  removeNaNFromPointCloud (const pcl::PointCloud<PointT> &cloud_in, pcl::PointCloud<PointT> &cloud_out, std::vector<int> &index);

  ////////////////////////////////////////////////////////////////////////////////////////////
  /** \brief @b Filter represents the base filter class. Some generic 3D operations that are applicable to all filters
    * are defined here as static methods.
    * \author Radu Bogdan Rusu
    * \ingroup filters
    */
  template<typename PointT>
  class Filter : public PCLBase<PointT>
  {
    public:
      using PCLBase<PointT>::indices_;
      using PCLBase<PointT>::input_;

      typedef boost::shared_ptr< Filter<PointT> > Ptr;
      typedef boost::shared_ptr< const Filter<PointT> > ConstPtr;

      typedef pcl::PointCloud<PointT> PointCloud;
      typedef typename PointCloud::Ptr PointCloudPtr;
      typedef typename PointCloud::ConstPtr PointCloudConstPtr;

      /** \brief Empty constructor. */
      Filter (bool extract_removed_indices = false) : 
        filter_field_name_ (""), 
        filter_limit_min_ (-FLT_MAX), filter_limit_max_ (FLT_MAX),
        filter_limit_negative_ (false), extract_removed_indices_ (extract_removed_indices)

      {
        removed_indices_ = boost::make_shared<std::vector<int> > ();
      }

      /** \brief Get the point indices being removed */
      inline IndicesConstPtr const
      getRemovedIndices ()
      {
        return (removed_indices_);
      }

      /** \brief Provide the name of the field to be used for filtering data. In conjunction with  \a setFilterLimits,
        * points having values outside this interval will be discarded.
        * \param field_name the name of the field that contains values used for filtering
        */
      inline void
      setFilterFieldName (const std::string &field_name)
      {
        filter_field_name_ = field_name;
      }

      /** \brief Get the name of the field used for filtering. */
      inline std::string const
      getFilterFieldName ()
      {
        return (filter_field_name_);
      }

      /** \brief Set the field filter limits. All points having field values outside this interval will be discarded.
        * \param limit_min the minimum allowed field value
        * \param limit_max the maximum allowed field value
        */
      inline void
      setFilterLimits (const double &limit_min, const double &limit_max)
      {
        filter_limit_min_ = limit_min;
        filter_limit_max_ = limit_max;
      }

      /** \brief Get the field filter limits (min/max) set by the user. The default values are -FLT_MAX, FLT_MAX. */
      inline void
      getFilterLimits (double &limit_min, double &limit_max)
      {
        limit_min = filter_limit_min_;
        limit_max = filter_limit_max_;
      }

      /** \brief Set to true if we want to return the data outside the interval specified by setFilterLimits (min, max).
        * Default: false.
        * \param limit_negative return data inside the interval (false) or outside (true)
        */
      inline void
      setFilterLimitsNegative (const bool limit_negative)
      {
        filter_limit_negative_ = limit_negative;
      }

      /** \brief Get whether the data outside the interval (min/max) is to be returned (true) or inside (false). */
      inline void
      getFilterLimitsNegative (bool &limit_negative)
      {
        limit_negative = filter_limit_negative_;
      }
      inline bool
      getFilterLimitsNegative ()
      {
        return (filter_limit_negative_);
      }

      /** \brief Calls the filtering method and returns the filtered dataset in output.
        * \param output the resultant filtered point cloud dataset
        */
      inline void
      filter (PointCloud &output)
      {
        if (!initCompute ())
          return;

        // Resize the output dataset
        //if (output.points.size () != indices_->size ())
        //  output.points.resize (indices_->size ());

        // Copy header at a minimum
        output.header = input_->header;
        output.sensor_origin_ = input_->sensor_origin_;
        output.sensor_orientation_ = input_->sensor_orientation_;

        // Apply the actual filter
        applyFilter (output);

        deinitCompute ();
      }

    protected:

      using PCLBase<PointT>::initCompute;
      using PCLBase<PointT>::deinitCompute;

      /** \brief Indices of the points that are removed */
      IndicesPtr removed_indices_;

      /** \brief The filter name. */
      std::string filter_name_;

      /** \brief The desired user filter field name. */
      std::string filter_field_name_;

      /** \brief The minimum allowed filter value a point will be considered from. */
      double filter_limit_min_;

      /** \brief The maximum allowed filter value a point will be considered from. */
      double filter_limit_max_;

      /** \brief Set to true if we want to return the data outside (\a filter_limit_min_;\a filter_limit_max_). Default: false. */
      bool filter_limit_negative_;

      /** \brief Set to true if we want to return the indices of the removed points. */
      bool extract_removed_indices_;

      /** \brief Abstract filter method. 
        *
        * The implementation needs to set output.{points, width, height, is_dense}.
        */
      virtual void
      applyFilter (PointCloud &output) = 0;

      /** \brief Get a string representation of the name of this class. */
      inline const std::string&
      getClassName () const
      {
        return (filter_name_);
      }
  };

  ////////////////////////////////////////////////////////////////////////////////////////////
  /** \brief @b Filter represents the base filter class. Some generic 3D operations that are applicable to all filters
    * are defined here as static methods.
    * \author Radu Bogdan Rusu
    * \ingroup filters
    */
  template<>
  class PCL_EXPORTS Filter<sensor_msgs::PointCloud2> : public PCLBase<sensor_msgs::PointCloud2>
  {
    public:
      typedef sensor_msgs::PointCloud2 PointCloud2;
      typedef PointCloud2::Ptr PointCloud2Ptr;
      typedef PointCloud2::ConstPtr PointCloud2ConstPtr;

      /** \brief Empty constructor. */
      Filter (bool extract_removed_indices = false) :
        filter_field_name_ (""), filter_limit_min_ (-FLT_MAX), filter_limit_max_ (FLT_MAX),
            filter_limit_negative_ (false), extract_removed_indices_ (extract_removed_indices)
      {
        removed_indices_ = boost::make_shared<std::vector<int> > ();
      }

      /** \brief Get the point indices being removed */
      inline IndicesConstPtr const
      getRemovedIndices ()
      {
        return (removed_indices_);
      }

      /** \brief Provide the name of the field to be used for filtering data. In conjunction with  \a setFilterLimits,
        * points having values outside this interval will be discarded.
        * \param field_name the name of the field that contains values used for filtering
        */
      inline void
      setFilterFieldName (const std::string &field_name)
      {
        filter_field_name_ = field_name;
      }

      /** \brief Get the name of the field used for filtering. */
      inline std::string const
      getFilterFieldName ()
      {
        return (filter_field_name_);
      }

      /** \brief Set the field filter limits. All points having field values outside this interval will be discarded.
        * \param limit_min the minimum allowed field value
        * \param limit_max the maximum allowed field value
        */
      inline void
      setFilterLimits (const double &limit_min, const double &limit_max)
      {
        filter_limit_min_ = limit_min;
        filter_limit_max_ = limit_max;
      }

      /** \brief Get the field filter limits (min/max) set by the user. The default values are -FLT_MAX, FLT_MAX. */
      inline void
      getFilterLimits (double &limit_min, double &limit_max)
      {
        limit_min = filter_limit_min_;
        limit_max = filter_limit_max_;
      }

      /** \brief Set to true if we want to return the data outside the interval specified by setFilterLimits (min, max).
        * Default: false.
        * \param limit_negative return data inside the interval (false) or outside (true)
        */
      inline void
      setFilterLimitsNegative (const bool limit_negative)
      {
        filter_limit_negative_ = limit_negative;
      }

      /** \brief Get whether the data outside the interval (min/max) is to be returned (true) or inside (false). */
      inline void
      getFilterLimitsNegative (bool &limit_negative)
      {
        limit_negative = filter_limit_negative_;
      }
      inline bool
      getFilterLimitsNegative ()
      {
        return (filter_limit_negative_);
      }

      /** \brief Calls the filtering method and returns the filtered dataset in output.
        * \param output the resultant filtered point cloud dataset
        */
      void
      filter (PointCloud2 &output);

    protected:

      /** \brief Indices of the points that are removed */
      IndicesPtr removed_indices_;

      /** \brief The filter name. */
      std::string filter_name_;

      /** \brief The desired user filter field name. */
      std::string filter_field_name_;

      /** \brief The minimum allowed filter value a point will be considered from. */
      double filter_limit_min_;

      /** \brief The maximum allowed filter value a point will be considered from. */
      double filter_limit_max_;

      /** \brief Set to true if we want to return the data outside (\a filter_limit_min_;\a filter_limit_max_). Default: false. */
      bool filter_limit_negative_;

      /** \brief Set to true if we want to return the indices of the removed points. */
      bool extract_removed_indices_;

      /** \brief Abstract filter method.
       *
       * The implementation needs to set output.{data, row_step, point_step, width, height, is_dense}.
       */
      virtual void
      applyFilter (PointCloud2 &output) = 0;

      /** \brief Get a string representation of the name of this class. */
      inline const std::string&
      getClassName () const
      {
        return (filter_name_);
      }
  };
}

#endif  //#ifndef PCL_FILTER_H_
