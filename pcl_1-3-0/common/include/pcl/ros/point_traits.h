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
 * $Id: point_traits.h 1380 2011-06-19 11:34:29Z bouffa $
 *
 */

#ifndef PCL_ROS_POINT_TRAITS_H_
#define PCL_ROS_POINT_TRAITS_H_

#include <std_msgs/Header.h>
#include <sensor_msgs/PointField.h>
#include <boost/type_traits/remove_all_extents.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/assert.hpp>

namespace pcl 
{

  namespace fields
  {
    // Tag types get put in this namespace
  }

  namespace traits
  {
    // Metafunction to return enum value representing a type
    template<typename T> struct asEnum {};
    template<> struct asEnum<int8_t>   { static const uint8_t value = sensor_msgs::PointField::INT8;    };
    template<> struct asEnum<uint8_t>  { static const uint8_t value = sensor_msgs::PointField::UINT8;   };
    template<> struct asEnum<int16_t>  { static const uint8_t value = sensor_msgs::PointField::INT16;   };
    template<> struct asEnum<uint16_t> { static const uint8_t value = sensor_msgs::PointField::UINT16;  };
    template<> struct asEnum<int32_t>  { static const uint8_t value = sensor_msgs::PointField::INT32;   };
    template<> struct asEnum<uint32_t> { static const uint8_t value = sensor_msgs::PointField::UINT32;  };
    template<> struct asEnum<float>    { static const uint8_t value = sensor_msgs::PointField::FLOAT32; };
    template<> struct asEnum<double>   { static const uint8_t value = sensor_msgs::PointField::FLOAT64; };

    // Metafunction to return type of enum value 
    template<int> struct asType {};
    template<> struct asType<sensor_msgs::PointField::INT8>    { typedef int8_t   type; };
    template<> struct asType<sensor_msgs::PointField::UINT8>   { typedef uint8_t  type; };
    template<> struct asType<sensor_msgs::PointField::INT16>   { typedef int16_t  type; };
    template<> struct asType<sensor_msgs::PointField::UINT16>  { typedef uint16_t type; };
    template<> struct asType<sensor_msgs::PointField::INT32>   { typedef int32_t  type; };
    template<> struct asType<sensor_msgs::PointField::UINT32>  { typedef uint32_t type; };
    template<> struct asType<sensor_msgs::PointField::FLOAT32> { typedef float    type; };
    template<> struct asType<sensor_msgs::PointField::FLOAT64> { typedef double   type; };

    // Metafunction to decompose a type (possibly of array of any number of dimensions) into
    // its scalar type and total number of elements.
    template<typename T> struct decomposeArray
    {
      typedef typename boost::remove_all_extents<T>::type type;
      static const uint32_t value = sizeof(T) / sizeof(type);
    };

    // For non-POD point types, this is specialized to return the corresponding POD type.
    template<typename PointT> struct POD
    {
      typedef PointT type;
    };

    // name
    /* This really only depends on Tag, but we go through some gymnastics to avoid ODR violations.
       We template it on the point type PointT to avoid ODR violations when registering multiple
       point types with shared tags.
       The dummy parameter is so we can partially specialize name on PointT and Tag but leave it
       templated on dummy. Each specialization declares a static char array containing the tag
       name. The definition of the static member would conflict when linking multiple translation
       units that include the point type registration. But when the static member definition is
       templated (on dummy), we sidestep the ODR issue.
    */
    template<class PointT, typename Tag, int dummy = 0>
    struct name : name<typename POD<PointT>::type, Tag, dummy>
    {
      // Contents of specialization:
      // static const char value[];
      
      // Avoid infinite compile-time recursion
      BOOST_MPL_ASSERT_MSG((!boost::is_same<PointT, typename POD<PointT>::type>::value),
                           POINT_TYPE_NOT_PROPERLY_REGISTERED, (PointT&));
    };

    // offset
    template<class PointT, typename Tag>
    struct offset : offset<typename POD<PointT>::type, Tag>
    {
      // Contents of specialization:
      // static const size_t value;
      
      // Avoid infinite compile-time recursion
      BOOST_MPL_ASSERT_MSG((!boost::is_same<PointT, typename POD<PointT>::type>::value),
                           POINT_TYPE_NOT_PROPERLY_REGISTERED, (PointT&));
    };

    // datatype
    template<class PointT, typename Tag>
    struct datatype : datatype<typename POD<PointT>::type, Tag>
    {
      // Contents of specialization:
      // typedef ... type;
      // static const uint8_t value;
      // static const uint32_t size;
      
      // Avoid infinite compile-time recursion
      BOOST_MPL_ASSERT_MSG((!boost::is_same<PointT, typename POD<PointT>::type>::value),
                           POINT_TYPE_NOT_PROPERLY_REGISTERED, (PointT&));
    };

    // fields
    template<typename PointT>
    struct fieldList : fieldList<typename POD<PointT>::type>
    {
      // Contents of specialization:
      // typedef boost::mpl::vector<...> type;
      
      // Avoid infinite compile-time recursion
      BOOST_MPL_ASSERT_MSG((!boost::is_same<PointT, typename POD<PointT>::type>::value),
                           POINT_TYPE_NOT_PROPERLY_REGISTERED, (PointT&));
    };

  } //namespace traits
}

#endif  //#ifndef PCL_ROS_POINT_TRAITS_H_
