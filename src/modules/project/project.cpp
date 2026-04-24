#include "project.h"

#include "../../core/renderer/object.h"

#include "../file/file.h"
#include "../gcode/gcode.h"
#include "../modelgen/layermapper.h"

Project::Project(){
    gcodeModule = std::make_unique<GCodeModule>();
    layerMapper = std::make_unique<LayerMapper>();
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

Object& Project::GetGCodeRenderObject(){
    return *GCodeRenderObject;
}

bool Project::isProjectLoaded(){
    return isGCodeFileLoaded;
}

void Project::ExtractLayers(){
    std::vector<GCodeLayer> layers = gcodeModule->ExtractLayers();

    printf("Extracted %zu layers from GCode.\n", layers.size());
}

void Project::Generate3DMeshFromLayers(){
    std::vector<GCodeLayer> layers = gcodeModule->ExtractLayers();
    
    //layerMapper.CreateRenderObjectFromMesh(layers);

    MeshRenderObject = std::make_unique<Object>(layerMapper->CreateRenderObjectFromMesh(layers));
    isMeshGenerated = true;

    printf("Generated 3D Mesh from Layers.\n");
}

bool Project::HasMeshGenerated(){
    return isMeshGenerated;
}

Object& Project::GetMeshRenderObject(){
    return *MeshRenderObject;
}

LayerMapper& Project::GetLayerMapper(){
    return *layerMapper;
}