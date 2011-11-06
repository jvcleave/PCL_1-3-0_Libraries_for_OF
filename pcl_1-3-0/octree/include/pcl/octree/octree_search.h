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
 * $Id: octree_search.h 3018 2011-11-01 03:24:12Z svn $
 */

#ifndef PCL_OCTREE_SEARCH_H_
#define PCL_OCTREE_SEARCH_H_

#include "octree_pointcloud.h"

#include "octree_base.h"
#include "octree2buf_base.h"

#include "octree_nodes.h"

namespace pcl
{
  namespace octree
  {
    /** \brief @b Octree pointcloud search class
     *  \note This class provides several methods for spatial neighbor search based on octree structure
     *  \note typename: PointT: type of point used in pointcloud
     *  \ingroup octree
     *  \author Julius Kammerl (julius@kammerl.de)
     */
    template<typename PointT, typename LeafT = OctreeLeafDataTVector<int> , typename OctreeT = OctreeBase<int, LeafT> >
    class OctreePointCloudSearch : public OctreePointCloud<PointT, LeafT, OctreeT>
    {
      public:
        // public typedefs
        typedef boost::shared_ptr<std::vector<int> > IndicesPtr;
        typedef boost::shared_ptr<const std::vector<int> > IndicesConstPtr;

        typedef pcl::PointCloud<PointT> PointCloud;
        typedef boost::shared_ptr<PointCloud> PointCloudPtr;
        typedef boost::shared_ptr<const PointCloud> PointCloudConstPtr;

        // public typedefs for single/double buffering
        typedef OctreePointCloudSearch<PointT, LeafT, OctreeBase<int, LeafT> > SingleBuffer;
        typedef OctreePointCloudSearch<PointT, LeafT, Octree2BufBase<int, LeafT> > DoubleBuffer;
        typedef OctreePointCloudSearch<PointT, LeafT, OctreeLowMemBase<int, LeafT> > LowMem;

        // Boost shared pointers
        typedef boost::shared_ptr<OctreePointCloudSearch<PointT, LeafT, OctreeT> > Ptr;
        typedef boost::shared_ptr<const OctreePointCloudSearch<PointT, LeafT, OctreeT> > ConstPtr;

        // Eigen aligned allocator
        typedef std::vector<PointT, Eigen::aligned_allocator<PointT> > AlignedPointTVector;

        typedef typename OctreeT::OctreeBranch OctreeBranch;
        typedef typename OctreeT::OctreeKey OctreeKey;
        typedef typename OctreeT::OctreeLeaf OctreeLeaf;

        /** \brief Constructor.
          *  \param resolution: octree resolution at lowest octree level
          */
        OctreePointCloudSearch (const double resolution) :
          OctreePointCloud<PointT, LeafT, OctreeT> (resolution)
        {
        }

        /** \brief Empty class constructor. */
        virtual
        ~OctreePointCloudSearch ()
        {
        }

        /** \brief Search for neighbors within a voxel at given point
          * \param[in] point point addressing a leaf node voxel
          * \param[out] pointIdx_data the resultant indices of the neighboring voxel points
          * \return "true" if leaf node exist; "false" otherwise
          */
        bool
        voxelSearch (const PointT& point, std::vector<int>& pointIdx_data);

        /** \brief Search for neighbors within a voxel at given point referenced by a point index
          * \param[in] index the index in input cloud defining the query point
          * \param[out] pointIdx_data the resultant indices of the neighboring voxel points
          * \return "true" if leaf node exist; "false" otherwise
          */
        bool
        voxelSearch (const int index, std::vector<int>& pointIdx_data);

        /** \brief Search for k-nearest neighbors at the query point.
         * \param cloud the point cloud data
         * \param index the index in \a cloud representing the query point
         * \param k the number of neighbors to search for
         * \param k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
         * \param k_sqr_distances the resultant squared distances to the neighboring points (must be resized to \a k
         * a priori!)
         * \return number of neighbors found
         */
        inline int
        nearestKSearch (const PointCloud &cloud, int index, int k, std::vector<int> &k_indices,
                        std::vector<float> &k_sqr_distances)
        {
          return (nearestKSearch (cloud[index], k, k_indices, k_sqr_distances));
        }

