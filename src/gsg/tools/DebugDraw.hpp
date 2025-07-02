#pragma once

#ifdef TOOLS_ENABLED

#include "servers/rendering_server.h"

#include "scene/resources/mesh.h"

class AABBRenderer {
private:
	RID mesh_rid;
	RID instance_rid;
	RID scenario_rid;
	RID material_rid;

	void initialize(const AABB &p_aabb, RID p_scenario) {
		scenario_rid = p_scenario;

		RenderingServer *rs = RenderingServer::get_singleton();
		material_rid = RenderingServer::get_singleton()->material_create();
		RID shader_rid = RenderingServer::get_singleton()->shader_create();
		String shader_code = R"(
            shader_type spatial;
            render_mode unshaded, vertex_lighting;
            void fragment() {
                ALBEDO = COLOR.rgb;
                ALPHA = COLOR.a;
            }
        )";

		RenderingServer::get_singleton()->shader_set_code(shader_rid, shader_code);
		RenderingServer::get_singleton()->material_set_shader(material_rid, shader_rid);
		RenderingServer::get_singleton()->material_set_param(material_rid, "albedo", Color(0.0, 1.0, 0.0, 0.8));
		rs->material_set_render_priority(material_rid, 100);

		PackedVector3Array vertices;
		PackedInt32Array indices;

		Vector3 size = p_aabb.size;
		Vector3 pos = p_aabb.position;

		Vector3 corners[8] = { pos + Vector3(0, 0, 0), pos + Vector3(size.x, 0, 0), pos + Vector3(size.x, 0, size.z), pos + Vector3(0, 0, size.z), pos + Vector3(0, size.y, 0),
			pos + Vector3(size.x, size.y, 0), pos + Vector3(size.x, size.y, size.z), pos + Vector3(0, size.y, size.z) };

		int edges[24] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };

		for (auto corner : corners)
			vertices.push_back(corner);

		for (int edge : edges)
			indices.push_back(edge);

		Array arrays;
		arrays.resize(ArrayMesh::ARRAY_MAX);
		arrays[ArrayMesh::ARRAY_VERTEX] = vertices;
		arrays[ArrayMesh::ARRAY_INDEX] = indices;

		mesh_rid = rs->mesh_create();
		rs->mesh_add_surface_from_arrays(mesh_rid, RenderingServer::PRIMITIVE_LINES, arrays);
		rs->mesh_surface_set_material(mesh_rid, 0, material_rid);

		instance_rid = rs->instance_create();
		rs->instance_set_base(instance_rid, mesh_rid);
		rs->instance_set_scenario(instance_rid, scenario_rid);
		rs->instance_set_visible(instance_rid, true);
	}

	void cleanup() {
		RenderingServer *rs = RenderingServer::get_singleton();

		if (mesh_rid.is_valid()) {
			rs->free(mesh_rid);
			mesh_rid = RID();
		}

		if (instance_rid.is_valid()) {
			rs->free(mesh_rid);
			instance_rid = RID();
		}

		if (material_rid.is_valid()) {
			rs->free(mesh_rid);
			material_rid = RID();
		}
	}

public:
	void update(const AABB &p_aabb) {
		cleanup();
		initialize(p_aabb, scenario_rid);
	}

	AABBRenderer(const AABB &p_aabb, RID p_scenario) { initialize(p_aabb, p_scenario); };

	~AABBRenderer() { cleanup(); }
};

#endif