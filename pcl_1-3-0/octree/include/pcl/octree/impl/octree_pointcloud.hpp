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
 * $Id: octree_pointcloud.hpp 3017 2011-11-01 03:24:04Z rusu $
 */

#ifndef OCTREE_POINTCLOUD_HPP_
#define OCTREE_POINTCLOUD_HPP_

#include <vector>
#include <assert.h>

#include "pcl/common/common.h"

namespace pcl
{
  namespace octree
  {

    using namespace std;

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      OctreePointCloud<PointT, LeafT, OctreeT>::OctreePointCloud (const double resolution) :
        OctreeT (), epsilon_ (0), resolution_ (resolution), minX_ (0.0f), maxX_ (resolution), minY_ (0.0f),
            maxY_ (resolution), minZ_ (0.0f), maxZ_ (resolution), maxKeys_ (1), boundingBoxDefined_ (false)
      {
        assert ( resolution > 0.0f );
        input_ = PointCloudConstPtr ();
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      OctreePointCloud<PointT, LeafT, OctreeT>::~OctreePointCloud ()
      {

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::addPointsFromInputCloud ()
      {
        size_t i;

        assert (this->leafCount_==0);

        {
          if (indices_)
          {
            std::vector<int>::const_iterator current = indices_->begin ();
            while (current != indices_->end ())
            {
              if ((input_->points[*current].x == input_->points[*current].x) && (input_->points[*current].y
                  == input_->points[*current].y) && (input_->points[*current].z == input_->points[*current].z))
              {
                // add points to octree
                this->addPointIdx (*current);
                ++current;
              }
            }
          }
          else
          {
            for (i = 0; i < input_->points.size (); i++)
            {
              if ((input_->points[i].x == input_->points[i].x) && (input_->points[i].y == input_->points[i].y)
                  && (input_->points[i].z == input_->points[i].z))
              {
                // add points to octree
                this->addPointIdx ((unsigned int)i);
              }
            }
          }

        }
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::addPointFromCloud (const int pointIdx_arg, IndicesPtr indices_arg)
      {

        this->addPointIdx (pointIdx_arg);

        if (indices_arg)
        {
          indices_arg->push_back (pointIdx_arg);
        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::addPointToCloud (const PointT& point_arg, PointCloudPtr cloud_arg)
      {
        assert (cloud_arg==input_);

        cloud_arg->points.push_back (point_arg);

        this->addPointIdx (cloud_arg->points.size () - 1);

      }

    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::addPointToCloud (const PointT& point_arg, PointCloudPtr cloud_arg,
                                                                 IndicesPtr indices_arg)
      {

        assert (cloud_arg==input_);
        assert (indices_arg==indices_);

        cloud_arg->points.push_back (point_arg);

        this->addPointFromCloud (cloud_arg->points.size () - 1, indices_arg);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      bool
      OctreePointCloud<PointT, LeafT, OctreeT>::isVoxelOccupiedAtPoint (const PointT& point_arg) const
      {
        OctreeKey key;

        // generate key for point
        this->genOctreeKeyforPoint (point_arg, key);

        // search for key in octree
        return this->existLeaf (key);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      bool
      OctreePointCloud<PointT, LeafT, OctreeT>::isVoxelOccupiedAtPoint (const int& pointIdx_arg) const
      {

        // retrieve point from input cloud
        const PointT& point = this->input_->points[pointIdx_arg];

        // search for voxel at point in octree
        return this->isVoxelOccupiedAtPoint (point);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      bool
      OctreePointCloud<PointT, LeafT, OctreeT>::isVoxelOccupiedAtPoint (const double pointX_arg,
                                                                        const double pointY_arg,
                                                                        const double pointZ_arg) const
      {
        OctreeKey key;

        // generate key for point
        this->genOctreeKeyforPoint (pointX_arg, pointY_arg, pointZ_arg, key);

        return this->existLeaf (key);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::deleteVoxelAtPoint (const PointT& point_arg)
      {
        OctreeKey key;

        // generate key for point
        this->genOctreeKeyforPoint (point_arg, key);

        this->removeLeaf (key);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::deleteVoxelAtPoint (const int& pointIdx_arg)
      {
        // retrieve point from input cloud
        const PointT& point = this->input_->points[pointIdx_arg];

        // delete leaf at point
        this->deleteVoxelAtPoint (point);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      int
      OctreePointCloud<PointT, LeafT, OctreeT>::getOccupiedVoxelCenters (
                                                                         std::vector<PointT, Eigen::aligned_allocator<
                                                                             PointT> > &voxelCenterList_arg) const
      {
        OctreeKey key;
        key.x = key.y = key.z = 0;

        voxelCenterList_arg.clear ();
        voxelCenterList_arg.reserve (this->leafCount_);

        return getOccupiedVoxelCentersRecursive (this->rootNode_, key, voxelCenterList_arg);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::defineBoundingBox ()
      {

        double minX, minY, minZ, maxX, maxY, maxZ;

        PointT min_pt;
        PointT max_pt;

        // bounding box cannot be changed once the octree contains elements
        assert ( this->leafCount_ == 0 );

        pcl::getMinMax3D (*input_, min_pt, max_pt);

        minX = min_pt.x;
        minY = min_pt.y;
        minZ = min_pt.z;

        maxX = max_pt.x;
        maxY = max_pt.y;
        maxZ = max_pt.z;

        minX -= this->resolution_ * 0.5f;
        minY -= this->resolution_ * 0.5f;
        minZ -= this->resolution_ * 0.5f;

        maxX += this->resolution_ * 0.5f;
        maxY += this->resolution_ * 0.5f;
        maxZ += this->resolution_ * 0.5f;

        // generate bit masks for octree
        defineBoundingBox (minX, minY, minZ, maxX, maxY, maxZ);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::defineBoundingBox (const double minX_arg, const double minY_arg,
                                                                   const double minZ_arg, const double maxX_arg,
                                                                   const double maxY_arg, const double maxZ_arg)
      {

        // bounding box cannot be changed once the octree contains elements
        assert ( this->leafCount_ == 0 );

        assert ( maxX_arg >= minX_arg );
        assert ( maxY_arg >= minY_arg );
        assert ( maxZ_arg >= minZ_arg );

        minX_ = minX_arg;
        maxX_ = maxX_arg;

        minY_ = minY_arg;
        maxY_ = maxY_arg;

        minZ_ = minZ_arg;
        maxZ_ = maxZ_arg;

        minX_ = min (minX_, maxX_);
        minY_ = min (minY_, maxY_);
        minZ_ = min (minZ_, maxZ_);

        maxX_ = max (minX_, maxX_);
        maxY_ = max (minY_, maxY_);
        maxZ_ = max (minZ_, maxZ_);

        // generate bit masks for octree
        getKeyBitSize ();

        boundingBoxDefined_ = true;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::defineBoundingBox (const double maxX_arg, const double maxY_arg,
                                                                   const double maxZ_arg)
      {

        // bounding box cannot be changed once the octree contains elements
        assert ( this->leafCount_ == 0 );

        assert ( maxX_arg >= 0.0f );
        assert ( maxY_arg >= 0.0f );
        assert ( maxZ_arg >= 0.0f );

        minX_ = 0.0f;
        maxX_ = maxX_arg;

        minY_ = 0.0f;
        maxY_ = maxY_arg;

        minZ_ = 0.0f;
        maxZ_ = maxZ_arg;

        minX_ = min (minX_, maxX_);
        minY_ = min (minY_, maxY_);
        minZ_ = min (minZ_, maxZ_);

        maxX_ = max (minX_, maxX_);
        maxY_ = max (minY_, maxY_);
        maxZ_ = max (minZ_, maxZ_);

        // generate bit masks for octree
        getKeyBitSize ();

        boundingBoxDefined_ = true;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::defineBoundingBox (const double cubeLen_arg)
      {

        // bounding box cannot be changed once the octree contains elements
        assert ( this->leafCount_ == 0 );

        assert ( cubeLen_arg >= 0.0f );

        minX_ = 0.0f;
        maxX_ = cubeLen_arg;

        minY_ = 0.0f;
        maxY_ = cubeLen_arg;

        minZ_ = 0.0f;
        maxZ_ = cubeLen_arg;

        minX_ = min (minX_, maxX_);
        minY_ = min (minY_, maxY_);
        minZ_ = min (minZ_, maxZ_);

        maxX_ = max (minX_, maxX_);
        maxY_ = max (minY_, maxY_);
        maxZ_ = max (minZ_, maxZ_);

        // generate bit masks for octree
        getKeyBitSize ();

        boundingBoxDefined_ = true;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::getBoundingBox (double& minX_arg, double& minY_arg, double& minZ_arg,
                                                                double& maxX_arg, double& maxY_arg, double& maxZ_arg) const
      {
        minX_arg = minX_;
        minY_arg = minY_;
        minZ_arg = minZ_;

        maxX_arg = maxX_;
        maxY_arg = maxY_;
        maxZ_arg = maxZ_;
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::adoptBoundingBoxToPoint (const PointT& pointIdx_arg)
      {

        const double minValue = 1e-10;

        // increase octree size until point fits into bounding box
        while (true)
        {

          bool bLowerBoundViolationX = (pointIdx_arg.x < minX_);
          bool bLowerBoundViolationY = (pointIdx_arg.y < minY_);
          bool bLowerBoundViolationZ = (pointIdx_arg.z < minZ_);

          bool bUpperBoundViolationX = (pointIdx_arg.x >= maxX_);
          bool bUpperBoundViolationY = (pointIdx_arg.y >= maxY_);
          bool bUpperBoundViolationZ = (pointIdx_arg.z >= maxZ_);

          // do we violate any bounds?
          if (bLowerBoundViolationX || bLowerBoundViolationY || bLowerBoundViolationZ || bUpperBoundViolationX
              || bUpperBoundViolationY || bUpperBoundViolationZ || (!boundingBoxDefined_))
          {

            double octreeSideLen;
            unsigned char childIdx;

            if (this->leafCount_ > 0)
            {

              // octree not empty - we add another tree level and thus increase its size by a factor of 2*2*2
              childIdx = ((!bUpperBoundViolationX) << 2) | ((!bUpperBoundViolationY) << 1) | ((!bUpperBoundViolationZ));

              OctreeBranch* newRootBranch;

              this->createBranch (newRootBranch);
              this->branchCount_++;

              this->setBranchChild (*newRootBranch, childIdx, this->rootNode_);

              this->rootNode_ = newRootBranch;

              octreeSideLen = maxX_ - minX_ - minValue;

              if (bUpperBoundViolationX)
              {
                maxX_ += octreeSideLen;
              }
              else
              {
                minX_ -= octreeSideLen;
              }

              if (bUpperBoundViolationY)
              {
                maxY_ += octreeSideLen;
              }
              else
              {
                minY_ -= octreeSideLen;
              }

              if (bUpperBoundViolationZ)
              {
                maxZ_ += octreeSideLen;
              }
              else
              {
                minZ_ -= octreeSideLen;
              }

            }
            else
            {

              // octree is empty - we set the center of the bounding box to our first pixel
              this->minX_ = pointIdx_arg.x - this->resolution_ / 2 + minValue;
              this->minY_ = pointIdx_arg.y - this->resolution_ / 2 + minValue;
              this->minZ_ = pointIdx_arg.z - this->resolution_ / 2 + minValue;

              this->maxX_ = pointIdx_arg.x + this->resolution_ / 2 - minValue;
              this->maxY_ = pointIdx_arg.y + this->resolution_ / 2 - minValue;
              this->maxZ_ = pointIdx_arg.z + this->resolution_ / 2 - minValue;

            }

            getKeyBitSize ();

            boundingBoxDefined_ = true;

          }
          else
          {
            // no bound violations anymore - leave while loop
            break;
          }
        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::addPointIdx (const int pointIdx_arg)
      {
        OctreeKey key;

        assert (pointIdx_arg < (int) input_->points.size());

        const PointT& point = input_->points[pointIdx_arg];

        // make sure bounding box is big enough
        adoptBoundingBoxToPoint (point);

        // generate key
        genOctreeKeyforPoint (point, key);

        // add point to octree at key
        this->add (key, pointIdx_arg);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      const PointT&
      OctreePointCloud<PointT, LeafT, OctreeT>::getPointByIndex (const unsigned int index_arg) const
      {
        // retrieve point from input cloud

        if (indices_ == 0)
        {
          assert (index_arg < (unsigned int)input_->points.size ());
          return this->input_->points[index_arg];
        }
        else
        {
          assert (index_arg < (unsigned int)indices_->size ());
          return input_->points[(*indices_)[index_arg]];
        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      LeafT*
      OctreePointCloud<PointT, LeafT, OctreeT>::findLeafAtPoint (const PointT& point) const
      {
        OctreeKey key;

        // generate key for point
        this->genOctreeKeyforPoint (point, key);

        return this->findLeaf (key);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::getKeyBitSize ()
      {
        unsigned int maxVoxels;

        unsigned int maxKeyX;
        unsigned int maxKeyY;
        unsigned int maxKeyZ;

        double octreeSideLen;

        const double minValue = 1e-10;

        // find maximum key values for x, y, z
        maxKeyX = ceil ((maxX_ - minX_) / resolution_);
        maxKeyY = ceil ((maxY_ - minY_) / resolution_);
        maxKeyZ = ceil ((maxZ_ - minZ_) / resolution_);

        // find maximum amount of keys
        maxVoxels = max (max (max (maxKeyX, maxKeyY), maxKeyZ), (unsigned int)2);

        // tree depth == amount of bits of maxVoxels
        this->octreeDepth_ = max ((min ((unsigned int)OCT_MAXTREEDEPTH, (unsigned int)ceil (this->Log2 (maxVoxels)))),
                                  (unsigned int)0);

        maxKeys_ = (1 << this->octreeDepth_);

        octreeSideLen = (double)maxKeys_ * resolution_ - minValue;

        if (this->leafCount_ == 0)
        {

          double octreeOversizeX;
          double octreeOversizeY;
          double octreeOversizeZ;

          octreeOversizeX = (octreeSideLen - (maxX_ - minX_)) / 2.0;
          octreeOversizeY = (octreeSideLen - (maxY_ - minY_)) / 2.0;
          octreeOversizeZ = (octreeSideLen - (maxZ_ - minZ_)) / 2.0;

          minX_ -= octreeOversizeX;
          minY_ -= octreeOversizeY;
          minZ_ -= octreeOversizeZ;

          maxX_ += octreeOversizeX;
          maxY_ += octreeOversizeY;
          maxZ_ += octreeOversizeZ;

        }
        else
        {

          maxX_ = minX_ + octreeSideLen;
          maxY_ = minY_ + octreeSideLen;
          maxZ_ = minZ_ + octreeSideLen;

        }

        // configure tree depth of octree
        this->setTreeDepth (this->octreeDepth_);

        return;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::genOctreeKeyforPoint (const PointT& point_arg, OctreeKey & key_arg) const
      {

        // calculate integer key for point coordinates
        key_arg.x = (unsigned int)((point_arg.x - this->minX_) / this->resolution_);
        key_arg.y = (unsigned int)((point_arg.y - this->minY_) / this->resolution_);
        key_arg.z = (unsigned int)((point_arg.z - this->minZ_) / this->resolution_);

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::genOctreeKeyforPoint (const double pointX_arg, const double pointY_arg,
                                                                      const double pointZ_arg, OctreeKey & key_arg) const
      {
        PointT tempPoint;

        tempPoint.x = pointX_arg;
        tempPoint.y = pointY_arg;
        tempPoint.z = pointZ_arg;

        // generate key for point
        genOctreeKeyforPoint (tempPoint, key_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      bool
      OctreePointCloud<PointT, LeafT, OctreeT>::genOctreeKeyForDataT (const int& data_arg, OctreeKey & key_arg) const
      {

        const PointT tempPoint = getPointByIndex (data_arg);

        // generate key for point
        genOctreeKeyforPoint (tempPoint, key_arg);

        return true;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::genLeafNodeCenterFromOctreeKey (const OctreeKey & key, PointT & point) const
      {

        // define point to leaf node voxel center
        point.x = ((double)key.x + 0.5f) * this->resolution_ + this->minX_;
        point.y = ((double)key.y + 0.5f) * this->resolution_ + this->minY_;
        point.z = ((double)key.z + 0.5f) * this->resolution_ + this->minZ_;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::genVoxelCenterFromOctreeKey (const OctreeKey & key_arg,
                                                                             unsigned int treeDepth_arg,
                                                                             PointT& point_arg) const
      {

        // generate point for voxel center defined by treedepth (bitLen) and key
        point_arg.x = ((double)(key_arg.x) + 0.5f) * (this->resolution_ * (double)(1 << (this->octreeDepth_
            - treeDepth_arg))) + this->minX_;
        point_arg.y = ((double)(key_arg.y) + 0.5f) * (this->resolution_ * (double)(1 << (this->octreeDepth_
            - treeDepth_arg))) + this->minY_;
        point_arg.z = ((double)(key_arg.z) + 0.5f) * (this->resolution_ * (double)(1 << (this->octreeDepth_
            - treeDepth_arg))) + this->minZ_;
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      void
      OctreePointCloud<PointT, LeafT, OctreeT>::genVoxelBoundsFromOctreeKey (const OctreeKey & key_arg,
                                                                             unsigned int treeDepth_arg,
                                                                             Eigen::Vector3f &min_pt,
                                                                             Eigen::Vector3f &max_pt) const
      {
        // calculate voxel size of current tree depth
        double voxel_side_len = this->resolution_ * (double)(1 << (this->octreeDepth_ - treeDepth_arg));

        // calculate voxel bounds
        min_pt (0) = (double)(key_arg.x) * voxel_side_len + this->minX_;
        min_pt (1) = (double)(key_arg.y) * voxel_side_len + this->minY_;
        min_pt (2) = (double)(key_arg.z) * voxel_side_len + this->minZ_;

        max_pt (0) = (double)(key_arg.x + 1) * voxel_side_len + this->minX_;
        max_pt (1) = (double)(key_arg.y + 1) * voxel_side_len + this->minY_;
        max_pt (2) = (double)(key_arg.z + 1) * voxel_side_len + this->minZ_;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      double
      OctreePointCloud<PointT, LeafT, OctreeT>::getVoxelSquaredSideLen (unsigned int treeDepth_arg) const
      {
        double sideLen;

        // side length of the voxel cube increases exponentially with the octree depth
        sideLen = this->resolution_ * (double)(1 << (this->octreeDepth_ - treeDepth_arg));

        // squared voxel side length
        sideLen *= sideLen;

        return sideLen;
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      double
      OctreePointCloud<PointT, LeafT, OctreeT>::getVoxelSquaredDiameter (unsigned int treeDepth_arg) const
      {
        // return the squared side length of the voxel cube as a function of the octree depth
        return getVoxelSquaredSideLen (treeDepth_arg) * 3;
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename PointT, typename LeafT, typename OctreeT>
      int
      OctreePointCloud<PointT, LeafT, OctreeT>::getOccupiedVoxelCentersRecursive (
                                                                                  const OctreeBranch* node_arg,
                                                                                  const OctreeKey& key_arg,
                                                                                  std::vector<PointT,
                                                                                      Eigen::aligned_allocator<PointT> > &voxelCenterList_arg) const
      {
        // child iterator
        unsigned char childIdx;

        int voxelCount = 0;

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {
          if (branchHasChild (*node_arg, childIdx))
          {
            const OctreeNode * childNode;
            childNode = getBranchChild (*node_arg, childIdx);

            // generate new key for current branch voxel
            OctreeKey newKey;
            newKey.x = (key_arg.x << 1) | (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) | (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) | (!!(childIdx & (1 << 0)));

            switch (childNode->getNodeType ())
            {
              case BRANCH_NODE:

                // recursively proceed with indexed child branch
                voxelCount += getOccupiedVoxelCentersRecursive ((OctreeBranch*)childNode, newKey, voxelCenterList_arg);
                break;

              case LEAF_NODE:
                PointT newPoint;

                genLeafNodeCenterFromOctreeKey (newKey, newPoint);

                voxelCenterList_arg.push_back (newPoint);

                voxelCount++;
                break;
            }

          }

        }

        return voxelCount;
      }

  }
}

#define PCL_INSTANTIATE_OctreePointCloudSingleBufferWithLeafDataTVector(T) template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafDataTVector<int> , pcl::octree::OctreeBase<int, pcl::octree::OctreeLeafDataTVector<int> > >;
#define PCL_INSTANTIATE_OctreePointCloudDoubleBufferWithLeafDataTVector(T) template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafDataTVector<int> , pcl::octree::Octree2BufBase<int, pcl::octree::OctreeLeafDataTVector<int> > >;
#define PCL_INSTANTIATE_OctreePointCloudLowMemWithLeafDataTVector(T)       template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafDataTVector<int> , pcl::octree::OctreeLowMemBase<int, pcl::octree::OctreeLeafDataTVector<int> > >;

#define PCL_INSTANTIATE_OctreePointCloudSingleBufferWithLeafDataT(T) template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafDataT<int> , pcl::octree::OctreeBase<int, pcl::octree::OctreeLeafDataT<int> > >;
#define PCL_INSTANTIATE_OctreePointCloudDoubleBufferWithLeafDataT(T) template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafDataT<int> , pcl::octree::Octree2BufBase<int, pcl::octree::OctreeLeafDataT<int> > >;
#define PCL_INSTANTIATE_OctreePointCloudLowMemWithLeafDataT(T)       template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafDataT<int> , pcl::octree::OctreeLowMemBase<int, pcl::octree::OctreeLeafDataT<int> > >;

#define PCL_INSTANTIATE_OctreePointCloudSingleBufferWithEmptyLeaf(T) template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafEmpty<int> , pcl::octree::OctreeBase<int, pcl::octree::OctreeLeafEmpty<int> > >;
#define PCL_INSTANTIATE_OctreePointCloudDoubleBufferWithEmptyLeaf(T) template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafEmpty<int> , pcl::octree::Octree2BufBase<int, pcl::octree::OctreeLeafEmpty<int> > >;
#define PCL_INSTANTIATE_OctreePointCloudLowMemWithEmptyLeaf(T)       template class PCL_EXPORTS pcl::octree::OctreePointCloud<T, pcl::octree::OctreeLeafEmpty<int> , pcl::octree::OctreeLowMemBase<int, pcl::octree::OctreeLeafEmpty<int> > >;

#endif /* OCTREE_POINTCLOUD_HPP_ */
