#pragma once
#include "../Tags/TagContainers.h"

#include "../../Utilities/NFD/include/nfd.h"

class UI {
public:
	void render_GEO(ModuleManager* Modules, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader, const XMMATRIX& viewProjectionMatrix) {
		render_runtimegeo_window(device, deviceContext, cb_vs_vertexshader, viewProjectionMatrix);
	}
	void render_UI(ModuleManager* Modules, ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
		render_module_window(Modules, device);
		render_loaded_tags_window(Modules);
		render_bitmap_window(Modules, device);
		runtimegeo_window();

	}
	const char* module_load_filters = "module";
	const char* module_load_defaultpath = "D:\\Programs\\Steam\\steamapps\\common\\Halo Infinite\\deploy";


	// ////////////// //
	// MODULE WINDOW //
	// //////////// //
	const uint32_t max_files_onscreen_count = 5;
	char* tag_UI_groups_buffer = new char[5 * max_files_onscreen_count]; // 5 bytes allocated per tag group (4 chars, 1 null terminator)
	// TODO: create the array during constructor, we need to clear the mem to 00's !!
	// else we have to set the 5th byte of each group to 00 each time
	void write_tag_group(uint32_t index, uint32_t tag_group) { // NOTE: this is going to cause issues
		tag_UI_groups_buffer[index * 5] = tag_group >> 24 & 0xff;
		tag_UI_groups_buffer[index * 5 + 1] = tag_group >> 16 & 0xff;
		tag_UI_groups_buffer[index * 5 + 2] = tag_group >> 8 & 0xff;
		tag_UI_groups_buffer[index * 5 + 3] = tag_group & 0xff;
		tag_UI_groups_buffer[index * 5 + 4] = 0; // null terminator
	}
	void render_module_window(ModuleManager* Modules, ID3D11Device* device) {
		ImGui::Begin("Modules");
		if (ImGui::Button("Open Module")) {
			char* outpath;
			if (NFD_OpenDialog(module_load_filters, module_load_defaultpath, &outpath) == NFD_OKAY) {
				Modules->OpenModule(string(outpath));
				if (outpath) delete[] outpath;
				//throw std::exception("button pressed");
			}
		}
		// loaded modules display
		ImGui::Text("Indexed tags [%d]", Modules->total_tags);
		ImGui::Text("Active Modules [%d]", Modules->open_modules);

		for (int i = 0; i < Modules->open_modules; i++) {
			Module* menu_active_module = Modules->GetModule_AtIndex(i);

			ImGui::PushID(i); // need this id and the next level one, because none of these items are actually in a container
			if (ImGui::CollapsingHeader(menu_active_module->filename.c_str())) {
				ImGui::DragInt("Selected File", &menu_active_module->selected_tag, 0.1f, 0, menu_active_module->file_count);
				// then populate with individual tag views
				for (uint32_t tag_ind = 0; tag_ind < max_files_onscreen_count; tag_ind++) {
					// make sure we are keeping the same id for a tagUI even if we "scroll" the list (how tf are we going to scroll) 
					uint32_t tag_moduleindex = tag_ind + menu_active_module->selected_tag;

					ImGui::PushID(tag_moduleindex);
					if (tag_moduleindex < menu_active_module->file_count) {
						module_file* menu_active_tag = menu_active_module->GetTagHeader_AtIndex(tag_moduleindex);
						write_tag_group(tag_ind, menu_active_tag->ClassId);
						// convert bytes to string format
						ImGui::Text("[%d] %s", tag_moduleindex, tag_UI_groups_buffer + tag_ind * 5);
						ImGui::SameLine();
						ImGui::Text("ID: %08X", menu_active_tag->GlobalTagId);
						ImGui::SameLine();
						// figure out size of byte
						if (menu_active_tag->TotalUncompressedSize >= 1000000)
							ImGui::Text("%d%s", menu_active_tag->TotalUncompressedSize / 1000000, "mb");
						else if (menu_active_tag->TotalUncompressedSize >= 1000)
							ImGui::Text("%d%s", menu_active_tag->TotalUncompressedSize / 1000, "kb");
						else ImGui::Text("%d%s", menu_active_tag->TotalUncompressedSize, "b");

						ImGui::SameLine();
						if (ImGui::Button("Open")) {
							Modules->OpenTag(menu_active_tag->GlobalTagId, device);
						}

					}
					ImGui::PopID();
				}

			}
			ImGui::PopID();
		}
		ImGui::End();
	}





