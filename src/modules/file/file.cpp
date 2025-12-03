
#include "file.h"

FilePath FileModule::SelectFile(){
    char filepath[PATH_SIZE];
    FILE *fop;
    FilePath fp;

    fop = popen("zenity --file-selection --file-filter=*.gcode", "r");

    if (fop == NULL) {
        return fp;
    }else{
        if (fgets(filepath, PATH_SIZE, fop) != NULL) {
            fclose(fop);
            fp.path_length = strlen(filepath);
            fp.path = new char[fp.path_length];
            strcpy(fp.path, filepath);
            fp.path[strcspn(fp.path, "\n")] = 0;
            return fp;
        } else {
            fclose(fop);
            return fp;
        }
    }
}