        /** \brief Search for k-nearest neighbors at given query point.
         * @param p_q the given query point
         * @param k the number of neighbors to search for
         * @param k_indices the resultant indices of the neighboring points (must be resized to k a priori!)
         * @param k_sqr_distances  the resultant squared distances to the neighboring points (must be resized to k a priori!)
         * @return number of neighbors found
         */
        int
        nearestKSearch (const PointT &p_q, int k, std::vector<int> &k_indices,
                        std::vector<float> &k_sqr_distances);

        /** \brief Search for k-nearest neighbors at query point
         * \param index index representing the query point in the dataset given by \a setInputCloud.
         *        If indices were given in setInputCloud, index will be the position in the indices vector.
         * \param k the number of neighbors to search for
         * \param k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
         * \param k_sqr_distances the resultant squared distances to the neighboring points (must be resized to \a k
         * a priori!)
         * \return number of neighbors found
         */
        int
        nearestKSearch (int index, int k, std::vector<int> &k_indices,
                        std::vector<float> &k_sqr_distances);

        /** \brief Search for approx. nearest neighbor at the query point.
         * \param cloud the point cloud data
         * \param query_index the index in \a cloud representing the query point
         * \param result_index the resultant index of the neighbor point
         * \param sqr_distance the resultant squared distance to the neighboring point
         * \return number of neighbors found
         */
        inline void
        approxNearestSearch (const PointCloud &cloud, int query_index, int &result_index,
                             float &sqr_distance)
        {
          return (approxNearestSearch (cloud.points[query_index], result_index, sqr_distance));
        }

        /** \brief Search for approx. nearest neighbor at the query point.
         * @param p_q the given query point
         * \param result_index the resultant index of the neighbor point
         * \param sqr_distance the resultant squared distance to the neighboring point
         */
        void
        approxNearestSearch (const PointT &p_q, int &result_index, float &sqr_distance);

        /** \brief Search for approx. nearest neighbor at the query point.
         * \param query_index index representing the query point in the dataset given by \a setInputCloud.
         *        If indices were given in setInputCloud, index will be the position in the indices vector.
         * \param result_index the resultant index of the neighbor point
         * \param sqr_distance the resultant squared distance to the neighboring point
         * \return number of neighbors found
         */
        void
        approxNearestSearch (int query_index, int &result_index, float &sqr_distance);

        /** \brief Search for all neighbors of query point that are within a given radius.
         * \param cloud the point cloud data
         * \param index the index in \a cloud representing the query point
         * \param radius the radius of the sphere bounding all of p_q's neighbors
         * \param k_indices the resultant indices of the neighboring points
         * \param k_sqr_distances the resultant squared distances to the neighboring points
         * \param max_nn if given, bounds the maximum returned neighbors to this value
         * \return number of neighbors found in radius
         */
        int
        radiusSearch (const PointCloud &cloud, int index, double radius,
                      std::vector<int> &k_indices, std::vector<float> &k_sqr_distances,
                      int max_nn = INT_MAX)
        {
          return (radiusSearch (cloud.points[index], radius, k_indices, k_sqr_distances, max_nn));
        }

        /** \brief Search for all neighbors of query point that are within a given radius.
         * \param p_q the given query point
         * \param radius the radius of the sphere bounding all of p_q's neighbors
         * \param k_indices the resultant indices of the neighboring points
         * \param k_sqr_distances the resultant squared distances to the neighboring points
         * \param max_nn if given, bounds the maximum returned neighbors to this value
         * \return number of neighbors found in radius
         */
        int
        radiusSearch (const PointT &p_q, const double radius, std::vector<int> &k_indices,
                      std::vector<float> &k_sqr_distances, int max_nn = INT_MAX) const;

        /** \brief Search for all neighbors of query point that are within a given radius.
         * \param index index representing the query point in the dataset given by \a setInputCloud.
         *        If indices were given in setInputCloud, index will be the position in the indices vector
         * \param radius radius of the sphere bounding all of p_q's neighbors
         * \param k_indices the resultant indices of the neighboring points
         * \param k_sqr_distances the resultant squared distances to the neighboring points
         * \param max_nn if given, bounds the maximum returned neighbors to this value
         * \return number of neighbors found in radius
         */
        int
        radiusSearch (int index, const double radius, std::vector<int> &k_indices,
                      std::vector<float> &k_sqr_distances, int max_nn = INT_MAX) const;

