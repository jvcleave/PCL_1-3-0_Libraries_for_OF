/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParseHierarchy.c

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright (c) 2010 David Gobbi.

  Contributed to the VisualizationToolkit by the author in June 2010
  under the terms of the Visualization Toolkit 2008 copyright.
-------------------------------------------------------------------------*/

#include "vtkParseHierarchy.h"
#include "vtkParseInternal.h"
#include "vtkParseExtras.h"
#include "vtkType.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

static size_t skip_space(const char *text)
{
  size_t i = 0;
  while (isspace(text[i]) && text[i] != '\n') { i++; }
  return i;
}

/* helper: comparison of entries */
static int compare_hierarchy_entries(const void *vp1, const void *vp2)
{
  const HierarchyEntry *entry1 = (const HierarchyEntry *)vp1;
  const HierarchyEntry *entry2 = (const HierarchyEntry *)vp2;

  return strcmp(entry1->Name, entry2->Name);
}

/* helper: sort the entries to facilitate searching */
static void sort_hierarchy_entries(HierarchyInfo *info)
{
  qsort(info->Entries, info->NumberOfEntries, sizeof(HierarchyEntry),
        &compare_hierarchy_entries);
}

/* Find an entry with a binary search */
HierarchyEntry *vtkParseHierarchy_FindEntry(
  const HierarchyInfo *info, const char *classname)
{
  HierarchyEntry key;
  HierarchyEntry *entry;
  size_t i, n;
  char name[32];
  char *cp;

  /* use classname as-is for the search if possible */
  cp = (char *)classname;

  /* get portion of name before final template parameters */
  n = vtkParse_UnscopedNameLength(classname);
  i = 0;
  while (classname[i+n] == ':' && classname[i+n+1] == ':')
    {
    i += n + 2;
    n = vtkParse_UnscopedNameLength(&classname[i]);
    }
  i += vtkParse_IdentifierLength(&classname[i]);
      
  /* create a new (shorter) search string if necessary */
  if (classname[i] != '\0')
    {
    /* use stack space if possible */
    cp = name;
    /* otherwise, use malloc */
    if (i > 31)
      {
      cp = (char *)malloc(i+1);
      }
    strncpy(cp, classname, i);
    cp[i] = '\0';
    }

  key.Name = cp;

  entry = (HierarchyEntry *)bsearch(&key, info->Entries,
    info->NumberOfEntries, sizeof(HierarchyEntry),
    &compare_hierarchy_entries);

  if (cp != classname && cp != name)
    {
    free(cp);
    }

  return entry;
}


