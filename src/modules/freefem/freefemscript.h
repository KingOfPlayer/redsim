#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include "freefemtype.h"

//Freefem script
class FreeFemScript {
  std::vector<std::unique_ptr<VertexGroupBaseType>> vertexGroups;
  std::string scriptPath;
  std::string meshFilePath;
  double EValue;
  double PoissonRatioValue;
public:
  FreeFemScript();

  std::string getBaseScript() const;
  void setVertexGroups(std::vector<std::unique_ptr<VertexGroupBaseType>> groups) {
      vertexGroups = std::move(groups);
  }
  void addVertexGroup(std::unique_ptr<VertexGroupBaseType> group);
  void setMaterialProperties(double E, double nu) {
      EValue = E;
      PoissonRatioValue = nu;
  }
  void setScriptPath(const std::string& path) {
      scriptPath = path;
  }
  void setMeshFilePath(const std::string& path) {
      meshFilePath = path;
  }

  std::string getScriptPath(){
      return scriptPath;
  }

  void replacePlaceholder(std::string& target, const std::string& placeholder, const std::string& value) const;

  bool GenerateScript();
};