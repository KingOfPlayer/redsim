#include "freefemscript.h"

FreeFemScript::FreeFemScript() : EValue(210e9), PoissonRatioValue(0.3) {
	
}

void FreeFemScript::addVertexGroup(std::unique_ptr<VertexGroupBaseType> group)
{
	vertexGroups.push_back(std::move(group));
}

void FreeFemScript::replacePlaceholder(std::string &target, const std::string &placeholder, const std::string &value) const
{
	size_t pos = target.find(placeholder);
	if (pos != std::string::npos)
	{
		target.replace(pos, placeholder.length(), value);
	}
}

bool FreeFemScript::GenerateScript()
{

	if (scriptPath.empty())
	{
		printf("Error: Script path is empty. Cannot generate FreeFEM script.\n");
		return false;
	}

	if (vertexGroups.empty())
	{
		printf("Warning: No vertex groups added. The generated script will not contain any boundary conditions or forces.");
	}
	else
	{
		// Check fixed and force groups for valid points
		bool fixed = false;
		bool force = false;
		for (const auto &group : vertexGroups)
		{
			if (group->getLabelType() == VertexGroupLabelType::Fixed)
			{
				fixed = true;
			}
			else if (group->getLabelType() == VertexGroupLabelType::Force)
			{
				force = true;
			}
		}
		if (!(fixed || force))
		{
			printf("Warning: No valid vertex groups (fixed or force) found. The generated script will not contain any boundary conditions or forces.");
			return false;
		}
	}

	if (EValue <= 0)
	{
		printf("Warning: Invalid Young's modulus value (E). The generated script may not run correctly.");
		return false;
	}

	if (PoissonRatioValue < 0 || PoissonRatioValue >= 0.5)
	{
		printf("Warning: Invalid Poisson's ratio value (nu). The generated script may not run correctly.");
		return false;
	}

	if (meshFilePath.empty())
	{
		printf("Warning: Mesh file path is empty. The generated script will not be able to load the mesh.");
		return false;
	}

	std::string scriptContent = getBaseScript();

	replacePlaceholder(scriptContent, "[[E_Value]]", std::to_string(EValue));
	replacePlaceholder(scriptContent, "[[Possion_Ratio_Value]]", std::to_string(PoissonRatioValue));
	replacePlaceholder(scriptContent, "[[MeshFilePath]]", meshFilePath);

	std::string stiffnessParts;
	std::string rhsParts;

	for (const auto &group : vertexGroups)
	{
		std::string stiffnessPart = group->generateStiffnessPart();
		if (!stiffnessPart.empty()) stiffnessParts += "\n +" + stiffnessPart;
		std::string rhsPart = group->generateRhsPart();
		if (!rhsPart.empty()){
			if (rhsParts.empty())
				rhsParts += rhsPart;
			else
				rhsParts += "\n +" +rhsPart;
		}
	}

	replacePlaceholder(scriptContent, "[[StiffnessPart]]", stiffnessParts);
	replacePlaceholder(scriptContent, "[[RhsPart]]", rhsParts);

	std::ofstream outFile(scriptPath);
	if (outFile.is_open())
	{
		outFile << scriptContent;
		outFile.close();
		printf("FreeFEM script generated successfully at: %s\n", scriptPath.c_str());
		return true;
	}
	else
	{
		printf("Error: Unable to open file for writing: %s\n", scriptPath.c_str());
		return false;
	}
}

std::string FreeFemScript::getBaseScript() const
{
	return R"fe_script(
load "msh3"
load "PETSc"
load "iovtk"
load "medit"


real E     = [[E_Value]];//[MPa]
real nu    = [[Possion_Ratio_Value]];

real lambda = E * nu / ((1. + nu) * (1. - 2.*nu));
real mu     = E / (2. * (1. + nu));

cout << "lambda=" << lambda << "  mu=" << mu << endl;

// 1. Load the mesh completely into memory
cout << "Reading mesh from disk..." << endl;
mesh3 Th = readmesh3("[[MeshFilePath]]");
cout << "Mesh Loaded: " << Th.nv << " vertices, " << Th.nt << " tets" << endl;

// 2. Define standard Sequential Finite Element Spaces
fespace Vh(Th, [P1, P1, P1]);
fespace Wh(Th,  P1);

// Macros for Hooke's Law
macro eps11(u1,u2,u3)  (dx(u1))                    //EOM
macro eps22(u1,u2,u3)  (dy(u2))                    //EOM
macro eps33(u1,u2,u3)  (dz(u3))                    //EOM
macro eps12(u1,u2,u3)  (0.5*(dy(u1)+dx(u2)))       //EOM
macro eps13(u1,u2,u3)  (0.5*(dz(u1)+dx(u3)))       //EOM
macro eps23(u1,u2,u3)  (0.5*(dz(u2)+dy(u3)))       //EOM
macro divU(u1,u2,u3)   (dx(u1)+dy(u2)+dz(u3))      //EOM

macro a(u1,u2,u3,v1,v2,v3) (
    lambda*divU(u1,u2,u3)*divU(v1,v2,v3)
  + 2.*mu*(
      eps11(u1,u2,u3)*eps11(v1,v2,v3)
    + eps22(u1,u2,u3)*eps22(v1,v2,v3)
    + eps33(u1,u2,u3)*eps33(v1,v2,v3)
    + 2.*eps12(u1,u2,u3)*eps12(v1,v2,v3)
    + 2.*eps13(u1,u2,u3)*eps13(v1,v2,v3)
    + 2.*eps23(u1,u2,u3)*eps23(v1,v2,v3)
  )
)                                                   //EOM

// 3. Define Variational Problems
varf vElasticity([ux,uy,uz],[vx,vy,vz])
    = int3d(Th)( a(ux,uy,uz,vx,vy,vz) ) 
    [[StiffnessPart]] 
    ;

varf vRhs([ux,uy,uz],[vx,vy,vz])
    = 
	[[RhsPart]]
    ;

// 4. Build System Matrix using PETSc
matrix Loc = vElasticity(Vh, Vh);
real[int] rhs = vRhs(0, Vh);

// Convert standard FreeFEM matrix to sequential PETSc matrix format
Mat A(Loc); 

// 5. Solve using PETSc 
Vh [ux, uy, uz];
set(A, sparams="-ksp_type preonly -pc_type lu -pc_factor_mat_solver_type mumps");

cout << "Solving system with PETSc solver..." << endl;
ux[] = A^-1 * rhs;

cout << "max |uz| = " << uz[].linfty << endl;

Wh s11, s22, s33, s12, s13, s23, vmises;

s11 = lambda*(dx(ux)+dy(uy)+dz(uz)) + 2.*mu*dx(ux);
s22 = lambda*(dx(ux)+dy(uy)+dz(uz)) + 2.*mu*dy(uy);
s33 = lambda*(dx(ux)+dy(uy)+dz(uz)) + 2.*mu*dz(uz);
s12 = mu*(dy(ux)+dx(uy));
s13 = mu*(dz(ux)+dx(uz));
s23 = mu*(dz(uy)+dy(uz));

vmises = sqrt(0.5*(
    (s11-s22)^2 + (s22-s33)^2 + (s33-s11)^2
  + 6.*(s12^2+s13^2+s23^2)
));

cout << "max von Mises = " << vmises[].max << " MPa" << endl;

cout << "Computation complete. Saving results to disk..." << endl;

)fe_script";
}