/* read a hierarchy file into a HeirarchyInfo struct, or return NULL */
HierarchyInfo *vtkParseHierarchy_ReadFile(const char *filename)
{
  HierarchyInfo *info;
  HierarchyEntry *entry;
  int maxClasses = 500;
  FILE *fp;
  char *line;
  char *cp;
  const char *ccp;
  size_t maxlen = 15;
  size_t i, j, n, m;
  unsigned int bits, pointers;

  line = (char *)malloc(maxlen);

  fp = fopen(filename, "r");

  if (fp == NULL)
    {
    return NULL;
    }

  info = (HierarchyInfo *)malloc(sizeof(HierarchyInfo));
  info->NumberOfEntries = 0;
  info->Entries = (HierarchyEntry *)malloc(maxClasses*sizeof(HierarchyEntry));

  while (fgets(line, (int)maxlen, fp))
    {
    n = strlen(line);

    /* if buffer not long enough, increase it */
    while (n == maxlen-1 && line[n-1] != '\n' && !feof(fp))
      {
      maxlen *= 2;
      line = (char *)realloc(line, maxlen);
      if (!fgets(&line[n], (int)(maxlen-n), fp)) { break; }
      n += strlen(&line[n]);
      }

    while (n > 0 && isspace(line[n-1]))
      {
      n--;
      }
    line[n] = '\0';

    if (line[0] == '\0')
      {
      continue;
      }

    if (info->NumberOfEntries == maxClasses)
      {
      maxClasses *= 2;
      info->Entries = (HierarchyEntry *)realloc(
        info->Entries, sizeof(HierarchyEntry)*maxClasses*2);
      }

    entry = &info->Entries[info->NumberOfEntries++];
    entry->Name = NULL;
    entry->HeaderFile = NULL;
    entry->Module = NULL;
    entry->NumberOfTemplateArgs = 0;
    entry->TemplateArgs = NULL;
    entry->TemplateArgDefaults = NULL;
    entry->NumberOfProperties = 0;
    entry->Properties = NULL;
    entry->NumberOfSuperClasses = 0;
    entry->SuperClasses = NULL;
    entry->SuperClassIndex = NULL;
    entry->Typedef = NULL;
    entry->IsTypedef = 0;
    entry->IsEnum = 0;

    i = skip_space(line);
    n = vtkParse_NameLength(&line[i]);
    for (m = 0; m < n; m++)
      {
      if (line[i+m] == '<') { break; }
      }

    cp = (char *)malloc(m+1);
    strncpy(cp, &line[i], m);
    cp[m] = '\0';
    entry->Name = cp;
    i += m;

    if (line[i] == '<')
      {
      i++;
      i += skip_space(&line[i]);

      for (j = 0; line[i] != '>' && line[i] != '\0'; j++)
        {
        if (j == 0)
          {
          entry->TemplateArgs = (const char **)malloc(sizeof(char *));
          entry->TemplateArgDefaults = (const char **)malloc(sizeof(char *));
          }
        else
          {
          entry->TemplateArgs = (const char **)realloc(
            (char **)entry->TemplateArgs, (j+1)*sizeof(char *));
          entry->TemplateArgDefaults = (const char **)realloc(
            (char **)entry->TemplateArgDefaults, (j+1)*sizeof(char *));
          }
        entry->NumberOfTemplateArgs++;
        entry->TemplateArgDefaults[j] = NULL;

        m = vtkParse_NameLength(&line[i]);

        cp = (char *)malloc(m+1);
        strncpy(cp, &line[i], m);
        cp[m] = '\0';
        entry->TemplateArgs[j] = cp;
        i += m;
        i += skip_space(&line[i]);

        if (line[i] == '=')
          {
          i++;
          i += skip_space(&line[i]);
          m = vtkParse_NameLength(&line[i]);

          cp = (char *)malloc(m+1);
          strncpy(cp, &line[i], m);
          cp[m] = '\0';
          entry->TemplateArgDefaults[j] = cp;
          i += m;
          i += skip_space(&line[i]);
          }

        if (line[i] == ',')
          {
          i++;
          i += skip_space(&line[i]);
          }
        }

      if (line[i] == '>')
        {
        i++;
        i += skip_space(&line[i]);
        }

      if (line[i] == ':' && line[i+1] == ':')
        {
        i += 2;
        m = vtkParse_NameLength(&line[i]);
        n = strlen(entry->Name);
        cp = (char *)malloc(n+m+3);
        strcpy(cp, entry->Name);
        strcpy(&cp[n], "::");
        strncpy(&cp[n+2], &line[i], m);
        i += m;
        cp[n+m+2] = '\0';
        free((char *)entry->Name);
        entry->Name = cp;
        }
      }

    i += skip_space(&line[i]);

    /* classes (and possibly enums) */
    if (line[i] == ':')
      {
      i++;
      i += skip_space(&line[i]);
      n = vtkParse_NameLength(&line[i]);
      /* check for enum indicators */
      if ((n == 3 && strncmp(&line[i], "int", n)) ||
          (n == 4 && strncmp(&line[i], "enum", n)))
        {
        entry->IsEnum = 1;
        i += n;
        i += skip_space(&line[i]);
        }
      /* else check for superclasses */
      else for (j = 0; ; j++)
        {
        if (j == 0)
          {
          entry->SuperClasses = (const char **)malloc(sizeof(char *));
          entry->SuperClassIndex = (int *)malloc(sizeof(int));
          }
        else
          {
          entry->SuperClasses = (const char **)realloc(
            (char **)entry->SuperClasses, (j+1)*sizeof(char *));
          entry->SuperClassIndex = (int *)realloc(
            entry->SuperClassIndex, (j+1)*sizeof(int));
          }
        entry->NumberOfSuperClasses++;

        i += skip_space(&line[i]);
        n = vtkParse_NameLength(&line[i]);
        cp = (char *)malloc(n+1);
        strncpy(cp, &line[i], n);
        cp[n] = '\0';
        entry->SuperClasses[j] = cp;
        entry->SuperClassIndex[j] = -1;
        i += n;

        i += skip_space(&line[i]);
        if (line[i] != ',')
          {
          break;
          }
        i++;
        }
      }

    /* read typedefs */
    else if (line[i] == '=')
      {
      i++;
      i += skip_space(&line[i]);
      entry->IsTypedef = 1;
      entry->Typedef = (ValueInfo *)malloc(sizeof(ValueInfo));
      vtkParse_InitValue(entry->Typedef);

      /* type is a reference (does this ever occur?) */
      if (line[i] == '&')
        {
        i++;
        i += skip_space(&line[i]);
        entry->Typedef->Type |= VTK_PARSE_REF;
        }

      /* type has array dimensions */
      if (line[i] == '[')
        {
        entry->Typedef->Count = 1;
        }

      while (line[i] == '[')
        {
        i++;
        n = 0;
        while (line[i+n] != ']' && line[i+n] != '\n' && line[i+n] != '\0')
          {
          n++;
          }
        ccp = vtkParse_DuplicateString(&line[i], n);
        vtkParse_AddStringToArray(&entry->Typedef->Dimensions,
                                  &entry->Typedef->NumberOfDimensions, ccp);
        if (ccp[0] >= '0' && ccp[0] <= '9')
          {
          entry->Typedef->Count *= (int)strtol(ccp, NULL, 0);
          }
        else
          {
          entry->Typedef->Count = 0;
          }
        i += n;
        if (line[i] == ']')
          {
          i++;
          }
        }
      i += skip_space(&line[i]);

      /* look for pointers (and const pointers) */
      bits = 0;
      while (line[i] == '*' || strncmp(&line[i], "const*", 6) == 0)
        {
        bits = (bits << 2);
        if (line[i] == '*')
          {
          bits = (bits | VTK_PARSE_POINTER);
          }
        else
          {
          bits = (bits | VTK_PARSE_CONST_POINTER);
          i += 5;
          }
        bits = (bits & VTK_PARSE_POINTER_MASK);
        i++;
        i += skip_space(&line[i]);
        }

      /* need to reverse to get correct pointer order */
      pointers = 0;
      while (bits)
        {
        pointers = (pointers << 2);
        pointers = (pointers | (bits & VTK_PARSE_POINTER_LOWMASK));
        bits = ((bits >> 2) & VTK_PARSE_POINTER_MASK);
        }

      /* add pointer indirection to correspond to first array dimension */
      if (entry->Typedef->NumberOfDimensions > 1)
        {
        pointers = ((pointers << 2) | VTK_PARSE_ARRAY);
        }
      else if (entry->Typedef->NumberOfDimensions == 1)
        {
        pointers = ((pointers << 2) | VTK_PARSE_POINTER);
        }

      /* include the pointers in the type */
      entry->Typedef->Type |= (pointers & VTK_PARSE_POINTER_MASK);

      /* read the base type (and const) */
      bits = 0;
      i += vtkParse_BasicTypeFromString(&line[i], &bits, &ccp, &n);
      entry->Typedef->Class = vtkParse_DuplicateString(ccp, n);
      entry->Typedef->Type |= bits;
      }

    /* get the header file */
    if (line[i] == ';')
      {
      i++;
      i += skip_space(&line[i]);
      n = 0;
      while(line[i+n] != '\0' && line[i+n] != ';' &&
            !isspace(line[i+n])) { n++; };
      cp = (char *)malloc(n+1);
      strncpy(cp, &line[i], n);
      cp[n] = '\0';
      entry->HeaderFile = cp;

      i += n;
      i += skip_space(&line[i]);

      /* get the module */
      if (line[i] == ';')
        {
        i++;
        i += skip_space(&line[i]);
        n = 0;
        while(line[i+n] != '\0' && line[i+n] != ';' &&
              !isspace(line[i+n])) { n++; };
        cp = (char *)malloc(n+1);
        strncpy(cp, &line[i], n);
        cp[n] = '\0';
        entry->Module = cp;

        i += n;
        i += skip_space(&line[i]);
        }

      /* get all flags */
      while (line[i] == ';')
        {
        i++;
        i += skip_space(&line[i]);
        if (entry->NumberOfProperties == 0)
          {
          entry->Properties = (const char **)malloc(sizeof(char **));
          }
        else
          {
          entry->Properties = (const char **)realloc(
            (char **)entry->Properties,
            (entry->NumberOfProperties+1)*sizeof(char **));
          }
        n = 0;
        while (line[i+n] != '\0' && line[i+n] != '\n' && line[i+n] != ';')
          { n++; }
        if (n && skip_space(&line[i]) != n)
          {
          cp = (char *)malloc((n+1)*sizeof(char *));
          strncpy(cp, &line[i], n);
          cp[n] = '\0';
          entry->Properties[entry->NumberOfProperties++] = cp;
          }
        i += n;
        }
      }
    }

  if (!feof(fp))
    {
    vtkParseHierarchy_Free(info);
    info = NULL;
    }

  free(line);

  sort_hierarchy_entries(info);

  return info;
}

