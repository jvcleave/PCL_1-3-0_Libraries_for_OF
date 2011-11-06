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
 * $Id: normal_3d.h 2631 2011-10-05 00:27:56Z mdixon $
 *
 */

#ifndef PCL_NORMAL_3D_H_
#define PCL_NORMAL_3D_H_

#include <pcl/features/feature.h>

namespace pcl
{
  /** \brief Compute the Least-Squares plane fit for a given set of points, and return the estimated plane
    * parameters together with the surface curvature.
    * \param cloud the input point cloud
    * \param plane_parameters the plane parameters as: a, b, c, d (ax + by + cz + d = 0)
    * \param curvature the estimated surface curvature as a measure of
    * \f[
    * \lambda_0 / (\lambda_0 + \lambda_1 + \lambda_2)
    * \f]
    * \ingroup features
    */
  template <typename PointT> inline void 
  computePointNormal (const pcl::PointCloud<PointT> &cloud, 
                      Eigen::Vector4f &plane_parameters, float &curvature)
  {
    if (cloud.points.empty ())
    {
      plane_parameters.setConstant (std::numeric_limits<float>::quiet_NaN ());
      curvature = std::numeric_limits<float>::quiet_NaN ();
      return;
    }
    // Placeholder for the 3x3 covariance matrix at each surface patch
    EIGEN_ALIGN16 Eigen::Matrix3f covariance_matrix;
    // 16-bytes aligned placeholder for the XYZ centroid of a surface patch
    Eigen::Vector4f xyz_centroid;

    // Estimate the XYZ centroid
    compute3DCentroid (cloud, xyz_centroid);

    // Compute the 3x3 covariance matrix
    computeCovarianceMatrix (cloud, xyz_centroid, covariance_matrix);

    // Get the plane normal and surface curvature
    solvePlaneParameters (covariance_matrix, xyz_centroid, plane_parameters, curvature);
  }

  /** \brief Compute the Least-Squares plane fit for a given set of points, using their indices,
    * and return the estimated plane parameters together with the surface curvature.
    * \param cloud the input point cloud
    * \param indices the point cloud indices that need to be used
    * \param plane_parameters the plane parameters as: a, b, c, d (ax + by + cz + d = 0)
    * \param curvature the estimated surface curvature as a measure of
    * \f[
    * \lambda_0 / (\lambda_0 + \lambda_1 + \lambda_2)
    * \f]
    * \ingroup features
    */
  template <typename PointT> inline void 
  computePointNormal (const pcl::PointCloud<PointT> &cloud, const std::vector<int> &indices, 
                      Eigen::Vector4f &plane_parameters, float &curvature)
  {
    if (indices.empty ())
    {
      plane_parameters.setConstant (std::numeric_limits<float>::quiet_NaN ());
      curvature = std::numeric_limits<float>::quiet_NaN ();
      return;
    }
    // Placeholder for the 3x3 covariance matrix at each surface patch
    EIGEN_ALIGN16 Eigen::Matrix3f covariance_matrix;
    // 16-bytes aligned placeholder for the XYZ centroid of a surface patch
    Eigen::Vector4f xyz_centroid;

    // Estimate the XYZ centroid
    compute3DCentroid (cloud, indices, xyz_centroid);

    // Compute the 3x3 covariance matrix
    computeCovarianceMatrix (cloud, indices, xyz_centroid, covariance_matrix);

    // Get the plane normal and surface curvature
    solvePlaneParameters (covariance_matrix, xyz_centroid, plane_parameters, curvature);
  }

  /** \brief Flip (in place) the estimated normal of a point towards a given viewpoint
    * \param point a given point
    * \param vp_x the X coordinate of the viewpoint
    * \param vp_y the X coordinate of the viewpoint
    * \param vp_z the X coordinate of the viewpoint
    * \param normal the plane normal to be flipped
    * \ingroup features
    */
  template <typename PointT> inline void 
  flipNormalTowardsViewpoint (const PointT &point, float vp_x, float vp_y, float vp_z, 
                              Eigen::Vector4f &normal)
  {
    Eigen::Vector4f vp (vp_x, vp_y, vp_z, 0);
    // See if we need to flip any plane normals
    vp -= point.getVector4fMap ();
    vp[3] = 0;  // enforce the last coordinate

    // Dot product between the (viewpoint - point) and the plane normal
    float cos_theta = vp.dot (normal);

    // Flip the plane normal
    if (cos_theta < 0)
    {
      normal *= -1;
      normal[3] = 0;  // enforce the last coordinate;
      // Hessian form (D = nc . p_plane (centroid here) + p)
      normal[3] = -1 * normal.dot (point.getVector4fMap ());
    }
  }

  /** \brief Flip (in place) the estimated normal of a point towards a given viewpoint
    * \param point a given point
    * \param vp_x the X coordinate of the viewpoint
    * \param vp_y the X coordinate of the viewpoint
    * \param vp_z the X coordinate of the viewpoint
    * \param nx the resultant X component of the plane normal
    * \param ny the resultant Y component of the plane normal
    * \param nz the resultant Z component of the plane normal
    * \ingroup features
    */
  template <typename PointT> inline void 
  flipNormalTowardsViewpoint (const PointT &point, float vp_x, float vp_y, float vp_z, 
                              float &nx, float &ny, float &nz)
  {
    // See if we need to flip any plane normals
    vp_x -= point.x;
    vp_y -= point.y;
    vp_z -= point.z;

    // Dot product between the (viewpoint - point) and the plane normal
    float cos_theta = (vp_x * nx + vp_y * ny + vp_z * nz);

    // Flip the plane normal
    if (cos_theta < 0)
    {
      nx *= -1;
      ny *= -1;
      nz *= -1;
    }
  }

