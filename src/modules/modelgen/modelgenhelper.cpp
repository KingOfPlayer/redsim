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
    
    for (const auto &v : mesh.vertices()) {
        Point_3 p = mesh.point(v);
        vertices.push_back(CGAL::to_double(p.x()));
        vertices.push_back(CGAL::to_double(p.y()));
        vertices.push_back(CGAL::to_double(p.z()));
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
    obj.indices = indices;
    obj.vertices = vertices;

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

// TODO Precondition error
Mesh_fast ModelgenHelper::MeshToMeshFast(const Mesh mesh) {
    Mesh_fast output_mesh;

    // Vertex conversion
    std::map<Mesh::Vertex_index, Mesh_fast::Vertex_index> vertex_map;
    for (const auto& v : mesh.vertices()) {
        Point_3 p = mesh.point(v);
        Mesh_fast::Vertex_index new_v = output_mesh.add_vertex(
            Point_3_fast(
                CGAL::to_double(p.x()),
                CGAL::to_double(p.y()),
                CGAL::to_double(p.z())
            )
        );
        vertex_map[v] = new_v;
    }

    // Face conversion
    for (const auto& f : mesh.faces()) {
        std::vector<Mesh_fast::Vertex_index> face_vertices;
        for (const auto& v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
            face_vertices.push_back(vertex_map[v]);
        }
        if (face_vertices.size() == 3) {
            output_mesh.add_face(
                face_vertices[0],
                face_vertices[1],
                face_vertices[2]
            );
        }
    }

    // Mesh repair and cleanup
    CGAL::Polygon_mesh_processing::merge_duplicated_vertices_in_boundary_cycles(output_mesh);
    CGAL::Polygon_mesh_processing::remove_degenerate_faces(output_mesh);
    CGAL::Polygon_mesh_processing::remove_degenerate_edges(output_mesh);
    CGAL::Polygon_mesh_processing::stitch_borders(output_mesh);
    CGAL::Polygon_mesh_processing::remove_almost_degenerate_faces(output_mesh);
    CGAL::Polygon_mesh_processing::remove_isolated_vertices(output_mesh);

    return output_mesh;
}