/* free a HierarchyInfo struct */
void vtkParseHierarchy_Free(HierarchyInfo *info)
{
  HierarchyEntry *entry;
  int i, j;

  for (i = 0; i < info->NumberOfEntries; i++)
    {
    entry = &info->Entries[i];
    free((char *)entry->Name);
    free((char *)entry->HeaderFile);
    for (j = 0; j < entry->NumberOfTemplateArgs; j++)
      {
      free((char *)entry->TemplateArgs[j]);
      if (entry->TemplateArgDefaults[j])
        {
        free((char *)entry->TemplateArgDefaults[j]);
        }
      }
    if (entry->NumberOfTemplateArgs)
      {
      free((char **)entry->TemplateArgs);
      free((char **)entry->TemplateArgDefaults);
      }
    for (j = 0; j < entry->NumberOfSuperClasses; j++)
      {
      free((char *)entry->SuperClasses[j]);
      }
    if (entry->NumberOfSuperClasses)
      {
      free((char **)entry->SuperClasses);
      free(entry->SuperClassIndex);
      }
    for (j = 0; j < entry->NumberOfProperties; j++)
      {
      free((char *)entry->Properties[j]);
      }
    if (entry->NumberOfProperties)
      {
      free((char **)entry->Properties);
      }
    }

  free(info->Entries);
  free(info);
}


