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
 * $Id: octree_search.hpp 3018 2011-11-01 03:24:12Z svn $
 */

#ifndef PCL_OCTREE_SEARCH_IMPL_H_
#define PCL_OCTREE_SEARCH_IMPL_H_

#include <pcl/common/common.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> bool
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::voxelSearch (
    const PointT& point, std::vector<int>& pointIdx_data)
{
  OctreeKey key;
  bool b_success = false;

  // generate key
  genOctreeKeyforPoint (point, key);

  LeafT* leaf = getLeaf (key);

  if (leaf)
  {
    leaf->getData (pointIdx_data);
    b_success = true;
  }

  return (b_success);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> bool
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::voxelSearch (
    const int index, std::vector<int>& pointIdx_data)
{
  const PointT search_point = this->getPointByIndex (index);
  return (this->voxelSearch (search_point, pointIdx_data));
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> int
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::nearestKSearch (
    const PointT &p_q, int k, std::vector<int> &k_indices, std::vector<float> &k_sqr_distances)
{
  unsigned int i;
  unsigned int resultCount;

  prioPointQueueEntry pointEntry;
  std::vector<prioPointQueueEntry> pointCandidates;

  assert (this->leafCount_>0);

  OctreeKey key;
  key.x = key.y = key.z = 0;

  // initalize smallest point distance in search with high value
  double smallestDist = numeric_limits<double>::max ();

  k_indices.clear ();
  k_sqr_distances.clear ();

  getKNearestNeighborRecursive (p_q, k, this->rootNode_, key, 1, smallestDist, pointCandidates);

  resultCount = pointCandidates.size ();

  for (i = 0; i < resultCount; i++)
  {
    pointEntry = pointCandidates.back ();

    k_indices.push_back (pointEntry.pointIdx_);
    k_sqr_distances.push_back (pointEntry.pointDistance_);

    pointCandidates.pop_back ();
  }

  return k_indices.size ();

}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> int
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::nearestKSearch (
    int index, int k, std::vector<int> &k_indices, std::vector<float> &k_sqr_distances)
{
  const PointT search_point = this->getPointByIndex (index);
  return (nearestKSearch (search_point, k, k_indices, k_sqr_distances));
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> void
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::approxNearestSearch (
    const PointT &p_q, int &result_index, float &sqr_distance)
{
  assert (this->leafCount_>0);

  OctreeKey key;
  key.x = key.y = key.z = 0;

  approxNearestSearchRecursive (p_q, this->rootNode_, key, 1, result_index, sqr_distance);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> void
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::approxNearestSearch (
    int query_index, int &result_index, float &sqr_distance)
{
  const PointT searchPoint = this->getPointByIndex (query_index);

  return (approxNearestSearch (searchPoint, result_index, sqr_distance));
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> int
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::radiusSearch (
    const PointT &p_q, const double radius, std::vector<int> &k_indices, 
    std::vector<float> &k_sqr_distances, int max_nn) const
{
  OctreeKey key;
  key.x = key.y = key.z = 0;

  k_indices.clear ();
  k_sqr_distances.clear ();

  getNeighborsWithinRadiusRecursive (p_q, radius * radius, this->rootNode_, key, 1, k_indices,
                                     k_sqr_distances, max_nn);

  return (k_indices.size ());
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> int
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::radiusSearch (
    int index, const double radius, std::vector<int> &k_indices,
    std::vector<float> &k_sqr_distances, int max_nn) const
{
  const PointT search_point = this->getPointByIndex (index);

  return (radiusSearch (search_point, radius, k_indices, k_sqr_distances, max_nn));
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> double
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::getKNearestNeighborRecursive (
    const PointT & point, unsigned int K, const OctreeBranch* node, const OctreeKey& key,
    unsigned int treeDepth, const double squaredSearchRadius, 
    std::vector<prioPointQueueEntry>& pointCandidates) const
{
  std::vector<prioBranchQueueEntry> searchEntryHeap;
  searchEntryHeap.resize (8);

  unsigned char childIdx;

  OctreeKey newKey;

  double smallestSquaredDist = squaredSearchRadius;

  // get spatial voxel information
  double voxelSquaredDiameter = this->getVoxelSquaredDiameter (treeDepth);

  // iterate over all children
  for (childIdx = 0; childIdx < 8; childIdx++)
  {
    if (branchHasChild (*node, childIdx))
    {
      PointT voxelCenter;

      searchEntryHeap[childIdx].key.x = (key.x << 1) + (!!(childIdx & (1 << 2)));
      searchEntryHeap[childIdx].key.y = (key.y << 1) + (!!(childIdx & (1 << 1)));
      searchEntryHeap[childIdx].key.z = (key.z << 1) + (!!(childIdx & (1 << 0)));

      // generate voxel center point for voxel at key
      genVoxelCenterFromOctreeKey (searchEntryHeap[childIdx].key, treeDepth, voxelCenter);

      // generate new priority queue element
      searchEntryHeap[childIdx].node = getBranchChild (*node, childIdx);
      searchEntryHeap[childIdx].pointDistance = pointSquaredDist (voxelCenter, point);
    }
    else
    {
      searchEntryHeap[childIdx].pointDistance = numeric_limits<double>::infinity ();
    }
  }

  std::sort (searchEntryHeap.begin (), searchEntryHeap.end ());

  // iterate over all children in priority queue
  // check if the distance to seach candidate is smaller than the best point distance (smallestSquaredDist)
  while ((!searchEntryHeap.empty ()) && (searchEntryHeap.back ().pointDistance < smallestSquaredDist
      + voxelSquaredDiameter / 4.0 + sqrt (smallestSquaredDist * voxelSquaredDiameter) - this->epsilon_))
  {
    const OctreeNode* childNode;

    // read from priority queue element
    childNode = searchEntryHeap.back ().node;
    newKey = searchEntryHeap.back ().key;

    if (treeDepth < this->octreeDepth_)
    {
      // we have not reached maximum tree depth
      smallestSquaredDist = getKNearestNeighborRecursive (point, K, (OctreeBranch*)childNode, newKey,
                                                          treeDepth + 1, smallestSquaredDist,
                                                          pointCandidates);
    }
    else
    {
      // we reached leaf node level

      double squaredDist;
      size_t i;
      vector<int> decodedPointVector;

      OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;

      // decode leaf node into decodedPointVector
      childLeaf->getData (decodedPointVector);

      // Linearly iterate over all decoded (unsorted) points
      for (i = 0; i < decodedPointVector.size (); i++)
      {

        const PointT& candidatePoint = this->getPointByIndex (decodedPointVector[i]);

        // calculate point distance to search point
        squaredDist = pointSquaredDist (candidatePoint, point);

        // check if a closer match is found
        if (squaredDist < smallestSquaredDist)
        {
          prioPointQueueEntry pointEntry;

          pointEntry.pointDistance_ = squaredDist;
          pointEntry.pointIdx_ = decodedPointVector[i];
          pointCandidates.push_back (pointEntry);
        }
      }

      std::sort (pointCandidates.begin (), pointCandidates.end ());

      if (pointCandidates.size () > K)
        pointCandidates.resize (K);

      if (pointCandidates.size () == K)
      {
        smallestSquaredDist = pointCandidates.back ().pointDistance_;
      }
    }
    // pop element from priority queue
    searchEntryHeap.pop_back ();
  }

  return (smallestSquaredDist);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> void
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::getNeighborsWithinRadiusRecursive (
    const PointT & point, const double radiusSquared, const OctreeBranch* node,
    const OctreeKey& key, unsigned int treeDepth, std::vector<int>& k_indices,
    std::vector<float>& k_sqr_distances, int max_nn) const
{
  // child iterator
  unsigned char childIdx;

  // get spatial voxel information
  double voxelSquaredDiameter = this->getVoxelSquaredDiameter (treeDepth);

  // iterate over all children
  for (childIdx = 0; childIdx < 8; childIdx++)
  {
    if (!branchHasChild (*node, childIdx))
      continue;

    const OctreeNode* childNode;
    childNode = getBranchChild (*node, childIdx);

    OctreeKey newKey;
    PointT voxelCenter;
    double squaredDist;

    // generate new key for current branch voxel
    newKey.x = (key.x << 1) + (!!(childIdx & (1 << 2)));
    newKey.y = (key.y << 1) + (!!(childIdx & (1 << 1)));
    newKey.z = (key.z << 1) + (!!(childIdx & (1 << 0)));

    // generate voxel center point for voxel at key
    genVoxelCenterFromOctreeKey (newKey, treeDepth, voxelCenter);

    // calculate distance to search point
    squaredDist = pointSquaredDist ((const PointT &)voxelCenter, point);

    // if distance is smaller than search radius
    if (squaredDist + this->epsilon_ <= voxelSquaredDiameter / 4.0 + radiusSquared
        + sqrt (voxelSquaredDiameter * radiusSquared))
    {

      if (treeDepth < this->octreeDepth_)
      {
        // we have not reached maximum tree depth
        getNeighborsWithinRadiusRecursive (point, radiusSquared, (OctreeBranch*)childNode, newKey,
                                           treeDepth + 1, k_indices, k_sqr_distances, max_nn);
        if (k_indices.size () == (unsigned int)max_nn)
          return;
      }
      else
      {
        // we reached leaf node level

        size_t i;
        OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;
        vector<int> decodedPointVector;

        // decode leaf node into decodedPointVector
        childLeaf->getData (decodedPointVector);

        // Linearly iterate over all decoded (unsorted) points
        for (i = 0; i < decodedPointVector.size (); i++)
        {
          const PointT& candidatePoint = this->getPointByIndex (decodedPointVector[i]);

          // calculate point distance to search point
          squaredDist = pointSquaredDist (candidatePoint, point);

          // check if a match is found
          if (squaredDist > radiusSquared)
            continue;

          // add point to result vector
          k_indices.push_back (decodedPointVector[i]);
          k_sqr_distances.push_back (squaredDist);

          if (k_indices.size () == (unsigned int)max_nn)
            return;
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> void
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::approxNearestSearchRecursive (
    const PointT & point, const OctreeBranch* node, const OctreeKey& key, 
    unsigned int treeDepth, int& result_index, float& sqr_distance)
{
  unsigned char childIdx;
  unsigned char minChildIdx;
  double minVoxelCenterDistance;

  OctreeKey minChildKey;
  OctreeKey newKey;

  const OctreeNode* childNode;

  // set minimum voxel distance to maximum value
  minVoxelCenterDistance = numeric_limits<double>::max ();

  minChildIdx = 0xFF;

  // iterate over all children
  for (childIdx = 0; childIdx < 8; childIdx++)
  {
    if (!branchHasChild (*node, childIdx))
      continue;

    PointT voxelCenter;
    double voxelPointDist;

    newKey.x = (key.x << 1) + (!!(childIdx & (1 << 2)));
    newKey.y = (key.y << 1) + (!!(childIdx & (1 << 1)));
    newKey.z = (key.z << 1) + (!!(childIdx & (1 << 0)));

    // generate voxel center point for voxel at key
    genVoxelCenterFromOctreeKey (newKey, treeDepth, voxelCenter);

    voxelPointDist = pointSquaredDist (voxelCenter, point);

    // search for child voxel with shortest distance to search point
    if (voxelPointDist >= minVoxelCenterDistance)
      continue;

    minVoxelCenterDistance = voxelPointDist;
    minChildIdx = childIdx;
    minChildKey = newKey;
  }

  // make sure we found at least one branch child
  assert (minChildIdx<8);

  childNode = getBranchChild (*node, minChildIdx);

  if (treeDepth < this->octreeDepth_)
  {
    // we have not reached maximum tree depth
    approxNearestSearchRecursive (point, (OctreeBranch*)childNode, minChildKey, treeDepth + 1,
                                  result_index, sqr_distance);
  }
  else
  {
    // we reached leaf node level

    double squaredDist;
    double smallestSquaredDist;
    size_t i;
    vector<int> decodedPointVector;

    OctreeLeaf* childLeaf = (OctreeLeaf*)childNode;

    smallestSquaredDist = numeric_limits<double>::max ();

    // decode leaf node into decodedPointVector
    childLeaf->getData (decodedPointVector);

    // Linearly iterate over all decoded (unsorted) points
    for (i = 0; i < decodedPointVector.size (); i++)
    {

      const PointT& candidatePoint = this->getPointByIndex (decodedPointVector[i]);

      // calculate point distance to search point
      squaredDist = pointSquaredDist (candidatePoint, point);

      // check if a closer match is found
      if (squaredDist >= smallestSquaredDist)
        continue;

      result_index = decodedPointVector[i];
      sqr_distance = smallestSquaredDist = squaredDist;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> double
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::pointSquaredDist (
    const PointT & pointA, const PointT & pointB) const
{
  double distX, distY, distZ;

  // distance between pointA and pointB for each axis
  distX = pointA.x - pointB.x;
  distY = pointA.y - pointB.y;
  distZ = pointA.z - pointB.z;

  // return squared absolute distance between pointA and pointB
  return (distX * distX + distY * distY + distZ * distZ);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> int
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::getIntersectedVoxelCenters (
    Eigen::Vector3f origin, Eigen::Vector3f direction, AlignedPointTVector &voxelCenterList) const
{
  OctreeKey key;
  key.x = key.y = key.z = 0;

  voxelCenterList.clear ();
  voxelCenterList.reserve (this->leafCount_);

  // Voxel childIdx remapping
  unsigned char a = 0;

  double minX, minY, minZ, maxX, maxY, maxZ;

  initIntersectedVoxel (origin, direction, minX, minY, minZ, maxX, maxY, maxZ, a);

  if (max (max (minX, minY), minZ) < min (min (maxX, maxY), maxZ))
    return getIntersectedVoxelCentersRecursive (minX, minY, minZ, maxX, maxY, maxZ, a, this->rootNode_, key,
                                                voxelCenterList);

  return (0);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> int
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::getIntersectedVoxelIndices (
    Eigen::Vector3f origin, Eigen::Vector3f direction, std::vector<int> &k_indices) const
{
  OctreeKey key;
  key.x = key.y = key.z = 0;

  k_indices.clear ();
  k_indices.reserve (this->leafCount_);

  // Voxel childIdx remapping
  unsigned char a = 0;
  double minX, minY, minZ, maxX, maxY, maxZ;

  initIntersectedVoxel (origin, direction, minX, minY, minZ, maxX, maxY, maxZ, a);

  if (max (max (minX, minY), minZ) < min (min (maxX, maxY), maxZ))
    return getIntersectedVoxelIndicesRecursive (minX, minY, minZ, maxX, maxY, maxZ, a, this->rootNode_, key,
                                                k_indices);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> int
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::getIntersectedVoxelCentersRecursive (
    double minX, double minY, double minZ,
    double maxX, double maxY, double maxZ,
    unsigned char a, const OctreeNode* node, const OctreeKey& key,
    AlignedPointTVector &voxelCenterList) const
{
  if (maxX < 0.0 || maxY < 0.0 || maxZ < 0.0)
    return (0);

  // If leaf node, get voxel center and increment intersection count
  if (node->getNodeType () == LEAF_NODE)
  {
    PointT newPoint;

    genLeafNodeCenterFromOctreeKey (key, newPoint);

    voxelCenterList.push_back (newPoint);

    return (1);
  }

  // Voxel intersection count for branches children
  int voxelCount = 0;

  // Voxel mid lines
  double midX = 0.5 * (minX + maxX);
  double midY = 0.5 * (minY + maxY);
  double midZ = 0.5 * (minZ + maxZ);

  // First voxel node ray will intersect
  int currNode = getFirstIntersectedNode (minX, minY, minZ, midX, midY, midZ);

  // Child index, node and key
  unsigned char childIdx;
  const OctreeNode *childNode;
  OctreeKey childKey;

  do
  {
    if (currNode != 0)
      childIdx = currNode ^ a;
    else
      childIdx = a;

    // childNode == 0 if childNode doesn't exist
    childNode = getBranchChild ((OctreeBranch&)*node, childIdx);

    // Generate new key for current branch voxel
    childKey.x = (key.x << 1) | (!!(childIdx & (1 << 2)));
    childKey.y = (key.y << 1) | (!!(childIdx & (1 << 1)));
    childKey.z = (key.z << 1) | (!!(childIdx & (1 << 0)));

    // Recursively call each intersected child node, selecting the next
    //   node intersected by the ray.  Children that do not intersect will
    //   not be traversed.

	  switch(currNode)                                                                             
    {                                                                                            
	    case 0:                                                                                    
	      if (childNode)                                                                           
          voxelCount += getIntersectedVoxelCentersRecursive (minX, minY, minZ, midX, midY, midZ, a, 
                       childNode, childKey, voxelCenterList);  
        currNode = getNextIntersectedNode(midX, midY, midZ, 4, 2, 1);                            
	      break;                                                                                   
                                                                                             
	    case 1:                                                                                    
	      if (childNode)                                                                           
          voxelCount += getIntersectedVoxelCentersRecursive (minX, minY, midZ, midX, midY, maxZ, a, 
                       childNode, childKey, voxelCenterList);    
	      currNode = getNextIntersectedNode(midX, midY, maxZ, 5, 3, 8);                            
	      break;                                                                                   
                                                                                             
	    case 2:                                                                                    
	      if (childNode)                                                                           
          voxelCount += getIntersectedVoxelCentersRecursive (minX, midY, minZ, midX, maxY, midZ, a, 
                       childNode, childKey, voxelCenterList);    
	      currNode = getNextIntersectedNode(midX, maxY, midZ, 6, 8, 3);                            
	      break;                                                                                   
                                                                                             
	    case 3:                                                                                    
	      if (childNode)                                                                           
          voxelCount += getIntersectedVoxelCentersRecursive (minX, midY, midZ, midX, maxY, maxZ, a, 
                       childNode, childKey, voxelCenterList);    
	      currNode = getNextIntersectedNode(midX, maxY, maxZ, 7, 8, 8);                            
	      break;                                                                                   
                                                                                             
	    case 4:                                                                                    
	      if (childNode)                                                                           
          voxelCount += getIntersectedVoxelCentersRecursive (midX, minY, minZ, maxX, midY, midZ, a, 
                       childNode, childKey, voxelCenterList);    
	      currNode = getNextIntersectedNode(maxX, midY, midZ, 8, 6, 5);                            
	      break;                                                                                   
                                                                                             
	    case 5:                                                                                  
	      if (childNode)                                                                         
          voxelCount += getIntersectedVoxelCentersRecursive (midX, minY, midZ, maxX, midY, maxZ, a, 
                       childNode, childKey, voxelCenterList);  
	      currNode = getNextIntersectedNode(maxX, midY, maxZ, 8, 7, 8);                          
	      break;                                                                                 
                                                                                              
	    case 6:                                                                                  
	      if (childNode)                                                                         
          voxelCount += getIntersectedVoxelCentersRecursive (midX, midY, minZ, maxX, maxY, midZ, a, 
                       childNode, childKey, voxelCenterList);  
	      currNode = getNextIntersectedNode(maxX, maxY, midZ, 8, 8, 7);                          
	      break;                                                                                 
                                                                                              
	    case 7:                                                                                  
	      if (childNode)                                                                         
          voxelCount += getIntersectedVoxelCentersRecursive (midX, midY, midZ, maxX, maxY, maxZ, a, 
                       childNode, childKey, voxelCenterList);  
	      currNode = 8;                                                                          
	      break;                                                                                 
	    }                                                                                          
	} 
  while (currNode < 8);                                           
  return (voxelCount);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename LeafT, typename OctreeT> int
pcl::octree::OctreePointCloudSearch<PointT, LeafT, OctreeT>::getIntersectedVoxelIndicesRecursive (
    double minX, double minY, double minZ,
    double maxX, double maxY, double maxZ,
    unsigned char a, const OctreeNode* node, const OctreeKey& key,
    std::vector<int> &k_indices) const
{
  if (maxX < 0.0 || maxY < 0.0 || maxZ < 0.0)
    return 0;

  // If leaf node, get voxel center and increment intersection count
  if (node->getNodeType () == LEAF_NODE)
  {
    OctreeLeaf* leaf = (OctreeLeaf*)node;
    vector<int> indices;

    // decode leaf node into decodedPointVector
    leaf->getData (indices);
    for (size_t i = 0; i < indices.size (); i++)
    {
      k_indices.push_back (indices[i]);
    }

    return 1;
  }

  // Voxel intersection count for branches children
  int voxelCount = 0;

  // Voxel mid lines
  double midX = 0.5 * (minX + maxX);
  double midY = 0.5 * (minY + maxY);
  double midZ = 0.5 * (minZ + maxZ);

  // First voxel node ray will intersect
  int currNode = getFirstIntersectedNode (minX, minY, minZ, midX, midY, midZ);

  // Child index, node and key
  unsigned char childIdx;
  const OctreeNode *childNode;
  OctreeKey childKey;
  do
  {
    if (currNode != 0)
      childIdx = currNode ^ a;
    else
      childIdx = a;

    // childNode == 0 if childNode doesn't exist
    childNode = getBranchChild ((OctreeBranch&)*node, childIdx);
    // Generate new key for current branch voxel
    childKey.x = (key.x << 1) | (!!(childIdx & (1 << 2)));
    childKey.y = (key.y << 1) | (!!(childIdx & (1 << 1)));
    childKey.z = (key.z << 1) | (!!(childIdx & (1 << 0)));

    // Recursively call each intersected child node, selecting the next
    //   node intersected by the ray.  Children that do not intersect will
    //   not be traversed.
    switch(currNode)
    {
      case 0:
        if (childNode)
          voxelCount += getIntersectedVoxelIndicesRecursive (minX, minY, minZ, midX, midY, midZ, a,
                                                             childNode, childKey, k_indices);
        currNode = getNextIntersectedNode(midX, midY, midZ, 4, 2, 1);
        break;

      case 1:
        if (childNode)
          voxelCount += getIntersectedVoxelIndicesRecursive (minX, minY, midZ, midX, midY, maxZ, a,
                                                             childNode, childKey, k_indices);
        currNode = getNextIntersectedNode(midX, midY, maxZ, 5, 3, 8);
        break;

      case 2:
        if (childNode)
          voxelCount += getIntersectedVoxelIndicesRecursive (minX, midY, minZ, midX, maxY, midZ, a,
                                                             childNode, childKey, k_indices);
        currNode = getNextIntersectedNode(midX, maxY, midZ, 6, 8, 3);
        break;

      case 3:
        if (childNode)
          voxelCount += getIntersectedVoxelIndicesRecursive (minX, midY, midZ, midX, maxY, maxZ, a,
                                                             childNode, childKey, k_indices);
        currNode = getNextIntersectedNode(midX, maxY, maxZ, 7, 8, 8);
        break;

      case 4:
        if (childNode)
          voxelCount += getIntersectedVoxelIndicesRecursive (midX, minY, minZ, maxX, midY, midZ, a,
                                                             childNode, childKey, k_indices);
        currNode = getNextIntersectedNode(maxX, midY, midZ, 8, 6, 5);
        break;

      case 5:
        if (childNode)
          voxelCount += getIntersectedVoxelIndicesRecursive (midX, minY, midZ, maxX, midY, maxZ, a,
                                                             childNode, childKey, k_indices);
        currNode = getNextIntersectedNode(maxX, midY, maxZ, 8, 7, 8);
        break;

      case 6:
        if (childNode)
          voxelCount += getIntersectedVoxelIndicesRecursive (midX, midY, minZ, maxX, maxY, midZ, a,
                                                             childNode, childKey, k_indices);
        currNode = getNextIntersectedNode(maxX, maxY, midZ, 8, 8, 7);
        break;

      case 7:
        if (childNode)
          voxelCount += getIntersectedVoxelIndicesRecursive (midX, midY, midZ, maxX, maxY, maxZ, a,
                                                             childNode, childKey, k_indices);
        currNode = 8;
        break;
    }
  } while (currNode < 8);

  return voxelCount;
}

#endif    // PCL_OCTREE_SEARCH_IMPL_H_
