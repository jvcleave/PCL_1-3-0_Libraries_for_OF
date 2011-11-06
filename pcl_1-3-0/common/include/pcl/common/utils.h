/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, www.pointclouds.org
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
 * $Id: utils.h 2572 2011-09-22 21:16:43Z cecciameo $
 */

#ifndef PCL_UTILS
#define PCL_UTILS

namespace pcl
{
  namespace utils
  {
    namespace details
    {
      template<typename T> struct epsilon;
    
      template<> struct epsilon<float> { static const float value; };

      template<> struct epsilon<double> { static const double value; };
    };
    /**
      * \brief Check if val1 and val2 are equals to an epsilon extent
      *
      * \param val1 first number to check.
      * \param val2 second number to check.
      * \return true if val1 is equal to val2, false otherwise.
      */
    template<typename T>
    bool equal (T val1, T val2, T eps = details::epsilon<T>::value)
    {
      return (fabs (val1 - val2) < eps);
    }
  };
}


float const pcl::utils::details::epsilon<float>::value = 1E-8f;
double const pcl::utils::details::epsilon<double>::value = 1E-15;

#endif