	// /////////////////// //
	// LOADED TAGS WINDOW //
	// ///////////////// //
	const uint32_t max_tags_onscreen_count = 5;
	int32_t loaded_tag_index = 0;
	void render_loaded_tags_window(ModuleManager* Modules) {
		ImGui::Begin("Tags");
		ImGui::Text("Loaded Tags [%d]", Modules->loaded_tags.size());


		ImGui::DragInt("Selected tag", &loaded_tag_index, 0.1f, 0, Modules->loaded_tags.size());
		for (uint32_t slot_id = 0; slot_id < max_tags_onscreen_count; slot_id++) {
			uint32_t tagindex = slot_id + loaded_tag_index;
			ImGui::PushID(tagindex);
			if (tagindex < Modules->loaded_tags.size()) {
				Tag* active_tag = Modules->loaded_tags[tagindex];


				//ImGui::Text("ID: %08X", active_tag->tagID);
				ImGui::Text("%s", active_tag->tagname.c_str());
				ImGui::SameLine();
				switch (active_tag->tag_FourCC) {
				case Tag::bitm: {
					ImGui::Text("Bitmap");
					// check whether bitmap is opened
					bool is_opened = false;
					for (int i = 0; i < OpenBitmaps.Size(); i++) {
						if (OpenBitmaps[i] == active_tag) {
							is_opened = true;
							break;
						}
					}
					ImGui::SameLine();
					if (!is_opened) {
						if (ImGui::Button("View"))
							OpenBitmaps.Append(active_tag);
					}
					else if (ImGui::Button("Close"))
						OpenBitmaps.Remove(active_tag);
				} break;
				case Tag::rtgo: {
					ImGui::Text("Runtime Geo");
					bool is_opened = false;
					for (int i = 0; i < OpenRuntimeGeos.Size(); i++) {
						if (OpenRuntimeGeos[i] == active_tag) {
							is_opened = true;
							break;
						}
					}
					ImGui::SameLine();
					if (!is_opened) {
						if (ImGui::Button("View"))
							OpenRuntimeGeos.Append(active_tag);
					}
					else if (ImGui::Button("Close"))
						OpenRuntimeGeos.Remove(active_tag);
				} break;
				case Tag::mode:
					ImGui::Text("Model");
					break;
				default:
					ImGui::Text("Tag");
					break;
				}

				//ImGui::SameLine();

			}
			ImGui::PopID();
		}


		ImGui::End();
	}