  /** \brief @b NormalEstimation estimates local surface properties at each 3D point, such as surface normals and
    * curvatures.
    *
    * @note The code is stateful as we do not expect this class to be multicore parallelized. Please look at
    * \ref NormalEstimationOMP for a parallel implementation.
    * \author Radu Bogdan Rusu
    * \ingroup features
    */
  template <typename PointInT, typename PointOutT>
  class NormalEstimation: public Feature<PointInT, PointOutT>
  {
    public:
      using Feature<PointInT, PointOutT>::feature_name_;
      using Feature<PointInT, PointOutT>::getClassName;
      using Feature<PointInT, PointOutT>::indices_;
      using Feature<PointInT, PointOutT>::input_;
      using Feature<PointInT, PointOutT>::surface_;
      using Feature<PointInT, PointOutT>::k_;
      using Feature<PointInT, PointOutT>::search_radius_;
      using Feature<PointInT, PointOutT>::search_parameter_;

      typedef typename Feature<PointInT, PointOutT>::PointCloudOut PointCloudOut;

      /** \brief Empty constructor. */
      NormalEstimation () : vpx_ (0), vpy_ (0), vpz_ (0) 
      {
        feature_name_ = "NormalEstimation";
      };

      /** \brief Compute the Least-Squares plane fit for a given set of points, using their indices,
        * and return the estimated plane parameters together with the surface curvature.
        * \param cloud the input point cloud
        * \param indices the point cloud indices that need to be used
        * \param plane_parameters the plane parameters as: a, b, c, d (ax + by + cz + d = 0)
        * \param curvature the estimated surface curvature as a measure of
        * \f[
        * \lambda_0 / (\lambda_0 + \lambda_1 + \lambda_2)
        * \f]
        */
      inline void 
      computePointNormal (const pcl::PointCloud<PointInT> &cloud, const std::vector<int> &indices, Eigen::Vector4f &plane_parameters, float &curvature)
      {
        if (indices.empty ())
        {
          plane_parameters.setConstant (std::numeric_limits<float>::quiet_NaN ());
          curvature = std::numeric_limits<float>::quiet_NaN ();
          return;
        }
        // Estimate the XYZ centroid
        compute3DCentroid (cloud, indices, xyz_centroid_);

        // Compute the 3x3 covariance matrix
        computeCovarianceMatrix (cloud, indices, xyz_centroid_, covariance_matrix_);

        // Get the plane normal and surface curvature
        solvePlaneParameters (covariance_matrix_, xyz_centroid_, plane_parameters, curvature);
      }

      /** \brief Compute the Least-Squares plane fit for a given set of points, using their indices,
        * and return the estimated plane parameters together with the surface curvature.
        * \param cloud the input point cloud
        * \param indices the point cloud indices that need to be used
        * \param nx the resultant X component of the plane normal
        * \param ny the resultant Y component of the plane normal
        * \param nz the resultant Z component of the plane normal
        * \param curvature the estimated surface curvature as a measure of
        * \f[
        * \lambda_0 / (\lambda_0 + \lambda_1 + \lambda_2)
        * \f]
        */
      inline void 
      computePointNormal (const pcl::PointCloud<PointInT> &cloud, const std::vector<int> &indices, float &nx, float &ny, float &nz, float &curvature)
      {
        if (indices.empty ())
        {
          nx = ny = nz = curvature = std::numeric_limits<float>::quiet_NaN ();
          return;
        }
        // Estimate the XYZ centroid
        compute3DCentroid (cloud, indices, xyz_centroid_);

        // Compute the 3x3 covariance matrix
        computeCovarianceMatrix (cloud, indices, xyz_centroid_, covariance_matrix_);

        // Get the plane normal and surface curvature
        solvePlaneParameters (covariance_matrix_, nx, ny, nz, curvature);
      }

      /** \brief Set the viewpoint.
        * \param vpx the X coordinate of the viewpoint
        * \param vpy the Y coordinate of the viewpoint
        * \param vpz the Z coordinate of the viewpoint
        */
      inline void
      setViewPoint (float vpx, float vpy, float vpz)
      {
        vpx_ = vpx;
        vpy_ = vpy;
        vpz_ = vpz;
      }

      /** \brief Get the viewpoint. */
      inline void
      getViewPoint (float &vpx, float &vpy, float &vpz)
      {
        vpx = vpx_;
        vpy = vpy_;
        vpz = vpz_;
      }

    protected:
      /** \brief Estimate normals for all points given in <setInputCloud (), setIndices ()> using the surface in
        * setSearchSurface () and the spatial locator in setSearchMethod ()
        * \note In situations where not enough neighbors are found, the normal and curvature values are set to -1.
        * \param output the resultant point cloud model dataset that contains surface normals and curvatures
        */
      void computeFeature (PointCloudOut &output);

    private:
      /** \brief Values describing the viewpoint ("pinhole" camera model assumed). For per point viewpoints, inherit
        * from NormalEstimation and provide your own computeFeature (). By default, the viewpoint is set to 0,0,0. */
      float vpx_, vpy_, vpz_;

      /** \brief Placeholder for the 3x3 covariance matrix at each surface patch. */
      EIGEN_ALIGN16 Eigen::Matrix3f covariance_matrix_;

      /** \brief 16-bytes aligned placeholder for the XYZ centroid of a surface patch. */
      Eigen::Vector4f xyz_centroid_;
  };
}

#endif  //#ifndef PCL_NORMAL_3D_H_


