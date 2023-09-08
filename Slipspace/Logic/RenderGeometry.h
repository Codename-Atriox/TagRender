#pragma once

static class RenderGeometry {
public:
	static rtgo::s_render_geometry* get_s_render_geometry_ptr(Tag* tag) {
		switch (tag->tag_FourCC) {
		case Tag::mode: {
			mode::render_model_definition* model_geo = (mode::render_model_definition*)tag->tag_data;
			return (rtgo::s_render_geometry*)&model_geo->render_geometry; }
		case Tag::rtgo: {
			rtgo::RuntimeGeoTag* runtime_geo = (rtgo::RuntimeGeoTag*)tag->tag_data;
			return &runtime_geo->render_geometry; }
		default:
			throw exception("Not a model supporting tag!!");
		}
	}
	// constructor basically
	static void build_buffers(Tag* tag) {
		
	}
	// destructor
	static void clear_buffers(Tag* tag) {

	}

	static void export_mesh() {

	}

	static void render(Tag* tag, Graphics* gfx, 
				XMMATRIX& worldMatrix, XMMATRIX& viewProjectionMatrix,
				uint32_t mesh_index, uint32_t lod_index) {

		rtgo::s_render_geometry* render_geo = get_s_render_geometry_ptr(tag);

		// no support for multiple 'mesh reosurce groups' yet, throw exception
		if (render_geo->mesh_package.mesh_resource_groups.count != 1)
			throw exception("bad number of resource groups in runtime geo");
		// buffer structs
		rtgo::RenderGeometryMeshPackageResourceGroup* runtime_geo_group = render_geo->mesh_package.mesh_resource_groups[0];
		// double check to make sure the file exists as expected, we dont know how non-chunked models work yet
		if (runtime_geo_group->mesh_resource.is_chunked_resource == 0)
			throw exception("non-chunked geo resources are not yet supported!!!");
		rtgo::s_render_geometry_api_resource* geo_resource = runtime_geo_group->mesh_resource.content_ptr;

		//XMMATRIX worldMatrix = XMMatrixRotationRollPitchYaw(0, 0, 0) * XMMatrixTranslation(0, 0, 0); // we need to calculate this elsewhere i think
		// set constant buffer params
		gfx->deviceContext->VSSetConstantBuffers(0, 1, gfx->cb_vs_vertexshader.GetAddressOf());
		gfx->cb_vs_vertexshader.data.wvpMatrix = DirectX::XMMatrixIdentity() * worldMatrix * viewProjectionMatrix;
		gfx->cb_vs_vertexshader.data.worldMatrix = DirectX::XMMatrixIdentity() * worldMatrix;
		// this data turns out to not be useful i think?
		//if (render_geo->compression_info.count > 0) {
		//	rtgo::s_compression_info* compression = render_geo->compression_info[0];
		//	cb_vs_vertexshader->data.minbounds.x = compression->position_bounds_0.f1;
		//	cb_vs_vertexshader->data.maxbounds.x = compression->position_bounds_0.f2;
		//	cb_vs_vertexshader->data.minbounds.y = compression->position_bounds_0.f3;
		//	cb_vs_vertexshader->data.maxbounds.y = compression->position_bounds_1.f1;
		//	cb_vs_vertexshader->data.minbounds.z = compression->position_bounds_1.f2;
		//	cb_vs_vertexshader->data.maxbounds.z = compression->position_bounds_1.f3;
		//}
		//else { // uh what the hell do we do here? we still need to pass something into the buffer struct
		//	throw exception("non-compressed position bounds are currently unsupported");
		//	cb_vs_vertexshader->data.minbounds.x = 0;
		//	cb_vs_vertexshader->data.minbounds.y = 0;
		//	cb_vs_vertexshader->data.minbounds.z = 0;
		//	cb_vs_vertexshader->data.maxbounds.x = 0;
		//	cb_vs_vertexshader->data.maxbounds.y = 0;
		//	cb_vs_vertexshader->data.maxbounds.z = 0;
		//}
		gfx->cb_vs_vertexshader.ApplyChanges();

		// error checking
		if (mesh_index >= render_geo->meshes.count || mesh_index < 0) return; // do not render
		rtgo::s_mesh* current_mesh = render_geo->meshes[mesh_index];

		if (lod_index >= render_geo->meshes.count || lod_index < 0) return; // do not render
		rtgo::LODRenderData* current_lod = current_mesh->LOD_render_data[lod_index];


		uint16_t ibuffer_index = current_lod->index_buffer_index;
		if (geo_resource->pc_index_buffers.count < 1)
			throw exception("no valid index buffers! we need to generate our own???");

		rtgo::RasterizerIndexBuffer* index_buffer = geo_resource->pc_index_buffers[current_lod->index_buffer_index];
		//rtgo::RasterizerVertexBuffer* vert_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[0].vertex_buffer_index];
		//rtgo::RasterizerVertexBuffer* uv0_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[1].vertex_buffer_index];
		//rtgo::RasterizerVertexBuffer* uv1_buffer     = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[2].vertex_buffer_index];
		//rtgo::RasterizerVertexBuffer* uv2_buffer     = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[3].vertex_buffer_index];
		//rtgo::RasterizerVertexBuffer* color_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[4].vertex_buffer_index];
		//rtgo::RasterizerVertexBuffer* norm_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[5].vertex_buffer_index];
		//rtgo::RasterizerVertexBuffer* tangent_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[6].vertex_buffer_index];

		/* all 'vertex buffer indicies' meanings via slot/index
		*  0 : Position (wordVector4DNormalized)
		*  1 : UV0 (wordVector2DNormalized)
		*  2 : UV1 (wordVector2DNormalized)
		*  3 : UV2 (wordVector2DNormalized)
		*  4 : Color (byteARGBColor)
		*  5 : Normal (10_10_10_10_2_signedNormalizedPackedAsUnorm)
		*  6 : Tangent (byteUnitVector3D)
		*  7 :
		*  8 :
		*  9 :
		* 10 :
		* 11 :
		* 12 :
		* 13 :
		* 14 :
		* 15 :
		* 16 :
		* 17 :
		* 18 :
		* 19 :
		*/
		ID3D11Buffer* vert_buffers[19] = {};
		UINT vert_strides[19] = {};
		UINT vert_offsets[19] = {};
		for (int vert_buffer_index = 0; vert_buffer_index < 19; vert_buffer_index++) { // statically assigned length because how do we even measure that
			if (current_lod->vertex_buffer_indices[vert_buffer_index].vertex_buffer_index == 65535)
				continue;
			rtgo::RasterizerVertexBuffer* vert_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[vert_buffer_index].vertex_buffer_index];
			vert_buffers[vert_buffer_index] = (ID3D11Buffer*)vert_buffer->m_resource;
			vert_strides[vert_buffer_index] = vert_buffer->stride;
			vert_offsets[vert_buffer_index] = 0; // im pretty sure we do not offset these, we only offset the indicies
		}
		// then apply the vertex buffers, as they do not change between mesh part
		// map the stride to a format
		DXGI_FORMAT index_format = (DXGI_FORMAT)0;
		switch (index_buffer->stride) {
		case 2:  index_format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT; break;
		case 4:  index_format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT; throw exception("unexpected length"); break;
		default: throw exception("invalid index buffer stride (has to be either 2 or 4 bytes!!!)");
		}

