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
 *
 */

#ifndef PCL_SPIN_IMAGE_H_
#define PCL_SPIN_IMAGE_H_

#include <pcl/point_types.h>
#include <pcl/features/feature.h>

namespace pcl
{
  /** \brief Estimates spin-image descriptors in the  given input points. 
    *  
    *  This class represents spin image descriptor. Spin image is
    *  a histogram of point locations summed along the bins of the image.
    *  A 2D accumulator indexed by <VAR>a</VAR> and <VAR>b</VAR> is created. Next, 
    *  the coordinates (<VAR>a</VAR>, <VAR>b</VAR>) are computed for a vertex in 
    *  the surface mesh that is within the support of the spin image 
    *  (explained below). The bin indexed by (<VAR>a</VAR>, <VAR>b</VAR>) in 
    *  the accumulator is then incremented; bilinear interpolation is used 
    *  to smooth the contribution of the vertex. This procedure is repeated 
    *  for all vertices within the support of the spin image. 
    *  The resulting accumulator can be thought of as an image; 
    *  dark areas in the image correspond to bins that contain many projected points. 
    *  As long as the size of the bins in the accumulator is greater 
    *  than the median distance between vertices in the mesh 
    *  (the definition of mesh resolution), the position of individual 
    *  vertices will be averaged out during spin image generation.
    *  
    *  For further information please see: <br>
    *  Johnson, A. E., & Hebert, M. (1998). Surface Matching for Object Recognition in 
    *  Complex 3D Scenes. Image and Vision Computing, 16, 635-651.
    *  
    *  The class also implements radial spin images and spin-images in angular domain 
    *  (or both).
    *  
    *  \author Roman Shapovalov, Alexander Velizhev
    *  \ingroup features
    */
  template <typename PointInT, typename PointNT, typename PointOutT>
  class SpinImageEstimation : public FeatureFromNormals<PointInT, PointNT, PointOutT>
  {
    public:
      using Feature<PointInT, PointOutT>::feature_name_;
      using Feature<PointInT, PointOutT>::getClassName;
      using Feature<PointInT, PointOutT>::indices_;
      using Feature<PointInT, PointOutT>::search_radius_;
      using Feature<PointInT, PointOutT>::surface_;
      using Feature<PointInT, PointOutT>::fake_surface_;
      using FeatureFromNormals<PointInT, PointNT, PointOutT>::normals_;
      using PCLBase<PointInT>::input_;

      typedef typename Feature<PointInT, PointOutT>::PointCloudOut PointCloudOut;

      typedef typename pcl::PointCloud<PointNT> PointCloudN;
      typedef typename PointCloudN::Ptr PointCloudNPtr;
      typedef typename PointCloudN::ConstPtr PointCloudNConstPtr;

      typedef typename pcl::PointCloud<PointInT> PointCloudIn;
      typedef typename PointCloudIn::Ptr PointCloudInPtr;
      typedef typename PointCloudIn::ConstPtr PointCloudInConstPtr;
      
      /** \brief Constructs empty spin image estimator.
        * 
        * \param image_width spin-image resolution, number of bins along one dimension
        * \param support_angle_cos minimal allowed cosine of the angle between 
        *   the normals of input point and search surface point for the point 
        *   to be retained in the support
        * \param min_pts_neighb min number of points in the support to correctly estimate 
        *   spin-image. If at some point the support contains less points, exception is thrown
        */
      SpinImageEstimation (unsigned int image_width = 8,
                           double support_angle_cos = 0.0,   // when 0, this is bogus, so not applied
                           unsigned int min_pts_neighb = 0);

      /** \brief Sets spin-image resolution.
        * 
        * \param bin_count spin-image resolution, number of bins along one dimension
        */
      void 
      setImageWidth (unsigned int bin_count)
      {
        image_width_ = bin_count;
      }

      /** \brief Sets the maximum angle for the point normal to get to support region.
        * 
        * \param support_angle_cos minimal allowed cosine of the angle between 
        *   the normals of input point and search surface point for the point 
        *   to be retained in the support
        */
      void 
      setSupportAngle (double support_angle_cos)
      {
        if (0.0 > support_angle_cos || support_angle_cos > 1.0)  // may be permit negative cosine?
        {
          throw PCLException ("Cosine of support angle should be between 0 and 1",
            "spin_image.h", "setSupportAngle");
        }

        support_angle_cos_ = support_angle_cos;
      }

      /** \brief Sets minimal points count for spin image computation.
        *
        * \param min_pts_neighb min number of points in the support to correctly estimate 
        *   spin-image. If at some point the support contains less points, exception is thrown
        */
      void 
      setMinPointCountInNeighbourhood (unsigned int min_pts_neighb)
      {
        min_pts_neighb_ = min_pts_neighb;
      }