	// ////////////// //
	// BITMAP WINDOW //
	// //////////// //
	enum export_type { DDS = 0, JPG = 1, PNG = 2 };
	const char* formats[3] = {{"DDS"}, {"JPG"}, {"PNG"}};
	const char* formats_ext[3] = { {".dds"}, {".jpg"}, {".png"} };
	bool is_exportable_format(DXGI_FORMAT format) {
		switch (format) {
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return true;
		}
		return false;
	}
	void export_bitmap_resource(DirectX::ScratchImage* image, string path, export_type type) {
		try{HRESULT hr;
			// fix up the path extension if we didn't do it right
			if (!path.ends_with(formats_ext[type]))
				path += formats_ext[type];
			
			wstring wide_export_path(path.begin(), path.end());
			if (type > DDS) { // write to regular file format
				DirectX::ScratchImage decompressedImage;
				if (DirectX::IsCompressed(image->GetMetadata().format)) {
					hr = DirectX::Decompress(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DXGI_FORMAT_R8G8B8A8_UNORM, decompressedImage);
					if (FAILED(hr))
						return;
				} else if (!is_exportable_format(image->GetMetadata().format)) { // BC bitmaps should always convert to that format anyway, so they shouldn't need to convert
					hr = DirectX::Convert(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, decompressedImage);
					if (FAILED(hr))
						return;
				}
				// for the codec version, it aligns with our enum when we plus 1
				hr = DirectX::SaveToWICFile(*decompressedImage.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WICCodecs(type + 1)), wide_export_path.c_str());
				if (FAILED(hr))
					return;
			} else { // write to DDS format

				hr = DirectX::SaveToDDSFile(*image->GetImage(0,0,0), (DirectX::DDS_FLAGS)0, wide_export_path.c_str());
				if (FAILED(hr))
					return;
			}


		}catch (exception ex) {
			// do nothing, no memory allocated so none required to be removed.
		}
	}
	void export_prompt(DirectX::ScratchImage* image, export_type type) {
		char* outpath;
		if (NFD_SaveDialog(formats[type], NULL, &outpath) == NFD_OKAY && outpath) { // idk why but it hides other files if we have an extension defined, seems inconvenient
			export_bitmap_resource(image, string(outpath), type);
			delete[] outpath;
		}
	}
	CTList<Tag> OpenBitmaps;
	void render_bitmap_window(ModuleManager* Modules, ID3D11Device* device) {
		// create each window
		for (uint32_t i = 0; i < OpenBitmaps.Size(); i++) {
			Tag* open_bitmap = OpenBitmaps[i];
			ImGui::PushID(open_bitmap->tagID);
			bool window_is_open = true;
			if (!ImGui::Begin(open_bitmap->tagname.c_str(), &window_is_open) && window_is_open) {
				ImGui::End();
				ImGui::PopID();
				continue;
			}
			if (!window_is_open) {
				OpenBitmaps.RemoveAt(i);
				i--;
				ImGui::End();
				ImGui::PopID();
				continue;
			}

			if (open_bitmap->tag_FourCC != 1651078253 || open_bitmap->tag_data == 0) {
				ImGui::Text("Error! Not a bitmap!!");
			} else { // read data from the bitmap
				BitmapGroup* bitmap_header = (BitmapGroup*)open_bitmap->tag_data;

				ImGui::Text("Bitmaps [%d]", bitmap_header->bitmaps.count);
				// maybe add logic here if needed

				uint32_t selected_bitmap = 0;
				bool multi_image = (bitmap_header->bitmaps.count > 1);
				if (multi_image){
					// if we have multiple images, then that means we probably dont have mips, so we preview slider the image index
					ImGui::DragInt("Active image", &open_bitmap->preview_1, 0.1f, 0, bitmap_header->bitmaps.count - 1);
					selected_bitmap = open_bitmap->preview_1;
				}
				if (selected_bitmap < bitmap_header->bitmaps.count && selected_bitmap >= 0) {
					BitmapData* curr_bitmap = bitmap_header->bitmaps[selected_bitmap];

					ImGui::Text("Size [%dx%d]", curr_bitmap->sourceWidth, curr_bitmap->sourceHeight);
					ImGui::Text("Type: ");
					ImGui::SameLine();
					switch (curr_bitmap->type) {
					case BitmapType::_2D_texture:
						ImGui::Text("2D");
						break;
					case BitmapType::_3D_texture:
						ImGui::Text("3D");
						break;
					case BitmapType::cube_map:
						ImGui::Text("Cube map");
						break;
					case BitmapType::array:
						ImGui::Text("Array");
						break;
					default:
						ImGui::Text("Undefined");
						break;
					}

					BitmapDataResource* bitm_resource = curr_bitmap->bitmap_resource_handle.content_ptr;
					ImGui::Text("Format [%d]", bitm_resource->format);

					uint32_t lods = 0;
					if (bitm_resource->pixels.content_ptr != 0)
						lods = 1;
					lods += bitm_resource->streamingData.count;
					ImGui::Text("LODs [%d]", lods);
					if (!multi_image) ImGui::DragInt("Active MIP", &open_bitmap->preview_1, 0.1f, 0, bitmap_header->bitmaps.count - 1);
					// then present the actual image
					if (curr_bitmap->type == BitmapType::_2D_texture) {
						BitmapResource* last_loaded_image = Modules->BITM_GetTexture(open_bitmap, device, open_bitmap->preview_1, false);
						if (last_loaded_image != nullptr) {
							ImGui::Text("Resolution: %dx%d", last_loaded_image->Width, last_loaded_image->Height);
							ImGui::SameLine();
							if (last_loaded_image->hd1)
								 ImGui::Text("[HD]");
							else ImGui::Text("[SD]");

							if (last_loaded_image->image_view != nullptr) {
								ImGui::Image((void*)last_loaded_image->image_view, ImVec2(256, 256));
								
							} else ImGui::Text("No image!");
							if (last_loaded_image->scratch_image.GetImageCount() > 0) {
								if (ImGui::Button("DDS")) export_prompt(&last_loaded_image->scratch_image, DDS);
								ImGui::SameLine();
								if (ImGui::Button("JPG")) export_prompt(&last_loaded_image->scratch_image, JPG);
								ImGui::SameLine();
								if (ImGui::Button("PNG")) export_prompt(&last_loaded_image->scratch_image, PNG);
							}
						}
						else ImGui::Text("Failed to load image!");
						
					} 
					else {
						ImGui::Text("Incompatible Image!");
					}

				}



			}
			

			ImGui::End();
			ImGui::PopID();
		}
	}
	
	// //////////////////// //
	// RENDER MODEL WINDOW //
	// ////////////////// //
	void configure_shaders() {

	}

	void runtimegeo_window() {
		for (uint32_t i = 0; i < OpenRuntimeGeos.Size(); i++) {
			Tag* tag = OpenRuntimeGeos[i];
			ImGui::PushID(tag->tagID);
			bool window_is_open = true;
			if (!ImGui::Begin(tag->tagname.c_str(), &window_is_open) && window_is_open) {
				ImGui::End();
				ImGui::PopID();
				continue;
			}
			ImGui::DragInt("Active Mesh", &tag->preview_1, 0.1f, 0, 100);
			ImGui::DragInt("Active Lod", &tag->preview_2, 0.1f, 0, 100);
			ImGui::DragInt("Active Part", &tag->preview_3, 0.1f, 0, 100);
			ImGui::End();
			ImGui::PopID();
		}
	}

