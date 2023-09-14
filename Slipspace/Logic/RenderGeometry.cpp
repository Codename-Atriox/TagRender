#include "RenderGeometry.h"

rtgo::s_render_geometry* RenderGeometry::get_s_render_geometry_ptr(Tag* tag) {
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
uint32_t RenderGeometry::get_mesh_count(Tag* tag) {
	rtgo::s_render_geometry* render_geo = RenderGeometry::get_s_render_geometry_ptr(tag);
	if (render_geo == nullptr) return 0; // not possible, but just incase we want to remove the exception at some stage
	return render_geo->meshes.count;
}
uint32_t RenderGeometry::get_lod_count(Tag* tag, uint32_t mesh_index) {
	rtgo::s_render_geometry* render_geo = RenderGeometry::get_s_render_geometry_ptr(tag);
	if (render_geo == nullptr) return 0;
	if (mesh_index >= render_geo->meshes.count || mesh_index < 0)
		return 0; // not a valid mesh
	rtgo::s_mesh* current_mesh = render_geo->meshes[mesh_index];

	return current_mesh->LOD_render_data.count;
}
uint32_t RenderGeometry::get_parts_count(Tag* tag, uint32_t mesh_index, uint32_t lod_index) {
	rtgo::s_render_geometry* render_geo = RenderGeometry::get_s_render_geometry_ptr(tag);
	if (render_geo == nullptr) return 0;
	if (mesh_index >= render_geo->meshes.count || mesh_index < 0)
		return 0; // not a valid mesh
	rtgo::s_mesh* current_mesh = render_geo->meshes[mesh_index];
	if (lod_index >= current_mesh->LOD_render_data.count || lod_index < 0)
		return 0; // do not render
	rtgo::LODRenderData* current_lod = current_mesh->LOD_render_data[lod_index];

	return current_lod->parts.count;
}
// this will count the verts from all parts, as we do not separate parts when we render yet
uint32_t RenderGeometry::get_verts_count(Tag* tag, uint32_t mesh_index, uint32_t lod_index) {
	rtgo::s_render_geometry* render_geo = RenderGeometry::get_s_render_geometry_ptr(tag);
	if (render_geo == nullptr) return 0;
	if (mesh_index >= render_geo->meshes.count || mesh_index < 0)
		return 0; // not a valid mesh
	rtgo::s_mesh* current_mesh = render_geo->meshes[mesh_index];
	if (lod_index >= current_mesh->LOD_render_data.count || lod_index < 0)
		return 0; // do not render
	rtgo::LODRenderData* current_lod = current_mesh->LOD_render_data[lod_index];

	uint32_t verts_count = 0;
	for (int part_index = 0; part_index < current_lod->parts.count; part_index++)
		verts_count += current_lod->parts[part_index]->budget_vertex_count;
	return verts_count;
}

void RenderGeometry::format_buffer(float*& buffer, uint32_t* source, rtgo::RasterizerVertexBuffer* buffer_info, uint32_t& output_size) {
	// determine expected output length
	uint32_t output_length = 0;
	uint32_t output_stride = 0;
	switch (buffer_info->usage) {
	case rtgo::eVertexBufferUsage::UV0:
	case rtgo::eVertexBufferUsage::UV1:
	case rtgo::eVertexBufferUsage::UV2:
		output_length = 2;
		output_stride = 8; 
		break;
	case rtgo::eVertexBufferUsage::Position:
	case rtgo::eVertexBufferUsage::Normal:
	case rtgo::eVertexBufferUsage::Tangent:
		output_length = 3; 
		output_stride = 12;
		break;
	case rtgo::eVertexBufferUsage::Color:
		output_length = 4; 
		output_stride = 16;
		break;
	default: throw exception("unsupported vertex buffer usage format!!");
	}
	buffer_info->d3dbuffer.stride = output_stride;

	// then determine source format & convert

	switch (buffer_info->format) {
	case rtgo::eRasterizerVertexFormat::wordVector4DNormalized: {
		if (buffer_info->stride != 8) throw exception("mismatching buffer stride and buffer format!!");
		if (output_length != 3) throw exception("vertex buffer's expected output size is not compatible with source format!");

		output_size = buffer_info->count * output_stride;
		buffer = new float[buffer_info->count * output_length];
		for (int i = 0; i < buffer_info->count; i++) {
			uint32_t block1 = source[i*2];
			uint32_t block2 = source[(i*2)+1];
			buffer[(i * 3)] = (((float)(block1 & 0xffff) / 0xffff) * 2.0f) - 1.0f;
			// im just going to swap the Y & Z so i can actually see the mesh upright
			buffer[(i * 3) + 1] = (((float)(block1 >> 16) / 0xffff) * 2.0f) - 1.0f;
			buffer[(i * 3) + 2] = (((float)(block2 & 0xffff) / 0xffff) * 2.0f) - 1.0f;
			// unsure what we're supposed to do with the 4th short in the 8byte source thingo
			
		}
		return; }
	case rtgo::eRasterizerVertexFormat::wordVector2DNormalized: {
		if (buffer_info->stride != 4) throw exception("mismatching buffer stride and buffer format!!");
		if (output_length != 2) throw exception("vertex buffer's expected output size is not compatible with source format!");

		output_size = buffer_info->count * output_stride;
		buffer = new float[buffer_info->count * output_length];
		for (int i = 0; i < buffer_info->count; i++) {
			uint32_t block1 = source[i];
			buffer[(i * 2) + 1] = (float)(block1 & 0xffff) / 0xffff;
			buffer[(i * 2)] = (float)(block1 >> 16) / 0xffff;
		}
		return; }
	case rtgo::eRasterizerVertexFormat::_10_10_10_2_signedNormalizedPackedAsUnorm: {
		if (buffer_info->stride != 4) throw exception("mismatching buffer stride and buffer format!!");
		if (output_length != 3) throw exception("vertex buffer's expected output size is not compatible with source format!");

		output_size = buffer_info->count * output_stride;
		buffer = new float[buffer_info->count * output_length];
		for (int i = 0; i < buffer_info->count; i++) {
			uint32_t block = source[i];
			buffer[(i * 3)] = ((float)(block & 0x3ff) / 1023u - 0.5f) * 2;
			buffer[(i * 3) + 1] = ((float)((block >> 10) & 0x3ff) / 1023u - 0.5f) * 2;
			buffer[(i * 3) + 2] = ((float)((block >> 20) & 0x3ff) / 1023u - 0.5f) * 2;
			// no clue what the extra 2 bits are
		}
		return; }
	case rtgo::eRasterizerVertexFormat::byteUnitVector3D: {
		if (buffer_info->stride != 4) throw exception("mismatching buffer stride and buffer format!!");
		if (output_length != 3) throw exception("vertex buffer's expected output size is not compatible with source format!");

		output_size = buffer_info->count * output_stride;
		buffer = new float[buffer_info->count * output_length];
		for (int i = 0; i < buffer_info->count; i++) {
			uint32_t block1 = source[i];
			buffer[(i * 3) + 2] = (float)(block1 & 0xff) / 255u;
			buffer[(i * 3) + 1] = (float)((block1 >> 8) & 0xff) / 255u;
			buffer[(i * 3)] = (float)((block1 >> 16) & 0xff) / 255u;
			// i think the 4th byte here is padding
		}
		return; }
	case rtgo::eRasterizerVertexFormat::byteARGBColor: {
		if (buffer_info->stride != 4) throw exception("mismatching buffer stride and buffer format!!");
		if (output_length != 4) throw exception("vertex buffer's expected output size is not compatible with source format!");

		output_size = buffer_info->count * output_stride;
		buffer = new float[buffer_info->count * output_length];
		for (int i = 0; i < buffer_info->count; i++) {
			uint32_t block1 = source[i];
			buffer[(i * 3) + 3] = (float)(block1 & 0xff) / 255u;
			buffer[(i * 3) + 2] = (float)((block1 >> 8) & 0xff) / 255u;
			buffer[(i * 3) + 1] = (float)((block1 >> 16) & 0xff) / 255u;
			buffer[(i * 3)] = (float)((block1 >> 24) & 0xff) / 255u;
		}
		return; }
	}
	// failsafe for if we didn't get any match with currently compatible types
	throw exception("unsupported vertex buffer format!!");
}
void RenderGeometry::build_buffers(Tag* tag, Graphics* gfx, Module* modules) {
	rtgo::s_render_geometry* render_geo = RenderGeometry::get_s_render_geometry_ptr(tag);

	//rtgo::RuntimeGeoTag* runtime_geo = (rtgo::RuntimeGeoTag*)tag->tag_data;
	// no support for multiple 'mesh reosurce groups' yet, throw exception
	if (render_geo->mesh_package.mesh_resource_groups.count != 1)
		throw exception("bad number of resource groups in runtime geo");

	if (render_geo->mesh_package.mesh_resource_groups.count == 0)
		throw exception("tag has no mesh resource groups!!!");
	if (render_geo->mesh_package.mesh_resource_groups.count != 1)
		throw exception("currently render geos with only 1 resource group are supported!!!");

	rtgo::RenderGeometryMeshPackageResourceGroup* current_runtime_geo_group = render_geo->mesh_package.mesh_resource_groups[0];
	// double check to make sure the file exists as expected, we dont know how non-chunked models work yet
	if (current_runtime_geo_group->mesh_resource.is_chunked_resource == 0)
		throw exception("non-chunked geo resources are not yet supported!!!");

	rtgo::s_render_geometry_api_resource* current_geo_resource = current_runtime_geo_group->mesh_resource.content_ptr;

	// first we have to iterate through all resources & write their contents to the buffers
	// all buffer datas will go into the one char buffer, which we then give the reference to the thing

	// find total buffer size
	uint64_t buffer_size = 0;

	// cache offsets of all buffers
	if (current_geo_resource->Streaming_Buffers.count == 0) throw exception("no streaming buffers to pull from!!!");
	uint64_t* offsets = new uint64_t[current_geo_resource->Streaming_Buffers.count];

	for (int i = 0; i < current_geo_resource->Streaming_Buffers.count; i++) {
		offsets[i] = buffer_size;
		buffer_size += current_geo_resource->Streaming_Buffers[i]->buffer_size;
	}
	char* streaming_buffer = new char[buffer_size];

	for (uint32_t i = 0; i < current_geo_resource->Streaming_Chunks.count; i++) {
		// loop through all buffers
		rtgo::StreamingGeometryChunk* curr_buffer_chunk = current_geo_resource->Streaming_Chunks[i];
		uint64_t buffer_offset = offsets[curr_buffer_chunk->buffer_index];
		if (curr_buffer_chunk->buffer_start > curr_buffer_chunk->buffer_end)
			throw exception("bad buffer indicies");
		if (curr_buffer_chunk->buffer_end > buffer_size)
			throw exception("buffer end index overflows buffer size!!");
		if (curr_buffer_chunk->buffer_start == curr_buffer_chunk->buffer_end)
			break; // using a break instead because if any other buffer beyonds this one does have information, then this would be error prone
		//continue; // this buffer has no content, thus likely no resource file

		// then just dump the data in
		if (FAILED(modules->ReturnResource(tag->source_tag_index, i, streaming_buffer + (buffer_offset + curr_buffer_chunk->buffer_start), curr_buffer_chunk->buffer_end - curr_buffer_chunk->buffer_start)))
			throw exception("resource chunk failed to load");
	}
	delete[] offsets;
	// leave a pointer to the buffer array in the runtime data slot, so it can be cleaned up later
	current_geo_resource->Runtime_Data = (uint64_t)streaming_buffer;

	// now we load to fill in the buffers for each of the d3d11 buffer things
	// which ironically all require us to create new d3dbuffers, because appanrently we couldn't just build them into the structs

	// iterate through vertex buffers and generate D3D11 buffers
	for (uint32_t b_index = 0; b_index < current_geo_resource->pc_vertex_buffers.count; b_index++) {
		rtgo::RasterizerVertexBuffer* vert_buffer = current_geo_resource->pc_vertex_buffers[b_index];
		char* source_ptr;
		// determine whether to get pointer from chunked resource or not
		if (vert_buffer->d3dbuffer.d3d_buffer.data_size == 0)
			source_ptr = streaming_buffer + vert_buffer->offset;
		else 
			source_ptr = vert_buffer->d3dbuffer.d3d_buffer.content_ptr;

		//throw exception("render geo with non-chunked data is not currently supported!! because i have no idea what could be contained in this buffer!!");
		float* formatted_buffer = nullptr;
		uint32_t buffer_size = vert_buffer->d3dbuffer.byte_width; // this s
		// test whether usage & format are compatible
		format_buffer(formatted_buffer, (uint32_t*)source_ptr, vert_buffer, buffer_size);
		if (formatted_buffer == nullptr)
			throw exception("failed to format vertex buffer!! (could be unsupported vertex buffer type)");

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc)); // do we even need to do this?

		vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT; // apparently 343's mapped value is wrong (D3D11_USAGE)vert_buffer->d3dbuffer.usage; // 0
		vertexBufferDesc.ByteWidth = buffer_size; // 144
		vertexBufferDesc.BindFlags = vert_buffer->d3dbuffer.bind_flags; // 1 'vertex buffer'
		vertexBufferDesc.CPUAccessFlags = vert_buffer->d3dbuffer.cpu_flags; // 0
		vertexBufferDesc.MiscFlags = 0; // vert_buffer->d3dbuffer.misc_flags; // 32 // this causes issues for the first 2 buffers

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = formatted_buffer;

		ID3D11Buffer* result;
		HRESULT hr = gfx->device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &result);
		vert_buffer->m_resource = (int64_t)formatted_buffer;
		vert_buffer->m_resourceView = (int64_t)result;
		if (FAILED(hr))
			throw exception("failed to generate d3d11 buffer!!");
	}
	// iterate through index buffers and generate D3D11 buffers
	for (uint32_t b_index = 0; b_index < current_geo_resource->pc_index_buffers.count; b_index++) {
		rtgo::RasterizerIndexBuffer* index_buffer = current_geo_resource->pc_index_buffers[b_index];

		char* source_ptr;
		// determine whether to get pointer from chunked resource or not
		if (index_buffer->d3dbuffer.d3d_buffer.data_size == 0)
			source_ptr = streaming_buffer + index_buffer->offset;
		else source_ptr = index_buffer->d3dbuffer.d3d_buffer.content_ptr;

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc)); // do we even need to do this?

		indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT; // not sure why 343's is some weird thing, i think its literally just the enum but as a number   (D3D11_USAGE)index_buffer->d3dbuffer.usage;
		indexBufferDesc.ByteWidth = index_buffer->d3dbuffer.byte_width;
		indexBufferDesc.BindFlags = index_buffer->d3dbuffer.bind_flags;
		indexBufferDesc.CPUAccessFlags = index_buffer->d3dbuffer.cpu_flags;
		indexBufferDesc.MiscFlags = 0; // index_buffer->d3dbuffer.misc_flags;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		if (index_buffer->d3dbuffer.d3d_buffer.data_size != 0)
			throw exception("render geo with non-chunked data is not currently supported!! because i have no idea what could be contained in this buffer!!");
		vertexBufferData.pSysMem = source_ptr;

		ID3D11Buffer* result = nullptr;
		HRESULT hr = gfx->device->CreateBuffer(&indexBufferDesc, &vertexBufferData, &result);
		index_buffer->m_resourceView = (int64_t)result;
		if (FAILED(hr))
			throw exception("failed to generate d3d11 buffer!!");
	}
	// thats all, we dont have anything to return, because we just wrote everything to the tag
	// we can now render this data via the interfaces we just created & referencing them using the meshes tagdata struct
}

