#include "XgcExtrudeCompute.h"
#include <vtkm/cont/ArrayHandleExtrudeCoords.h>
#include <vtkm/cont/CellSetExtrude.h>
#include <vtkm/filter/ClipWithField.h>

#include <kittie.h>



void XgcExtrudeCompute::initializeReaders(std::string mp, std::string mn,
                                            std::string dp, std::string dn,
                                            MPI_Comm comm)
{
    diagname = dn;
    meshname = mn;
    diagIO = kdeclare_io(this->diag, diagname);
    meshIO = kdeclare_io(this->mesh, meshname);
    diagReader = kopen(diagIO, dp, dn, adios2::Mode::Read, comm);
    meshReader = kopen(meshIO, mp, mn, adios2::Mode::Read, comm);



    
    adios2::Variable<int> nVar = meshIO.InquireVariable<int>("n_n");
    adios2::Variable<int> triVar = meshIO.InquireVariable<int>("n_t");
    adios2::Variable<int> phiVar = fileIO.InquireVariable<int>("nphi");

    if (nVar){
        meshReader.Get(nVar, &numNodes, adios2::Mode::Sync);

    }
    if (triVar)
        meshReader.Get(triVar, &numTris, adios2::Mode::Sync);

    if (phiVar){
        fileReader.Get(phiVar, &numPhi, adios2::Mode::Sync);
        std::cout << "phi: " << numPhi << std::endl;
    }
    
}

void XgcExtrudeCompute::readMesh()

{
   std::cout << "numNodes: " << numNodes << ", numTris " << numTris << ", numPhi " << numPhi << std::endl;
   adios2::Variable<double> coordVar = meshIO.InquireVariable<double>("/coordinates/values");

  std::vector<double> buff;

  //const int newPhi = phiMultiplier * numPhi;
  int newPhi = numPhi/2 + 1; //+1 for the picket fence problem

  meshReader.Get(coordVar, buff, adios2::Mode::Sync);

  //TODO: go back to how it was before
  coords = vtkm::cont::make_ArrayHandleExtrudeCoords( vtkm::cont::make_ArrayHandle(buff, vtkm::CopyFlag::On), newPhi, false,vtkm::Pi());
  //coords = vtkm::cont::make_ArrayHandleExtrudeCoords(buff, newPhi, false, vtkm::CopyFlag::On);
  std::vector<int> ibuffc, ibuffn;

  //vtkDataArray *conn = NULL, *nextNode = NULL;
  // meshFile->ReadScalarData("/cell_set[0]/node_connect_list", timestate, &conn);
  // if (!meshFile->ReadScalarData("/nextnode", timestate, &nextNode))
  //     meshFile->ReadScalarData("nextnode", timestate, &nextNode);
  auto nodeConnectorVar = meshIO.InquireVariable<int>("/cell_set[0]/node_connect_list");
  auto nextNodeVar = meshIO.InquireVariable<int>("nextnode");
  // if (!nodeConnectorVar || !nextNodeVar)
  //     return vtkm::cont::DataSet();

  meshReader.Get(nodeConnectorVar, ibuffc,adios2::Mode::Sync);

  meshReader.Get(nextNodeVar, ibuffn, adios2::Mode::Sync);
  // if (ibuffn.size() < 1)
  //     return vtkm::cont::DataSet();

  auto connectivity = vtkm::cont::make_ArrayHandle(ibuffc, vtkm::CopyFlag::On);
  auto nextNode = vtkm::cont::make_ArrayHandle(ibuffn, vtkm::CopyFlag::On);
  cells = vtkm::cont::make_CellSetExtrude(connectivity, coords, nextNode, false);

  ds.AddCoordinateSystem(vtkm::cont::CoordinateSystem("coords", coords));
  ds.SetCellSet(cells);

  //return ds;

}


void XgcExtrudeCompute::clip(vtkm::cont::ArrayHandle<double> output)
{
    vtkm::filter::ClipWithField clip;
    clip.SetClipValue(0.1);
    clip.SetActiveField("pointvar");
    vtkm::cont::DataSet ret;
    ret.AddCoordinateSystem(vtkm::cont::CoordinateSystem("coords", coords));
    ret.SetCellSet(cells);
    ret.AddField(vtkm::cont::make_FieldPoint("pointvar",  output));
    ds = clip.Execute(ret);
}

vtkm::cont::ArrayHandleTransform<vtkm::cont::ArrayHandle<double>, internal::Clamp<double>> 
    XgcExtrudeCompute::clamp(vtkm::cont::ArrayHandle<double> output)
{
    internal::Clamp<double> functor(-0.5, 0.5);
    return vtkm::cont::make_ArrayHandleTransform(output, functor);
}

void XgcExtrudeCompute::readValues()
{

    // auto var = fileIO->InquireVariable<double>("dpot");
    // std::vector<double> buff;
    // fileReader->Get(var, buff,adios2::Mode::Sync);
    // auto dpot = vtkm::cont::make_ArrayHandle(buff, vtkm::CopyFlag::On );

    vtkm::cont::ArrayHandle<double> temperature;

    //auto output =  GetiTurbulence(temperature);
    auto output = clamp(GetiTurbulence(temperature));

    ds = vtkm::cont::DataSet();
    ds.AddCoordinateSystem(vtkm::cont::CoordinateSystem("coords", coords));
    ds.SetCellSet(cells);
    ds.AddField(vtkm::cont::make_FieldPoint("pointvar",  output));
}

void XgcExtrudeCompute::close()
{
    XgcExtrude::close();
}