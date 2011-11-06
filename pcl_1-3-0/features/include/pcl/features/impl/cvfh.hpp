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
 * $Id: cvfh.hpp 3022 2011-11-01 03:42:22Z rusu $
 *
 */

#ifndef PCL_FEATURES_IMPL_CVFH_H_
#define PCL_FEATURES_IMPL_CVFH_H_

#include "pcl/features/cvfh.h"
#include "pcl/features/pfh.h"

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointInT, typename PointNT, typename PointOutT>
  void
  pcl::CVFHEstimation<PointInT, PointNT, PointOutT>::extractEuclideanClustersSmooth (
                                                                                     const pcl::PointCloud<
                                                                                         pcl::PointNormal> &cloud,
                                                                                     const pcl::PointCloud<
                                                                                         pcl::PointNormal> &normals,
                                                                                     float tolerance,
                                                                                     const pcl::search::Search<pcl::PointNormal>::Ptr &tree,
                                                                                     std::vector<pcl::PointIndices> &clusters,
                                                                                     double eps_angle,
                                                                                     unsigned int min_pts_per_cluster,
                                                                                     unsigned int max_pts_per_cluster)
  {
    if (tree->getInputCloud ()->points.size () != cloud.points.size ())
    {
      PCL_ERROR ("[pcl::extractEuclideanClusters] Tree built for a different point cloud dataset (%lu) than the input cloud (%lu)!\n", (unsigned long)tree->getInputCloud ()->points.size (), (unsigned long)cloud.points.size ());
      return;
    }
    if (cloud.points.size () != normals.points.size ())
    {
      PCL_ERROR ("[pcl::extractEuclideanClusters] Number of points in the input point cloud (%lu) different than normals (%lu)!\n", (unsigned long)cloud.points.size (), (unsigned long)normals.points.size ());
      return;
    }

    // Create a bool vector of processed point indices, and initialize it to false
    std::vector<bool> processed (cloud.points.size (), false);

    std::vector<int> nn_indices;
    std::vector<float> nn_distances;
    // Process all points in the indices vector
    for (size_t i = 0; i < cloud.points.size (); ++i)
    {
      if (processed[i])
        continue;

      std::vector<unsigned int> seed_queue;
      int sq_idx = 0;
      seed_queue.push_back (i);

      processed[i] = true;

      while (sq_idx < (int)seed_queue.size ())
      {
        // Search for sq_idx
        if (!tree->radiusSearch (seed_queue[sq_idx], tolerance, nn_indices, nn_distances))
        {
          sq_idx++;
          continue;
        }

        for (size_t j = 1; j < nn_indices.size (); ++j) // nn_indices[0] should be sq_idx
        {
          if (processed[nn_indices[j]]) // Has this point been processed before ?
            continue;

          //processed[nn_indices[j]] = true;
          // [-1;1]

          double dot_p = normals.points[seed_queue[sq_idx]].normal[0] * normals.points[nn_indices[j]].normal[0]
              + normals.points[seed_queue[sq_idx]].normal[1] * normals.points[nn_indices[j]].normal[1]
              + normals.points[seed_queue[sq_idx]].normal[2] * normals.points[nn_indices[j]].normal[2];

          if (fabs (acos (dot_p)) < eps_angle)
          {
            processed[nn_indices[j]] = true;
            seed_queue.push_back (nn_indices[j]);
          }
        }

        sq_idx++;
      }

      // If this queue is satisfactory, add to the clusters
      if (seed_queue.size () >= min_pts_per_cluster && seed_queue.size () <= max_pts_per_cluster)
      {
        pcl::PointIndices r;
        r.indices.resize (seed_queue.size ());
        for (size_t j = 0; j < seed_queue.size (); ++j)
          r.indices[j] = seed_queue[j];

        std::sort (r.indices.begin (), r.indices.end ());
        r.indices.erase (std::unique (r.indices.begin (), r.indices.end ()), r.indices.end ());

        r.header = cloud.header;
        clusters.push_back (r); // We could avoid a copy by working directly in the vector
      }
    }
  }

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointInT, typename PointNT, typename PointOutT>
  void
  pcl::CVFHEstimation<PointInT, PointNT, PointOutT>::filterNormalsWithHighCurvature (
                                                                                     const pcl::PointCloud<PointNT> & cloud,
                                                                                     std::vector<int> & indices_out,
                                                                                     std::vector<int> & indices_in,
                                                                                     float threshold)
  {
    indices_out.resize (cloud.points.size ());
    indices_in.resize (cloud.points.size ());

    size_t in, out;
    in = out = 0;

    for (size_t i = 0; i < cloud.points.size (); i++)
    {
      if (cloud.points[i].curvature > threshold)
      {
        indices_out[out] = i;
        out++;
      }
      else
      {
        indices_in[in] = i;
        in++;
      }
    }

    indices_out.resize (out);
    indices_in.resize (in);
  }

