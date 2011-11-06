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
 * $Id: octree_lowmemory_base.hpp 3017 2011-11-01 03:24:04Z rusu $
 */

#ifndef OCTREE_LOWMEM_TREE_BASE_HPP
#define OCTREE_LOWMEM_TREE_BASE_HPP

#include <vector>

#include "pcl/impl/instantiate.hpp"
#include "pcl/point_types.h"
#include "pcl/octree/octree.h"

namespace pcl
{
  namespace octree
  {

    using namespace std;

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      OctreeLowMemBase<DataT, LeafT>::OctreeLowMemBase ()
      {

        // Initialization of globals
        rootNode_ = new OctreeBranch ();
        leafCount_ = 0;
        depthMask_ = 0;
        branchCount_ = 1;
        objectCount_ = 0;
        octreeDepth_ = 0;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      OctreeLowMemBase<DataT, LeafT>::~OctreeLowMemBase ()
      {

        // deallocate tree structure
        deleteTree ();
        delete (rootNode_);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::setMaxVoxelIndex (unsigned int maxVoxelIndex_arg)
      {
        unsigned int treeDepth;

        assert (maxVoxelIndex_arg>0);

        // tree depth == amount of bits of maxVoxels
        treeDepth = max ((min ((unsigned int)OCT_MAXTREEDEPTH, (unsigned int)ceil (Log2 (maxVoxelIndex_arg)))),
                         (unsigned int)0);

        // define depthMask_ by setting a single bit to 1 at bit position == tree depth
        depthMask_ = (1 << (treeDepth - 1));

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::setTreeDepth (unsigned int depth_arg)
      {

        assert (depth_arg>0);

        // set octree depth
        octreeDepth_ = depth_arg;

        // define depthMask_ by setting a single bit to 1 at bit position == tree depth
        depthMask_ = (1 << (depth_arg - 1));

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::add (const unsigned int idxX_arg, const unsigned int idxY_arg,
                                     const unsigned int idxZ_arg, const DataT& data_arg)
      {

        OctreeKey key;

        // generate key
        genOctreeKeyByIntIdx (idxX_arg, idxY_arg, idxZ_arg, key);

        // add data_arg to octree
        add (key, data_arg);

        objectCount_++;

      }



    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      bool
      OctreeLowMemBase<DataT, LeafT>::get (const unsigned int idxX_arg, const unsigned int idxY_arg,
                                     const unsigned int idxZ_arg, DataT& data_arg) const
      {
        OctreeKey key;

        // generate key
        genOctreeKeyByIntIdx (idxX_arg, idxY_arg, idxZ_arg, key);

        // search for leaf at key
        LeafT* leaf = findLeaf (key);
        if (leaf)
        {
          const DataT * dataPtr;
          // if successful, decode data to data_arg
          leaf->getData (dataPtr);
          if (dataPtr)
            data_arg = *dataPtr;
        }

        // returns true on success
        return (leaf != 0);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      bool
      OctreeLowMemBase<DataT, LeafT>::existLeaf (const unsigned int idxX_arg, const unsigned int idxY_arg,
                                           const unsigned int idxZ_arg) const
      {
        OctreeKey key;

        // generate key
        this->genOctreeKeyByIntIdx (idxX_arg, idxY_arg, idxZ_arg, key);

        // check if key exist in octree
        return existLeaf (key);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::removeLeaf (const unsigned int idxX_arg, const unsigned int idxY_arg,
                                            const unsigned int idxZ_arg)
      {
        OctreeKey key;

        // generate key
        this->genOctreeKeyByIntIdx (idxX_arg, idxY_arg, idxZ_arg, key);

        // check if key exist in octree
        deleteLeafRecursive (key, depthMask_, rootNode_);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deleteTree ( )
      {

        if (rootNode_)
        {
          // reset octree
          deleteBranch (*rootNode_);
          leafCount_ = 0;
          branchCount_ = 1;
          objectCount_ = 0;

        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::serializeTree (std::vector<char>& binaryTreeOut_arg,
                                                     bool doXOREncoding_arg)
      {
        OctreeKey newKey;
        newKey.x = newKey.y = newKey.z = 0;

        // clear binary vector
        binaryTreeOut_arg.clear ();
        binaryTreeOut_arg.reserve (this->branchCount_);

        serializeTreeRecursive (binaryTreeOut_arg, rootNode_, newKey);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::serializeTree (std::vector<char>& binaryTreeOut_arg,
                                                     std::vector<DataT>& dataVector_arg, bool doXOREncoding_arg)
      {
        OctreeKey newKey;
        newKey.x = newKey.y = newKey.z = 0;

        // clear output vectors
        binaryTreeOut_arg.clear ();
        dataVector_arg.clear ();

        dataVector_arg.reserve (this->objectCount_);
        binaryTreeOut_arg.reserve (this->branchCount_);

        OctreeLowMemBase<DataT, LeafT>::serializeTreeRecursive (binaryTreeOut_arg, rootNode_, newKey, dataVector_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::serializeLeafs (std::vector<DataT>& dataVector_arg)
      {

        OctreeKey newKey;
        newKey.x = newKey.y = newKey.z = 0;

        // clear output vector
        dataVector_arg.clear ();

        dataVector_arg.reserve(this->objectCount_);

        serializeLeafsRecursive (rootNode_, newKey, dataVector_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeTree (std::vector<char>& binaryTreeIn_arg,
                                                       bool doXORDecoding_arg)
      {

        OctreeKey newKey;
        newKey.x = newKey.y = newKey.z = 0;

        // free existing tree before tree rebuild
        deleteTree ();

        //iterator for binary tree structure vector
        vector<char>::const_iterator binaryTreeVectorIterator = binaryTreeIn_arg.begin ();

        deserializeTreeRecursive (binaryTreeVectorIterator, rootNode_, depthMask_, newKey);

        objectCount_ = this->leafCount_;
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeTree (std::vector<char>& binaryTreeIn_arg,
                                                       std::vector<DataT>& dataVector_arg,
                                                       bool doXORDecoding_arg)
      {
        OctreeKey newKey;
        newKey.x = newKey.y = newKey.z = 0;

        // set data iterator to first element
        typename std::vector<DataT>::const_iterator dataVectorIterator = dataVector_arg.begin ();

        // set data iterator to last element
        typename std::vector<DataT>::const_iterator dataVectorEndIterator = dataVector_arg.end ();

        // free existing tree before tree rebuild
        deleteTree ();

        //iterator for binary tree structure vector
        vector<char>::const_iterator binaryTreeVectorIterator = binaryTreeIn_arg.begin ();

        deserializeTreeRecursive (binaryTreeVectorIterator, rootNode_, depthMask_, newKey, dataVectorIterator,
                                  dataVectorEndIterator);

        objectCount_ = dataVector_arg.size();
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeTreeAndOutputLeafData (std::vector<char>& binaryTreeIn_arg,
                                                                  std::vector<DataT>& dataVector_arg)
      {

        OctreeKey newKey;
        newKey.x = newKey.y = newKey.z = 0;

        // free existing tree before tree rebuild
        deleteTree ();

        //iterator for binary tree structure vector
        vector<char>::const_iterator binaryTreeVectorIterator = binaryTreeIn_arg.begin ();

        deserializeTreeAndOutputLeafDataRecursive (binaryTreeVectorIterator, rootNode_, depthMask_, newKey,
                                                   dataVector_arg);

        objectCount_ = dataVector_arg.size();
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      LeafT*
      OctreeLowMemBase<DataT, LeafT>::getLeafRecursive (const OctreeKey& key_arg, const unsigned int depthMask_arg,
                                                  OctreeBranch* branch_arg)
      {

        // index to branch child
        unsigned char childIdx;
        LeafT* result = 0;

        // find branch child from key
        childIdx = ((!!(key_arg.x & depthMask_arg)) << 2) | ((!!(key_arg.y & depthMask_arg)) << 1) | (!!(key_arg.z
            & depthMask_arg));

        if (depthMask_arg > 1)
        {
          // we have not reached maximum tree depth

          OctreeBranch* childBranch;
          if (!branchHasChild (*branch_arg, childIdx))
          {

            // if required branch does not exist -> create it
            createBranchChild (*branch_arg, childIdx, childBranch);

            branchCount_++;

          }
          else
          {

            // required branch exists
            childBranch = (OctreeBranch*)getBranchChild (*branch_arg, childIdx);
          }

          // recursively proceed with indexed child branch
          result = getLeafRecursive (key_arg, depthMask_arg / 2, childBranch);

        }
        else
        {
          // branch childs are leaf nodes

          OctreeLeaf* childLeaf;
          if (!branchHasChild (*branch_arg, childIdx))
          {

            // if leaf node at childIdx does not exist
            createLeafChild (*branch_arg, childIdx, childLeaf);
            leafCount_++;

            // return leaf node
            result = childLeaf;

          }
          else
          {

            // leaf node already exist
            childLeaf = (OctreeLeaf*)getBranchChild (*branch_arg, childIdx);

            // return leaf node
            result = childLeaf;

          }

        }

        return result;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      LeafT*
      OctreeLowMemBase<DataT, LeafT>::findLeafRecursive (const OctreeKey& key_arg, const unsigned int depthMask_arg,
                                                   OctreeBranch* branch_arg) const
      {
        // index to branch child
        unsigned char childIdx;
        LeafT* result = 0;

        // find branch child from key
        childIdx = ((!!(key_arg.x & depthMask_arg)) << 2) | ((!!(key_arg.y & depthMask_arg)) << 1) | (!!(key_arg.z
            & depthMask_arg));

        if (depthMask_arg > 1)
        {
          // we have not reached maximum tree depth
          OctreeBranch* childBranch;
          childBranch = (OctreeBranch*)getBranchChild (*branch_arg, childIdx);

          if (childBranch)
            // recursively proceed with indexed child branch
            result = findLeafRecursive (key_arg, depthMask_arg / 2, childBranch);

        }
        else
        {
          // we reached leaf node level
          if (branchHasChild (*branch_arg, childIdx))
          {

            // return existing leaf node
            OctreeLeaf* childLeaf = (OctreeLeaf*)getBranchChild (*branch_arg, childIdx);
            result = childLeaf;
          }

        }

        return result;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      bool
      OctreeLowMemBase<DataT, LeafT>::deleteLeafRecursive (const OctreeKey& key_arg, const unsigned int depthMask_arg,
                                                     OctreeBranch* branch_arg)
      {
        // index to branch child
        unsigned char childIdx;
        // indicates if branch is empty and can be safely removed
        bool bNoChilds;

        // find branch child from key
        childIdx = ((!!(key_arg.x & depthMask_arg)) << 2) | ((!!(key_arg.y & depthMask_arg)) << 1) | (!!(key_arg.z
            & depthMask_arg));

        if (depthMask_arg > 1)
        {
          // we have not reached maximum tree depth

          OctreeBranch* childBranch;
          bool bBranchOccupied;

          // next branch child on our path through the tree
          childBranch = (OctreeBranch*)getBranchChild (*branch_arg, childIdx);

          if (childBranch)
          {
            // recursively explore the indexed child branch
            bBranchOccupied = deleteLeafRecursive (key_arg, depthMask_arg / 2, childBranch);

            if (!bBranchOccupied)
            {
              // child branch does not own any sub-child nodes anymore -> delete child branch
              delete (childBranch);
              setBranchChild (*branch_arg, childIdx, 0);
              branchCount_--;
            }
          }

        }
        else
        {

          // our child is a leaf node -> delete it
          deleteBranchChild (*branch_arg, childIdx);
          leafCount_--;
        }

        // check if current branch still owns childs
        bNoChilds = false;
        for (childIdx = 0; childIdx < 8; childIdx++)
        {
          bNoChilds = branchHasChild (*branch_arg, childIdx);
          if (bNoChilds)
            break;
        }

        // return true if current branch can be deleted
        return bNoChilds;

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::serializeTreeRecursive (std::vector<char>& binaryTreeOut_arg,
                                                        const OctreeBranch* branch_arg, const OctreeKey& key_arg)
      {

        // child iterator
        unsigned char childIdx;
        char nodeBitPattern;

        // branch occupancy bit pattern
        nodeBitPattern = getBranchBitPattern (*branch_arg);

        // write bit pattern to output vector
        binaryTreeOut_arg.push_back (nodeBitPattern);

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {

          // if child exist
          if (branchHasChild (*branch_arg, childIdx))
          {

            // generate new key for current branch voxel
            OctreeKey newKey;
            newKey.x = (key_arg.x << 1) | (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) | (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) | (!!(childIdx & (1 << 0)));

            const OctreeNode * childNode;
            childNode = getBranchChild (*branch_arg, childIdx);

            switch (childNode->getNodeType ())
            {
              case BRANCH_NODE:

                // recursively proceed with indexed child branch
                serializeTreeRecursive (binaryTreeOut_arg, (OctreeBranch*)childNode, newKey);
                break;

              case LEAF_NODE:
                OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;

                // we reached a leaf node -> execute serialization callback
                serializeLeafCallback (*childLeaf, newKey);
                break;
            }

          }

        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::serializeTreeRecursive (std::vector<char>& binaryTreeOut_arg,
                                                        const OctreeBranch* branch_arg, const OctreeKey& key_arg,
                                                        typename std::vector<DataT>& dataVector_arg)
      {

        // child iterator
        unsigned char childIdx;
        char nodeBitPattern;

        // branch occupancy bit pattern
        nodeBitPattern = getBranchBitPattern (*branch_arg);

        // write bit pattern to output vector
        binaryTreeOut_arg.push_back (nodeBitPattern);

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {

          // if child exist
          if (branchHasChild (*branch_arg, childIdx))
          {

            // generate new key for current branch voxel
            OctreeKey newKey;
            newKey.x = (key_arg.x << 1) | (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) | (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) | (!!(childIdx & (1 << 0)));

            const OctreeNode * childNode;
            childNode = getBranchChild (*branch_arg, childIdx);

            switch (childNode->getNodeType ())
            {
              case BRANCH_NODE:

                // recursively proceed with indexed child branch
                serializeTreeRecursive (binaryTreeOut_arg, (OctreeBranch*)childNode, newKey, dataVector_arg);
                break;

              case LEAF_NODE:
                OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;

                // we reached a leaf node -> execute serialization callback
                serializeLeafCallback (*childLeaf, newKey, dataVector_arg);
                break;
            }

          }

        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::serializeLeafsRecursive (const OctreeBranch* branch_arg, const OctreeKey& key_arg,
                                                         std::vector<DataT>& dataVector_arg)
      {
        // child iterator
        unsigned char childIdx;

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {

          // if child exist
          if (branchHasChild (*branch_arg, childIdx))
          {
            const OctreeNode * childNode;
            childNode = getBranchChild (*branch_arg, childIdx);

            // generate new key for current branch voxel
            OctreeKey newKey;
            newKey.x = (key_arg.x << 1) | (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) | (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) | (!!(childIdx & (1 << 0)));

            switch (childNode->getNodeType ())
            {
              case BRANCH_NODE:

                // recursively proceed with indexed child branch
                serializeLeafsRecursive ((OctreeBranch*)childNode, newKey, dataVector_arg);
                break;

              case LEAF_NODE:
                OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;

                // we reached a leaf node -> execute serialization callback
                serializeLeafCallback (*childLeaf, newKey, dataVector_arg);
                break;
            }

          }

        }
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeTreeRecursive (typename std::vector<char>::const_iterator& binaryTreeIn_arg,
                                                          OctreeBranch* branch_arg, const unsigned int depthMask_arg,
                                                          const OctreeKey& key_arg)
      {
        // child iterator
        unsigned char childIdx;
        char nodeBits;

        // read branch occupancy bit pattern from input vector
        nodeBits = (*binaryTreeIn_arg);
        binaryTreeIn_arg++;

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {
          // if occupancy bit for childIdx is set..
          if (nodeBits & (1 << childIdx))
          {

            // generate new key for current branch voxel
            OctreeKey newKey;
            newKey.x = (key_arg.x << 1) | (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) | (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) | (!!(childIdx & (1 << 0)));

            if (depthMask_arg > 1)
            {
              // we have not reached maximum tree depth
              OctreeBranch * newBranch;

              // create new child branch
              createBranchChild (*branch_arg, childIdx, newBranch);

              branchCount_++;

              // recursively proceed with new child branch
              deserializeTreeRecursive (binaryTreeIn_arg, newBranch, depthMask_arg / 2, newKey);

            }
            else
            {
              // we reached leaf node level
              OctreeLeaf* childLeaf;

              // create leaf node
              createLeafChild (*branch_arg, childIdx, childLeaf);

              // execute deserialization callback
              deserializeLeafCallback (*childLeaf, newKey);

              leafCount_++;
            }
          }
        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeTreeRecursive (
                                                          typename std::vector<char>::const_iterator& binaryTreeIn_arg,
                                                          OctreeBranch* branch_arg,
                                                          const unsigned int depthMask_arg,
                                                          const OctreeKey& key_arg,
                                                          typename std::vector<DataT>::const_iterator& dataVectorIterator_arg,
                                                          typename std::vector<DataT>::const_iterator& dataVectorEndIterator_arg)
      {
        // child iterator
        unsigned char childIdx;
        char nodeBits;

        // read branch occupancy bit pattern from input vector
        nodeBits = (*binaryTreeIn_arg);
        binaryTreeIn_arg++;

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {
          // if occupancy bit for childIdx is set..
          if (nodeBits & (1 << childIdx))
          {
            // generate new key for current branch voxel
            OctreeKey newKey;
            newKey.x = (key_arg.x << 1) | (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) | (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) | (!!(childIdx & (1 << 0)));

            if (depthMask_arg > 1)
            {
              // we have not reached maximum tree depth
              OctreeBranch * newBranch;

              // create new child branch
              createBranchChild (*branch_arg, childIdx, newBranch);

              branchCount_++;

              // recursively proceed with new child branch
              deserializeTreeRecursive (binaryTreeIn_arg, newBranch, depthMask_arg / 2, newKey, dataVectorIterator_arg,
                                        dataVectorEndIterator_arg);

            }
            else
            {
              // we reached leaf node level

              OctreeLeaf* childLeaf;

              // create leaf node
              createLeafChild (*branch_arg, childIdx, childLeaf);

              // execute deserialization callback
              deserializeLeafCallback (*childLeaf, newKey, dataVectorIterator_arg, dataVectorEndIterator_arg);

              leafCount_++;
            }
          }
        }
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeTreeAndOutputLeafDataRecursive (typename std::vector<char>::const_iterator& binaryTreeIn_arg,
                                                                           OctreeBranch* branch_arg,
                                                                           const unsigned int depthMask_arg,
                                                                           const OctreeKey& key_arg,
                                                                           std::vector<DataT>& dataVector_arg)
      {
        // child iterator
        unsigned char childIdx;
        char nodeBits;

        // read branch occupancy bit pattern from input vector
        nodeBits = (*binaryTreeIn_arg);
        binaryTreeIn_arg++;

        // iterate over all children
        for (childIdx = 0; childIdx < 8; childIdx++)
        {
          // if occupancy bit for childIdx is set..
          if (nodeBits & (1 << childIdx))
          {

            // generate new key for current branch voxel
            OctreeKey newKey;
            newKey.x = (key_arg.x << 1) | (!!(childIdx & (1 << 2)));
            newKey.y = (key_arg.y << 1) | (!!(childIdx & (1 << 1)));
            newKey.z = (key_arg.z << 1) | (!!(childIdx & (1 << 0)));

            if (depthMask_arg > 1)
            {
              // we have not reached maximum tree depth
              OctreeBranch * newBranch;

              // create new child branch
              createBranchChild (*branch_arg, childIdx, newBranch);

              branchCount_++;

              // recursively proceed with new child branch
              deserializeTreeAndOutputLeafDataRecursive (binaryTreeIn_arg, newBranch, depthMask_arg / 2, newKey,
                                                         dataVector_arg);

            }
            else
            {
              // we reached leaf node level
              OctreeLeaf* childLeaf;

              // create leaf node
              createLeafChild (*branch_arg, childIdx, childLeaf);

              // execute deserialization callback
              deserializeTreeAndSerializeLeafCallback (*childLeaf, newKey, dataVector_arg);

              leafCount_++;
            }
          }
        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::serializeLeafCallback (OctreeLeaf& leaf_arg, const OctreeKey& key_arg)
      {
        // nothing to do
      }


    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::serializeLeafCallback (OctreeLeaf& leaf_arg, const OctreeKey& key_arg,
                                                       std::vector<DataT>& dataVector_arg)
      {
        leaf_arg.getData (dataVector_arg);
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeLeafCallback (
                                                         OctreeLeaf& leaf_arg,
                                                         const OctreeKey& key_arg,
                                                         typename std::vector<DataT>::const_iterator& dataVectorIterator_arg,
                                                         typename std::vector<DataT>::const_iterator& dataVectorEndIterator_arg)
      {
        OctreeKey dataKey;
        bool bKeyBasedEncoding = false;

        // add DataT objects to octree leaf as long as their key fit to voxel
        while ((dataVectorIterator_arg != dataVectorEndIterator_arg)
            && (this->genOctreeKeyForDataT (*dataVectorIterator_arg, dataKey) && (dataKey == key_arg)))
        {
          leaf_arg.setData (*dataVectorIterator_arg);
          dataVectorIterator_arg++;
          bKeyBasedEncoding = true;
        }

        // add single DataT object to octree if key-based encoding is disabled
        if (!bKeyBasedEncoding)
        {
          leaf_arg.setData (*dataVectorIterator_arg);
          dataVectorIterator_arg++;
        }
      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeLeafCallback (OctreeLeaf& leaf_arg, const OctreeKey& key_arg)
      {

        DataT newDataT;

        // initialize new leaf child
        if (genDataTByOctreeKey (key_arg, newDataT))
        {
          leaf_arg.setData (newDataT);
        }

      }

    //////////////////////////////////////////////////////////////////////////////////////////////
    template<typename DataT, typename LeafT>
      void
      OctreeLowMemBase<DataT, LeafT>::deserializeTreeAndSerializeLeafCallback (OctreeLeaf& leaf_arg,
                                                                         const OctreeKey& key_arg,
                                                                         std::vector<DataT>& dataVector_arg)
      {

        DataT newDataT;

        // initialize new leaf child
        if (genDataTByOctreeKey (key_arg, newDataT))
        {
          leaf_arg.setData (newDataT);
          dataVector_arg.push_back (newDataT);
        }
      }

  }
}

#define PCL_INSTANTIATE_OctreeLowMemBase(T) template class PCL_EXPORTS pcl::octree::OctreeLowMemBase<T>;

#endif

