IF (WIN32)
  IF (NOT BORLAND)
    IF(NOT CYGWIN)
      VTK_GET_TCL_TK_VERSION ("TCL_TK_MAJOR_VERSION" "TCL_TK_MINOR_VERSION")
      IF (TCL_TK_MAJOR_VERSION AND TCL_TK_MINOR_VERSION)
        SET (VTK_TK_RESOURCE_FILE_TRY 
          "${VTK_SOURCE_DIR}/Utilities/TclTk/resources/tk${TCL_TK_MAJOR_VERSION}.${TCL_TK_MINOR_VERSION}/win/rc/tk.rc")
        IF (EXISTS ${VTK_TK_RESOURCE_FILE_TRY})
          GET_FILENAME_COMPONENT(dir ${VTK_TK_RESOURCE_FILE_TRY} PATH)
          SET(VTK_TK_RESOURCES_DIR ${dir} CACHE INTERNAL 
            "The directory where the tk.rc and other Tk resource files can be found. They are required to add the proper resources to a Tk command-line interpreter (vtk.exe for example)")
          INCLUDE_DIRECTORIES(${VTK_TK_RESOURCES_DIR})
        ENDIF (EXISTS ${VTK_TK_RESOURCE_FILE_TRY})
      ENDIF (TCL_TK_MAJOR_VERSION AND TCL_TK_MINOR_VERSION)
    ENDIF(NOT CYGWIN)
  ENDIF (NOT BORLAND)
ENDIF (WIN32)

