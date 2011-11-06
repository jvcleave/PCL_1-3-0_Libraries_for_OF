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
 * $Id: convex_hull.hpp 3031 2011-11-01 04:25:15Z rusu $
 *
 */

#include <pcl/pcl_config.h>
#ifdef HAVE_QHULL

#ifndef PCL_SURFACE_IMPL_CONVEX_HULL_H_
#define PCL_SURFACE_IMPL_CONVEX_HULL_H_

#include "pcl/surface/convex_hull.h"
#include <pcl/common/common.h>
#include <pcl/common/eigen.h>
#include <pcl/common/transforms.h>
#include <pcl/common/io.h>
#include <pcl/kdtree/kdtree.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#ifdef HAVE_QHULL_2011
#  include "libqhull/libqhull.h"
#  include "libqhull/mem.h"
#  include "libqhull/qset.h"
#  include "libqhull/geom.h"
#  include "libqhull/merge.h"
#  include "libqhull/poly.h"
#  include "libqhull/io.h"
#  include "libqhull/stat.h"
#else
#  include "qhull/qhull.h"
#  include "qhull/mem.h"
#  include "qhull/qset.h"
#  include "qhull/geom.h"
#  include "qhull/merge.h"
#  include "qhull/poly.h"
#  include "qhull/io.h"
#  include "qhull/stat.h"
#endif
}

