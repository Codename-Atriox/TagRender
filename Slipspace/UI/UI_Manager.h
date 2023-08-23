#pragma once
#include "Utilities/UI_Bitmap.h"
#include "../Tags/TagContainers.h"

#include "../../Utilities/NFD/include/nfd.h"

class UI {
public:
	void render_UI(ModuleManager* Modules, ID3D11Device* device) {
		render_module_window(Modules);
		render_loaded_tags_window(Modules);
		render_bitmap_window(Modules, device);



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
	void render_module_window(ModuleManager* Modules) {
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
							Modules->OpenTag(menu_active_tag->GlobalTagId);
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
				case 1651078253: {
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
				case 1:
					ImGui::Text("Runtime Geo");
					break;
				case 2:
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
				if (selected_bitmap < bitmap_header->bitmaps.count) {
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
					ImGui::DragInt("Active MIP", &open_bitmap->preview_1, 0.1f, 0, lods-1);
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
	void render_model_window() {

	}
};