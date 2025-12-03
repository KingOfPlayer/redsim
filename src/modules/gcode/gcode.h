
#pragma once
#include <cstdio>
#include "../file/file.h"
#include <vector>

struct GcodePoint {
    float x;
    float y;
    float z;
    float e;
};

struct GCodeMachineState{
    GcodePoint position;
    float feedrate;
    float absolute_positioning;
    float absolute_extrusion;

};

struct GcodePath{
    GCodeMachineState* state;
    GcodePoint* start;
    GcodePoint* end;
};

struct GCodeProgramCommand{
    char command;
    int id;
    std::vector<GCodeArgument> arguments;
};

struct GCodeArgument{
    char letter;
    float value;
};

class GCodeModule{
public:
    std::vector<GcodePoint> points;
    std::vector<GcodePath> paths;
    GCodeMachineState state;
    void OpenFile(FilePath* filepath);
    void ExtractPaths(FilePath* filepath);
    GCodeProgramCommand ExtractProgramFromLine(char* line);
};