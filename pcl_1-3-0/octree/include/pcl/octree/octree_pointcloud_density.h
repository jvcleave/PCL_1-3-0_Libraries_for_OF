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
 * $Id: octree_pointcloud_density.h 3017 2011-11-01 03:24:04Z rusu $
 */

#ifndef OCTREE_DENSITY_H
#define OCTREE_DENSITY_H

#include "octree_pointcloud.h"

#include "octree_base.h"
#include "octree2buf_base.h"

namespace pcl
{
  namespace octree
  {

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /** \brief @b Octree pointcloud density leaf node class
     *  \note This class implements a leaf node that counts the amount of points which fall into its voxel space.
     *  \author Julius Kammerl (julius@kammerl.de)
     */
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT>
      class OctreePointCloudDensityLeaf : public OctreeLeafAbstract<DataT>
      {
      public:
        /** \brief Class initialization. */
        OctreePointCloudDensityLeaf ()
        {
          pointCounter_ = 0;
        }

        /** \brief Empty class deconstructor. */
        ~OctreePointCloudDensityLeaf ()
        {
        }

        /** \brief Read input data. Only an internal counter is increased.
         * /param point_arg: input point - this argument is ignored
         *  */
        virtual void
        setData (const DataT& point_arg)
        {
          pointCounter_++;
        }

        /** \brief Returns a null pointer as this leaf node does not store any data.
         *  \param data_arg: reference to return pointer of leaf node DataT element (will be set to 0).
         */
        virtual void
        getData (const DataT*& data_arg) const
        {
          data_arg = 0;
        }

        /** \brief Empty getData data vector implementation as this leaf node does not store any data. \
       *  \param dataVector_arg: reference to dummy DataT vector that is extended with leaf node DataT elements.
         */
        virtual void
        getData (std::vector<DataT>& dataVector_arg) const
        {
        }

        /** \brief Return point counter.
         *  \return Amaount of points
         * */
        unsigned int
        getPointCounter ()
        {
          return pointCounter_;
        }

        /** \brief Empty reset leaf node implementation as this leaf node does not store any data. */
        virtual void
        reset ()
        {
          pointCounter_ = 0;
        }

      private:
        unsigned int pointCounter_;

      };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /** \brief @b Octree pointcloud density class
     *  \note This class generate an octrees from a point cloud (zero-copy). Only the amount of points that fall into the leaf node voxel are stored.
     *  \note The octree pointcloud is initialized with its voxel resolution. Its bounding box is automatically adjusted or can be predefined.
     *  \note
     *  \note typename: PointT: type of point used in pointcloud
     *  \ingroup octree
     *  \author Julius Kammerl (julius@kammerl.de)
     */
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT = OctreePointCloudDensityLeaf<int> , typename OctreeT = OctreeBase<int,
        LeafT> >
      class OctreePointCloudDensity : public OctreePointCloud<PointT, LeafT, OctreeT>
      {

      public:
        // public typedefs for single/double buffering
        typedef OctreePointCloudDensity<PointT, LeafT, OctreeBase<int, LeafT> > SingleBuffer;
        typedef OctreePointCloudDensity<PointT, LeafT, Octree2BufBase<int, LeafT> > DoubleBuffer;

        /** \brief OctreePointCloudDensity class constructor.
         *  \param resolution_arg:  octree resolution at lowest octree level
         * */
        OctreePointCloudDensity (const double resolution_arg) :
          OctreePointCloud<PointT, LeafT, OctreeT> (resolution_arg)
        {
        }

        /** \brief Empty class deconstructor. */
        virtual
        ~OctreePointCloudDensity ()
        {
        }

        /** \brief Get the amount of points within a leaf node voxel which is addressed by a point
         *  \param point_arg: a point addressing a voxel
         *  \return amount of points that fall within leaf node voxel
         * */
        unsigned int
        getVoxelDensityAtPoint (const PointT& point_arg) const
        {
          unsigned int pointCount = 0;

          OctreePointCloudDensityLeaf<int>* leaf = this->findLeafAtPoint (point_arg);

          if (leaf)
          {
            pointCount = leaf->getPointCounter ();
          }

          return pointCount;
        }

      };
  }

}

#define PCL_INSTANTIATE_OctreePointCloudDensity(T) template class PCL_EXPORTS pcl::octree::OctreePointCloudDensity<T>;

#define PCL_INSTANTIATE_OctreePointCloudSingleBufferWithDensityLeaf(T) template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreePointCloudDensityLeaf<int> , pcl::octree::OctreeBase<int, pcl::octree::OctreePointCloudDensityLeaf<int> > >;
#define PCL_INSTANTIATE_OctreePointCloudDoubleBufferWithDensityLeaf(T) template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreePointCloudDensityLeaf<int> , pcl::octree::Octree2BufBase<int, pcl::octree::OctreePointCloudDensityLeaf<int> > >;


#endif

