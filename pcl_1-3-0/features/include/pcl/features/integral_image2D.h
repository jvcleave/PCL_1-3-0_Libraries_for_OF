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
 * $Id: integral_image2D.h 3023 2011-11-01 03:42:32Z svn $
 */

#ifndef PCL_INTEGRAL_IMAGE2D_H_
#define PCL_INTEGRAL_IMAGE2D_H_

#include <vector>

namespace pcl
{
  template <typename DataType>
  struct IntegralImageTypeTraits
  {
    typedef DataType Type;
    typedef DataType IntegralType;
  };
  
  template <>
  struct IntegralImageTypeTraits<float>
  {
    typedef float Type;
    typedef double IntegralType;
  };
  
  template <>
  struct IntegralImageTypeTraits<char>
  {
    typedef char Type;
    typedef int IntegralType;
  };

  template <>
  struct IntegralImageTypeTraits<short>
  {
    typedef short Type;
    typedef long IntegralType;
  };

  template <>
  struct IntegralImageTypeTraits<unsigned short>
  {
    typedef unsigned short Type;
    typedef unsigned long IntegralType;
  };
  
  template <>
  struct IntegralImageTypeTraits<unsigned char>
  {
    typedef unsigned char Type;
    typedef unsigned int IntegralType;
  };
  
  template <>
  struct IntegralImageTypeTraits<int>
  {
    typedef int Type;
    typedef long IntegralType;
  };
  
  template <>
  struct IntegralImageTypeTraits<unsigned int>
  {
    typedef unsigned int Type;
    typedef unsigned long IntegralType;
  };

  /** \brief Determines an integral image representation for a given organized data array
    * \author Suat Gedikli
    */
  template <class DataType, unsigned Dimension>
  class IntegralImage2Dim
  {
    public:
      static const unsigned second_order_size = (Dimension * (Dimension + 1)) >> 1;
      typedef Eigen::Matrix<typename IntegralImageTypeTraits<DataType>::IntegralType, Dimension, 1> ElementType;
      typedef Eigen::Matrix<typename IntegralImageTypeTraits<DataType>::IntegralType, second_order_size, 1> SecondOrderType;
      
      /** \brief Constructor for an Integral Image 
        * \param[in] compute_second_order_integral_images set to true if we want to compute a second order image
        */
      IntegralImage2Dim (bool compute_second_order_integral_images)
        : width_ (1), height_ (1), compute_second_order_integral_images_ (compute_second_order_integral_images)
      {
      }
      
      /** \brief Destructor */
      virtual 
      ~IntegralImage2Dim () { }
      
      /** \brief Set the input data to compute the integral image for
        * \param[in] data the input data
        * \param[in] width the width of the data
        * \param[in] height the height of the data
        * \param[in] element_stride the element stride of the data
        * \param[in] row_stride the row stride of the data
        */
      void 
      setInput (const DataType * data, 
                unsigned width, unsigned height, unsigned element_stride, unsigned row_stride);
      
      /** \brief Compute the first order sum
        * \param[in] start_x
        * \param[in] start_y
        * \param[in] width
        * \param[in] height
        */
      inline ElementType 
      getFirstOrderSum (unsigned start_x, unsigned start_y, unsigned width, unsigned height) const;

      /** \brief Compute the second order sum
        * \param[in] start_x
        * \param[in] start_y
        * \param[in] width
        * \param[in] height
        */
      inline SecondOrderType 
      getSecondOrderSum (unsigned start_x, unsigned start_y, unsigned width, unsigned height) const;
    
    private:
      typedef Eigen::Matrix<typename IntegralImageTypeTraits<DataType>::Type, Dimension, 1> InputType;

      /** \brief Compute the actual integral image data
        * \param[in] data the input data
        * \param[in] element_stride the element stride of the data
        * \param[in] row_stride the row stride of the data
        */ 
      void 
      computeIntegralImages (const DataType * data, unsigned row_stride, unsigned element_stride);
      
      std::vector<ElementType, Eigen::aligned_allocator<ElementType> > first_order_integral_image_;
      std::vector<SecondOrderType, Eigen::aligned_allocator<SecondOrderType> > second_order_integral_image_;
      
      /** \brief The width of the 2d input data array */
      unsigned width_;
      /** \brief The height of the 2d input data array */
      unsigned height_;
      
      /** \brief Indicates whether second order integral images are available **/
      bool compute_second_order_integral_images_;
   };
 }

#include <pcl/features/impl/integral_image2D.hpp>

#endif    // PCL_INTEGRAL_IMAGE2D_H_

