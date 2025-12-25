
#include "gcode.h"

void GCodeModule::OpenFile(FilePath *filepath)
{
    FILE *file = fopen(filepath->path, "r");
    if (file == nullptr)
    {
        printf("Failed to open file: %s", filepath->path);
        return;
    }

    currentFile = new FilePath();
    *currentFile = *filepath;
    
    while (!feof(file))
    {
        long index = ftell(file);
        char buffer[READER_BUFFER_SIZE];
        bool comment = false;
        bool program = false;
        bool newLine = false;
        char programBuffer[PROGRAM_BUFFER_SIZE];
        int programBufferIndex = 0;
        if (fgets(buffer, sizeof(buffer), file) != nullptr)
        {
            for (int i = 0; buffer[i] != '\0'; i++)
            {
                // Process each character in the line
                char c = buffer[i];
                if (c == ';' && !comment)
                {
                    comment = true;
                }
                else if (comment && c == '\n')
                {
                    comment = false;
                    continue;
                }
                else if (!comment)
                {
                    if ((c == 'G' || c == 'M') && !program)
                    {
                        program = true;
                        programBuffer[programBufferIndex++] = c;
                    }
                    else if (program)
                    {
                        if (c == '\n' || c == '\r')
                        {
                            programBuffer[programBufferIndex] = '\0';
                            program = false;

                            // printf("Program Buffer: %s\n", programBuffer);
                            
                            GCodeProgramCommand command = ParseGCodeLine(programBuffer);

                            programCommands.push_back(command);

                            /*printf("Extracted Command: %c-%d\n", command.command, command.id);
                            printf("Arguments:\n");
                            for (const auto &arg : command.arguments)
                            {
                                printf("  %c: %.2f\n", arg.letter, arg.value);
                            }*/

                            programBufferIndex = 0;
                        }
                        else
                        {
                            programBuffer[programBufferIndex++] = c;
                        }
                    }
                }
            }
        }
    }

    fclose(file);
}

GCodeProgramCommand GCodeModule::ParseGCodeLine(char *line)
{
    GCodeProgramCommand command;
    command.id = -1; // Default ID

    char *buffer = strtok(line, " ");
    while (buffer != nullptr)
    {
        if (buffer[0] == 'G' || buffer[0] == 'M')
        {
            command.command = buffer[0];
            command.id = atoi(&buffer[1]);
        }
        else
        {
            GCodeArgument arg;
            arg.letter = buffer[0];
            arg.value = atof(&buffer[1]);
            command.arguments.push_back(arg);
        }
        buffer = strtok(nullptr, " ");
    }
    return command;
}

void GCodeModule::ExtractPointsAndPaths()
{

    // Reset Machine
    state.globalPosition = {0.0f, 0.0f, 0.0f, 0.0f};
    state.homePosition = {0.0f, 0.0f, 0.0f, 0.0f};
    state.feedrate = 0.0f;
    state.absolutePositioning = true;
    state.absoluteExtrusion = true;

    for (const auto &cmd : programCommands)
    {
        switch (cmd.command)
        {
        case 'G':
            ProcessGCommand(cmd);
            break;
        case 'M':
            ProcessMCommand(cmd);
            break;
        default:
            break;
        }
    }
}

void GCodeModule::ProcessGCommand(const GCodeProgramCommand &cmd)
{
    switch (cmd.id)
    {
    case 0: // G0 - Rapid Move
    case 1: // G1 - Linear Move
    {
        GcodePath path;
        GCodePoint newPoint = state.GetPosition();
        bool isMove = false;
        bool isExtrusionMove = false;
        for (const auto &arg : cmd.arguments)
        {
            switch (arg.letter)
            {
            case 'X':
                isMove = true;
                newPoint.x = arg.value;
                break;
            case 'Y':
                isMove = true;
                newPoint.z = arg.value;
                break;
            case 'Z':
                isMove = true;
                newPoint.y = -arg.value;
                break;
            case 'E':
                newPoint.e = arg.value;
                isExtrusionMove = true;
                break;
            case 'F':
                state.feedrate = arg.value;
                break;
            default:
                break;
            }
        }

        if (isMove && isExtrusionMove)
        {
            // Check extrusion start point vaild
            // If point not exist or different than last point, add it
            if (points.size() == 0)
            {
                points.push_back(state.GetPosition());
            }
            else
            {
                GCodePoint lastPos = points.back();
                GCodePoint currentPos = state.GetPosition();
                if (points.size() == 0 ||
                    currentPos.x != lastPos.x ||
                    currentPos.y != lastPos.y ||
                    currentPos.z != lastPos.z)
                {
                    points.push_back(currentPos);
                }
            }

            path.start = points.size();
            points.push_back(newPoint);
            path.end = points.size();
            paths.push_back(path);
        }
        state.UpdatePosition(newPoint);
    }
    break;
    case 28: // G28 - Home
    {
        state.globalPosition = state.homePosition;
    }
    break;
    case 90: // G90 - Absolute Positioning
        state.absolutePositioning = true;
        break;
    case 91: // G91 - Relative Positioning
        state.absolutePositioning = false;
        break;
    case 92: // G92 - Set Position
    {
        for (const auto &arg : cmd.arguments)
        {
            switch (arg.letter)
            {
            case 'X':
                state.globalPosition.x = arg.value;
                break;
            case 'Y':
                state.globalPosition.z = arg.value;
                break;
            case 'Z':
                state.globalPosition.y = arg.value;
                break;
            case 'E':
                state.globalPosition.e = arg.value;
                break;
            }
        }
    }
    break;
    default:
        printf("Unhandled G command: G%d\n", cmd.id);
        break;
    }
}

