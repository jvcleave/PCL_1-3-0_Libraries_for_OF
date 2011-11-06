/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2011, Willow Garage, Inc.
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
 * $Id: elch.hpp 3042 2011-11-01 04:44:47Z svn $
 *
 */

#ifndef PCL_REGISTRATION_IMPL_ELCH_H_
#define PCL_REGISTRATION_IMPL_ELCH_H_

#include <list>
#include <algorithm>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <Eigen/Geometry>

#include <pcl/common/transforms.h>
#include <pcl/registration/registration.h>

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointT> void
pcl::registration::ELCH<PointT>::loopOptimizerAlgorithm (LOAGraph &g, int f, int l, double *weights)
{
  std::list<int> crossings, branches;
  crossings.push_back (f);
  crossings.push_back (l);
  weights[f] = 0;
  weights[l] = 1;

  int *p = new int[num_vertices (g)];
  int *p_min = new int[num_vertices (g)];
  double *d = new double[num_vertices (g)];
  double *d_min = new double[num_vertices (g)];
  double dist;
  bool do_swap = false;
  std::list<int>::iterator crossings_it, end_it, start_min, end_min;

  // process all junctions
  while (!crossings.empty ())
  {
    dist = -1;
    // find shortest crossing for all vertices on the loop
    for (crossings_it = crossings.begin (); crossings_it != crossings.end (); )
    {
      dijkstra_shortest_paths (g, *crossings_it, boost::predecessor_map (p).distance_map (d));
      end_it = crossings_it;
      end_it++;
      // find shortest crossing for one vertex
      for (; end_it != crossings.end (); end_it++)
      {
        if (*end_it != p[*end_it] && (dist < 0 || d[*end_it] < dist))
        {
          dist = d[*end_it];
          start_min = crossings_it;
          end_min = end_it;
          do_swap = true;
        }
      }
      if (do_swap)
      {
        std::swap (p, p_min);
        std::swap (d, d_min);
        do_swap = false;
      }
      // vertex starts a branch
      if (dist < 0)
      {
        branches.push_back (*crossings_it);
        crossings_it = crossings.erase (crossings_it);
      }
      else
        crossings_it++;
    }

    if (dist > -1)
    {
      remove_edge (*end_min, p_min[*end_min], g);
      for (int i = p_min[*end_min]; i != *start_min; i = p_min[i])
      {
        //even right with weights[*start_min] > weights[*end_min]! (math works)
        weights[i] = weights[*start_min] + (weights[*end_min] - weights[*start_min]) * d_min[i] / d_min[*end_min];
        remove_edge (i, p_min[i], g);
        if (degree (i, g) > 0)
        {
          crossings.push_back (i);
        }
      }

      if (degree (*start_min, g) == 0)
        crossings.erase (start_min);

      if (degree (*end_min, g) == 0)
        crossings.erase (end_min);
    }
  }

  delete[] p;
  delete[] p_min;
  delete[] d;
  delete[] d_min;

  boost::graph_traits<LOAGraph>::adjacency_iterator adjacent_it, adjacent_it_end;
  int s;

  // error propagation
  while (!branches.empty ())
  {
    s = branches.front ();
    branches.pop_front ();

    for (tie (adjacent_it, adjacent_it_end) = adjacent_vertices (s, g); adjacent_it != adjacent_it_end; ++adjacent_it)
    {
      weights[*adjacent_it] = weights[s];
      if (degree (*adjacent_it, g) > 1)
        branches.push_back (*adjacent_it);
    }
    clear_vertex (s, g);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointT> bool
pcl::registration::ELCH<PointT>::initCompute ()
{
  if (!PCLBase<PointT>::initCompute ())
  {
    PCL_ERROR ("[pcl::registration:ELCH::initCompute] Init failed.\n");
    return (false);
  }

  if (!loop_start_)
  {
    PCL_ERROR ("[pcl::registration::ELCH::initCompute] no start of loop defined is empty!\n");
    deinitCompute ();
    return (false);
  }

  if (!loop_end_)
  {
    PCL_ERROR ("[pcl::registration::ELCH::initCompute] no end of loop defined is empty!\n");
    deinitCompute ();
    return (false);
  }

  //compute transformation if it's not given
  if (!loop_transform_)
  {
    //TODO use real pose instead of centroid
    Eigen::Vector4f pose_start;
    pcl::compute3DCentroid (*loop_start_, pose_start);

    Eigen::Vector4f pose_end;
    pcl::compute3DCentroid (*loop_end_, pose_end);

    PointCloudPtr tmp (new PointCloud);
    pcl::transformPointCloud (*loop_end_, *tmp, pose_end - pose_start);

    //reg_->setMaximumIterations (50);
    //setMaxCorrespondenceDistance (1.5);
    //setRANSACOutlierRejectionThreshold (1.5);
    //reg_->setRANSACOutlierRejectionThreshold (DBL_MAX);

    reg_->setInputTarget (loop_start_);

    reg_->setInputCloud (tmp);

    reg_->align (*tmp);

    loop_transform_ = reg_->getFinalTransformation ();
    //TODO hack
    //t += ce2;

  }

  return (true);
}

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointT> void
pcl::registration::ELCH<PointT>::compute ()
{
  if (!initCompute ())
  {
    return;
  }

  LOAGraph grb[4];

  typename boost::graph_traits<LoopGraph>::edge_iterator edge_it, edge_it_end;
  for (tie (edge_it, edge_it_end) = edges (loop_graph_); edge_it != edge_it_end; edge_it++)
  {
    for (int j = 0; j < 4; j++)
      add_edge (source (*edge_it, loop_graph_), target (*edge_it, loop_graph_), 1, grb[j]);  //TODO add variance
  }

  double *weights[4];
  for (int i = 0; i < 4; i++)
  {
    weights[i] = new double[num_vertices (loop_graph_)];
    loopOptimizerAlgorithm (grb[i], weights[i]);
  }

  //TODO use pose
  Eigen::Vector4f cend;
  pcl::compute3DCentroid (*loop_end_, cend);
  Eigen::Translation3f tend (cend[0], cend[1], cend[2]);
  Eigen::Affine3f aend (tend);
  Eigen::Affine3f aendI = aend.inverse ();

  //TODO iterate ovr loop_graph_
  typename boost::graph_traits<LoopGraph>::vertex_iterator vertex_it, vertex_it_end;
  for (tie (vertex_it, vertex_it_end) = vertices (loop_graph_); vertex_it != vertex_it_end; vertex_it++)
  {
    Eigen::Vector3f t2;
    //t2[0] = loop_transform_->translation ()[0] * weights[0][i]; //TODO
    //t2[1] = loop_transform_->translation ()[1] * weights[1][i];
    //t2[2] = loop_transform_->translation ()[2] * weights[2][i];

    Eigen::Quaternionf q2;
    //q2 = Eigen::Quaternionf::Identity ().slerp (weights[3][i], q); //TODO

    //TODO use rotation from branch start
    Eigen::Translation3f t3 (t2);
    Eigen::Affine3f a (t3 * q2);
    a = aend * a * aendI;

    //std::cout << "transform cloud " << i << " to:" << std::endl << a.matrix () << std::endl;
    pcl::transformPointCloud (*vertex_it, *vertex_it, a);
  }

  deinitCompute ();
}

#endif // PCL_REGISTRATION_IMPL_ELCH_H_
