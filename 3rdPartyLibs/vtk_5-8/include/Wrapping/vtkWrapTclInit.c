#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *Capitalized(const char *input)
{
  char *result;
  size_t i;

  result = strdup(input);
  if (result[0] > 90) 
    {
    result[0] -= 32;
    }
  for (i = 1; i < strlen(result); i++)
    {
    if ((result[i] > 64)&&(result[i] < 91))
      {
      result[i] += 32;
      }
    }

  return result;
}

/* this roputine creates the init file */
static void CreateInitFile(const char *libName, 
                           int numConcrete, char **concrete, 
                           int numCommands, char **commands,
                           const char *version,
                           FILE *fout) 
{
  /* we have to make sure that the name is the correct case */
  char *kitName = Capitalized(libName);
  int i;
  char **capcommands = (char **)malloc(numCommands*sizeof(char *));

  /* capitalize commands just once */
  for (i = 0; i < numCommands; i++)
    {
    capcommands[i] = Capitalized(commands[i]);
    }

  fprintf(fout,"#include \"vtkTclUtil.h\"\n");
  fprintf(fout,"#include \"vtkVersion.h\"\n");
  fprintf(fout,"#define VTK_TCL_TO_STRING(x) VTK_TCL_TO_STRING0(x)\n");
  fprintf(fout,"#define VTK_TCL_TO_STRING0(x) #x\n");

  fprintf(fout,
    "extern \"C\"\n"
    "{\n"
    "#if (TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION >= 4)\n"
    "  typedef int (*vtkTclCommandType)(ClientData, Tcl_Interp *,int, CONST84 char *[]);\n"
    "#else\n"
    "  typedef int (*vtkTclCommandType)(ClientData, Tcl_Interp *,int, char *[]);\n"
    "#endif\n"
    "}\n"
    "\n");

  for (i = 0; i < numConcrete; i++)
    {
    fprintf(fout,"int %sCommand(ClientData cd, Tcl_Interp *interp,\n             int argc, char *argv[]);\n",concrete[i]);
    fprintf(fout,"ClientData %sNewCommand();\n",concrete[i]);
    }

  if (!strcmp(kitName,"Vtkcommontcl"))
    {
    fprintf(fout,"int vtkCreateCommand(ClientData cd, Tcl_Interp *interp,\n"
      "               int argc, char *argv[]);\n");
    fprintf(fout,"\nTcl_HashTable vtkInstanceLookup;\n");
    fprintf(fout,"Tcl_HashTable vtkPointerLookup;\n");
    fprintf(fout,"Tcl_HashTable vtkCommandLookup;\n");
    }
  else
    {
    fprintf(fout,"\nextern Tcl_HashTable vtkInstanceLookup;\n");
    fprintf(fout,"extern Tcl_HashTable vtkPointerLookup;\n");
    fprintf(fout,"extern Tcl_HashTable vtkCommandLookup;\n");
    }
  fprintf(fout,"extern void vtkTclDeleteObjectFromHash(void *);\n");  
  fprintf(fout,"extern void vtkTclListInstances(Tcl_Interp *interp, ClientData arg);\n");

  for (i = 0; i < numCommands; i++)
    {
    fprintf(fout,
      "\nextern \"C\" {int VTK_EXPORT %s_Init(Tcl_Interp *interp);}\n",
      capcommands[i]);
    }

  fprintf(fout,
    "\n\nextern \"C\" {int VTK_EXPORT %s_SafeInit(Tcl_Interp *interp);}\n",
    kitName);
  fprintf(fout,
    "\nextern \"C\" {int VTK_EXPORT %s_Init(Tcl_Interp *interp);}\n",
    kitName);

  /* create an extern ref to the generic delete function */
  fprintf(fout,"\nextern void vtkTclGenericDeleteObject(ClientData cd);\n");

  if (!strcmp(kitName,"Vtkcommontcl"))
    {
    fprintf(fout,
      "extern \"C\"\n{\nvoid vtkCommonDeleteAssocData(ClientData cd)\n");
    fprintf(fout,"  {\n");
    fprintf(fout,"  vtkTclInterpStruct *tis = static_cast<vtkTclInterpStruct*>(cd);\n");
    fprintf(fout,"  delete tis;\n  }\n}\n");
    }

  /* the main declaration */
  fprintf(fout,
    "\n\nint VTK_EXPORT %s_SafeInit(Tcl_Interp *interp)\n{\n",kitName);
  fprintf(fout,"  return %s_Init(interp);\n}\n",kitName);

  fprintf(fout,"\n\nint VTK_EXPORT %s_Init(Tcl_Interp *interp)\n{\n",
    kitName);
  if (!strcmp(kitName,"Vtkcommontcl"))
    {
    fprintf(fout,
      "  vtkTclInterpStruct *info = new vtkTclInterpStruct;\n");
    fprintf(fout,
      "  info->Number = 0; info->InDelete = 0; info->DebugOn = 0; info->DeleteExistingObjectOnNew = 0;\n");
    fprintf(fout,"\n");
    fprintf(fout,"\n");
    fprintf(fout,
      "  Tcl_InitHashTable(&info->InstanceLookup, TCL_STRING_KEYS);\n");
    fprintf(fout,
      "  Tcl_InitHashTable(&info->PointerLookup, TCL_STRING_KEYS);\n");
    fprintf(fout,
      "  Tcl_InitHashTable(&info->CommandLookup, TCL_STRING_KEYS);\n");
    fprintf(fout,
      "  Tcl_SetAssocData(interp,(char *)(\"vtk\"),NULL,reinterpret_cast<ClientData *>(info));\n");
    fprintf(fout,
      "  Tcl_CreateExitHandler(vtkCommonDeleteAssocData,reinterpret_cast<ClientData *>(info));\n");

    /* create special vtkCommand command */
    fprintf(fout,"  Tcl_CreateCommand(interp,(char *)(\"vtkCommand\"),\n"
      "                    reinterpret_cast<vtkTclCommandType>(vtkCreateCommand),\n"
      "                    static_cast<ClientData *>(NULL), NULL);\n\n");

    /*
     * Set the default precision of tcl to match the legacy vtk default
     * precsion.  Wrapped code will use the tcl_precision variable to
     * define the output resolution for converting doubles to strings
     */
    fprintf(fout,
            "  Tcl_SetVar2(interp, \"tcl_precision\", (char *) NULL, \"6\", TCL_GLOBAL_ONLY);\n");
    }

  for (i = 0; i < numCommands; i++)
    {
    fprintf(fout,"  %s_Init(interp);\n", capcommands[i]);
    }
  fprintf(fout,"\n");

  for (i = 0; i < numConcrete; i++)
    {
    fprintf(fout,"  vtkTclCreateNew(interp,const_cast<char *>(\"%s\"), %sNewCommand,\n",
      concrete[i], concrete[i]);
    fprintf(fout,"                  %sCommand);\n",concrete[i]);
    }

  fprintf(fout,"  char pkgName[]=\"%s\";\n", libName);
  if (version && *version)
    {
    fprintf(fout,"  char pkgVers[]=VTK_TCL_TO_STRING(%s);\n", version);
    }
  else
    {
    fprintf(fout,"  char pkgVers[]=VTK_TCL_TO_STRING(VTK_MAJOR_VERSION)"
            " \".\" "
            "VTK_TCL_TO_STRING(VTK_MINOR_VERSION);\n");
    }
  fprintf(fout,"  Tcl_PkgProvide(interp, pkgName, pkgVers);\n");
  fprintf(fout,"  return TCL_OK;\n}\n");

  for ( i = 0; i < numCommands; i++ )
    {
    free(capcommands[i]);
    }
  free(capcommands);
  free(kitName);
}