        /** \brief Get a PointT vector of centers of all voxels that intersected by a ray (origin, direction).
          * \param[in] origin ray origin
          * \param[in] direction ray direction vector
          * \param[out] voxelCenterList results are written to this vector of PointT elements
          * \return number of intersected voxels
          */
        int
        getIntersectedVoxelCenters (Eigen::Vector3f origin, Eigen::Vector3f direction,
                                    AlignedPointTVector &voxelCenterList) const;

        /** \brief Get indices of all voxels that are intersected by a ray (origin, direction).
          * \param[in] origin ray origin
          * \param[in] direction ray direction vector
          * \param[out] k_indices resulting indices
          * \return number of intersected voxels
          */
        int
        getIntersectedVoxelIndices (Eigen::Vector3f origin, Eigen::Vector3f direction,
                                    std::vector<int> &k_indices) const;


      protected:

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Octree-based search routines & helpers
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /** \brief @b Priority queue entry for branch nodes
         *  \note This class defines priority queue entries for the nearest neighbor search.
         *  \author Julius Kammerl (julius@kammerl.de)
         */
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        class prioBranchQueueEntry
        {
        public:

          /** \brief Empty constructor  */
          prioBranchQueueEntry ()
          {
          }

          /** \brief Constructor for initializing priority queue entry.
           * \param node pointer to octree node
           * \param key octree key addressing voxel in octree structure
           * \param pointDistance distance of query point to voxel center
           * */
          prioBranchQueueEntry (OctreeNode* node, OctreeKey& key, double pointDistance)
          {
            node = node;
            pointDistance = pointDistance;
            key = key;
          }

          /** \brief Operator< for comparing priority queue entries with each other.  */
          bool
          operator< (const prioBranchQueueEntry rhs) const
          {
            return (this->pointDistance > rhs.pointDistance);
          }

          // pointer to octree node
          const OctreeNode* node;

          // distance to query point
          double pointDistance;

          // octree key
          OctreeKey key;

        };

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /** \brief @b Priority queue entry for point candidates
         *  \note This class defines priority queue entries for the nearest neighbor point candidates.
         *  \author Julius Kammerl (julius@kammerl.de)
         */
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        class prioPointQueueEntry
        {
        public:

          /** \brief Empty constructor  */
          prioPointQueueEntry ()
          {
          }

          /** \brief Constructor for initializing priority queue entry.
           * \param pointIdx an index representing a point in the dataset given by \a setInputCloud
           * \param pointDistance distance of query point to voxel center
           * */
          prioPointQueueEntry (unsigned int& pointIdx, double pointDistance)
          {
            pointIdx_ = pointIdx;
            pointDistance_ = pointDistance;
          }

          /** \brief Operator< for comparing priority queue entries with each other.  */
          bool
          operator< (const prioPointQueueEntry& rhs) const
          {
            return (this->pointDistance_ < rhs.pointDistance_);
          }

          // index representing a point in the dataset given by \a setInputCloud
          int pointIdx_;

          // distance to query point
          double pointDistance_;

        };

        /** \brief Helper function to calculate the squared distance between two points
         * \param pointA point A
         * \param pointB point B
         * \return squared distance between point A and point B
         */
        double
        pointSquaredDist (const PointT & pointA, const PointT & pointB) const;

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Recursive search routine methods
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        /** \brief Recursive search method that explores the octree and finds neighbors within a given radius
         * \param point query point
         * \param radiusSquared squared search radius
         * \param node current octree node to be explored
         * \param key octree key addressing a leaf node.
         * \param treeDepth current depth/level in the octree
         * \param k_indices vector of indices found to be neighbors of query point
         * \param k_sqr_distances squared distances of neighbors to query point
         * \param max_nn maximum of neighbors to be found
         */
        void
        getNeighborsWithinRadiusRecursive (const PointT & point, const double radiusSquared,
                                           const OctreeBranch* node, const OctreeKey& key,
                                           unsigned int treeDepth, std::vector<int>& k_indices,
                                           std::vector<float>& k_sqr_distances, int max_nn) const;

