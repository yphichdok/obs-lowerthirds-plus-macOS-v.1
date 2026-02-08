#include "json-loader.hpp"
#include <obs-module.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string JSONLoader::get_templates_path()
{
	char *config_path = obs_module_config_path("");
	if (!config_path)
		return "";
	
	std::string path = config_path;
	bfree(config_path);
	
	path += "/templates";
	
	// Create directory if it doesn't exist
	os_mkdirs(path.c_str());
	
	return path;
}

std::vector<std::string> JSONLoader::get_available_templates()
{
	std::vector<std::string> templates;
	
	// Always include default
	templates.push_back("default");
	
	std::string templates_path = get_templates_path();
	if (templates_path.empty())
		return templates;
	
	// Scan directory for JSON files
	os_dir_t *dir = os_opendir(templates_path.c_str());
	if (!dir)
		return templates;
	
	struct os_dirent *entry;
	while ((entry = os_readdir(dir)) != nullptr) {
		if (entry->directory)
			continue;
		
		std::string filename = entry->d_name;
		if (filename.length() > 5 && 
			filename.substr(filename.length() - 5) == ".json") {
			// Remove .json extension
			std::string name = filename.substr(0, filename.length() - 5);
			templates.push_back(name);
		}
	}
	
	os_closedir(dir);
	
	return templates;
}

TemplateData JSONLoader::load_template(const std::string &name)
{
	// Return default template
	if (name == "default" || name.empty()) {
		TemplateData tmpl;
		tmpl.name = "default";
		return tmpl;
	}
	
	// Try to load from file
	std::string templates_path = get_templates_path();
	if (templates_path.empty()) {
		blog(LOG_WARNING, "Could not get templates path");
		return TemplateData();
	}
	
	std::string filepath = templates_path + "/" + name + ".json";
	
	if (!os_file_exists(filepath.c_str())) {
		blog(LOG_WARNING, "Template file not found: %s", filepath.c_str());
		return TemplateData();
	}
	
	return parse_json_file(filepath);
}

TemplateData JSONLoader::parse_json_file(const std::string &filepath)
{
	TemplateData tmpl;
	
	try {
		std::ifstream file(filepath);
		if (!file.is_open()) {
			blog(LOG_ERROR, "Failed to open template file: %s", 
				filepath.c_str());
			return tmpl;
		}
		
		json j;
		file >> j;
		
		// Parse fields
		if (j.contains("name"))
			tmpl.name = j["name"].get<std::string>();
		
		if (j.contains("font"))
			tmpl.font = j["font"].get<std::string>();
		
		if (j.contains("title_size"))
			tmpl.title_size = j["title_size"].get<int>();
		
		if (j.contains("subtitle_size"))
			tmpl.subtitle_size = j["subtitle_size"].get<int>();
		
		if (j.contains("padding")) {
			auto padding = j["padding"];
			if (padding.contains("left"))
				tmpl.padding_left = padding["left"].get<int>();
			if (padding.contains("right"))
				tmpl.padding_right = padding["right"].get<int>();
			if (padding.contains("top"))
				tmpl.padding_top = padding["top"].get<int>();
			if (padding.contains("bottom"))
				tmpl.padding_bottom = padding["bottom"].get<int>();
		}
		
		if (j.contains("default_bg_color")) {
			std::string color_str = j["default_bg_color"].get<std::string>();
			tmpl.default_bg_color = parse_color_string(color_str);
		}
		
		if (j.contains("animation_speed"))
			tmpl.animation_speed = j["animation_speed"].get<float>();
		
		blog(LOG_INFO, "Loaded template: %s", tmpl.name.c_str());
		
	} catch (const std::exception &e) {
		blog(LOG_ERROR, "Failed to parse template JSON: %s", e.what());
	}
	
	return tmpl;
}

uint32_t JSONLoader::parse_color_string(const std::string &color_str)
{
	// Expected format: "#RRGGBB" or "#AARRGGBB"
	if (color_str.empty() || color_str[0] != '#')
		return 0xFF1E88E5;
	
	std::string hex = color_str.substr(1);
	
	try {
		uint32_t value = std::stoul(hex, nullptr, 16);
		
		// If 6 digits, add full alpha
		if (hex.length() == 6) {
			value |= 0xFF000000;
		}
		
		return value;
	} catch (...) {
		return 0xFF1E88E5;
	}
}
