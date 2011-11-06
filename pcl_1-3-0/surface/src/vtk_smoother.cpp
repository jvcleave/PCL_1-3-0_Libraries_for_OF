/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
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
 * $Id$
 *
 */

#include <pcl/surface/vtk_smoother.h>
#include <pcl/point_types.h>
#include <pcl/pcl_base.h>
#include <pcl/ros/conversions.h>
#include <pcl/common/common.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkTriangleFilter.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkPolyData.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkButterflySubdivisionFilter.h>
#include <vtkPolyDataWriter.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>

//////////////////////////////////////////////////////////////////////////////////////////////
int
pcl::surface::VTKSmoother::convertToVTK (const pcl::PolygonMesh &triangles)
{
  if (triangles.cloud.data.empty ())
  {
    PCL_ERROR ("[pcl::surface::convertToVTK] Input point cloud has no data!\n");
    return (-1);
  }
  mesh2vtk (triangles, vtk_polygons_);

  vtkSmartPointer<vtkTriangleFilter> vtk_triangles = vtkTriangleFilter::New ();
  vtk_triangles->SetInput (vtk_polygons_);
  vtk_triangles->Update();

  vtk_polygons_ = vtk_triangles->GetOutput ();

  return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::surface::VTKSmoother::subdivideMesh()
{
  vtkSmartPointer<vtkPolyDataAlgorithm> vtk_subdivision_filter;
  switch(subdivision_filter_)
  {
    case 0:
      return;
      break;
    case 1:
      vtk_subdivision_filter = vtkLinearSubdivisionFilter::New ();
      break;
    case 2:
      vtk_subdivision_filter = vtkLoopSubdivisionFilter::New ();
      break;
    case 3:
      vtk_subdivision_filter = vtkButterflySubdivisionFilter::New ();
      break;
    default:
      PCL_ERROR ("[pcl::surface::VTKSmoother::subdivideMesh] Invalid filter selection!\n");
      return;
      break;
  }

  vtk_subdivision_filter->SetInput (vtk_polygons_);
  vtk_subdivision_filter->Update ();

  vtk_polygons_ = vtk_subdivision_filter->GetOutput ();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::surface::VTKSmoother::smoothMeshWindowedSinc()
{
  vtkSmartPointer<vtkWindowedSincPolyDataFilter> vtk_smoother = vtkWindowedSincPolyDataFilter::New ();
  vtk_smoother->SetInput (vtk_polygons_);
  vtk_smoother->SetNumberOfIterations (num_iter_);
  vtk_smoother->SetFeatureAngle (feature_angle_);
  vtk_smoother->SetPassBand (pass_band_);
  vtk_smoother->BoundarySmoothingOff ();
  vtk_smoother->FeatureEdgeSmoothingOff ();
  vtk_smoother->NonManifoldSmoothingOff ();
  vtk_smoother->NormalizeCoordinatesOn ();
  vtk_smoother->Update ();

  vtk_polygons_ = vtk_smoother->GetOutput ();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::surface::VTKSmoother::smoothMeshLaplacian()
{
  vtkSmartPointer<vtkSmoothPolyDataFilter> vtk_smoother = vtkSmoothPolyDataFilter::New ();
  vtk_smoother->SetInput (vtk_polygons_);
  vtk_smoother->SetNumberOfIterations (num_iter_);
  vtk_smoother->Update ();

  vtk_polygons_ = vtk_smoother->GetOutput ();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::surface::VTKSmoother::convertToPCL (pcl::PolygonMesh &triangles)
{
  vtk2mesh (vtk_polygons_, triangles);
}

//////////////////////////////////////////////////////////////////////////////////////////////
int
pcl::surface::VTKSmoother::vtk2mesh (const vtkSmartPointer<vtkPolyData>& poly_data, pcl::PolygonMesh& mesh)
{
  mesh.polygons.clear ();
  mesh.cloud.data.clear ();
  mesh.cloud.width = mesh.cloud.height = 0;
  mesh.cloud.is_dense = true;

  vtkSmartPointer<vtkPoints> mesh_points = poly_data->GetPoints ();
  vtkIdType nr_points = mesh_points->GetNumberOfPoints ();
  vtkIdType nr_polygons = poly_data->GetNumberOfPolys ();

  if (nr_points == 0)
    return 0;

  vtkUnsignedCharArray* poly_colors = NULL;
  if (poly_data->GetPointData() != NULL)
    poly_colors = vtkUnsignedCharArray::SafeDownCast (poly_data->GetPointData ()->GetScalars ("Colors"));

  // Some applications do not save the name of scalars (including PCL's native vtk_io)
  if (!poly_colors)
    poly_colors = vtkUnsignedCharArray::SafeDownCast (poly_data->GetPointData ()->GetScalars ("scalars"));

  // TODO: currently only handles rgb values with 3 components
  if (poly_colors && (poly_colors->GetNumberOfComponents () == 3))
  {
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_temp (new pcl::PointCloud<pcl::PointXYZRGB>());
    cloud_temp->points.resize (nr_points);
    double point_xyz[3];
    unsigned char point_color[3];
    for (vtkIdType i = 0; i < mesh_points->GetNumberOfPoints (); ++i)
    {
      mesh_points->GetPoint (i, &point_xyz[0]);
      cloud_temp->points[i].x = (float)(point_xyz[0]);
      cloud_temp->points[i].y = (float)(point_xyz[1]);
      cloud_temp->points[i].z = (float)(point_xyz[2]);

      poly_colors->GetTupleValue (i, &point_color[0]);
      cloud_temp->points[i].r = point_color[0];
      cloud_temp->points[i].g = point_color[1];
      cloud_temp->points[i].b = point_color[2];
    }
    cloud_temp->width = cloud_temp->points.size ();
    cloud_temp->height = 1;
    cloud_temp->is_dense = true;

    pcl::toROSMsg (*cloud_temp, mesh.cloud);
  }
  else // in case points do not have color information:
  {
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_temp (new pcl::PointCloud<pcl::PointXYZ> ());
    cloud_temp->points.resize (nr_points);
    double point_xyz[3];
    for (vtkIdType i = 0; i < mesh_points->GetNumberOfPoints (); ++i)
    {
      mesh_points->GetPoint (i, &point_xyz[0]);
      cloud_temp->points[i].x = (float)(point_xyz[0]);
      cloud_temp->points[i].y = (float)(point_xyz[1]);
      cloud_temp->points[i].z = (float)(point_xyz[2]);
    }
    cloud_temp->width = cloud_temp->points.size ();
    cloud_temp->height = 1;
    cloud_temp->is_dense = true;

    pcl::toROSMsg (*cloud_temp, mesh.cloud);
  }

  mesh.polygons.resize (nr_polygons);
  vtkIdType* cell_points;
  vtkIdType nr_cell_points;
  vtkCellArray * mesh_polygons = poly_data->GetPolys ();
  mesh_polygons->InitTraversal ();
  int id_poly = 0;
  while (mesh_polygons->GetNextCell (nr_cell_points, cell_points))
  {
    mesh.polygons[id_poly].vertices.resize (nr_cell_points);
    for (int i = 0; i < nr_cell_points; ++i)
      mesh.polygons[id_poly].vertices[i] = cell_points[i];
    ++id_poly;
  }

  return (nr_points);
}

//////////////////////////////////////////////////////////////////////////////////////////////
int
pcl::surface::VTKSmoother::mesh2vtk (const pcl::PolygonMesh& mesh, vtkSmartPointer<vtkPolyData> &poly_data)
{
  int nr_points = mesh.cloud.width * mesh.cloud.height;
  int nr_polygons = mesh.polygons.size ();

  // reset vtkPolyData object
  poly_data = vtkSmartPointer<vtkPolyData>::New (); // OR poly_data->Reset();
  vtkSmartPointer<vtkPoints> vtk_mesh_points = vtkSmartPointer<vtkPoints>::New ();
  vtkSmartPointer<vtkCellArray> vtk_mesh_polygons = vtkSmartPointer<vtkCellArray>::New ();
  poly_data->SetPoints (vtk_mesh_points);

  // get field indices for x, y, z (as well as rgb and/or rgba)
  int idx_x = -1, idx_y = -1, idx_z = -1, idx_rgb = -1, idx_rgba = -1, idx_normal_x = -1, idx_normal_y = -1, idx_normal_z = -1;
  for (size_t d = 0; d < mesh.cloud.fields.size (); ++d)
  {
    if (mesh.cloud.fields[d].name == "x") idx_x = d;
    else if (mesh.cloud.fields[d].name == "y") idx_y = d;
    else if (mesh.cloud.fields[d].name == "z") idx_z = d;
    else if (mesh.cloud.fields[d].name == "rgb") idx_rgb = d;
    else if (mesh.cloud.fields[d].name == "rgba") idx_rgba = d;
    else if (mesh.cloud.fields[d].name == "normal_x") idx_normal_x = d;
    else if (mesh.cloud.fields[d].name == "normal_y") idx_normal_y = d;
    else if (mesh.cloud.fields[d].name == "normal_z") idx_normal_z = d;
  }
  if ( ( idx_x == -1 ) || ( idx_y == -1 ) || ( idx_z == -1 ) )
    nr_points = 0;

  // copy point data
  vtk_mesh_points->SetNumberOfPoints (nr_points);
  if (nr_points > 0)
  {
    Eigen::Vector4f pt = Eigen::Vector4f::Zero ();
    Eigen::Array4i xyz_offset (mesh.cloud.fields[idx_x].offset, mesh.cloud.fields[idx_y].offset, mesh.cloud.fields[idx_z].offset, 0);
    for (vtkIdType cp = 0; cp < nr_points; ++cp, xyz_offset += mesh.cloud.point_step)
    {
      memcpy(&pt[0], &mesh.cloud.data[xyz_offset[0]], sizeof(float));
      memcpy(&pt[1], &mesh.cloud.data[xyz_offset[1]], sizeof(float));
      memcpy(&pt[2], &mesh.cloud.data[xyz_offset[2]], sizeof(float));
      vtk_mesh_points->InsertPoint(cp, pt[0], pt[1], pt[2]);
    }
  }

  // copy polygon data
  if (nr_polygons > 0)
  {
    for (int i = 0; i < nr_polygons; i++)
    {
      unsigned int nr_points_in_polygon = mesh.polygons[i].vertices.size ();
      vtk_mesh_polygons->InsertNextCell (nr_points_in_polygon);
      for (unsigned int j = 0; j < nr_points_in_polygon; j++)
        vtk_mesh_polygons->InsertCellPoint(mesh.polygons[i].vertices[j]);
    }
    poly_data->SetPolys (vtk_mesh_polygons);
  }

  // copy color information
  if (idx_rgb != -1 || idx_rgba != -1)
  {
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New ();
    colors->SetNumberOfComponents (3);
    colors->SetName ("Colors");
    pcl::RGB rgb;
    int offset = (idx_rgb != -1) ? mesh.cloud.fields[idx_rgb].offset : mesh.cloud.fields[idx_rgba].offset;
    for (vtkIdType cp = 0; cp < nr_points; ++cp)
    {
      memcpy (&rgb, &mesh.cloud.data[cp * mesh.cloud.point_step + offset], sizeof (pcl::RGB));
      const unsigned char color[3] = {rgb.r, rgb.g, rgb.b};
      colors->InsertNextTupleValue (color);
    }
    poly_data->GetPointData ()->SetScalars (colors);
  }

  // copy normal information
  if ( ( idx_normal_x != -1 ) && ( idx_normal_y != -1 ) && ( idx_normal_z != -1 ) )
  {
    vtkSmartPointer<vtkFloatArray> normals = vtkSmartPointer<vtkFloatArray>::New ();
    normals->SetNumberOfComponents (3);
    float nx = 0.0f, ny = 0.0f, nz = 0.0f;
    for (vtkIdType cp = 0; cp < nr_points; ++cp)
    {
      memcpy (&nx, &mesh.cloud.data[cp*mesh.cloud.point_step+mesh.cloud.fields[idx_normal_x].offset], sizeof(float));
      memcpy (&ny, &mesh.cloud.data[cp*mesh.cloud.point_step+mesh.cloud.fields[idx_normal_y].offset], sizeof(float));
      memcpy (&nz, &mesh.cloud.data[cp*mesh.cloud.point_step+mesh.cloud.fields[idx_normal_z].offset], sizeof(float));
      const float normal[3] = {nx, ny, nz};
      normals->InsertNextTupleValue (normal);
    }
    poly_data->GetPointData()->SetNormals (normals);
  }

  if (poly_data->GetPoints() == NULL)
    return (0);
  return ((int)(poly_data->GetPoints()->GetNumberOfPoints ()));
}

