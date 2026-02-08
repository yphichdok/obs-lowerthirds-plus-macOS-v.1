#pragma once

#include <string>
#include <vector>

struct TemplateData {
	std::string name;
	std::string font;
	int title_size;
	int subtitle_size;
	int padding_left;
	int padding_right;
	int padding_top;
	int padding_bottom;
	uint32_t default_bg_color;
	float animation_speed;
	
	TemplateData()
		: name("default")
		, font("Arial")
		, title_size(48)
		, subtitle_size(32)
		, padding_left(40)
		, padding_right(40)
		, padding_top(20)
		, padding_bottom(20)
		, default_bg_color(0xFF1E88E5)
		, animation_speed(0.5f)
	{}
};

class JSONLoader {
public:
	static TemplateData load_template(const std::string &name);
	static std::vector<std::string> get_available_templates();
	static std::string get_templates_path();
	
private:
	static TemplateData parse_json_file(const std::string &filepath);
	static uint32_t parse_color_string(const std::string &color_str);
};
