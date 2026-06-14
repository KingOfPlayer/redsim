#include "project.h"

#include "../../core/renderer/object.h"

#include "../file/file.h"
#include "../gcode/gcode.h"

#include "../modelgen/layermapper.h"
#include "../modelgen/modelgenhelper.h"
#include "../modelgen/tetrahedralmesher.h"
#include "../freefem/freefemtype.h"
#include "../freefem/freefem.h"
#include "../freefem/freefemscript.h"

Project::Project(){
    gcodeModule = std::make_unique<GCodeModule>();
    layerMapper = std::make_unique<LayerMapper>();
    tetrahedralMesher = std::make_unique<TetrahedralMesher>();
    freefemScript = std::make_unique<FreeFemScript>();
    freefemModule = std::make_unique<FreeFemModule>();
}

Project::~Project(){
}

std::string Project::GetCurrentFilePath(){
    FilePath* currentFilepath = gcodeModule->currentFile.get();
    if(currentFilepath) {
        return currentFilepath->path;
    } else {
        return "";
    }
}

std::string Project::GetFileDirectory(){
    std::string currentFilepath = GetCurrentFilePath();
    if (currentFilepath.empty()) {
        return "";
    }
    std::string fileDirectory = currentFilepath.substr(0, currentFilepath.find_last_of("/\\"));
    return fileDirectory;
}

std::string Project::GetFilenameWithoutExtension(){
    std::string currentFilepath = GetCurrentFilePath();
    if (currentFilepath.empty()) {
        return "";
    }
    std::string filenameWithoutExt = currentFilepath.substr(currentFilepath.find_last_of("/\\") + 1);
    filenameWithoutExt = filenameWithoutExt.substr(0, filenameWithoutExt.find_last_of('.'));
    return filenameWithoutExt;
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

bool Project::HasShellMeshGenerated(){
    return isMeshGenerated;
}

std::unique_ptr<Object>& Project::GetMeshRenderObject(){
    return MeshRenderObject;
}

LayerMapper& Project::GetLayerMapper(){
    return *layerMapper;
}

void Project::GenerateTetrahedralMesh(){
    if(!HasShellMeshGenerated()) {
        printf("No shell mesh generated yet. Cannot generate tetrahedral mesh.\n");
        return;
    }

    TetrahedralMesherResult result = tetrahedralMesher->ProcessMeshForTetrahedral(*shellMesh);
    isTetrahedralMeshGenerated = true;
    tetrahedralMeshResult = std::make_unique<TetrahedralMesherResult>(std::move(result));

    TetrahedralMeshRenderObject = std::make_unique<Object>(
        ModelgenHelper::MeshToRenderObject(tetrahedralMeshResult->mesh)
    );
}

bool Project::HasTetrahedralMeshGenerated(){
    return isTetrahedralMeshGenerated;
}


std::unique_ptr<Object>& Project::GetTetrahedralMeshMeshRenderObject(){
    return TetrahedralMeshRenderObject;
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

    tetrahedralMesher->SaveTetrahedralMesherResultToFile(*tetrahedralMeshResult, outputFilePath);
    freefemScript->setMeshFilePath(outputFilePath);
    isTetrahedralMeshSaved = true;
    printf("Tetrahedral mesh saved to: %s\n", outputFilePath.c_str());
}

TetrahedralMesher& Project::GetTetrahedralMesher(){
    return *tetrahedralMesher;
}

FreeFemScript& Project::GetFreeFemScriptInstance(){
    return *freefemScript;
}

FreeFemModule& Project::GetFreeFemModuleInstance(){
    return *freefemModule;
}

void Project::ApplyLabel(std::vector<std::unique_ptr<VertexGroupBaseType>> groups){
    if(!HasTetrahedralMeshGenerated()) {
        printf("No tetrahedral mesh generated yet. Cannot label mesh.\n");
        return;
    }

    tetrahedralMeshResult->c3t3 = tetrahedralMesher->LabelC3t3(tetrahedralMeshResult->c3t3, groups);
    
    freefemScript->setVertexGroups(std::move(groups));
    
}