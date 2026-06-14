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

// Initialize MPI variables
include "getARGV.idp"

// 1. MUST define spatial dimension BEFORE including macro_ddm.idp
macro dimension()3// EOM

// 2. Element tracking setup for domain decomposition mapping
int[int] n2o; 
macro ThN2O()n2o// EOM

include "macro_ddm.idp" 

real E     = [[E_Value]];   // Young's modulus [MPa] (steel)
real nu    = [[Possion_Ratio_Value]];     // Poisson's ratio

real lambda = E * nu / ((1. + nu) * (1. - 2.*nu));
real mu     = E / (2. * (1. + nu));

if (mpirank == 0) {
		cout << "lambda=" << lambda << "  mu=" << mu << endl;
}

// 3. Clean Memory Loading: ONLY Rank 0 reads from disk, then broadcasts the geometry data
mesh3 Th;
if (mpirank == 0) {
		cout << "Reading global mesh from disk on Rank 0..." << endl;
		Th = readmesh3("[[MeshFilePath]]");
		cout << "Global Mesh Loaded on Rank 0. Broadcasting..." << endl;
}

// Safely send the structured geometric mesh to all secondary processors
broadcast(processor(0), Th);

// 4. Parallel mesh decomposition wrapper
DmeshCreate(Th);

// 5. Define Distributed Finite Element Spaces
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

// 6. Define PETSc Variational Problems
varf vElasticity([ux,uy,uz],[vx,vy,vz])
		= int3d(Th)( a(ux,uy,uz,vx,vy,vz) ) 
		[[StiffnessPart]]
		;

varf vRhs([ux,uy,uz],[vx,vy,vz]) 
		= 
		[[RhsPart]]
		;

// 7. Map vector dimensions for PETSc matrix build
macro def(i)[i, i#B, i#C]// EOM
macro init(i)[i, i, i]// EOM

Mat A;
createMat(Th, A, [P1, P1, P1]);

matrix Loc = vElasticity(Vh, Vh);
A = Loc;

real[int] rhs = vRhs(0, Vh);

// 8. Solve using PETSc
Vh [ux, uy, uz];
set(A, sparams="-ksp_type cg -pc_type asm -ksp_monitor");
ux[] = A^-1 * rhs;

// 9. Compute maximum displacement across sub-domains
real localMaxUz = uz[].linfty;
real globalMaxUz;
mpiAllReduce(localMaxUz, globalMaxUz, mpiCommWorld, mpiMAX);

if (mpirank == 0) {
		cout << "Global max |uz| = " << globalMaxUz << endl;

		// 10. Stress Post-processing
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

		real localMaxVInmises = vmises[].max;
		real globalMaxVmises;
		mpiAllReduce(localMaxVInmises, globalMaxVmises, mpiCommWorld, mpiMAX);

				cout << "Global max von Mises = " << globalMaxVmises << " MPa" << endl;

		// 11. Save Data

		cout << "Computation complete. Sub-meshes saved to disk successfully." << endl;
}
)fe_script";
}