/* Check whether class is derived from baseclass.  You must supply
 * the entry for the class (returned by FindEntry) as well as the
 * classname.  If the class is templated, the classname can include 
 * template args in angle brackets.  If you provide a pointer for
 * baseclass_with_args, then it will be used to return the name of
 * name of the baseclass with template args in angle brackets. */

int vtkParseHierarchy_IsTypeOfTemplated(
  const HierarchyInfo *info,
  const HierarchyEntry *entry, const char *classname,
  const char *baseclass, const char **baseclass_with_args)
{
  HierarchyEntry *tmph;
  const char *name;
  const char *supername;
  char *tmp;
  int templated;
  int baseclass_is_template_parameter;
  int supername_needs_free = 0;
  int classname_needs_free = 0;
  int i, j, k;
  int nargs;
  const char **args;
  size_t m;
  int iterating = 1;
  int rval = 0;

  while (iterating)
    {
    iterating = 0;
    templated = 0;
    baseclass_is_template_parameter = 0;
    nargs = 0;
    args = NULL;

    /* if classname is the same as baseclass, done! */
    if (strcmp(entry->Name, baseclass) == 0)
      {
      if (baseclass_with_args)
        {
        if (!classname_needs_free)
          {
          tmp = (char *)malloc(strlen(classname) + 1);
          strcpy(tmp, classname);
          classname = tmp;
          }
        *baseclass_with_args = classname;
        classname_needs_free = 0;
        }
      rval = 1;
      break;
      }
    else if (entry->NumberOfSuperClasses == 0)
      {
      rval = 0;
      break;
      }

    /* if class is templated */
    if (entry->NumberOfTemplateArgs)
      {
      /* check for template args for classname */
      m = strlen(entry->Name);
      if (classname[m] == '<')
        {
        templated = 1;

        nargs = entry->NumberOfTemplateArgs;
        vtkParse_DecomposeTemplatedType(classname, &name, nargs, &args,
          entry->TemplateArgDefaults);
        }
      }

    /* check all baseclasses */
    for (j = 0; j < entry->NumberOfSuperClasses && rval == 0; j++)
      {
      supername = entry->SuperClasses[j];

      if (templated)
        {
        for (k = 0; k < entry->NumberOfTemplateArgs; k++)
          {
          /* check if the baseclass itself is a template parameter */
          m = strlen(entry->TemplateArgs[k]);
          if (strncmp(entry->TemplateArgs[k], supername, m) == 0 &&
              !isalnum(supername[m]) && supername[m] != '_')
            {
            baseclass_is_template_parameter = 1;
            break;
            }
          }

        /* use the class template args to find baseclass template args */
        supername = vtkParse_StringReplace(
          supername, entry->NumberOfTemplateArgs, entry->TemplateArgs, args);
        if (supername != entry->SuperClasses[j])
          {
          supername_needs_free = 1;
          }
        }

      /* check the cached index for the baseclass entry */
      i = entry->SuperClassIndex[j];
      if (i == -1)
        {
        /* index was not set yet, so search for the entry */
        tmph = vtkParseHierarchy_FindEntry(info, supername);
        while (tmph && tmph->IsTypedef)
          {
          if (tmph->Typedef->Class)
            {
            tmph = vtkParseHierarchy_FindEntry(info, tmph->Typedef->Class);
            continue;
            }
          break;
          }

        if (tmph)
          {
          i = (int)(tmph - info->Entries);
          }
        else
          {
          /* entry not found, don't try again */
          /* i = -2; messes things up for templates */
          /* fprintf(stderr, "not found \"%s\"\n", entry->SuperClasses[j]); */
          }

        /* if baseclass is a template parameter, its entry cannot be cached */
        if (!baseclass_is_template_parameter)
          {
          /* cache the position of the baseclass */
          ((HierarchyEntry *)entry)->SuperClassIndex[j] = i;
          }
        }

      /* if entry was found, continue down the chain */
      if (i >= 0)
        {
        if (classname_needs_free)
          {
          free((char *)classname);
          }
        classname = supername;
        classname_needs_free = supername_needs_free;
        supername_needs_free = 0;

        /* use the iteration loop instead of recursion */
        if (j+1 >= entry->NumberOfSuperClasses)
          {
          entry = &info->Entries[i];
          iterating = 1;
          }

        /* recurse for multiple inheritance */
        else
          {
          rval = vtkParseHierarchy_IsTypeOfTemplated(
                   info, &info->Entries[i], classname, baseclass,
                   baseclass_with_args);
          }
        }

      if (supername_needs_free)
        {
        free((char *)supername);
        supername_needs_free = 0;
        }

      } /* end of loop over superclasses */

    if (templated)
      {
      vtkParse_FreeTemplateDecomposition(name, nargs, args);
      }

    } /* end of "while (iterating)" */

  if (classname_needs_free)
    {
    free((char *)classname);
    }

  if (baseclass_with_args && !rval)
    {
    *baseclass_with_args = NULL;
    }

  return rval;
}