void GCodeModule::ProcessMCommand(const GCodeProgramCommand &cmd)
{
    switch (cmd.id)
    {
    case 82: // M82 - Set Extruder to Absolute Mode
        state.absoluteExtrusion = true;
        break;
    case 84: // M84 - Stop Idle Hold
        // Handle idle hold stop
        break;
    case 104: // M104 - Set Extruder Temperature
        // Handle extruder temperature setting
        break;
    case 105: // M105 - Get Extruder Temperature
        // Handle extruder temperature query
        break;
    case 106: // M106 - Fan On
        // Handle fan on
        break;
    case 107: // M107 - Fan Off
        // Handle fan off
        break;
    case 109: // M109 - Set Extruder Temperature and Wait
        // Handle extruder temperature setting and wait
        break;
    case 140: // M140 - Set Bed Temperature
        // Handle bed temperature setting
        break;
    case 190: // M190 - Set Bed Temperature and Wait
        // Handle bed temperature setting and wait
        break;
    default:
        printf("Unhandled M command: M%d\n", cmd.id);
        break;
    }
}

void GCodeModule::SavePointsAndPathsToObj(const char *outputPath)
{
    FILE *file = fopen(outputPath, "w");
    if (file == nullptr)
    {
        printf("Failed to open file for writing: %s\n", outputPath);
        return;
    }

    // Vertices
    for (const auto &point : points)
    {
        fprintf(file, "v %.4f %.4f %.4f\n", point.x, point.y, point.z);
    }

    // Edges
    for (const auto &path : paths)
    {
        fprintf(file, "l %d %d\n", path.start, path.end);
    }

    fclose(file);
    printf("Saved points and paths to OBJ file: %s\n", outputPath);
}

Object GCodeModule::ConvertPathToRenderObject() {

    glm::vec3 min(1e9), max(-1e9);
    for (auto& p : points) {
        if(p.x < min.x) min.x = p.x; if(p.x > max.x) max.x = p.x;
        if(p.y < min.y) min.y = p.y; if(p.y > max.y) max.y = p.y;
        if(p.z < min.z) min.z = p.z; if(p.z > max.z) max.z = p.z;
    }

    // 2. Calculate the Center
    glm::vec3 center = (min + max) * 0.5f;

    // 3. Create Vertices relative to that center

    Object obj;
    obj.drawMode = GL_LINES;
    obj.useIndices = true;
    obj.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Bright Red to see easily

    std::vector<float> vertices;
    for (const auto& p : points) {
        vertices.push_back(p.x - center.x);
        vertices.push_back(p.y - center.y);
        vertices.push_back(p.z - center.z);
    }

    /*std::vector<float> vertices;
    for (const auto& p : points) {
        vertices.push_back(p.x);
        vertices.push_back(p.y);
        vertices.push_back(p.z);
    }*/

    std::vector<unsigned int> indices;
    for (const auto& path : paths) {
        // IMPORTANT: Subtract 1 because OBJ is 1-indexed, OpenGL is 0-indexed
        indices.push_back((unsigned int)path.start - 1);
        indices.push_back((unsigned int)path.end - 1);
    }
    obj.vertexCount = (uint32_t)indices.size();

    glGenVertexArrays(1, &obj.VAO);
    glBindVertexArray(obj.VAO);

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return obj;
}