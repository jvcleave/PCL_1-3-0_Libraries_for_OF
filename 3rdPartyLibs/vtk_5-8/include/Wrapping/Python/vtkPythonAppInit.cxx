/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPythonAppInit.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/* Minimal main program -- everything is loaded from the library */

#ifdef VTK_COMPILED_USING_MPI
# include <mpi.h>
# include "vtkMPIController.h"
#endif // VTK_COMPILED_USING_MPI

#include "vtkPython.h"
#include "vtkVersion.h"
#include "Wrapping/Python/vtkPythonAppInitConfigure.h"

#if defined(CMAKE_INTDIR)
# define VTK_PYTHON_LIBRARY_DIR VTK_PYTHON_LIBRARY_DIR_BUILD "/" CMAKE_INTDIR
#else
# define VTK_PYTHON_LIBRARY_DIR VTK_PYTHON_LIBRARY_DIR_BUILD
#endif

#include <sys/stat.h>

/*
 * Make sure all the kits register their classes with vtkInstantiator.
 */
#include "vtkCommonInstantiator.h"
#include "vtkFilteringInstantiator.h"
#include "vtkIOInstantiator.h"
#include "vtkImagingInstantiator.h"
#include "vtkGraphicsInstantiator.h"

#include "vtkpythonmodules.h"

#ifdef VTK_USE_RENDERING
#include "vtkRenderingInstantiator.h"
#include "vtkVolumeRenderingInstantiator.h"
#include "vtkHybridInstantiator.h"
#endif

#ifdef VTK_USE_PARALLEL
#include "vtkParallelInstantiator.h"
#endif

#ifdef VTK_USE_CHARTS
#include "vtkChartsInstantiator.h"
#endif

#ifdef VTK_USE_GEOVIS
#include "vtkGeovisInstantiator.h"
#endif

#ifdef VTK_USE_INFOVIS
#include "vtkInfovisInstantiator.h"
#endif

#ifdef VTK_USE_VIEWS
#include "vtkViewsInstantiator.h"
#endif

#include <vtkstd/string>
#include <vtksys/SystemTools.hxx>

#ifdef VTK_COMPILED_USING_MPI
class vtkMPICleanup {
public:
  vtkMPICleanup()
    {
      this->Controller = 0;
    }
  void Initialize(int* argc, char ***argv)
    {
      MPI_Init(argc, argv);
      this->Controller = vtkMPIController::New();
      this->Controller->Initialize(argc, argv, 1);
      vtkMultiProcessController::SetGlobalController(this->Controller);
    }
  ~vtkMPICleanup()
    {
      if ( this->Controller )
        {
        this->Controller->Finalize();
        this->Controller->Delete();
        }
    }
private:
  vtkMPIController *Controller;
};

static vtkMPICleanup VTKMPICleanup;

#endif // VTK_COMPILED_USING_MPI

extern "C" {
  extern DL_IMPORT(int) Py_Main(int, char **);
}

static void vtkPythonAppInitEnableMSVCDebugHook();
static void vtkPythonAppInitPrependPath(const char* self_dir);

/* The maximum length of a file name.  */
#if defined(PATH_MAX)
# define VTK_PYTHON_MAXPATH PATH_MAX
#elif defined(MAXPATHLEN)
# define VTK_PYTHON_MAXPATH MAXPATHLEN
#else
# define VTK_PYTHON_MAXPATH 16384
#endif

/* Python major.minor version string.  */
#define VTK_PYTHON_TO_STRING(x) VTK_PYTHON_TO_STRING0(x)
#define VTK_PYTHON_TO_STRING0(x) VTK_PYTHON_TO_STRING1(x)
#define VTK_PYTHON_TO_STRING1(x) #x
#define VTK_PYTHON_VERSION VTK_PYTHON_TO_STRING(PY_MAJOR_VERSION.PY_MINOR_VERSION)

int main(int argc, char **argv)
{
  vtkPythonAppInitEnableMSVCDebugHook();

#ifdef VTK_COMPILED_USING_MPI
  VTKMPICleanup.Initialize(&argc, &argv);
#endif // VTK_COMPILED_USING_MPI

  int displayVersion = 0;
  if ( argc > 1 )
    {
    int cc;
    for ( cc = 1; cc < argc; cc ++ )
      {
      if ( strcmp(argv[cc], "-V") == 0 )
        {
        displayVersion = 1;
        break;
        }
      }
    }
  else
    {
    displayVersion = 1;
    }
  if ( displayVersion )
    {
    cout << vtkVersion::GetVTKSourceVersion() << endl;
    }

  // The following code will hack in the path for running VTK/Python
  // from the build tree. Do not try this at home. We are
  // professionals.

  // Set the program name, so that we can ask python to provide us
  // full path.  We need to collapse the path name to aid relative
  // path computation for the VTK python module installation.
  static char argv0[VTK_PYTHON_MAXPATH];
  vtkstd::string av0 = vtksys::SystemTools::CollapseFullPath(argv[0]);
  strcpy(argv0, av0.c_str());
  Py_SetProgramName(argv0);

  CMakeLoadAllPythonModules();

  // Initialize interpreter.
  Py_Initialize();

  // Initialize python thread support. This function should first be
  // called from the main thread, after Py_Initialize.
#ifndef VTK_NO_PYTHON_THREADS
  PyEval_InitThreads();
#endif

  // Compute the directory containing this executable.  The python
  // sys.executable variable contains the full path to the interpreter
  // executable.
  char tmpExe[] = "executable";
  PyObject* executable = PySys_GetObject(tmpExe);
  if(const char* exe_str = PyString_AsString(executable))
    {
    // Use the executable location to try to set sys.path to include
    // the VTK python modules.
    vtkstd::string self_dir = vtksys::SystemTools::GetFilenamePath(exe_str);
    vtkPythonAppInitPrependPath(self_dir.c_str());
    }

  // Ok, all done, now enter python main.
  return Py_Main(argc, argv);
}