//////////////////////////////////////////////////////////////////////////
template <typename PointInT> void
pcl::ConvexHull<PointInT>::performReconstruction (PointCloud &hull, std::vector<pcl::Vertices> &polygons,
                                                  bool fill_polygon_data)
{
  // FInd the principal directions
  EIGEN_ALIGN16 Eigen::Matrix3f covariance_matrix;
  Eigen::Vector4f xyz_centroid;
  compute3DCentroid (*input_, *indices_, xyz_centroid);
  computeCovarianceMatrix (*input_, *indices_, xyz_centroid, covariance_matrix);
  EIGEN_ALIGN16 Eigen::Vector3f eigen_values;
  EIGEN_ALIGN16 Eigen::Matrix3f eigen_vectors;
  pcl::eigen33 (covariance_matrix, eigen_vectors, eigen_values);

  Eigen::Affine3f transform1;
  transform1.setIdentity ();
  int dim = 3;

  if (eigen_values[0] / eigen_values[2] < 1.0e-5)
  {
    // We have points laying on a plane, using 2d convex hull
    // Compute transformation bring eigen_vectors.col(i) to z-axis

    eigen_vectors.col (2) = eigen_vectors.col (0).cross (eigen_vectors.col (1));
    eigen_vectors.col (1) = eigen_vectors.col (2).cross (eigen_vectors.col (0));

    transform1 (0, 2) = eigen_vectors (0, 0);
    transform1 (1, 2) = eigen_vectors (1, 0);
    transform1 (2, 2) = eigen_vectors (2, 0);

    transform1 (0, 1) = eigen_vectors (0, 1);
    transform1 (1, 1) = eigen_vectors (1, 1);
    transform1 (2, 1) = eigen_vectors (2, 1);
    transform1 (0, 0) = eigen_vectors (0, 2);
    transform1 (1, 0) = eigen_vectors (1, 2);
    transform1 (2, 0) = eigen_vectors (2, 2);

    transform1 = transform1.inverse ();
    dim = 2;
  }
  else
    transform1.setIdentity ();

  PointCloud cloud_transformed;
  pcl::demeanPointCloud (*input_, *indices_, xyz_centroid, cloud_transformed);
  pcl::transformPointCloud (cloud_transformed, cloud_transformed, transform1);

  // True if qhull should free points in qh_freeqhull() or reallocation
  boolT ismalloc = True;
  // output from qh_produce_output(), use NULL to skip qh_produce_output()
  FILE *outfile = NULL;

  std::string flags_str;
  flags_str = "qhull Tc";

  if (compute_area_)
  {
    flags_str.append (" FA");
    outfile = stderr;
  }

  // option flags for qhull, see qh_opt.htm
  //char flags[] = "qhull Tc FA";
  char * flags = (char *)flags_str.c_str();
  // error messages from qhull code
  FILE *errfile = stderr;
  // 0 if no error from qhull
  int exitcode;

  // Array of coordinates for each point
  coordT *points = (coordT *)calloc (cloud_transformed.points.size () * dim, sizeof(coordT));

  for (size_t i = 0; i < cloud_transformed.points.size (); ++i)
  {
    points[i * dim + 0] = (coordT)cloud_transformed.points[i].x;
    points[i * dim + 1] = (coordT)cloud_transformed.points[i].y;

    if (dim > 2)
      points[i * dim + 2] = (coordT)cloud_transformed.points[i].z;
  }

  // Compute convex hull
  exitcode = qh_new_qhull (dim, cloud_transformed.points.size (), points, ismalloc, flags, outfile, errfile);

  if (exitcode != 0)
  {
    PCL_ERROR ("[pcl::%s::performReconstrution] ERROR: qhull was unable to compute a convex hull for the given point cloud (%lu)!\n", getClassName ().c_str (), (unsigned long) input_->points.size ());

    //check if it fails because of NaN values...
    if (!cloud_transformed.is_dense)
    {
      bool NaNvalues = false;
      for (size_t i = 0; i < cloud_transformed.size (); ++i)
      {
        if (!pcl_isfinite (cloud_transformed.points[i].x) || 
            !pcl_isfinite (cloud_transformed.points[i].y) ||
            !pcl_isfinite (cloud_transformed.points[i].z))
        {
          NaNvalues = true;
          break;
        }
      }

      if (NaNvalues)
        PCL_ERROR ("[pcl::%s::performReconstruction] ERROR: point cloud contains NaN values, consider running pcl::PassThrough filter first to remove NaNs!\n", getClassName ().c_str ());
    }

    hull.points.resize (0);
    hull.width = hull.height = 0;
    polygons.resize (0);

    qh_freeqhull (!qh_ALL);
    int curlong, totlong;
    qh_memfreeshort (&curlong, &totlong);

    return;
  }

  qh_triangulate ();

  int num_facets = qh num_facets;

  int num_vertices = qh num_vertices;
  hull.points.resize (num_vertices);

  vertexT * vertex;
  int i = 0;
  // Max vertex id
  int max_vertex_id = -1;
  FORALLvertices
  {
    if ((int)vertex->id > max_vertex_id)
    max_vertex_id = vertex->id;
  }

  ++max_vertex_id;
  std::vector<int> qhid_to_pcidx (max_vertex_id);

  FORALLvertices
  {
    // Add vertices to hull point_cloud
    hull.points[i].x = vertex->point[0];
    hull.points[i].y = vertex->point[1];

    if (dim>2)
    hull.points[i].z = vertex->point[2];
    else
    hull.points[i].z = 0;

    qhid_to_pcidx[vertex->id] = i; // map the vertex id of qhull to the point cloud index
    ++i;
  }

  if (compute_area_)
  {
    total_area_  = qh totarea;
    total_volume_ = qh totvol;
  }

  if (fill_polygon_data)
  {
    if (dim == 3)
    {
      polygons.resize (num_facets);
      int dd = 0;

      facetT * facet;
      FORALLfacets
      {
        polygons[dd].vertices.resize (3);

        // Needed by FOREACHvertex_i_
        int vertex_n, vertex_i;
        FOREACHvertex_i_((*facet).vertices)
        //facet_vertices.vertices.push_back (qhid_to_pcidx[vertex->id]);
        polygons[dd].vertices[vertex_i] = qhid_to_pcidx[vertex->id];
        ++dd;
      }


    }
    else
    {
      // dim=2, we want to return just a polygon with all vertices sorted
      // so that they form a non-intersecting polygon...
      // this can be moved to the upper loop probably and here just
      // the sorting + populate

      Eigen::Vector4f centroid;
      pcl::compute3DCentroid (hull, centroid);
      centroid[3] = 0;
      polygons.resize (1);

      int num_vertices = qh num_vertices, dd = 0;

      // Copy all vertices
      std::vector<std::pair<int, Eigen::Vector4f>, Eigen::aligned_allocator<std::pair<int, Eigen::Vector4f> > > idx_points (num_vertices);

      FORALLvertices
      {
        idx_points[dd].first = qhid_to_pcidx[vertex->id];
        idx_points[dd].second = hull.points[idx_points[dd].first].getVector4fMap () - centroid;
        ++dd;
      }

      // Sort idx_points
      std::sort (idx_points.begin (), idx_points.end (), comparePoints2D);
      polygons[0].vertices.resize (idx_points.size () + 1);

      //Sort also points...
      PointCloud hull_sorted;
      hull_sorted.points.resize (hull.points.size ());

      for (size_t j = 0; j < idx_points.size (); ++j)
      hull_sorted.points[j] = hull.points[idx_points[j].first];

      hull.points = hull_sorted.points;

      // Populate points
      for (size_t j = 0; j < idx_points.size (); ++j)
      polygons[0].vertices[j] = j;

      polygons[0].vertices[idx_points.size ()] = 0;
    }
  }
  else
  {
    if (dim == 2)
    {
      // We want to sort the points
      Eigen::Vector4f centroid;
      pcl::compute3DCentroid (hull, centroid);
      centroid[3] = 0;
      polygons.resize (1);

      int num_vertices = qh num_vertices, dd = 0;

      // Copy all vertices
      std::vector<std::pair<int, Eigen::Vector4f>, Eigen::aligned_allocator<std::pair<int, Eigen::Vector4f> > > idx_points (num_vertices);

      FORALLvertices
      {
        idx_points[dd].first = qhid_to_pcidx[vertex->id];
        idx_points[dd].second = hull.points[idx_points[dd].first].getVector4fMap () - centroid;
        ++dd;
      }

      // Sort idx_points
      std::sort (idx_points.begin (), idx_points.end (), comparePoints2D);

      //Sort also points...
      PointCloud hull_sorted;
      hull_sorted.points.resize (hull.points.size ());

      for (size_t j = 0; j < idx_points.size (); ++j)
      hull_sorted.points[j] = hull.points[idx_points[j].first];

      hull.points = hull_sorted.points;
    }
  }

  // Deallocates memory (also the points)
  qh_freeqhull(!qh_ALL);
  int curlong, totlong;
  qh_memfreeshort (&curlong, &totlong);

  // Rotate the hull point cloud by transform's inverse
  // If the input point cloud has been rotated
  if (dim == 2)
  {
    Eigen::Affine3f transInverse = transform1.inverse ();
    pcl::transformPointCloud (hull, hull, transInverse);

    //for 2D sets, the qhull library delivers the actual area of the 2d hull in the volume
    if(compute_area_)
    {
      total_area_ = total_volume_;
      total_volume_ = 0.0;
    }

  }

  xyz_centroid[0] = -xyz_centroid[0];
  xyz_centroid[1] = -xyz_centroid[1];
  xyz_centroid[2] = -xyz_centroid[2];
  pcl::demeanPointCloud (hull, xyz_centroid, hull);

  //if keep_information_
  if (keep_information_)
  {
    //build a tree with the original points
    pcl::KdTreeFLANN<PointInT> tree (true);
    tree.setInputCloud (input_, indices_);

    std::vector<int> neighbor;
    std::vector<float> distances;
    neighbor.resize (1);
    distances.resize (1);

    //for each point in the convex hull, search for the nearest neighbor

    std::vector<int> indices;
    indices.resize (hull.points.size ());

    for (size_t i = 0; i < hull.points.size (); i++)
    {
      tree.nearestKSearch (hull.points[i], 1, neighbor, distances);
      indices[i] = (*indices_)[neighbor[0]];
    }

    //replace point with the closest neighbor in the original point cloud
    pcl::copyPointCloud (*input_, indices, hull);
  }

  hull.width = hull.points.size ();
  hull.height = 1;
  hull.is_dense = false;
}

//////////////////////////////////////////////////////////////////////////
template <typename PointInT> void
pcl::ConvexHull<PointInT>::reconstruct (PointCloud &output)
{
  output.header = input_->header;
  if (!initCompute ())
  {
    output.points.clear ();
    return;
  }

  // Perform the actual surface reconstruction
  std::vector<pcl::Vertices> polygons;
  performReconstruction (output, polygons, false);

  output.width = output.points.size ();
  output.height = 1;
  output.is_dense = true;

  deinitCompute ();
}

//////////////////////////////////////////////////////////////////////////
template <typename PointInT> void
pcl::ConvexHull<PointInT>::reconstruct (PointCloud &points, std::vector<pcl::Vertices> &polygons)
{
  points.header = input_->header;
  if (!initCompute ())
  {
    points.points.clear ();
    return;
  }

  // Perform the actual surface reconstruction
  performReconstruction (points, polygons, true);

  points.width = points.points.size ();
  points.height = 1;
  points.is_dense = true;

  deinitCompute ();
}

#define PCL_INSTANTIATE_ConvexHull(T) template class PCL_EXPORTS pcl::ConvexHull<T>;

#endif    // PCL_SURFACE_IMPL_CONVEX_HULL_H_
#endif
