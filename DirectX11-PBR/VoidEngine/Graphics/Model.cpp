#include "Model.h"
#include "Dependencies/tinyobjloader/tiny_obj_loader.h"
#include "Dependencies/OpenFBX/ofbx.h"
#include "Utility/Types.h"
#include <unordered_map>
#include <filesystem>

struct Vertex
{
    vec3f pos;
    vec3f normal;
    vec2f coord;

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && normal == other.normal && coord == other.coord;
    }

    struct hash_fn
    {
        std::size_t operator() (const Vertex& node) const
        {
            std::size_t p1 = std::hash<float>()(node.pos.x);
            std::size_t p2 = std::hash<float>()(node.pos.y);
            std::size_t p3 = std::hash<float>()(node.pos.z);
            std::size_t n1 = std::hash<float>()(node.normal.x);
            std::size_t n2 = std::hash<float>()(node.normal.y);
            std::size_t n3 = std::hash<float>()(node.normal.z);
            std::size_t c1 = std::hash<float>()(node.coord.x);
            std::size_t c2 = std::hash<float>()(node.coord.y);

            return (p1 ^ (p2 << 1) ^ (p3 >> 2)) ^ (n1 ^ (n2 << 3) ^ (n3 >> 4)) ^ c1 ^ (c2 << 2);
        }
    };
};

void Model::Create(std::string fileName)
{

    std::filesystem::path path = fileName;
    std::string extension = path.extension().string();

    if (extension == ".obj")  LoadOBJ(fileName);
    if (extension == ".fbx" || extension == ".FBX")  LoadFBX(fileName);
}

void Model::LoadOBJ(std::string fileName)
{
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = ""; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(fileName, reader_config)) {
        LOG_IF_CORE_ERROR(!reader.Error().empty(), "TinyObjReader: %s", reader.Error().c_str());
        return;
    }

    LOG_IF_CORE_WARN(!reader.Warning().empty(), "TinyObjReader: %s", reader.Warning().c_str());

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::vector<Vertex> vertices;
    vertices.reserve(shapes[0].mesh.num_face_vertices.size());
    std::vector<uint32_t> indices;
    indices.reserve(shapes[0].mesh.indices.size());
    std::unordered_map<Vertex, uint32_t, Vertex::hash_fn> uniqueVertices;
    indexBuffer.reserve(shapes.size());
    index_count.emplace_back(shapes.size());

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        indices.clear();

        indexBuffer.emplace_back();
        index_count.emplace_back();
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {

                Vertex vertex;
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                vertex.pos.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                vertex.pos.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                vertex.pos.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    vertex.normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    vertex.normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    vertex.normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    vertex.coord.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    vertex.coord.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }

            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
        indexBuffer[s].Create(indices.data(), indices.size());
        index_count[s] = indices.size();
    }

    vertexBuffer.Create(vertices.data(), vertices.size(), sizeof(Vertex), false);
}

