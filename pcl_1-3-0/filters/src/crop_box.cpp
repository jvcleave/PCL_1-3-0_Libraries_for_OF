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
 * $Id: extract_indices.cpp 1385 2011-06-19 19:15:56Z rusu $
 *
 */

#include "pcl/impl/instantiate.hpp"
#include "pcl/point_types.h"
#include "pcl/filters/crop_box.h"
#include "pcl/filters/impl/crop_box.hpp"


///////////////////////////////////////////////////////////////////////////////
void
pcl::CropBox<sensor_msgs::PointCloud2>::applyFilter (PointCloud2 &output)
{
  // Resize output cloud to sample size
  output.data.resize (input_->data.size ());

  // Copy the common fields
  output.fields = input_->fields;
  output.is_bigendian = input_->is_bigendian;
  output.row_step = input_->row_step;
  output.point_step = input_->point_step;
  output.height = 1;

  int indice_count = 0;

  Eigen::Affine3f transform = Eigen::Affine3f::Identity();
  Eigen::Affine3f inverse_transform = Eigen::Affine3f::Identity();

  if (rotation_ != Eigen::Vector3f::Zero ())
  {
    pcl::getTransformation (0, 0, 0,
                            rotation_ (0), rotation_ (1), rotation_ (2),
                            transform);
    inverse_transform = transform.inverse();
  }

  //PointXYZ local_pt;
  Eigen::Vector3f local_pt (Eigen::Vector3f::Zero ());

  for (size_t index = 0; index < indices_->size (); ++index)
  {
    // Get local point
    int point_offset = ((*indices_)[index] * input_->point_step);
    int offset = point_offset + input_->fields[x_idx_].offset;
    memcpy (&local_pt, &input_->data[offset], sizeof (float)*3);

    // Transform point to world space
    if (!(transform_.matrix().isIdentity()))
      local_pt = transform_ * local_pt;

    if (translation_ != Eigen::Vector3f::Zero ())
    {
      local_pt.x () = local_pt.x () - translation_ (0);
      local_pt.y () = local_pt.y () - translation_ (1);
      local_pt.z () = local_pt.z () - translation_ (2);
    }

    // Transform point to local space of crop box
    if (!(inverse_transform.matrix ().isIdentity ()))
      local_pt = inverse_transform * local_pt;

    if (local_pt.x () < min_pt_[0] || local_pt.y () < min_pt_[1] || local_pt.z () < min_pt_[2])
      continue;
    if (local_pt.x () > max_pt_[0] || local_pt.y () > max_pt_[1] || local_pt.z () > max_pt_[2])
      continue;

    memcpy (&output.data[indice_count++ * output.point_step],
            &input_->data[index * output.point_step], output.point_step);
  }

  output.width = indice_count;
  output.row_step = output.point_step * output.width;
  output.data.resize (output.width * output.height * output.point_step);
}

///////////////////////////////////////////////////////////////////////////////
void
pcl::CropBox<sensor_msgs::PointCloud2>::applyFilter (std::vector<int> &indices)
{
  indices.resize (input_->width * input_->height);
  int indice_count = 0;

  Eigen::Affine3f transform = Eigen::Affine3f::Identity();
  Eigen::Affine3f inverse_transform = Eigen::Affine3f::Identity();

  if (rotation_ != Eigen::Vector3f::Zero ())
  {
    pcl::getTransformation (0, 0, 0,
                            rotation_ (0), rotation_ (1), rotation_ (2),
                            transform);
    inverse_transform = transform.inverse();
  }

  //PointXYZ local_pt;
  Eigen::Vector3f local_pt (Eigen::Vector3f::Zero ());

  for (size_t index = 0; index < indices_->size (); index++)
  {
    // Get local point
    int point_offset = ((*indices_)[index] * input_->point_step);
    int offset = point_offset + input_->fields[x_idx_].offset;
    memcpy (&local_pt, &input_->data[offset], sizeof (float)*3);

    // Transform point to world space
    if (!(transform_.matrix().isIdentity()))
      local_pt = transform_ * local_pt;

    if (translation_ != Eigen::Vector3f::Zero ())
    {
      local_pt.x () -= translation_ (0);
      local_pt.y () -= translation_ (1);
      local_pt.z () -= translation_ (2);
    }

    // Transform point to local space of crop box
    if (!(inverse_transform.matrix().isIdentity()))
      local_pt = inverse_transform * local_pt;

    if (local_pt.x () < min_pt_[0] || local_pt.y () < min_pt_[1] || local_pt.z () < min_pt_[2])
      continue;
    if (local_pt.x () > max_pt_[0] || local_pt.y () > max_pt_[1] || local_pt.z () > max_pt_[2])
      continue;

    indices[indice_count++] = (*indices_)[index];
  }

  indices.resize (indice_count);
}

PCL_INSTANTIATE(CropBox, PCL_XYZ_POINT_TYPES);
