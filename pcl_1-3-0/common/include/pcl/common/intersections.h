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
 * $Id: intersections.h 1370 2011-06-19 01:06:01Z jspricke $
 *
 */
#ifndef PCL_INTERSECTIONS_H_
#define PCL_INTERSECTIONS_H_

#include <pcl/ModelCoefficients.h>
#include <pcl/common/common.h>
#include <pcl/common/distances.h>

/** 
  * \file pcl/common/intersections.h
  * Define line with line intersection functions
  * \ingroup common
  */

/*@{*/
namespace pcl
{
  /** \brief Get the intersection of a two 3D lines in space as a 3D point
    * \param line_a the coefficients of the first line (point, direction)
    * \param line_b the coefficients of the second line (point, direction)
    * \param point holder for the computed 3D point
    * \param sqr_eps maximum allowable squared distance to the true solution
    * \ingroup common
    */
  PCL_EXPORTS bool
  lineWithLineIntersection (const Eigen::VectorXf &line_a, 
                            const Eigen::VectorXf &line_b, 
                            Eigen::Vector4f &point, double sqr_eps = 1e-4);

  /** \brief Get the intersection of a two 3D lines in space as a 3D point
    * \param line_a the coefficients of the first line (point, direction)
    * \param line_b the coefficients of the second line (point, direction)
    * \param point holder for the computed 3D point
    * \param sqr_eps maximum allowable squared distance to the true solution
    * \ingroup common
    */
  PCL_EXPORTS bool
  lineWithLineIntersection (const pcl::ModelCoefficients &line_a, 
                            const pcl::ModelCoefficients &line_b, 
                            Eigen::Vector4f &point, double sqr_eps = 1e-4);
}
/*@}*/
#endif  //#ifndef PCL_INTERSECTIONS_H_