      /** \brief Sets input cloud along with corresponding normals.
        * 
        * Note that if no search surface is given explicitly, these points
        *   and normals are used to estimate the descriptor
        * \param input the cloud that provides the query points where the descriptor is estimated
        * \param normals provides normals for input, they should be normalized
        */
      void 
      setInputWithNormals (const PointCloudInConstPtr& input, 
                           const PointCloudNConstPtr& normals)
      {
        setInputCloud (input);
        input_normals_ = normals;
      }

      /** \brief Sets surface that is used to estimate descriptor.
        * 
        * This method is not mandatory. If it has not been called, the points
        *   and normals from setInputWithNormals() are used to estimate the descriptor
        *   Calling this method is tantamount to calling setSearchSurface() and setInputNormals()
        *   with the same parameters (note that setInputNormals() in fact sets surface normals)
        * \param surface search surface used to estimate descriptor
        * \param normals provides normals for surface, they should be normalized
        */
      void 
      setSearchSurfaceWithNormals (const PointCloudInConstPtr& surface, 
                                   const PointCloudNConstPtr& normals)
      {
        setSearchSurface (surface);
        setInputNormals (normals);
      }

      /** \brief Sets single vector a rotation axis for all input points.
        * 
        * It could be useful e.g. when the vertical axis is known.
        * \param axis unit-length vector that serves as rotation axis for reference frame
        */
      void 
      setRotationAxis (const PointNT& axis)
      {
        rotation_axis_ = axis;
        use_custom_axis_ = true;
        use_custom_axes_cloud_ = false;
      }

      /** \brief Sets array of vectors as rotation axes for input points.
        * 
        * Useful e.g. when one wants to use tangents instead of normals as rotation axes
        * \param axes unit-length vectors that serves as rotation axes for 
        *   the corresponding input points' reference frames
        */
      void 
      setInputRotationAxes (const PointCloudNConstPtr& axes)
      {
        rotation_axes_cloud_ = axes;

        use_custom_axes_cloud_ = true;
        use_custom_axis_ = false;
      }

      /** \brief Sets input normals as rotation axes (default setting).
        * 
        */
      void 
      useNormalsAsRotationAxis () 
      { 
        use_custom_axis_ = false; 
        use_custom_axes_cloud_ = false;
      }

      /** \brief Sets/unsets flag for angular spin-image domain.
        * 
        *  Angular spin-image differs from the vanilla one in the way that not 
        *  the points are collected in the bins but the angles between their
        *  normals and the normal to the reference point. For further
        *  information please see 
        *  Endres, F., Plagemann, C., Stachniss, C., & Burgard, W. (2009). 
        *  Unsupervised Discovery of Object Classes from Range Data using Latent Dirichlet Allocation. 
        *  In Robotics: Science and Systems. Seattle, USA.
        * \param is_angular true for angular domain, false for point domain
        */
      void 
      setAngularDomain (bool is_angular = true) { is_angular_ = is_angular; }

      /** \brief Sets/unsets flag for radial spin-image structure.
        * 
        *  Instead of rectangular coordinate system for reference frame 
        *  polar coordinates are used. Binning is done depending on the distance and 
        *  inclination angle from the reference point
        * \param is_radial true for radial spin-image structure, false for rectangular
        */
      void 
      setRadialStructure (bool is_radial = true) { is_radial_ = is_radial; }

    protected:
      /** \brief Estimate the Spin Image descriptors at a set of points given by
        * setInputWithNormals() using the surface in setSearchSurfaceWithNormals() and the spatial locator 
        * \param output the resultant point cloud that contains the Spin Image feature estimates
        */
      virtual void 
      computeFeature (PointCloudOut &output); 

      /** \brief initializes computations specific to spin-image.
        * 
        * \return true iff input data and initialization are correct
        */
      virtual bool
      initCompute ();


    private:
      /** \brief Computes a spin-image for the point of the scan. 
        * 
        * \param index the index of the reference point in the input cloud
        * \return estimated spin-image (or its variant) as a matrix
        */
      Eigen::ArrayXXd 
      computeSiForPoint (int index) const;

      PointCloudNConstPtr input_normals_;
      PointCloudNConstPtr rotation_axes_cloud_;
      
      bool is_angular_;

      PointNT rotation_axis_;
      bool use_custom_axis_;
      bool use_custom_axes_cloud_;

      bool is_radial_;

      unsigned int image_width_;
      double support_angle_cos_;
      unsigned int min_pts_neighb_;


      static const double PI;
  
      /** \brief rounds the argument.
        * 
        * \param dubl double number not greater than maxint by absolute value
        * \return rounded value as int
        */
      static int round (double dubl)
      {
        assert (fabs (dubl) < (std::numeric_limits<int>::max) ());   // check that this is small enough
    
        return int(::floor (dubl) + ((dubl >= 0) ? 0.5 : -0.5));
      }
  };
}

#endif  //#ifndef PCL_SPIN_IMAGE_H_


