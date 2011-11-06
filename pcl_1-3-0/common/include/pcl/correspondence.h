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
 *  $Id: correspondence.h 3006 2011-10-31 23:25:06Z rusu $
 */
#ifndef PCL_COMMON_CORRESPONDENCE_H_
#define PCL_COMMON_CORRESPONDENCE_H_

#include <Eigen/Core>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace pcl
{
  /** @b Correspondence represents a match between two entities (e.g., points, descriptors, etc). This is 
    * represesented via the indices of a \a source point and a \a target point, and the distance between them.
    *
    * \author Dirk Holz, Radu B. Rusu
    * \ingroup common
    */
  struct Correspondence
  {
    /** \brief Index of the query (source) point. */
    int index_query;
    /** \brief Index of the matching (target) point. Set to -1 if no correspondence found. */
    int index_match;
    /** \brief Distance between query and matching point (w.r.t. the used feature descriptors) */
    float distance;
    
    /** \brief Standard constructor. 
      * Sets \ref index_query to 0, \ref index_match to -1, and \ref distance to FLT_MAX.
      */
    inline Correspondence () : index_query (0), index_match (-1), 
                               distance (std::numeric_limits<float>::max ())
    {}

    /** \brief Constructor. */
    inline Correspondence (int _index_query, int _index_match, float _distance) : 
      index_query (_index_query), index_match (_index_match), distance (_distance)
    {}
    
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  };
  
  /** \brief overloaded << operator */
  inline std::ostream& 
  operator << (std::ostream& os, const Correspondence& c)
  {
    os << c.index_query << " " << c.index_match << " " << c.distance;
    return (os);
  }

  typedef std::vector< pcl::Correspondence, Eigen::aligned_allocator<pcl::Correspondence> > Correspondences;
  typedef boost::shared_ptr<Correspondences> CorrespondencesPtr;
  typedef boost::shared_ptr<const Correspondences > CorrespondencesConstPtr;

  /**
    * \brief Get the query points of correspondences that are present in
    * one correspondence vector but not in the other, e.g., to compare
    * correspondences before and after rejection.
    * \param[in] correspondences_before Vector of correspondences before rejection
    * \param[in] correspondences_after Vector of correspondences after rejection
    * \param[out] indices Query point indices of correspondences that have been rejected
    * \param[in] presorting_required Enable/disable internal sorting of vectors.
    * By default (true), vectors are internally sorted before determining their difference.
    * If the order of correspondences in \a correspondences_after is not different (has not been changed)
    * from the order in \b correspondences_before this pre-processing step can be disabled
    * in order to gain efficiency. In order to disable pre-sorting set \a presorting_requered to false.
    */
  void
  getRejectedQueryIndices (const pcl::Correspondences &correspondences_before,
                           const pcl::Correspondences &correspondences_after,
                           std::vector<int>& indices,
                           bool presorting_required = true);



}

#endif /* PCL_COMMON_CORRESPONDENCE_H_ */
