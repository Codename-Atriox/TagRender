#pragma once
#include "RenderGeometry.h"
#include "../Tags/TagStructs/sbsp.h"
#include "../../Graphics/Graphics.h"

class RenderBSP {


public:

	static void Render(Tag* tag, Graphics* gfx, XMMATRIX projectionMatrix, XMFLOAT3 cam_pos) {
		if (tag->tag_FourCC != Tag::sbsp)
			throw exception("tag is not an sbsp but is attempted to render as one");

		sbsp::structure_bsp* bsp_struct = (sbsp::structure_bsp*)tag->tag_data;

		for (int i = 0; i < bsp_struct->instanced_geometry_instances.count; i++) {
			sbsp::structure_instanced_geometry_instance* runtime_geo = bsp_struct->instanced_geometry_instances[i];

			if (runtime_geo->Runtime_geo_mesh_reference.content_ptr == nullptr)
				continue; // im pretty sure we write -1 here?
			// we always store a reference to the tag header at 0x0 of the tagdata
			Tag* geo_tag = *(Tag**)runtime_geo->Runtime_geo_mesh_reference.content_ptr;


			// rotation matrix3x3
			XMFLOAT3X3 rotation = *(XMFLOAT3X3*)&runtime_geo->forward;
			
			// also get mesh index
			uint32_t mesh_index = runtime_geo->Runtime_geo_mesh_index;

			XMMATRIX worldMatrix = XMMatrixTranslation(runtime_geo->position.f1, runtime_geo->position.f2, runtime_geo->position.f3)
									// *XMLoadFloat3x3(&rotation);
								* XMMatrixScaling(runtime_geo->scale.f1, runtime_geo->scale.f2, runtime_geo->scale.f3);



			// then render
			RenderGeometry::render(geo_tag, gfx, worldMatrix, projectionMatrix, cam_pos, mesh_index, 0);
		}
	}

};