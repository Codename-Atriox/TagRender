#pragma once
#include "Utilities/UI_Bitmap.h"
#include "../Tags/TagContainers.h"

class UI {
public:
	void render_UI(ModuleManager* Modules, ID3D11Device* device) {
		render_module_window(Modules);
		render_loaded_tags_window(Modules);
		render_bitmap_window(Modules, device);



	}


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

			std::string test;
			if (NativeFileDialogue::NFD_OpenDialog(test)) {
				Modules->OpenModule(test);
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
				case 1651078253:{
					ImGui::Text("Bitmap");
					// check whether bitmap is opened
					bool is_opened = false;
					for (int i = 0; i < OpenBitmaps.Size(); i++) {
						if (OpenBitmaps[i] == active_tag) {
							is_opened = true;
							break;
					}}
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
	CTList<Tag> OpenBitmaps;
	void render_bitmap_window(ModuleManager* Modules, ID3D11Device* device) {
		// create each window
		for (uint32_t i = 0; i < OpenBitmaps.Size(); i++) {
			Tag* open_bitmap = OpenBitmaps[i];
			ImGui::PushID(open_bitmap->tagID);
			ImGui::Begin(open_bitmap->tagname.c_str()); // use name of the tag?
			if (ImGui::Button("Close")){
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
					ImGui::DragInt("Active LOD", &open_bitmap->preview_1, 0.1f, 0, lods-1);
					// then present the actual image
					if (curr_bitmap->type == BitmapType::_2D_texture) {
						BitmapResource* last_loaded_image = Modules->BITM_GetTexture(open_bitmap, device, open_bitmap->preview_1);
						if (last_loaded_image != nullptr)
							ImGui::Image((void*)last_loaded_image->image_view, ImVec2(256, 256));
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