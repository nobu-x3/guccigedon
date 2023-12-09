#include "render/vulkan/mesh.h"
#include <tiny_obj_loader.h>
#include <vk_mem_alloc.h>

namespace render::vulkan {

	void Mesh::deinit(VmaAllocator alloc) { buffer.destroy(); }

	Mesh& Mesh::set_vertices(ArrayList<Vertex>& data) {
		vertices = std::move(data);
		return *this;
	}

	bool Mesh::load_from_obj(const char* path) {
		// attrib will contain the vertex arrays of the file
		tinyobj::attrib_t attrib;
		// shapes contains the info for each separate object in the file
		std::vector<tinyobj::shape_t> shapes;
		// materials contains the information about the material of each shape,
		// but we wont use it.
		std::vector<tinyobj::material_t> materials;
		// error and warning output from the load function
		std::string warn;
		std::string err;
		// load the OBJ file
		tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path,
						 nullptr);
		// make sure to output the warnings to the console, in case there are
		// issues with the file
		if (!warn.empty()) {
			// core::Logger::Warning("%s", warn);
		}
		// if we have any error, print it to the console, and break the mesh
		// loading. This happens if the file cant be found or is malformed
		if (!err.empty()) {
			// core::Logger::Error("%s", err);
			return false;
		}
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size();
				 f++) {
				// hardcode loading to triangles
				int fv = 3;
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx =
						shapes[s].mesh.indices[index_offset + v];
					// vertex position
					tinyobj::real_t vx =
						attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy =
						attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz =
						attrib.vertices[3 * idx.vertex_index + 2];
					// vertex normal
					tinyobj::real_t nx =
						attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny =
						attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz =
						attrib.normals[3 * idx.normal_index + 2];
					// vertex uv
					tinyobj::real_t ux =
						attrib.texcoords[2 * idx.texcoord_index + 0];
					tinyobj::real_t uy =
						attrib.texcoords[2 * idx.texcoord_index + 1];
					// copy it into our vertex
					Vertex new_vert;
					new_vert.position.x = vx;
					new_vert.position.y = vy;
					new_vert.position.z = vz;
					new_vert.normal.x = nx;
					new_vert.normal.y = ny;
					new_vert.normal.z = nz;
					new_vert.uv.x = ux;
					new_vert.uv.y = 1 - uy;
					// we are setting the vertex color as the vertex normal.
					// This is just for display purposes
					new_vert.color = new_vert.normal;
					vertices.push_back(new_vert);
				}
				index_offset += fv;
			}
		}
		return true;
	}
	bool Mesh::load_primitive(PrimitiveType type) {
		switch (type) {
		case PrimitiveType::Cube:
			for (int i = 0; i < 108; i += 3) {
				Vertex new_vert{};
				new_vert.position.x = cube[i];
				new_vert.position.y = cube[i + 1];
				new_vert.position.z = cube[i + 2];
				vertices.push_back(new_vert);
			}
			break;
		}
		return true;
	}
} // namespace render::vulkan