#include <iostream>
#include <fstream>
	CTList<Tag> OpenRuntimeGeos;
	void render_runtimegeo_window(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader, const XMMATRIX& viewProjectionMatrix) {




		for (uint32_t i = 0; i < OpenRuntimeGeos.Size(); i++) {
			Tag* tag = OpenRuntimeGeos[i];

			if (tag->tag_FourCC == Tag::rtgo) {
				rtgo::RuntimeGeoTag* runtime_geo = (rtgo::RuntimeGeoTag*)tag->tag_data;
				render_struct_render_geometry(&runtime_geo->render_geometry, tag, device, deviceContext, cb_vs_vertexshader, viewProjectionMatrix);
			} 
			else if (tag->tag_FourCC == Tag::mode) {
				rtgo::RuntimeGeoTag* runtime_geo = (mode::RuntimeGeoTag*)tag->tag_data;
				render_struct_render_geometry(&runtime_geo->render_geometry, tag, device, deviceContext, cb_vs_vertexshader, viewProjectionMatrix);
			}
			else {
				throw exception("unsupported render object type, idk how we got here");
			}
		}


		



	}
	
	void render_struct_render_geometry(rtgo::s_render_geometry* render_geo, Tag* tag, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader, const XMMATRIX& viewProjectionMatrix) {
		// tag can ONLY be used for storing preview values, as this will support multiple types

		// no support for multiple 'mesh reosurce groups' yet, throw exception
		if (render_geo->mesh_package.mesh_resource_groups.count != 1)
			throw exception("bad number of resource groups in runtime geo");
		// buffer structs
		rtgo::RenderGeometryMeshPackageResourceGroup* runtime_geo_group = render_geo->mesh_package.mesh_resource_groups[0];
		// double check to make sure the file exists as expected, we dont know how non-chunked models work yet
		if (runtime_geo_group->mesh_resource.is_chunked_resource == 0)
			throw exception("non-chunked geo resources are not yet supported!!!");
		rtgo::s_render_geometry_api_resource* geo_resource = runtime_geo_group->mesh_resource.content_ptr;

		XMMATRIX worldMatrix = XMMatrixRotationRollPitchYaw(0, 0, 0) * XMMatrixTranslation(0, 0, 0); // we need to calculate this elsewhere i think
		// set constant buffer params
		deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader->GetAddressOf());
		cb_vs_vertexshader->data.wvpMatrix = DirectX::XMMatrixIdentity() * worldMatrix * viewProjectionMatrix;
		cb_vs_vertexshader->data.worldMatrix = DirectX::XMMatrixIdentity() * worldMatrix;
		if (render_geo->compression_info.count > 0) {
			rtgo::s_compression_info* compression = render_geo->compression_info[0];
			cb_vs_vertexshader->data.minbounds.x = compression->position_bounds_0.f1;
			cb_vs_vertexshader->data.maxbounds.x = compression->position_bounds_0.f2;
			cb_vs_vertexshader->data.minbounds.y = compression->position_bounds_0.f3;
			cb_vs_vertexshader->data.maxbounds.y = compression->position_bounds_1.f1;
			cb_vs_vertexshader->data.minbounds.z = compression->position_bounds_1.f2;
			cb_vs_vertexshader->data.maxbounds.z = compression->position_bounds_1.f3;
		}
		else { // uh what the hell do we do here? we still need to pass something into the buffer struct
			throw exception("non-compressed position bounds are currently unsupported");
			cb_vs_vertexshader->data.minbounds.x = 0;
			cb_vs_vertexshader->data.minbounds.y = 0;
			cb_vs_vertexshader->data.minbounds.z = 0;
			cb_vs_vertexshader->data.maxbounds.x = 0;
			cb_vs_vertexshader->data.maxbounds.y = 0;
			cb_vs_vertexshader->data.maxbounds.z = 0;
		}
		cb_vs_vertexshader->ApplyChanges();


		for (int m_index = 0; m_index < render_geo->meshes.count; m_index++) {
			if (tag->preview_1 != m_index) continue;
			rtgo::s_mesh* current_mesh = render_geo->meshes[m_index];

			for (int lod_index = 0; lod_index < current_mesh->LOD_render_data.count; lod_index++) {
				if (tag->preview_2 != lod_index) continue;
				rtgo::LODRenderData* current_lod = current_mesh->LOD_render_data[lod_index];

				uint16_t ibuffer_index = current_lod->index_buffer_index;

				rtgo::RasterizerIndexBuffer* index_buffer = geo_resource->pc_index_buffers[current_lod->index_buffer_index];

				rtgo::RasterizerVertexBuffer* vert_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[0].vertex_buffer_index];
				rtgo::RasterizerVertexBuffer* uv0_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[1].vertex_buffer_index];
				//rtgo::RasterizerVertexBuffer* uv1_buffer     = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[2].vertex_buffer_index];
				//rtgo::RasterizerVertexBuffer* uv2_buffer     = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[3].vertex_buffer_index];
				rtgo::RasterizerVertexBuffer* color_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[4].vertex_buffer_index];
				rtgo::RasterizerVertexBuffer* norm_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[5].vertex_buffer_index];
				rtgo::RasterizerVertexBuffer* tangent_buffer = geo_resource->pc_vertex_buffers[current_lod->vertex_buffer_indices[6].vertex_buffer_index];

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
				for (int part_index = 0; part_index < current_lod->parts.count; part_index++) {
					rtgo::s_part* mesh_part = current_lod->parts[part_index];
					deviceContext->IASetVertexBuffers(0, 19, vert_buffers, vert_strides, vert_offsets);
					deviceContext->IASetIndexBuffer((ID3D11Buffer*)index_buffer->m_resource, index_format, 0);
					deviceContext->DrawIndexed(mesh_part->index_count, mesh_part->index_start, 0);

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

		}
	}
	
	/* // useless junk
	uint16_t swap_bytes(uint16_t target) {
		uint16_t upper_to_lower = target >> 8;
		uint16_t lower_to_upper = target << 8;

		return upper_to_lower + lower_to_upper;
	}

	void test_half_mantisa_bit(int bitmask, double bit_half_value, int mantisa, double& output) {
		if (mantisa & bitmask) output += bit_half_value;
	}
	double half_to_float(uint16_t half) {
		int mantisa = half & 0x3FF;
		int exponent = (half >> 10) & 0x1F;
		double sign = ((half & 0b1000000000000000) != 0) ? -1.0 : 1.0;
		//if ((half & 0b1000000000000000) != 0) {
		//	throw exception("code test");
		//}

		// removed because i dont think our format actually uses this
		// test for special cases
		//if (exponent == 0x1F) {
		//	if (mantisa) return NAN; // if mantisa is not 0, then return NaN
		//	else return INFINITY * sign; // else return signed infinity if mantisa is 0
		//}
		// shortcut for returning 0 
		if (mantisa == 0 && exponent == 0) return 0.0 * sign;

		// conmvert matisa to double
		double out_mantisa = 0.0;
		test_half_mantisa_bit(0b1, 0.000977,     mantisa, out_mantisa);
		test_half_mantisa_bit(0b10, 0.001954,    mantisa, out_mantisa);
		test_half_mantisa_bit(0b100, 0.00391,    mantisa, out_mantisa);
		test_half_mantisa_bit(0b1000, 0.00782,   mantisa, out_mantisa);
		test_half_mantisa_bit(0b10000, 0.01563,  mantisa, out_mantisa);
		test_half_mantisa_bit(0b100000, 0.03125, mantisa, out_mantisa);
		test_half_mantisa_bit(0b1000000, 0.0625, mantisa, out_mantisa);
		test_half_mantisa_bit(0b10000000, 0.125, mantisa, out_mantisa);
		test_half_mantisa_bit(0b100000000, 0.25, mantisa, out_mantisa);
		test_half_mantisa_bit(0b1000000000, 0.5, mantisa, out_mantisa);

		// convert exponent to double
		int exponent_out = 0;
		exponent_out = exponent - 15;
		if (exponent > 0) { // leading bit, if exponent is greater than 1, mantisa is actually += 1.0
			out_mantisa += 1.0; 
			exponent_out--;
		}
		

		double expontent_result = pow(2, exponent_out);

		// calculate double
		return sign * expontent_result * out_mantisa;
	}
	*/
};