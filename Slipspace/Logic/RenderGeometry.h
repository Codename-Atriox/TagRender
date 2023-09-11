#pragma once
#include <DirectXTex.h>
#include "../../Graphics/Graphics.h"
#include "../Tags/TagContainers.h"
#include "../Tags/TagStructs/rtgo.h"
#include "../Tags/TagStructs/mode.h"
#include "../Tags/ModuleLoading/ModuleFramework.h"

static class RenderGeometry {
public:
	static rtgo::s_render_geometry* get_s_render_geometry_ptr(Tag* tag);
	static uint32_t get_mesh_count(Tag* tag);
	static uint32_t get_lod_count(Tag* tag, uint32_t mesh_index);
	static uint32_t get_parts_count(Tag* tag, uint32_t mesh_index, uint32_t lod_index);
	// this will count the verts from all parts, as we do not separate parts when we render yet
	static uint32_t get_verts_count(Tag* tag, uint32_t mesh_index, uint32_t lod_index);

	static void format_buffer(float*& buffer, uint32_t* source, rtgo::RasterizerVertexBuffer* buffer_info, uint32_t& output_size);
	static void build_buffers(Tag* tag, Graphics* gfx, Module* modules);
	

	// destructor
	static void clear_buffers(Tag* tag);

	static void export_mesh();

	static void render(Tag* tag, Graphics* gfx,
		XMMATRIX& worldMatrix, XMMATRIX& viewProjectionMatrix,
		uint32_t mesh_index, uint32_t lod_index);
};