		// now loop through all parts & draw
		gfx->deviceContext->IASetVertexBuffers(0, 19, vert_buffers, vert_strides, vert_offsets);
		gfx->deviceContext->IASetIndexBuffer((ID3D11Buffer*)index_buffer->m_resource, index_format, 0);
		for (int part_index = 0; part_index < current_lod->parts.count; part_index++) {
			rtgo::s_part* mesh_part = current_lod->parts[part_index];
			gfx->deviceContext->DrawIndexed(mesh_part->index_count, mesh_part->index_start, 0);

			// export mesh // DEBUG //
			//string filename = tag->tagname + "_m" + std::to_string(m_index) + "_l" + std::to_string(lod_index) + "_p" + std::to_string(part_index);
			//std::ofstream out("C:\\Users\\Joe bingle\\Downloads\\test\\" + filename + ".obj");
			//out << "o Test" << "\n";
			//rtgo::s_compression_info* compression = runtime_geo->render_geometry.compression_info[0];
			//for (int vi = 0; vi < vert_buffer->count; vi++) {
			//	uint64_t float4d = *((uint64_t*)((char*)geo_resource->Runtime_Data + vert_buffer->offset) + vi);

			//	float w = (float)((float4d >> 48) & 0xffff) / 65535.0;

			//	float x = (float)(float4d & 0xffff) / 65535.0;
			//	x *= compression->position_bounds_0.f2 - compression->position_bounds_0.f1;
			//	x += compression->position_bounds_0.f1;
			//	float y = (float)((float4d >> 16) & 0xffff) / 65535.0;
			//	y *= compression->position_bounds_1.f1 - compression->position_bounds_0.f3;
			//	y += compression->position_bounds_0.f3;
			//	float z = (float)((float4d >> 32) & 0xffff) / 65535.0;
			//	z *= compression->position_bounds_1.f3 - compression->position_bounds_1.f2;
			//	z += compression->position_bounds_1.f2;
			//	out << "v " << x << " " << y << " " << z << " # " << w << "\n";
			//}
			//// repeat the process for the vertex normals
			//for (int vi = 0; vi < norm_buffer->count; vi++) {
			//	uint32_t* norm4d_address = (uint32_t*)((char*)geo_resource->Runtime_Data + norm_buffer->offset) + vi;
			//	uint32_t norm4d = *norm4d_address;
			//	// these values are signed
			//	float nx = ((float)( norm4d        & 0x3ff) / 511.0) - 1.0;
			//	float ny = ((float)((norm4d >> 10) & 0x3ff) / 511.0) - 1.0;
			//	float nz = ((float)((norm4d >> 20) & 0x3ff) / 511.0) - 1.0;
			//	float nw = norm4d >> 30;

			//	out << "vn " << nx << " " << ny << " " << nz << " # " << nw << "\n";

			//}


			//for (int ii = 0; ii < mesh_part->index_count; ii += 3) {
			//	uint16_t* index_ptr = (uint16_t*)((char*)geo_resource->Runtime_Data + index_buffer->offset) + mesh_part->index_start + ii;
			//	out << "f " << ((*index_ptr)+1) << " " << (*(index_ptr + 1)+1) << " " << (*(index_ptr + 2)+1) << "\n";
			//}

		}

		// debugging to peep at the structures

		//std::ofstream out("C:\\Users\\Joe bingle\\Downloads\\test\\oddmodel.txt");
		//rtgo::RasterizerVertexBuffer* vb = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[0].vertex_buffer_index];
		//for (int i = 0; i < vb->count; i++) {
		//	uint64_t float4d = *((uint64_t*)((char*)geo_resource->Runtime_Data + vb->offset) + i);
		//	uint16_t f1 = (float4d >> 48) & 0xffff;
		//	uint16_t f2 = (float4d >> 32) & 0xffff;
		//	uint16_t f3 = (float4d >> 16) & 0xffff;
		//	uint16_t f4 = (float4d >> 00) & 0xffff;
		//	out << f1 << ',' << f2 << ',' << f3 << ',' << f4 << '\r';
		//}

		//std::ofstream outfile("C:\\Users\\Joe bingle\\Downloads\\test\\oddmodel.bin", std::ios::out | std::ios::binary);
		//outfile.write((((char*)geo_resource->Runtime_Data) + vb->offset), vb->count * vb->stride); //no need to cast
		//OpenRuntimeGeos.RemoveAt(i);
		//i--;
		//return;
	}
};