
#pragma once
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <string>
#include <vector>
#include "../file/file.h"

#define READER_BUFFER_SIZE 256
#define PROGRAM_BUFFER_SIZE 128


struct GCodePoint {
    float x;
    float y;
    float z;
    float e;
};

struct GCodeMachineState{
    GCodePoint globalPosition;
    GCodePoint absolutePosition;
    GCodePoint homePosition;
    float feedrate;
    bool absolutePositioning;
    bool absoluteExtrusion;

    GCodeMachineState(){
        globalPosition = {0.0f, 0.0f, 0.0f, 0.0f};
        absolutePosition = {0.0f, 0.0f, 0.0f, 0.0f};
        homePosition = {0.0f, 0.0f, 0.0f, 0.0f};
        feedrate = 0.0f;
        absolutePositioning = true;
        absoluteExtrusion = true;
    }

    GCodePoint GetGlobalPosition(){
        return globalPosition;
    }

    GCodePoint GetAbsolutePosition(){
        return absolutePosition;
    }

    GCodePoint GetHomePosition(){
        return homePosition;
    }

    GCodePoint GetPosition(){
        GCodePoint pos = globalPosition;
        if(absolutePositioning){
            pos.x -= absolutePosition.x;
            pos.y -= absolutePosition.y;
            pos.z -= absolutePosition.z;
        }
        if(absoluteExtrusion){
            pos.e -= absolutePosition.e;
        }
        return pos;
    }

    void SetAbsulutePosition(const GCodePoint pos){
        absolutePosition = pos;
    }

    void SetHomePosition(const GCodePoint pos){
        homePosition = pos;
    }

    void UpdatePosition(const GCodePoint newPos){

        if(absolutePositioning){
            globalPosition.x = newPos.x + absolutePosition.x;
            globalPosition.y = newPos.y + absolutePosition.y;
            globalPosition.z = newPos.z + absolutePosition.z;
            globalPosition.e = newPos.e + absolutePosition.e;
            return;
        }

        globalPosition.x = newPos.x;
        globalPosition.y = newPos.y;
        globalPosition.z = newPos.z;
        globalPosition.e = newPos.e;
    }
};

struct GcodePath{
    GCodeMachineState state;
    int start;
    int end;
};

struct GCodeArgument{
    char letter;
    float value;
};

struct GCodeProgramCommand{
    char command;
    int id;
    std::vector<GCodeArgument> arguments;
};

class GCodeModule{
public:
    std::vector<GCodePoint> points;
    std::vector<GcodePath> paths;
    GCodeMachineState state;

    std::vector<GCodeProgramCommand> programCommands;
    void OpenFile(FilePath* filepath);
    GCodeProgramCommand ParseGCodeLine(char* line);

    void ExtractPointsAndPaths();
    void SavePointsAndPathsToObj(const char* outputPath);

private:
    void ProcessGCommand(const GCodeProgramCommand& cmd);
    void ProcessMCommand(const GCodeProgramCommand& cmd);
};