int vtkParseHierarchy_IsTypeOf(
  const HierarchyInfo *info, const HierarchyEntry *entry,
  const char *baseclass)
{
  return vtkParseHierarchy_IsTypeOfTemplated(
    info, entry, entry->Name, baseclass, NULL);
}

/* Free args returned by IsTypeOfTemplated */
void vtkParseHierarchy_FreeTemplateArgs(int n, const char *args[])
{
  int i;

  for (i = 0; i < n; i++)
    {
    free((char *)args[i]);
    }

  free((char **)args);
}

/* Given a classname with template parameters, get the superclass name
 * with corresponding template parameters.  Returns null if 'i' is out
 * of range, i.e. greater than or equal to the number of superclasses.
 * The returned classname must be freed with "free()". */
const char *vtkParseHierarchy_TemplatedSuperClass(
  const HierarchyEntry *entry, const char *classname, int i)
{
  const char *supername = NULL;
  const char *name;
  const char **args;
  char *cp;
  size_t j;

  if (i < entry->NumberOfSuperClasses)
    {
    supername = entry->SuperClasses[i];
    j = vtkParse_IdentifierLength(classname);

    if (classname[j] == '<')
      {
      vtkParse_DecomposeTemplatedType(classname, &name,
        entry->NumberOfTemplateArgs, &args, entry->TemplateArgDefaults);
      supername = vtkParse_StringReplace(entry->SuperClasses[i],
        entry->NumberOfTemplateArgs, entry->TemplateArgs, args);
      vtkParse_FreeTemplateDecomposition(
        name, entry->NumberOfTemplateArgs, args);
      }

    if (supername == entry->SuperClasses[i])
      {
      cp = (char *)malloc(strlen(supername) + 1);
      strcpy(cp, supername);
      supername = cp;
      }
    }

  return supername;
}