        /** \brief Recursive search method that explores the octree and finds the K nearest neighbors
         * \param point query point
         * \param K amount of nearest neighbors to be found
         * \param node current octree node to be explored
         * \param key octree key addressing a leaf node.
         * \param treeDepth current depth/level in the octree
         * \param squaredSearchRadius squared search radius distance
         * \param pointCandidates priority queue of nearest neigbor point candidates
         * \return squared search radius based on current point candidate set found
         */
        double
        getKNearestNeighborRecursive (const PointT & point, unsigned int K, const OctreeBranch* node,
                                      const OctreeKey& key, unsigned int treeDepth,
                                      const double squaredSearchRadius,
                                      std::vector<prioPointQueueEntry>& pointCandidates) const;

        /** \brief Recursive search method that explores the octree and finds the approximate nearest neighbor
         * \param point query point
         * \param node current octree node to be explored
         * \param key octree key addressing a leaf node.
         * \param treeDepth current depth/level in the octree
         * \param result_index result index is written to this reference
         * \param sqr_distance squared distance to search
         */
        void
        approxNearestSearchRecursive (const PointT & point, const OctreeBranch* node, const OctreeKey& key,
                                      unsigned int treeDepth, int& result_index, float& sqr_distance);

        /** \brief Recursively search the tree for all intersected leaf nodes and return a vector of voxel centers.
         * This algorithm is based off the paper An Efficient Parametric Algorithm for Octree Traversal:
         * http://wscg.zcu.cz/wscg2000/Papers_2000/X31.pdf
         * \param minX octree nodes X coordinate of lower bounding box corner
         * \param minY octree nodes Y coordinate of lower bounding box corner
         * \param minZ octree nodes Z coordinate of lower bounding box corner
         * \param maxX octree nodes X coordinate of upper bounding box corner
         * \param maxY octree nodes Y coordinate of upper bounding box corner
         * \param maxZ octree nodes Z coordinate of upper bounding box corner
         * \param a
         * \param node current octree node to be explored
         * \param key octree key addressing a leaf node.
         * \param voxelCenterList results are written to this vector of PointT elements
         * \return number of voxels found
         */
        int
        getIntersectedVoxelCentersRecursive (double minX, double minY, double minZ, double maxX, double maxY,
                                             double maxZ, unsigned char a, const OctreeNode* node,
                                             const OctreeKey& key, AlignedPointTVector &voxelCenterList) const;

        /** \brief Recursively search the tree for all intersected leaf nodes and return a vector of indices.
          * This algorithm is based off the paper An Efficient Parametric Algorithm for Octree Traversal:
          * http://wscg.zcu.cz/wscg2000/Papers_2000/X31.pdf
          * \param minX octree nodes X coordinate of lower bounding box corner
          * \param minY octree nodes Y coordinate of lower bounding box corner
          * \param minZ octree nodes Z coordinate of lower bounding box corner
          * \param maxX octree nodes X coordinate of upper bounding box corner
          * \param maxY octree nodes Y coordinate of upper bounding box corner
          * \param maxZ octree nodes Z coordinate of upper bounding box corner
          * \param a
          * \param node current octree node to be explored
          * \param key octree key addressing a leaf node.
          * \param k_indices resulting indices
          * \return number of voxels found
          */
        int
        getIntersectedVoxelIndicesRecursive (double minX, double minY, double minZ,
                                             double maxX, double maxY, double maxZ,
                                             unsigned char a, const OctreeNode* node, const OctreeKey& key,
                                             std::vector<int> &k_indices) const;
        /** \brief Initialize raytracing algorithm
          * \param minX octree nodes X coordinate of lower bounding box corner
          * \param minY octree nodes Y coordinate of lower bounding box corner
          * \param minZ octree nodes Z coordinate of lower bounding box corner
          * \param maxX octree nodes X coordinate of upper bounding box corner
          * \param maxY octree nodes Y coordinate of upper bounding box corner
          * \param maxZ octree nodes Z coordinate of upper bounding box corner
          * \param a
          */
        inline void
        initIntersectedVoxel (Eigen::Vector3f &origin, Eigen::Vector3f &direction,
                              double &minX, double &minY, double &minZ,
                              double &maxX, double &maxY, double &maxZ,
                              unsigned char &a) const
        {
          // Account for division by zero when direction vector is 0.0
          const double epsilon = 1e-10;
          if (direction.x () == 0.0)
            direction.x () = epsilon;
          if (direction.y () == 0.0)
            direction.y () = epsilon;
          if (direction.z () == 0.0)
            direction.z () = epsilon;

          // Voxel childIdx remapping
          a = 0;

          // Handle negative axis direction vector
          if (direction.x () < 0.0)
          {
            origin.x () = this->minX_ + this->maxX_ - origin.x ();
            direction.x () = -direction.x ();
            a |= 4;
          }
          if (direction.y () < 0.0)
          {
            origin.y () = this->minY_ + this->maxY_ - origin.y ();
            direction.y () = -direction.y ();
            a |= 2;
          }
          if (direction.z () < 0.0)
          {
            origin.z () = this->minZ_ + this->maxZ_ - origin.z ();
            direction.z () = -direction.z ();
            a |= 1;
          }
          minX = (this->minX_ - origin.x ()) / direction.x ();
          maxX = (this->maxX_ - origin.x ()) / direction.x ();
          minY = (this->minY_ - origin.y ()) / direction.y ();
          maxY = (this->maxY_ - origin.y ()) / direction.y ();
          minZ = (this->minZ_ - origin.z ()) / direction.z ();
          maxZ = (this->maxZ_ - origin.z ()) / direction.z ();
        }

