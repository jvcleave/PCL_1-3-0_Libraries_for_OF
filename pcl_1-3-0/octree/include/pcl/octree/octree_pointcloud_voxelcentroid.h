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
 * $Id: octree_pointcloud_voxelcentroid.h 3017 2011-11-01 03:24:04Z rusu $
 */

#ifndef OCTREE_VOXELCENTROID_H
#define OCTREE_VOXELCENTROID_H

#include "octree_pointcloud.h"

#include "octree_base.h"
#include "octree2buf_base.h"

namespace pcl
{
  namespace octree
  {

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /** \brief @b Octree pointcloud voxel centroid class
     *  \note This class generate an octrees from a point cloud (zero-copy). It provides a vector of centroids for all occupied voxels. .
     *  \note The octree pointcloud is initialized with its voxel resolution. Its bounding box is automatically adjusted or can be predefined.
     *  \note
     *  \note typename: PointT: type of point used in pointcloud
     *  \ingroup octree
     *  \author Julius Kammerl (julius@kammerl.de)
     */
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT = OctreeLeafDataTVector<int> , typename OctreeT = OctreeBase<int, LeafT> >
      class OctreePointCloudVoxelCentroid : public OctreePointCloud<PointT, LeafT, OctreeT>
      {

      public:
        // public typedefs for single/double buffering
        typedef OctreePointCloudVoxelCentroid<PointT, LeafT, OctreeBase<int, LeafT> > SingleBuffer;
        typedef OctreePointCloudVoxelCentroid<PointT, LeafT, Octree2BufBase<int, LeafT> > DoubleBuffer;

        /** \brief OctreePointCloudVoxelCentroids class constructor.
         *  \param resolution_arg:  octree resolution at lowest octree level
         * */
        OctreePointCloudVoxelCentroid (const double resolution_arg) :
          OctreePointCloud<PointT, LeafT, OctreeT> (resolution_arg)
        {
        }

        /** \brief Empty class deconstructor. */
        virtual
        ~OctreePointCloudVoxelCentroid ()
        {
        }

        /** \brief Get PointT vector of centroids for all occupied voxels.
         * \param voxelCentroidList_arg: results are written to this vector of PointT elements
         * \return number of occupied voxels
         */
        unsigned int
        getVoxelCentroids (std::vector<PointT, Eigen::aligned_allocator<PointT> > &voxelCentroidList_arg)
        {

          size_t i;
          unsigned int pointCounter;
          typename OctreePointCloud<PointT, LeafT, OctreeT>::OctreeKey keyC, keyP;
          PointT meanPoint;
          PointT idxPoint;

          std::vector<int> indicesVector;

          voxelCentroidList_arg.clear();
          voxelCentroidList_arg.reserve(this->leafCount_);

          // serialize leafs - this returns a list of point indices. Points indices from the same voxel are locates next to each other within this vector.
          this->serializeLeafs (indicesVector);

          // initializing
          keyP.x = keyP.y = keyP.z = std::numeric_limits<unsigned int>::max ();
          meanPoint.x = meanPoint.y = meanPoint.z = 0.0;
          pointCounter = 0;

          // iterate over all point indices
          for (i = 0; i < indicesVector.size (); i++)
          {
            idxPoint = this->input_->points[indicesVector[i]];

            // get octree key for point (key specifies octree voxel)
            this->genOctreeKeyforPoint (idxPoint, keyC);

            if (keyC == keyP)
            {
              // key addresses same voxel - add point
              meanPoint.x += idxPoint.x;
              meanPoint.y += idxPoint.y;
              meanPoint.z += idxPoint.z;

              pointCounter++;
            }
            else
            {
              // voxel key did change - calculate centroid and push it to result vector
              if (pointCounter > 0)
              {
                meanPoint.x /= (float)pointCounter;
                meanPoint.y /= (float)pointCounter;
                meanPoint.z /= (float)pointCounter;

                voxelCentroidList_arg.push_back (meanPoint);
              }

              // reset centroid to current input point
              meanPoint.x = idxPoint.x;
              meanPoint.y = idxPoint.y;
              meanPoint.z = idxPoint.z;
              pointCounter = 1;

              keyP = keyC;
            }
          }

          // push last centroid to result vector if necessary
          if (pointCounter > 0)
          {
            meanPoint.x /= (float)pointCounter;
            meanPoint.y /= (float)pointCounter;
            meanPoint.z /= (float)pointCounter;

            voxelCentroidList_arg.push_back (meanPoint);
          }

          // return size of centroid vector
          return voxelCentroidList_arg.size ();
        }

        /** \brief Get centroid for a single voxel addressed by a PointT point.
         * \param point_arg: point addressing a voxel in octree
         * \param voxelCentroid_arg: centroid is written to this PointT reference
         * \return "true" if voxel is found; "false" otherwise
         */
        bool
        getVoxelCentroidAtPoint (const PointT& point_arg, PointT& voxelCentroid_arg)
        {

          size_t i;
          unsigned int pointCounter;
          std::vector<int> indicesVector;
          PointT meanPoint;
          PointT idxPoint;

          bool bResult;

          // get all point indixes from voxel at point point_arg
          bResult = this->voxelSearch (point_arg, indicesVector);

          if (bResult)
          {
            meanPoint.x = meanPoint.y = meanPoint.z = 0.0;
            pointCounter = 0;

            // iterate over all point indices
            for (i = 0; i < indicesVector.size (); i++)
            {
              idxPoint = this->input_->points[indicesVector[i]];

              meanPoint.x += idxPoint.x;
              meanPoint.y += idxPoint.y;
              meanPoint.z += idxPoint.z;

              pointCounter++;
            }

            // calculate centroid
            voxelCentroid_arg.x = meanPoint.x / (float)pointCounter;
            voxelCentroid_arg.y = meanPoint.y / (float)pointCounter;
            voxelCentroid_arg.z = meanPoint.z / (float)pointCounter;
          }

          return bResult;
        }

        /** \brief Get centroid for a single voxel addressed by a PointT point from input cloud.
         * \param pointIdx_arg: point index from input cloud addressing a voxel in octree
         * \param voxelCentroid_arg: centroid is written to this PointT reference
         * \return "true" if voxel is found; "false" otherwise
         */
        inline bool
        getVoxelCentroidAtPoint (const int& pointIdx_arg, PointT& voxelCentroid_arg)
        {

          // retrieve point from input cloud
          const PointT& point = this->input_->points[pointIdx_arg];

          // get centroid at point
          return this->getVoxelCentroidAtPoint (point, voxelCentroid_arg);

        }

      };

  }

}

#define PCL_INSTANTIATE_OctreePointCloudVoxelCentroid(T) template class PCL_EXPORTS pcl::octree::OctreePointCloudVoxelCentroid<T>;

#endif