void Model::LoadFBX(std::string fileName)
{
    FILE* fp;
    fopen_s(&fp, fileName.c_str(), "rb");

    if (!fp) return;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto* content = new ofbx::u8[file_size];
    fread(content, 1, file_size, fp);
    ofbx::IScene* scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
    if (!scene) {
        LOG_CORE_INFO("Could not load file: %s error: %s", fileName.c_str(), ofbx::getError());
    }
    
    int obj_idx = 0;
    int indices_offset = 0;
    int normals_offset = 0;
    int mesh_count = scene->getMeshCount();

    std::vector<Vertex> vertices_vec;
    //vertices.reserve(shapes[0].mesh.num_face_vertices.size());
    std::vector<uint32_t> indices_vec;
    //indices.reserve(shapes[0].mesh.indices.size());
    std::unordered_map<Vertex, uint32_t, Vertex::hash_fn> uniqueVertices;
    //indexBuffer.reserve();
    //vertices_vec.emplace_back();

    uint32_t index = 0;
    for (int i = 0; i < mesh_count; ++i)
    {
        indices_vec.clear();
        indexBuffer.emplace_back();
        index_count.emplace_back();

        const ofbx::Mesh& mesh = *scene->getMesh(i);
        const ofbx::Geometry& geom = *mesh.getGeometry();
        int vertex_count = geom.getVertexCount();
        const ofbx::Vec3* vertices = geom.getVertices();
        const ofbx::Vec3* normals = geom.getNormals();
        const ofbx::Vec2* uvs = geom.getUVs();

        Vertex vertex;

        for (int j = 0; j < vertex_count; ++j)
        {
            ofbx::Vec3 v = vertices[j];
            ofbx::Vec3 n = normals[j];
            ofbx::Vec2 uv = uvs[j];
            vertex.pos.x = v.x;
            vertex.pos.y = v.y;
            vertex.pos.z = v.z;
            vertex.normal.x = n.x;
            vertex.normal.y = n.y;
            vertex.normal.z = n.z;
            vertex.coord.x = uv.x;
            vertex.coord.y = uv.y;

            vertices_vec.push_back(vertex);
            indices_vec.emplace_back(index);
            index++;
        }

        //bool has_normals = geom.getNormals() != nullptr;
        //if (has_normals)
        //{
        //    const ofbx::Vec3* normals = geom.getNormals();
        //    int count = geom.getIndexCount();
        //
        //    for (int i = 0; i < count; ++i)
        //    {
        //        ofbx::Vec3 n = normals[i];
        //        vertex.normal.x = n.x;
        //        vertex.normal.y = n.y;
        //        vertex.normal.z = n.z;
        //    }
        //}
        //
        //bool has_uvs = geom.getUVs() != nullptr;
        //if (has_uvs)
        //{
        //    const ofbx::Vec2* uvs = geom.getUVs();
        //    int count = geom.getIndexCount();
        //
        //    for (int i = 0; i < count; ++i)
        //    {
        //        ofbx::Vec2 uv = uvs[i];
        //        vertex.coord.x = uv.x;
        //        vertex.coord.y = uv.y;
        //    }
        //}
       

        //const int* faceIndices = geom.getFaceIndices();
        //int index_count = geom.getIndexCount();
        //bool new_face = true;
        //for (int i = 0; i < index_count; ++i)
        //{
        //    if (new_face)
        //    {
        //        fputs("f ", fp);
        //        new_face = false;
        //    }
        //    int idx = (faceIndices[i] < 0) ? -faceIndices[i] : (faceIndices[i] + 1);
        //    int vertex_idx = indices_offset + idx;
        //    fprintf(fp, "%d", vertex_idx);
        //
        //    if (has_uvs)
        //    {
        //        int uv_idx = normals_offset + i + 1;
        //        fprintf(fp, "/%d", uv_idx);
        //    }
        //    else
        //    {
        //        fprintf(fp, "/");
        //    }
        //
        //    if (has_normals)
        //    {
        //        int normal_idx = normals_offset + i + 1;
        //        fprintf(fp, "/%d", normal_idx);
        //    }
        //    else
        //    {
        //        fprintf(fp, "/");
        //    }
        //
        //    new_face = faceIndices[i] < 0;
        //    fputc(new_face ? '\n' : ' ', fp);
        //}

        //indices_offset += vertex_count;
        //normals_offset += index_count;
        ++obj_idx;
        indexBuffer[i].Create(indices_vec.data(), indices_vec.size());
        index_count[i] = indices_vec.size();
    }
    vertexBuffer.Create(vertices_vec.data(), vertices_vec.size(), sizeof(Vertex), false);
    fclose(fp);
}

void Model::Draw()
{
    vertexBuffer.Bind();
    for (int i = 0; i < indexBuffer.size(); i++)
    {
        indexBuffer[i].Bind();
        GraphicsDevice::Get().context->DrawIndexed(index_count[i], 0, 0);
    }
}