// For a DEBUG build on MSVC, add a hook to prevent error dialogs when
// being run from DART.
#if defined(_MSC_VER) && defined(_DEBUG)
# include <crtdbg.h>
static int vtkPythonAppInitDebugReport(int, char* message, int*)
{
  fprintf(stderr, message);
  exit(1);
}
void vtkPythonAppInitEnableMSVCDebugHook()
{
  if(getenv("DART_TEST_FROM_DART") ||
    getenv("DASHBOARD_TEST_FROM_CTEST"))
    {
    _CrtSetReportHook(vtkPythonAppInitDebugReport);
    }
}
#else
void vtkPythonAppInitEnableMSVCDebugHook()
{
}
#endif

//----------------------------------------------------------------------------
static void vtkPythonAppInitPrependPythonPath(const char* dir)
{
  // Convert slashes for this platform.
  vtkstd::string out_dir = dir;
#if defined(_WIN32) && !defined(__CYGWIN__)
  for(vtkstd::string::size_type i = 0; i < out_dir.length(); ++i)
    {
    if(out_dir[i] == '/')
      {
      out_dir[i] = '\\';
      }
    }
#endif

  // Append the path to the python sys.path object.
  char tmpPath[] = "path";
  PyObject* path = PySys_GetObject(tmpPath);
  PyObject* newpath;
  newpath = PyString_FromString(out_dir.c_str());
  PyList_Insert(path, 0, newpath);
  Py_DECREF(newpath);
}

//----------------------------------------------------------------------------
static void vtkPythonAppInitPrependPath(const char* self_dir)
{
  // Try to put the VTK python module location in sys.path.
  const char* build_dirs[] = {
    "/../Wrapping/Python",
    "/../VTK/Wrapping/Python",
    0
  };

  int found_vtk = 0;
  for (const char** build_dir = build_dirs; *build_dir && !found_vtk; ++build_dir)
    {
    vtkstd::string package_dir = self_dir;
#if defined(CMAKE_INTDIR)
    package_dir += "/..";
#endif
    package_dir += (*build_dir);
    package_dir = vtksys::SystemTools::CollapseFullPath(package_dir.c_str());
    if(vtksys::SystemTools::FileIsDirectory(package_dir.c_str()))
      {
      // This executable is running from the build tree.  Prepend the
      // library directory and package directory to the search path.
      vtkPythonAppInitPrependPythonPath(package_dir.c_str());
      vtkPythonAppInitPrependPythonPath(VTK_PYTHON_LIBRARY_DIR);
      found_vtk = 1;
      }
    }

  if (!found_vtk)
    {
    // This executable is running from an install tree.  Check for
    // possible VTK python module locations.  See
    // http://python.org/doc/2.4.1/inst/alt-install-windows.html for
    // information about possible install locations.  If the user
    // changes the prefix to something other than VTK's prefix or
    // python's native prefix then he/she will have to get the
    // packages in sys.path himself/herself.
    const char* inst_dirs[] = {
      "/lib/python" VTK_PYTHON_VERSION "/site-packages/vtk", // UNIX --prefix
      "/python" VTK_PYTHON_VERSION "/site-packages/vtk", // UNIX + Forwarding exe --prefix
      "/../Library/Python/" VTK_PYTHON_VERSION "/site-packages/vtk", // Apple + Forwarding exe
      "/lib/python/vtk", // UNIX --home
      "/Lib/site-packages/vtk", "/Lib/vtk", // Windows
      "/site-packages/vtk", "/vtk", // Windows
      0
    };
    vtkstd::string prefix = vtksys::SystemTools::GetFilenamePath(self_dir);
    for(const char** dir = inst_dirs; *dir; ++dir)
      {
      vtkstd::string package_dir = prefix;
      package_dir += *dir;
      package_dir = vtksys::SystemTools::CollapseFullPath(package_dir.c_str());
      if(vtksys::SystemTools::FileIsDirectory(package_dir.c_str()))
        {
        // We found the modules.  Add the location to sys.path, but
        // without the "/vtk" suffix.
        vtkstd::string path_dir =
          vtksys::SystemTools::GetFilenamePath(package_dir);
        vtkPythonAppInitPrependPythonPath(path_dir.c_str());
        break;
        }
      }

    // This executable does not actually link to the python wrapper
    // libraries, though it probably should now that the stub-modules
    // are separated from them.  Since it does not we have to make
    // sure the wrapper libraries can be found by the dynamic loader
    // when the stub-modules are loaded.  On UNIX this executable must
    // be running in an environment where the main VTK libraries (to
    // which this executable does link) have been found, so the
    // wrapper libraries will also be found.  On Windows this
    // executable may have simply found its .dll files next to itself
    // so the wrapper libraries may not be found when the wrapper
    // modules are loaded.  Solve this problem by adding this
    // executable's location to the system PATH variable.  Note that
    // this need only be done for an installed VTK because in the
    // build tree the wrapper modules are in the same directory as the
    // wrapper libraries.
#if defined(_WIN32)
    static char system_path[(VTK_PYTHON_MAXPATH+1)*10] = "PATH=";
    strcat(system_path, self_dir);
    if(char* oldpath = getenv("PATH"))
      {
      strcat(system_path, ";");
      strcat(system_path, oldpath);
      }
    putenv(system_path);
#endif
    }
}
