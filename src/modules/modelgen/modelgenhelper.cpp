#include "modelgenhelper.h"

#include "../../core/renderer/object.h"

Object ModelgenHelper::MeshToRenderObject(const Mesh mesh)
{
    
    Object obj;
    obj.drawMode = GL_TRIANGLES;
    obj.useIndices = true;
    obj.color = glm::vec4(0.2f, 0.7f, 0.3f, 1.0f);
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::map<Mesh::Vertex_index, uint32_t> vertex_index_map;
    uint32_t current_index = 0;
    
    
    glm::vec3 min(1e9), max(-1e9);
    for (const auto &v : mesh.vertices()) {
        Point_3 p = mesh.point(v);
        if(CGAL::to_double(p.x()) < min.x) min.x = CGAL::to_double(p.x());
        if(CGAL::to_double(p.x()) > max.x) max.x = CGAL::to_double(p.x());
        if(CGAL::to_double(p.y()) < min.y) min.y = CGAL::to_double(p.y());
        if(CGAL::to_double(p.y()) > max.y) max.y = CGAL::to_double(p.y());
        if(CGAL::to_double(p.z()) < min.z) min.z = CGAL::to_double(p.z());
        if(CGAL::to_double(p.z()) > max.z) max.z = CGAL::to_double(p.z());
    }

    glm::vec3 center = (min + max) * 0.5f;
    for (const auto &v : mesh.vertices()) {
        Point_3 p = mesh.point(v);
        vertices.push_back(CGAL::to_double(p.x()) - center.x);
        vertices.push_back(CGAL::to_double(p.y()) - center.y);
        vertices.push_back(CGAL::to_double(p.z()) - center.z);
        vertex_index_map[v] = current_index++;
    }

    for (const auto &f : mesh.faces()) {
        std::vector<Mesh::Vertex_index> face_vertices;
        for (const auto &v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
            face_vertices.push_back(v);
        }
        if (face_vertices.size() == 3) {
            indices.push_back(vertex_index_map[face_vertices[0]]);
            indices.push_back(vertex_index_map[face_vertices[1]]);
            indices.push_back(vertex_index_map[face_vertices[2]]);
        }
    }

    obj.vertexCount = static_cast<uint32_t>(indices.size());
    
    printf("Converted mesh to render object with %u vertices and %u indices.\n",
            (uint32_t)(vertices.size() / 3),
            (uint32_t)indices.size());

    
    glGenVertexArrays(1, &obj.VAO);
    glBindVertexArray(obj.VAO);

    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return obj;
}

Mesh ModelgenHelper::MeshFastToMesh(const Mesh_fast mesh) {
    Mesh output_mesh;

    for (const auto &v : mesh.vertices()) {
        Point_3_fast p = mesh.point(v);
        output_mesh.add_vertex(Point_3(CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z())));
    }

    for (const auto &f : mesh.faces()) {
        std::vector<Mesh_fast::Vertex_index> face_vertices;
        for (const auto &v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
            face_vertices.push_back(v);
        }
        if (face_vertices.size() == 3) {
            output_mesh.add_face(
                Mesh::Vertex_index(face_vertices[0]),
                Mesh::Vertex_index(face_vertices[1]),
                Mesh::Vertex_index(face_vertices[2])
            );
        }
    }

    return output_mesh;
}

Mesh_fast ModelgenHelper::MeshToMeshFast(const Mesh mesh) {
    Mesh_fast output_mesh;

    for (const auto &v : mesh.vertices()) {
        Point_3 p = mesh.point(v);
        output_mesh.add_vertex(Point_3_fast(CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z())));
    }

    for (const auto &f : mesh.faces()) {
        std::vector<Mesh::Vertex_index> face_vertices;
        for (const auto &v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
            face_vertices.push_back(v);
        }
        if (face_vertices.size() == 3) {
            output_mesh.add_face(
                Mesh_fast::Vertex_index(face_vertices[0]),
                Mesh_fast::Vertex_index(face_vertices[1]),
                Mesh_fast::Vertex_index(face_vertices[2])
            );
        }
    }

    return output_mesh;
}
