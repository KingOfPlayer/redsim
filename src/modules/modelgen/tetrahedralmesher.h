#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "modelgenhelper.h"
#include "tetrahedralmeshertypes.h"

#include <CGAL/IO/File_medit.h>

class VertexGroupBaseType;

struct TetrahedralMesherResult
{
	C3t3 c3t3;
	//Triangulation_3 volume;
	Mesh mesh;
};

class TetrahedralMesher {

public:
	double cell_size        = 2.0;
	double cell_radius_edge = 2.0;
	double facet_angle      = 25.0;
	double facet_size       = 2.0;
	double facet_distance   = 0.05;
	int    remesh_iterations = 1;  
	
	C3t3 MeshToC3t3(const Mesh& input_mesh);
	C3t3 LabelC3t3(C3t3& c3t3, const std::vector<std::unique_ptr<VertexGroupBaseType>>& groups);
	Triangulation_3 C3t3ToMesh(C3t3);

	TetrahedralMesherResult ProcessMeshForTetrahedral(const Mesh& input_mesh);
	Mesh TetrahedralToMesh(const C3t3& c3t3);
	static void SaveTetrahedralMesherResultToFile(const TetrahedralMesherResult& result, const std::string& filename);
};
