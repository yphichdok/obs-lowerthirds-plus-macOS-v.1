#pragma once

#include <obs-module.h>
#include <graphics/image-file.h>
#include <string>

// Animation style options
enum AnimationStyle {
	ANIM_SLIDE_LEFT = 0,
	ANIM_SLIDE_RIGHT = 1,
	ANIM_SLIDE_BOTTOM = 2,
	ANIM_SLIDE_TOP = 3,
	ANIM_FADE = 4,
	ANIM_ZOOM = 5,
	ANIM_EXPAND_LEFT = 6,
	ANIM_EXPAND_RIGHT = 7,
	ANIM_PUSH_LEFT = 8,
	ANIM_PUSH_RIGHT = 9,
	ANIM_WIPE_LEFT = 10,
	ANIM_WIPE_RIGHT = 11,
	ANIM_SPIN = 12,
	ANIM_SCROLL = 13,
	ANIM_ROLL = 14,
	ANIM_INSTANT = 15
};

// Gradient direction options
enum GradientType {
	GRADIENT_NONE = 0,
	GRADIENT_HORIZONTAL = 1,
	GRADIENT_VERTICAL = 2,
	GRADIENT_DIAGONAL_TL_BR = 3,
	GRADIENT_DIAGONAL_BL_TR = 4
};

struct lowerthirds_source {
	obs_source_t *source;
	
	// Current active profile (0-4 for 5 profiles/tabs)
	int current_profile;
	
	// Number of visible tabs (1-5, starts at 1)
	int num_visible_tabs;
	
	// Text sources for rendering
	obs_source_t *title_text_source;
	obs_source_t *subtitle_text_source;
	obs_source_t *title_right_text_source;
	obs_source_t *subtitle_right_text_source;
	
	// Text fields for 5 profiles (tabs)
	char *title[5];
	char *subtitle[5];
	char *title_right[5];
	char *subtitle_right[5];
	
	char *font_face;
	
	// Background image
	char *bg_image_path;
	gs_image_file_t *bg_image;
	
	// Logo image (left side)
	char *logo_image_path;
	gs_image_file_t *logo_image;
	int logo_size;
	int logo_opacity;
	int logo_padding_horizontal;
	int logo_padding_vertical;
	
	// Visual properties
	uint32_t bg_color;
	uint32_t text_color;
	int opacity;
	int title_size;
	int subtitle_size;
	int padding_horizontal;
	int padding_vertical;
	int bar_height_pixels;
	bool show_background;
	
	// Shadow properties for text
	bool text_shadow_enabled;
	uint32_t text_shadow_color;
	int text_shadow_opacity;
	int text_shadow_offset_x;
	int text_shadow_offset_y;
	
	// Shadow properties for logo
	bool logo_shadow_enabled;
	uint32_t logo_shadow_color;
	int logo_shadow_opacity;
	int logo_shadow_offset_x;
	int logo_shadow_offset_y;
	
	// Gradient properties
	GradientType gradient_type;
	uint32_t gradient_color2;
	
	// Responsive scaling
	bool auto_scale;
	float scale_factor;
	
	// Animation state
	bool is_visible;
	float animation_progress;
	AnimationStyle animation_style;        // Background animation
	AnimationStyle logo_animation_style;   // Logo animation (independent)
	AnimationStyle text_animation_style;   // Text animation (independent)
	
	// Auto-hide timer
	bool auto_hide_enabled;
	float display_duration;
	float display_timer;
	bool force_replay;
	
	lowerthirds_source(obs_source_t *source, obs_data_t *settings);
	~lowerthirds_source();
	
	void update(obs_data_t *settings);
	void tick(float seconds);
	void render();
	uint32_t get_width();
	uint32_t get_height();
	
	void update_text_sources();
	void draw_gradient_rect(float x, float y, float width, float height, 
		struct vec4 color1, struct vec4 color2, GradientType type);
	void draw_logo_with_alpha(gs_texture_t *texture, float width, float height, float alpha);
};

void register_lowerthirds_source();