        /** \brief Find first child node ray will enter
         * \param minX octree nodes X coordinate of lower bounding box corner
         * \param minY octree nodes Y coordinate of lower bounding box corner
         * \param minZ octree nodes Z coordinate of lower bounding box corner
         * \param midX octree nodes X coordinate of bounding box mid line
         * \param midY octree nodes Y coordinate of bounding box mid line
         * \param midZ octree nodes Z coordinate of bounding box mid line
         * \return the first child node ray will enter
         */
        inline int
        getFirstIntersectedNode (double minX, double minY, double minZ, double midX, double midY, double midZ) const
        {
          int currNode = 0;

          if (minX > minY)
          {
            if (minX > minZ)
            {
              // max(minX, minY, minZ) is minX. Entry plane is YZ.
              if (midY < minX)
                currNode |= 2;
              if (midZ < minX)
                currNode |= 1;
            }
            else
            {
              // max(minX, minY, minZ) is minZ. Entry plane is XY.
              if (midX < minZ)
                currNode |= 4;
              if (midY < minZ)
                currNode |= 2;
            }
          }
          else
          {
            if (minY > minZ)
            {
              // max(minX, minY, minZ) is minY. Entry plane is XZ.
              if (midX < minY)
                currNode |= 4;
              if (midZ < minY)
                currNode |= 1;
            }
            else
            {
              // max(minX, minY, minZ) is minZ. Entry plane is XY.
              if (midX < minZ)
                currNode |= 4;
              if (midY < minZ)
                currNode |= 2;
            }
          }

          return currNode;
        }

        /** \brief Get the next visited node given the current node upper
         *   bounding box corner. This function accepts three float values, and
         *   three int values. The function returns the ith integer where the
         *   ith float value is the minimum of the three float values.
         * \param x current nodes X coordinate of upper bounding box corner
         * \param y current nodes Y coordinate of upper bounding box corner
         * \param z current nodes Z coordinate of upper bounding box corner
         * \param a next node if exit Plane YZ
         * \param b next node if exit Plane XZ
         * \param c next node if exit Plane XY
         * \return the next child node ray will enter or 8 if exiting
         */
        inline int
        getNextIntersectedNode (double x, double y, double z, int a, int b, int c) const
        {
          if (x < y)
          {
            if (x < z)
              return a;
            else
              return c;
          }
          else
          {
            if (y < z)
              return b;
            else
              return c;
          }

          return 0;
        }

      };
  }
}

#define PCL_INSTANTIATE_OctreePointCloudSearch(T) template class PCL_EXPORTS pcl::octree::OctreePointCloudSearch<T>;

#endif    // PCL_OCTREE_SEARCH_H_
