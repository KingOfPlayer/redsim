
#include "gcode.h"
#include <vector>
#include <unordered_map>
#include <string>

void GCodeModule::OpenFile(FilePath* filepath) {
    FILE* file = fopen(filepath->path, "r");
    if (file == nullptr) {
        // Handle error: file could not be opened
        printf("Failed to open file: %s", filepath->path);
        return;
    }

    int newline_count = 0;
    int comment_count = 0;
    // Hash map for functions and their counts
    std::unordered_map<std::string, int> functionCounts;

    while (!feof(file)) {
        long index = ftell(file);
        char buffer[256];
        bool commentDetected = false;
        bool programDetected = false;
        char program_buffer[16];
        int program_index = 0;
        if (fgets(buffer, sizeof(buffer), file) != nullptr) {
            for(int i = 0; buffer[i] != '\0'; i++ ) {
                // Process each character in the line
                char c = buffer[i];
                
                if(c == '\n') {
                    // End of line reached
                    newline_count++;
                    programDetected = false;
                }else{
                    // Comment detected
                    if(c == ';' && !commentDetected){
                        comment_count++;
                        commentDetected = true;
                    }else if((c == 'G' || c == 'M') && !programDetected && !commentDetected){
                        // Program command detected
                        programDetected = true;
                        program_buffer[program_index++] = c;
                    }
                }

                if(programDetected){
                    if((c >= '0' && c <= '9')){
                        program_buffer[program_index++] = c;
                    }else if (c == ' ' || c == '\n' || c == '\0'){
                        // End of program command
                        program_buffer[program_index] = '\0';
                        // Store the command in the map
                        char* command = new char[program_index];
                        strcpy(command, program_buffer);
                        functionCounts[command]++;
                        // Reset for next command
                        program_index = 0;
                        programDetected = false;
                    }
                }
            }
        }
    }

    printf("Total new lines: %d\n", newline_count);
    printf("Total comments: %d\n", comment_count);

    for (const auto& pair : functionCounts) {
        printf("Function %s: %d times\n", pair.first.c_str(), pair.second);
    }    

    fclose(file);
}

void GCodeModule::ExtractPaths() {
    // Implementation for extracting paths from G-code points
    // This is a placeholder implementation and should be replaced with actual logic
    for (size_t i = 1; i < points.size(); ++i) {
        GcodePath path;
        path.start = &points[i - 1];
        path.end = &points[i];
        path.state = &state; // Assign current machine state
        paths.push_back(path);
    }
}

GCodeProgramCommand GCodeModule::ExtractProgramFromLine(char* line) {
    GCodeProgramCommand command;
    command.id = -1; // Default ID

    // Simple parsing logic for demonstration purposes
    if (line[0] == 'G' || line[0] == 'M') {
        command.command = line[0];
        command.id = atoi(&line[1]);

        // Further parsing for arguments can be added here
    }

    return command;
}