// destructor
static void clear_buffers(Tag* tag) {

}

static void export_mesh() {

}

void RenderGeometry::render(Tag* tag, Graphics* gfx,
	XMMATRIX& worldMatrix, XMMATRIX& viewProjectionMatrix, XMFLOAT3 cam_position,
	uint32_t mesh_index, uint32_t lod_index) {

	rtgo::s_render_geometry* render_geo = RenderGeometry::get_s_render_geometry_ptr(tag);

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
	gfx->cb_vs_vertexshader.data.camera_position = cam_position;
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

	if (lod_index >= current_mesh->LOD_render_data.count || lod_index < 0) return; // do not render
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
		vert_buffers[vert_buffer_index] = (ID3D11Buffer*)vert_buffer->m_resourceView;
		vert_strides[vert_buffer_index] = vert_buffer->d3dbuffer.stride; // we now store the reformatted stride here
		vert_offsets[vert_buffer_index] = 0; // im pretty sure we do not offset these, we only offset the indicies
	}
	// then apply the vertex buffers, as they do not change between mesh part
	// map the stride to a format
	DXGI_FORMAT index_format = (DXGI_FORMAT)0;
	switch (index_buffer->stride) {
	case 2:  index_format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT; break;
	case 4:  index_format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT; break;
	default: throw exception("invalid index buffer stride (has to be either 2 or 4 bytes!!!)");
	}


	// set index mode
	D3D_PRIMITIVE_TOPOLOGY index_mode = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	switch (current_mesh->index_buffer_type) {
	case rtgo::IndexBufferPrimitiveType::line_list:
		index_mode = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case rtgo::IndexBufferPrimitiveType::line_strip:
		index_mode = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	case rtgo::IndexBufferPrimitiveType::triangle_list:
		index_mode = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case rtgo::IndexBufferPrimitiveType::triangle_strip:
		index_mode = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		break;
	default:
		throw exception("Unsupported index buffer type!!! should be either tri/line list/strip");
	}
	gfx->deviceContext->IASetPrimitiveTopology(index_mode);

	// now loop through all parts & draw
	gfx->deviceContext->IASetVertexBuffers(0, 19, vert_buffers, vert_strides, vert_offsets);
	gfx->deviceContext->IASetIndexBuffer((ID3D11Buffer*)index_buffer->m_resourceView, index_format, 0);
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
