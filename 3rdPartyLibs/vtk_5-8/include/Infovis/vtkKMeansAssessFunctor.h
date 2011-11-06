#ifndef __vtkKMeansAssessFunctor_h
#define __vtkKMeansAssessFunctor_h

class vtkKMeansAssessFunctor : public vtkStatisticsAlgorithm::AssessFunctor
{
  vtkDoubleArray* Distances;
  vtkIdTypeArray* ClusterMemberIDs;
  int NumRuns;
  
public:
  static vtkKMeansAssessFunctor* New();
  vtkKMeansAssessFunctor() { }
  virtual ~vtkKMeansAssessFunctor();
  virtual void operator () ( vtkVariantArray* result, vtkIdType row );
  bool Initialize( vtkTable *inData, vtkTable *reqModel, vtkKMeansDistanceFunctor *distFunc );
  int GetNumberOfRuns() { return NumRuns; }
};

#endif // __vtkKMeansAssessFunctor_h
