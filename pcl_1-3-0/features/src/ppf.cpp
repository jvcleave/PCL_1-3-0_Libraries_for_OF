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
 * $Id: ppf.cpp 3022 2011-11-01 03:42:22Z rusu $
 */

#include "pcl/features/ppf.h"
#include "pcl/features/impl/ppf.hpp"
#include <pcl/point_types.h>
#include <pcl/impl/instantiate.hpp>

//////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::computePPFPairFeature (const Eigen::Vector4f &p1, const Eigen::Vector4f &n1,
                            const Eigen::Vector4f &p2, const Eigen::Vector4f &n2,
                            float &f1, float &f2, float &f3, float &f4)
{
  Eigen::Vector4f delta = p2 - p1;
  delta[3] = 0.0f;
  // f4 = ||delta||
  f4 = delta.norm ();

  delta /= f4;

  // f1 = n1 dot delta
  f1 = n1[0] * delta[0] + n1[1] * delta[1] + n1[2] * delta[2];
  // f2 = n2 dot delta
  f2 = n2[0] * delta[0] + n2[1] * delta[1] + n2[2] * delta[2];
  // f3 = n1 dot n2
  f3 = n1[0] * n2[0] + n1[1] * n2[1] + n1[2] * n2[2];

  return (true);
}

// Instantiations of specific point types
PCL_INSTANTIATE_PRODUCT(PPFEstimation, (PCL_XYZ_POINT_TYPES)(PCL_NORMAL_POINT_TYPES)((pcl::PPFSignature)));
