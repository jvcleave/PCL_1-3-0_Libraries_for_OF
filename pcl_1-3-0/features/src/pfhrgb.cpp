/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Alexandru-Eugen Ichim
 *                      Willow Garage, Inc
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
 *  $Id: pfhrgb.cpp 3023 2011-11-01 03:42:32Z svn $
 */

#include "pcl/point_types.h"
#include "pcl/impl/instantiate.hpp"
#include "pcl/features/pfhrgb.h"
#include "pcl/features/impl/pfhrgb.hpp"


bool
pcl::computeRGBPairFeatures (const Eigen::Vector4f &p1, const Eigen::Vector4f &n1, const Eigen::Vector4i &colors1,
                             const Eigen::Vector4f &p2, const Eigen::Vector4f &n2, const Eigen::Vector4i &colors2,
                             float &f1, float &f2, float &f3, float &f4, float &f5, float &f6, float &f7)
{
  Eigen::Vector4f dp2p1 = p2 - p1;
  dp2p1[3] = 0.0f;
  f4 = dp2p1.norm ();

  if (f4 == 0.0f)
  {
    PCL_ERROR ("Euclidean distance between points is 0!\n");
    f1 = f2 = f3 = f4 = 0.0f;
    return (false);
  }

  Eigen::Vector4f n1_copy = n1,
      n2_copy = n2;
  n1_copy[3] = n2_copy[3] = 0.0f;
  float angle1 = n1_copy.dot (dp2p1) / f4;

  f3 = angle1;

  // Create a Darboux frame coordinate system u-v-w
  // u = n1; v = (p_idx - q_idx) x u / || (p_idx - q_idx) x u ||; w = u x v
  Eigen::Vector4f v = dp2p1.cross3 (n1_copy);
  v[3] = 0.0f;
  float v_norm = v.norm ();
  if (v_norm == 0.0f)
  {
    PCL_ERROR ("Norm of Delta x U is 0!\n");
    f1 = f2 = f3 = f4 = 0.0f;
    return (false);
  }
  // Normalize v
  v /= v_norm;

  Eigen::Vector4f w = n1_copy.cross3 (v);
  // Do not have to normalize w - it is a unit vector by construction

  v[3] = 0.0f;
  f2 = v.dot (n2_copy);
  w[3] = 0.0f;
  // Compute f1 = arctan (w * n2, u * n2) i.e. angle of n2 in the x=u, y=w coordinate system
  f1 = atan2f (w.dot (n2_copy), n1_copy.dot (n2_copy)); // @todo optimize this



  // everything before was standard 4D-Darboux frame feature pair
  // now, for the experimental color stuff
  f5 = ((float) colors1[0]) / colors2[0];
  f6 = ((float) colors1[1]) / colors2[1];
  f7 = ((float) colors1[2]) / colors2[2];

  // make sure the ratios are in the [-1, 1] interval
  if (f5 > 1.0f) f5 = - 1.0f / f5;
  if (f6 > 1.0f) f6 = - 1.0f / f6;
  if (f7 > 1.0f) f7 = - 1.0f / f7;

  return (true);
}

// Instantiations of specific point types
PCL_INSTANTIATE_PRODUCT(PFHRGBEstimation, ((pcl::PointXYZRGB) (pcl::PointXYZRGBNormal))
                        (PCL_NORMAL_POINT_TYPES)
                        ((pcl::PFHRGBSignature250)));
