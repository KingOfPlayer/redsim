#include "project.h"

#include "../../core/renderer/object.h"

#include "../file/file.h"
#include "../gcode/gcode.h"

#include "../modelgen/layermapper.h"
#include "../modelgen/modelgenhelper.h"
#include "../modelgen/tetrahedralmesher.h"

Project::Project(){
    gcodeModule = std::make_unique<GCodeModule>();
    layerMapper = std::make_unique<LayerMapper>();
    tetrahedralMesher = std::make_unique<TetrahedralMesher>();
}

Project::~Project(){
}

void Project::LoadGCode(FilePath* filepath){
    gcodeModule->OpenFile(filepath);
    gcodeModule->ExtractPointsAndPaths();
    isGCodeFileLoaded = true;
}

FilePath* Project::GetCurrentGCodeFilePath(){
    return gcodeModule->currentFile.get();
}

void Project::GenerateRenderObjectFromGCode(){
    GCodeRenderObject = std::make_unique<Object>(gcodeModule->ConvertPathToRenderObject());
    isGCodeRenderObjectGenerated = true;
}

bool Project::HasGCodeRenderObject(){
    return isGCodeRenderObjectGenerated;
}

std::unique_ptr<Object>& Project::GetGCodeRenderObject(){
    return GCodeRenderObject;
}

bool Project::isProjectLoaded(){
    return isGCodeFileLoaded;
}

void Project::ExtractLayers(){
    std::vector<GCodeLayer> layers = gcodeModule->ExtractLayers();

    printf("Extracted %zu layers from GCode.\n", layers.size());
}

void Project::GenerateShellMesh(){
    std::vector<GCodeLayer> layers = gcodeModule->ExtractLayers();
    
    //layerMapper.CreateRenderObjectFromMesh(layers);

    shellMesh = std::make_unique<Mesh>(
        layerMapper->GenerateMesh(layers)
    );

    MeshRenderObject = std::make_unique<Object>(
        ModelgenHelper::MeshToRenderObject(*shellMesh)
    );
    isMeshGenerated = true;

    printf("Generated 3D Mesh from Layers.\n");
}



bool Project::HasMeshGenerated(){
    return isMeshGenerated;
}

std::unique_ptr<Object>& Project::GetMeshRenderObject(){
    return MeshRenderObject;
}

LayerMapper& Project::GetLayerMapper(){
    return *layerMapper;
}


void Project::GenerateTetrahedralMesh(){
    if(!HasMeshGenerated()) {
        printf("No shell mesh generated yet. Cannot generate tetrahedral mesh.\n");
        return;
    }

    TetrahedralMesherResult result = tetrahedralMesher->MeshToTetrahedral(*shellMesh);
    isTetrahedralMeshGenerated = true;
    tetrahedralMeshResult = std::make_unique<TetrahedralMesherResult>(std::move(result));
}

bool Project::HasTetrahedralMeshGenerated(){
    return isTetrahedralMeshGenerated;
}

void Project::SaveTetrahedralMeshToFile(){

    if(!gcodeModule->currentFile) {
        printf("No GCode file loaded. Cannot determine output file path.\n");
        return;
    }

    if(!HasTetrahedralMeshGenerated()) {
        printf("No tetrahedral mesh generated yet. Cannot save to file.\n");
        return;
    }

    std::string currentFilepath = gcodeModule->currentFile->path; // ".../.../test.gcode"

    std::string fileDirectory = currentFilepath.substr(0, currentFilepath.find_last_of("/\\")); // ".../..."
    std::string filenameWithoutExt = currentFilepath.substr(currentFilepath.find_last_of("/\\") + 1); // "test.gcode"
    filenameWithoutExt = filenameWithoutExt.substr(0, filenameWithoutExt.find_last_of('.')); // "test"
    std::string outputPath = filenameWithoutExt + "_tetrahedral.mesh"; // "test_tetrahedral.mesh"
    std::string outputFilePath = fileDirectory + "/" + outputPath; // ".../.../test_tetrahedral.mesh"

    tetrahedralMesher->SaveTetrahedralMeshToFile(*tetrahedralMeshResult, outputFilePath);
}