int main(int argc,char *argv[])
{
  FILE *file;
  FILE *fout;
  int numConcrete = 0;
  int numCommands = 0;
  char libName[250];
  char tmpVal[250];
  char *concrete[4000];
  char *commands[4000];
  char version[4000] = {'\0'};

  if (argc < 3)
    {
    fprintf(stderr,"Usage: %s input_file output_file\n",argv[0]);
    return 1;
    }

  file = fopen(argv[1],"r");
  if (!file) 
    {
    fprintf(stderr,"Input file %s could not be opened\n",argv[1]);
    return 1;
    }

  fout = fopen(argv[2],"w");
  if (!fout)
    {
    fclose(file);
    return 1;
    }

  /* read the info from the file */
  fscanf(file,"%s",libName);

  /* read in the classes and commands */
  while (fscanf(file,"%s",tmpVal) != EOF)
    {
    if (!strcmp(tmpVal,"COMMAND"))
      {
      fscanf(file,"%s",tmpVal);
      commands[numCommands] = strdup(tmpVal);
      numCommands++;
      }
    else if (!strcmp(tmpVal,"VERSION"))
      {
      fscanf(file,"%s",version);
      }
    else
      {
      concrete[numConcrete] = strdup(tmpVal);
      numConcrete++;
      }
    }
  /* close the file */
  fclose(file);

  CreateInitFile(libName, numConcrete, concrete, numCommands, commands, version, fout);
  fclose(fout);

  return 0;
}