//////////////////////////////////////////////////////////////////////////////////////////////
template<typename PointInT, typename PointNT, typename PointOutT>
  void
  pcl::CVFHEstimation<PointInT, PointNT, PointOutT>::computeFeature (PointCloudOut &output)
  {
    // Check if input was set
    if (!normals_)
    {
      PCL_ERROR ("[pcl::%s::computeFeature] No input dataset containing normals was given!\n", getClassName ().c_str ());
      output.width = output.height = 0;
      output.points.clear ();
      return;
    }
    if (normals_->points.size () != surface_->points.size ())
    {
      PCL_ERROR ("[pcl::%s::computeFeature] The number of points in the input dataset differs from the number of points in the dataset containing the normals!\n", getClassName ().c_str ());
      output.width = output.height = 0;
      output.points.clear ();
      return;
    }

    centroids_dominant_orientations_.clear ();

    // ---[ Step 0: remove normals with high curvature
    float curv_threshold = curv_threshold_;
    std::vector<int> indices_out;
    std::vector<int> indices_in;
    filterNormalsWithHighCurvature (*normals_, indices_out, indices_in, curv_threshold);

    pcl::PointCloud<pcl::PointNormal>::Ptr filtered (new pcl::PointCloud<pcl::PointNormal> ());

    filtered->width = indices_in.size ();
    filtered->height = 1;
    filtered->points.resize (filtered->width);

    for (size_t i = 0; i < indices_in.size (); ++i)
    {
      filtered->points[i].x = surface_->points[indices_in[i]].x;
      filtered->points[i].y = surface_->points[indices_in[i]].y;
      filtered->points[i].z = surface_->points[indices_in[i]].z;

      filtered->points[i].normal[0] = normals_->points[indices_in[i]].normal[0];
      filtered->points[i].normal[1] = normals_->points[indices_in[i]].normal[1];
      filtered->points[i].normal[2] = normals_->points[indices_in[i]].normal[2];
    }

    // ---[ Step 1a : compute clustering
    pcl::PointCloud<pcl::PointNormal>::Ptr normals_filtered_cloud (new pcl::PointCloud<pcl::PointNormal> ());
    if (indices_in.size () >= 100) //TODO: parameter
    {
      normals_filtered_cloud->width = indices_in.size ();
      normals_filtered_cloud->height = 1;
      normals_filtered_cloud->points.resize (normals_filtered_cloud->width);

      for (size_t i = 0; i < indices_in.size (); ++i)
      {
        normals_filtered_cloud->points[i].x = surface_->points[indices_in[i]].x;
        normals_filtered_cloud->points[i].y = surface_->points[indices_in[i]].y;
        normals_filtered_cloud->points[i].z = surface_->points[indices_in[i]].z;

        normals_filtered_cloud->points[i].normal[0] = normals_->points[indices_in[i]].normal[0];
        normals_filtered_cloud->points[i].normal[1] = normals_->points[indices_in[i]].normal[1];
        normals_filtered_cloud->points[i].normal[2] = normals_->points[indices_in[i]].normal[2];
      }
    }
    else
    {
      normals_filtered_cloud->width = surface_->size ();
      normals_filtered_cloud->height = 1;
      normals_filtered_cloud->points.resize (normals_filtered_cloud->width);

      for (size_t i = 0; i < surface_->size (); ++i)
      {
        normals_filtered_cloud->points[i].x = surface_->points[i].x;
        normals_filtered_cloud->points[i].y = surface_->points[i].y;
        normals_filtered_cloud->points[i].z = surface_->points[i].z;

        normals_filtered_cloud->points[i].normal[0] = normals_->points[i].normal[0];
        normals_filtered_cloud->points[i].normal[1] = normals_->points[i].normal[1];
        normals_filtered_cloud->points[i].normal[2] = normals_->points[i].normal[2];
      }
    }

    //recompute normals normals and use them for clustering!
    KdTreePtr normals_tree_filtered (new pcl::search::KdTree<pcl::PointNormal> (false));
    normals_tree_filtered->setInputCloud (normals_filtered_cloud);

    NormalEstimator n3d;
    n3d.setRadiusSearch (radius_normals_);
    n3d.setSearchMethod (normals_tree_filtered);
    n3d.setInputCloud (normals_filtered_cloud);
    n3d.compute (*normals_filtered_cloud);

    KdTreePtr normals_tree (new pcl::search::KdTree<pcl::PointNormal> (false));
    normals_tree->setInputCloud (normals_filtered_cloud);
    std::vector<pcl::PointIndices> clusters;

    extractEuclideanClustersSmooth (*normals_filtered_cloud, *normals_filtered_cloud, cluster_tolerance_, normals_tree,
                                    clusters, eps_angle_threshold_, min_points_);

    /*clusters_colored_.reset (new pcl::PointCloud<pcl::PointXYZINormal> ());
    clusters_colored_->points.resize(normals_filtered_cloud->points.size());
    clusters_colored_->width = clusters_colored_->points.size();
    clusters_colored_->height = 1;

    float intensity = 0.1;
    int n_points=0;
    for (size_t i = 0; i < clusters.size (); ++i) //for each cluster
    {
      for (size_t j = 0; j < clusters[i].indices.size (); j++)
      {
        clusters_colored_->points[n_points].intensity = intensity;
        clusters_colored_->points[n_points].getVector4fMap() = normals_filtered_cloud->points[clusters[i].indices[j]].getVector4fMap();
        clusters_colored_->points[n_points].getNormalVector4fMap() = normals_filtered_cloud->points[clusters[i].indices[j]].getNormalVector4fMap();
        clusters_colored_->points[n_points].curvature = normals_filtered_cloud->points[clusters[i].indices[j]].curvature;

        n_points++;
      }

      intensity += 0.1;
    }

    clusters_colored_->points.resize(n_points);
    clusters_colored_->width = n_points;*/

    std::vector<Eigen::Vector3f> dominant_normals;

    VFHEstimator vfh;
    vfh.setInputCloud (surface_);
    vfh.setInputNormals (normals_);
    vfh.setSearchMethod (this->tree_);
    vfh.setUseGivenNormal (true);
    vfh.setUseGivenCentroid (true);
    vfh.setNormalizeBins (normalize_bins_);
    vfh.setNormalizeDistance (true);
    vfh.setFillSizeComponent (true);
    output.height = 1;

    // ---[ Step 1b : check if any dominant cluster was found
    if (clusters.size () > 0)
    { // ---[ Step 1b.1 : If yes, compute CVFH using the cluster information

      for (size_t i = 0; i < clusters.size (); ++i) //for each cluster
      {
        Eigen::Vector4f avg_normal = Eigen::Vector4f::Zero ();
        Eigen::Vector4f avg_centroid = Eigen::Vector4f::Zero ();

        for (size_t j = 0; j < clusters[i].indices.size (); j++)
        {
          avg_normal += normals_filtered_cloud->points[clusters[i].indices[j]].getNormalVector4fMap ();
          avg_centroid += normals_filtered_cloud->points[clusters[i].indices[j]].getVector4fMap ();
        }

        avg_normal /= clusters[i].indices.size ();
        avg_centroid /= clusters[i].indices.size ();

        Eigen::Vector4f centroid_test;
        pcl::compute3DCentroid (*normals_filtered_cloud, centroid_test);
        avg_normal.normalize ();

        Eigen::Vector3f avg_norm (avg_normal[0], avg_normal[1], avg_normal[2]);
        Eigen::Vector3f avg_dominant_centroid (avg_centroid[0], avg_centroid[1], avg_centroid[2]);

        //append normal and centroid for the clusters
        dominant_normals.push_back (avg_norm);
        centroids_dominant_orientations_.push_back (avg_dominant_centroid);
      }

      //compute modified VFH for all dominant clusters and add them to the list!
      output.points.resize (dominant_normals.size ());
      output.width = dominant_normals.size ();

      for (size_t i = 0; i < dominant_normals.size (); ++i)
      {
        //configure VFH computation for CVFH
        vfh.setNormalToUse (dominant_normals[i]);
        vfh.setCentroidToUse (centroids_dominant_orientations_[i]);
        pcl::PointCloud<pcl::VFHSignature308> vfh_signature;
        vfh.compute (vfh_signature);
        output.points[i] = vfh_signature.points[0];
      }
    }
    else
    { // ---[ Step 1b.1 : If no, compute CVFH using all the object points
      Eigen::Vector4f avg_centroid;
      pcl::compute3DCentroid (*surface_, avg_centroid);
      Eigen::Vector3f cloud_centroid (avg_centroid[0], avg_centroid[1], avg_centroid[2]);
      centroids_dominant_orientations_.push_back (cloud_centroid);

      //configure VFH computation for CVFH using all object points
      vfh.setCentroidToUse (cloud_centroid);
      vfh.setUseGivenNormal (false);

      pcl::PointCloud<pcl::VFHSignature308> vfh_signature;
      vfh.compute (vfh_signature);

      output.points.resize (1);
      output.width = 1;

      output.points[0] = vfh_signature.points[0];
    }
  }

#define PCL_INSTANTIATE_CVFHEstimation(T,NT,OutT) template class PCL_EXPORTS pcl::CVFHEstimation<T,NT,OutT>;

#endif    // PCL_FEATURES_IMPL_VFH_H_ 