/* get the specified property, or return NULL */
const char *vtkParseHierarchy_GetProperty(
  const HierarchyEntry *entry, const char *property)
{
  int i;
  size_t k;

  if (entry)
    {
    for (i = 0; i < entry->NumberOfProperties; i++)
      {
      /* skip the property name, everything after is the property */
      k = vtkParse_NameLength(entry->Properties[i]);
      if (k == strlen(property) &&
          strncmp(entry->Properties[i], property, k) == 0)
        {
        if (entry->Properties[i][k] == ' ' ||
            entry->Properties[i][k] == '=') { k++; }
        return &entry->Properties[i][k];
        }
      }
    }

  return NULL;
}

/* Expand all unrecognized types in a ValueInfo struct by
 * using the typedefs in the HierarchyInfo struct. */
int vtkParseHierarchy_ExpandTypedefsInValue(
  const HierarchyInfo *info, ValueInfo *val, const char *scope)
{
  char text[128];
  char *cp;
  const char *newclass;
  size_t n, m;
  int i;
  HierarchyEntry *entry;
  int scope_needs_free = 0;
  int result = 1;

  while (((val->Type & VTK_PARSE_BASE_TYPE) == VTK_PARSE_OBJECT ||
          (val->Type & VTK_PARSE_BASE_TYPE) == VTK_PARSE_UNKNOWN) &&
         val->Class != 0)
    {
    entry = 0;

    /* search for the type in the provided scope */
    while (entry == 0 && scope != 0)
      {
      cp = text;
      n = strlen(scope);
      m = strlen(val->Class);
      /* only malloc if more than 128 chars needed */
      if (n + m + 2 >= 128)
        {
        cp = (char *)malloc(n+m+3);
        }

      /* scope the name */
      strncpy(cp, scope, n);
      cp[n++] = ':';
      cp[n++] = ':';
      strncpy(&cp[n], val->Class, m);
      cp[n+m] = '\0';

      entry = vtkParseHierarchy_FindEntry(info, cp);

      if (cp != text) { free(cp); }

      /* if not found, try inherited scopes */
      if (entry == 0)
        {
        entry = vtkParseHierarchy_FindEntry(info, scope);
        scope = 0;
        scope_needs_free = 0;
        if (entry && entry->NumberOfSuperClasses)
          {
          for (i = 0; i+1 < entry->NumberOfSuperClasses; i++)
            {
            if (scope_needs_free) { free((char *)scope); }
            scope = vtkParseHierarchy_ExpandTypedefsInName(
              info, entry->SuperClasses[i], NULL);
            scope_needs_free = (scope != entry->SuperClasses[i]);
            /* recurse if more than one superclass */
            if (vtkParseHierarchy_ExpandTypedefsInValue(info, val, scope))
              {
              if (scope_needs_free) { free((char *)scope); }
              return 1;
              }
            }
          if (scope_needs_free) { free((char *)scope); }
          scope = vtkParseHierarchy_ExpandTypedefsInName(
            info, entry->SuperClasses[i], NULL);
          scope_needs_free = (scope != entry->SuperClasses[i]);
          }
        entry = 0;
        }
      }

    /* if not found, try again with no scope */
    if (entry == 0)
      {
      entry = vtkParseHierarchy_FindEntry(info, val->Class);
      }

    if (entry && entry->IsTypedef)
      {
      vtkParse_ExpandTypedef(val, entry->Typedef);
      }
    else if (entry)
      {
      newclass = vtkParseHierarchy_ExpandTypedefsInName(
         info, val->Class, scope);
      if (newclass != val->Class)
        {
        val->Class = vtkParse_DuplicateString(newclass, strlen(newclass));
        free((char *)newclass);
        }
      result = 1;
      break;
      }
    else
      {
      result = 0;
      break;
      }
    }

  if (scope_needs_free) { free((char *)scope); }

  return result;
}

/* Expand typedefs found in an expression stored as a string.
 * The value of "text" will be returned if no expansion occurred,
 * else a new string is returned that must be freed with "free()". */
const char *vtkParseHierarchy_ExpandTypedefsInName(
  const HierarchyInfo *info, const char *name, const char *scope)
{
  char text[128];
  char *cp;
  size_t n, m;
  const char *newname = name;
  HierarchyEntry *entry = NULL;

  /* note: unlike ExpandTypedefsInValue, this does not yet recurse
   * or look in superclass scopes */

  /* doesn't yet handle names that are scoped or templated */
  m = vtkParse_IdentifierLength(name);
  if (name[m] != '\0')
    {
    return name;
    }

  if (scope)
    {
    cp = text;
    n = strlen(scope);
    m = strlen(name);
    /* only malloc if more than 128 chars needed */
    if (n + m + 2 >= 128)
      {
      cp = (char *)malloc(n+m+3);
      }

    /* scope the name */
    strncpy(cp, scope, n);
    cp[n++] = ':';
    cp[n++] = ':';
    strncpy(&cp[n], name, m);
    cp[n+m] = '\0';

    entry = vtkParseHierarchy_FindEntry(info, cp);

    if (cp != text) { free(cp); }
    }

  if (!entry)
    {
    entry = vtkParseHierarchy_FindEntry(info, name);
    }

  newname = NULL;
  if (entry && entry->IsTypedef && entry->Typedef->Class)
    {
    newname = entry->Typedef->Class;
    }
  if (newname)
    {
    cp = (char *)malloc(strlen(newname) + 1);
    strcpy(cp, newname);
    name = cp;
    }

  return name;
}
