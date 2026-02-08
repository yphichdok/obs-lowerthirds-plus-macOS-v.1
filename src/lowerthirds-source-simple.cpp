/**
 * Lower Thirds Plus - Professional Lower Thirds Plugin for OBS Studio
 * 
 * Author: Y Phic Hdok
 * Company: MTD Technologies
 * Version: 1.0
 * 
 * A professional lower thirds plugin featuring tabbed profiles, modern animations,
 * and extensive customization options for broadcast-quality graphics.
 */

#include "lowerthirds-source-simple.hpp"
#include "json-loader.hpp"
#include <obs-module.h>
#include <graphics/graphics.h>
#include <graphics/vec4.h>
#include <graphics/matrix4.h>
#include <graphics/image-file.h>
#include <util/bmem.h>
#include <util/dstr.h>
#include <util/platform.h>
#include <math.h>

// Source callbacks
static const char *lowerthirds_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "Lower Thirds Plus";
}

static void *lowerthirds_create(obs_data_t *settings, obs_source_t *source)
{
	lowerthirds_source *context = new lowerthirds_source(source, settings);
	return context;
}

static void lowerthirds_destroy(void *data)
{
	lowerthirds_source *context = (lowerthirds_source *)data;
	delete context;
}

static void lowerthirds_update(void *data, obs_data_t *settings)
{
	lowerthirds_source *context = (lowerthirds_source *)data;
	context->update(settings);
}

static void lowerthirds_video_tick(void *data, float seconds)
{
	lowerthirds_source *context = (lowerthirds_source *)data;
	context->tick(seconds);
}

static void lowerthirds_video_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	lowerthirds_source *context = (lowerthirds_source *)data;
	context->render();
}

static void lowerthirds_show(void *data)
{
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// ALWAYS force a fresh replay when eye icon is clicked
	context->force_replay = true;
	context->display_timer = 0.0f;
	context->animation_progress = 0.0f;
	context->is_visible = true;
	
	// Trigger update with visible=true
	obs_data_t *settings = obs_source_get_settings(context->source);
	obs_data_set_bool(settings, "visible", true);
	obs_source_update(context->source, settings);
	obs_data_release(settings);
}

static void lowerthirds_hide(void *data)
{
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// Hide immediately and reset state
	context->is_visible = false;
	context->display_timer = 0.0f;
	context->force_replay = false;
	
	// Update settings
	obs_data_t *settings = obs_source_get_settings(context->source);
	obs_data_set_bool(settings, "visible", false);
	obs_source_update(context->source, settings);
	obs_data_release(settings);
}

static uint32_t lowerthirds_get_width(void *data)
{
	lowerthirds_source *context = (lowerthirds_source *)data;
	return context->get_width();
}

static uint32_t lowerthirds_get_height(void *data)
{
	lowerthirds_source *context = (lowerthirds_source *)data;
	return context->get_height();
}

static void lowerthirds_get_defaults(obs_data_t *settings)
{
	// Preview mode (disabled by default)
	obs_data_set_default_bool(settings, "preview_mode", false);
	
	// Current profile selection
	obs_data_set_default_int(settings, "current_profile", 0);
	
	// Number of visible tabs (start with 1)
	obs_data_set_default_int(settings, "num_visible_tabs", 1);
	
	// Profile 1 (Tab 1) defaults
	obs_data_set_default_string(settings, "profile1_title", "Guest 1");
	obs_data_set_default_string(settings, "profile1_subtitle", "CEO");
	obs_data_set_default_string(settings, "profile1_title_right", "");
	obs_data_set_default_string(settings, "profile1_subtitle_right", "");
	
	// Profile 2 (Tab 2) defaults
	obs_data_set_default_string(settings, "profile2_title", "Guest 2");
	obs_data_set_default_string(settings, "profile2_subtitle", "CTO");
	obs_data_set_default_string(settings, "profile2_title_right", "");
	obs_data_set_default_string(settings, "profile2_subtitle_right", "");
	
	// Profile 3 (Tab 3) defaults
	obs_data_set_default_string(settings, "profile3_title", "Guest 3");
	obs_data_set_default_string(settings, "profile3_subtitle", "Designer");
	obs_data_set_default_string(settings, "profile3_title_right", "");
	obs_data_set_default_string(settings, "profile3_subtitle_right", "");
	
	// Profile 4 (Tab 4) defaults
	obs_data_set_default_string(settings, "profile4_title", "Guest 4");
	obs_data_set_default_string(settings, "profile4_subtitle", "Developer");
	obs_data_set_default_string(settings, "profile4_title_right", "");
	obs_data_set_default_string(settings, "profile4_subtitle_right", "");
	
	// Profile 5 (Tab 5) defaults
	obs_data_set_default_string(settings, "profile5_title", "Guest 5");
	obs_data_set_default_string(settings, "profile5_subtitle", "Manager");
	obs_data_set_default_string(settings, "profile5_title_right", "");
	obs_data_set_default_string(settings, "profile5_subtitle_right", "");
	obs_data_set_default_int(settings, "bg_color", 0xFFF5A542); // Light blue (ABGR format)
	obs_data_set_default_int(settings, "text_color", 0xFFFFFFFF);
	obs_data_set_default_int(settings, "opacity", 100); // 100% solid - no transparency!
	obs_data_set_default_bool(settings, "show_background", true); // Background ON by default
	obs_data_set_default_int(settings, "title_size", 72);  // Larger, more prominent title
	obs_data_set_default_int(settings, "subtitle_size", 48);  // Larger, more readable subtitle
	obs_data_set_default_int(settings, "padding_horizontal", 60);  // More breathing room
	obs_data_set_default_int(settings, "padding_vertical", 25);
	obs_data_set_default_int(settings, "bar_height", 200);  // Taller box for larger content
	
	// Set default font as OBS font object
	obs_data_t *font_obj = obs_data_create();
	obs_data_set_string(font_obj, "face", "Helvetica Neue");
	obs_data_set_default_obj(settings, "font_face", font_obj);
	obs_data_release(font_obj);
	
	obs_data_set_default_bool(settings, "visible", false);
	obs_data_set_default_bool(settings, "bold", true);
	obs_data_set_default_bool(settings, "auto_hide", true);
	obs_data_set_default_double(settings, "duration", 5.0);
	obs_data_set_default_string(settings, "bg_image", "");
	obs_data_set_default_bool(settings, "auto_scale", false); // OFF by default - keeps text at exact sizes
	obs_data_set_default_int(settings, "animation_style", ANIM_SLIDE_LEFT);
	
	// Logo defaults
	obs_data_set_default_string(settings, "logo_image", "");
	obs_data_set_default_int(settings, "logo_size", 140);  // Larger logo to match bigger box
	obs_data_set_default_int(settings, "logo_opacity", 100);
	obs_data_set_default_int(settings, "logo_padding_horizontal", 20);  // More padding
	obs_data_set_default_int(settings, "logo_padding_vertical", 0);
	
	// Gradient defaults
	obs_data_set_default_int(settings, "gradient_type", GRADIENT_NONE);
	obs_data_set_default_int(settings, "gradient_color2", 0xFFD27619); // Darker blue for gradient (ABGR format)
}

// Button callback for Play Profile 1
static bool lowerthirds_play_profile1_clicked(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// IMPORTANT: Hide completely first to prevent any old content from showing
	context->is_visible = false;
	context->animation_progress = 0.0f;
	
	// NOW switch profile and update text while hidden
	context->current_profile = 0;
	context->update_text_sources(); // Update text while invisible
	
	// NOW start fresh animation with correct content
	context->force_replay = true;
	context->display_timer = 0.0f;
	context->is_visible = true;
	
	return true;
}

// Button callback for Play Profile 2
static bool lowerthirds_play_profile2_clicked(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// IMPORTANT: Hide completely first to prevent any old content from showing
	context->is_visible = false;
	context->animation_progress = 0.0f;
	
	// NOW switch profile and update text while hidden
	context->current_profile = 1;
	context->update_text_sources(); // Update text while invisible
	
	// NOW start fresh animation with correct content
	context->force_replay = true;
	context->display_timer = 0.0f;
	context->is_visible = true;
	
	return true;
}

// Button callback for Play Profile 3
static bool lowerthirds_play_profile3_clicked(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// IMPORTANT: Hide completely first to prevent any old content from showing
	context->is_visible = false;
	context->animation_progress = 0.0f;
	
	// NOW switch profile and update text while hidden
	context->current_profile = 2;
	context->update_text_sources(); // Update text while invisible
	
	// NOW start fresh animation with correct content
	context->force_replay = true;
	context->display_timer = 0.0f;
	context->is_visible = true;
	
	return true;
}

// Button callback for Play Profile 4
static bool lowerthirds_play_profile4_clicked(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// IMPORTANT: Hide completely first to prevent any old content from showing
	context->is_visible = false;
	context->animation_progress = 0.0f;
	
	// NOW switch profile and update text while hidden
	context->current_profile = 3;
	context->update_text_sources(); // Update text while invisible
	
	// NOW start fresh animation with correct content
	context->force_replay = true;
	context->display_timer = 0.0f;
	context->is_visible = true;
	
	return true;
}

// Button callback for Play Profile 5
static bool lowerthirds_play_profile5_clicked(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// IMPORTANT: Hide completely first to prevent any old content from showing
	context->is_visible = false;
	context->animation_progress = 0.0f;
	
	// NOW switch profile and update text while hidden
	context->current_profile = 4;
	context->update_text_sources(); // Update text while invisible
	
	// NOW start fresh animation with correct content
	context->force_replay = true;
	context->display_timer = 0.0f;
	context->is_visible = true;
	
	return true;
}

// Button callback for Add Another Tab
static bool lowerthirds_add_tab_clicked(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// Increase visible tabs count (max 5)
	if (context->num_visible_tabs < 5) {
		context->num_visible_tabs++;
		
		// Save to settings and update source
		obs_data_t *settings = obs_source_get_settings(context->source);
		obs_data_set_int(settings, "num_visible_tabs", context->num_visible_tabs);
		
		// Trigger a full update to refresh properties
		obs_source_update(context->source, settings);
		obs_data_release(settings);
		
		// Return true to trigger properties refresh
		return true;
	}
	
	return false;
}

// Button callback for Remove Last Tab
static bool lowerthirds_remove_tab_clicked(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// Decrease visible tabs count (min 1)
	if (context->num_visible_tabs > 1) {
		context->num_visible_tabs--;
		
		// Save to settings and update source
		obs_data_t *settings = obs_source_get_settings(context->source);
		obs_data_set_int(settings, "num_visible_tabs", context->num_visible_tabs);
		
		// Trigger a full update to refresh properties
		obs_source_update(context->source, settings);
		obs_data_release(settings);
		
		// Return true to trigger properties refresh
		return true;
	}
	
	return false;
}

// Callback for Preview toggle
static bool lowerthirds_preview_toggled(obs_properties_t *props, obs_property_t *property, obs_data_t *settings)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	
	// When preview is enabled, force the lower third to show
	bool preview_enabled = obs_data_get_bool(settings, "preview_mode");
	obs_data_set_bool(settings, "visible", preview_enabled);
	
	return true;
}

static obs_properties_t *lowerthirds_get_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();
	lowerthirds_source *context = (lowerthirds_source *)data;
	
	// === PREVIEW MODE (toggle to see live while editing) ===
	obs_property_t *preview_prop = obs_properties_add_bool(props, "preview_mode", "👁️ Preview (Show Live While Editing)");
	obs_property_set_modified_callback(preview_prop, lowerthirds_preview_toggled);
	
	// === QUICK PLAY TABS (with actual titles) - all 5 tabs ===
	char button_label[256];
	
	// Tab 1 button
	const char *title1 = (context && context->title[0] && strlen(context->title[0]) > 0) 
		? context->title[0] : "Tab 1";
	snprintf(button_label, sizeof(button_label), "[▶ %s]", title1);
	obs_properties_add_button2(props, "play_profile1", button_label, lowerthirds_play_profile1_clicked, data);
	
	// Tab 2 button
	const char *title2 = (context && context->title[1] && strlen(context->title[1]) > 0) 
		? context->title[1] : "Tab 2";
	snprintf(button_label, sizeof(button_label), "[▶ %s]", title2);
	obs_properties_add_button2(props, "play_profile2", button_label, lowerthirds_play_profile2_clicked, data);
	
	// Tab 3 button
	const char *title3 = (context && context->title[2] && strlen(context->title[2]) > 0) 
		? context->title[2] : "Tab 3";
	snprintf(button_label, sizeof(button_label), "[▶ %s]", title3);
	obs_properties_add_button2(props, "play_profile3", button_label, lowerthirds_play_profile3_clicked, data);
	
	// Tab 4 button
	const char *title4 = (context && context->title[3] && strlen(context->title[3]) > 0) 
		? context->title[3] : "Tab 4";
	snprintf(button_label, sizeof(button_label), "[▶ %s]", title4);
	obs_properties_add_button2(props, "play_profile4", button_label, lowerthirds_play_profile4_clicked, data);
	
	// Tab 5 button
	const char *title5 = (context && context->title[4] && strlen(context->title[4]) > 0) 
		? context->title[4] : "Tab 5";
	snprintf(button_label, sizeof(button_label), "[▶ %s]", title5);
	obs_properties_add_button2(props, "play_profile5", button_label, lowerthirds_play_profile5_clicked, data);
	
	// === TAB CONTENT (all 5 tabs always visible) ===
	
	// TAB 1
	obs_properties_add_text(props, "profile1_title", "Tab 1 · Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile1_subtitle", "Tab 1 · Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile1_title_right", "Tab 1 · Title (Right)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile1_subtitle_right", "Tab 1 · Subtitle (Right)", OBS_TEXT_DEFAULT);
	
	// TAB 2
	obs_properties_add_text(props, "profile2_title", "Tab 2 · Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile2_subtitle", "Tab 2 · Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile2_title_right", "Tab 2 · Title (Right)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile2_subtitle_right", "Tab 2 · Subtitle (Right)", OBS_TEXT_DEFAULT);
	
	// TAB 3
	obs_properties_add_text(props, "profile3_title", "Tab 3 · Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile3_subtitle", "Tab 3 · Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile3_title_right", "Tab 3 · Title (Right)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile3_subtitle_right", "Tab 3 · Subtitle (Right)", OBS_TEXT_DEFAULT);
	
	// TAB 4
	obs_properties_add_text(props, "profile4_title", "Tab 4 · Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile4_subtitle", "Tab 4 · Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile4_title_right", "Tab 4 · Title (Right)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile4_subtitle_right", "Tab 4 · Subtitle (Right)", OBS_TEXT_DEFAULT);
	
	// TAB 5
	obs_properties_add_text(props, "profile5_title", "Tab 5 · Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile5_subtitle", "Tab 5 · Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile5_title_right", "Tab 5 · Title (Right)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "profile5_subtitle_right", "Tab 5 · Subtitle (Right)", OBS_TEXT_DEFAULT);
	
	// === ADVANCED SETTINGS (COLLAPSIBLE) ===
	obs_properties_t *advanced_group = obs_properties_create();
	
	// PLAYBACK CONTROLS
	obs_properties_add_bool(advanced_group, "auto_hide", "Auto-Hide After Duration");
	obs_properties_add_float_slider(advanced_group, "duration", "Duration (seconds)", 1.0, 30.0, 0.5);
	
	obs_property_t *anim_list = obs_properties_add_list(advanced_group, "animation_style", 
		"Animation Style", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(anim_list, "↖ Slide from Left", ANIM_SLIDE_LEFT);
	obs_property_list_add_int(anim_list, "↗ Slide from Right", ANIM_SLIDE_RIGHT);
	obs_property_list_add_int(anim_list, "↑ Slide from Bottom", ANIM_SLIDE_BOTTOM);
	obs_property_list_add_int(anim_list, "↓ Slide from Top", ANIM_SLIDE_TOP);
	obs_property_list_add_int(anim_list, "⊙ Fade In", ANIM_FADE);
	obs_property_list_add_int(anim_list, "⊕ Zoom In", ANIM_ZOOM);
	obs_property_list_add_int(anim_list, "⇤ Expand from Left", ANIM_EXPAND_LEFT);
	obs_property_list_add_int(anim_list, "⇥ Expand from Right", ANIM_EXPAND_RIGHT);
	obs_property_list_add_int(anim_list, "⟵ Push from Left", ANIM_PUSH_LEFT);
	obs_property_list_add_int(anim_list, "⟶ Push from Right", ANIM_PUSH_RIGHT);
	obs_property_list_add_int(anim_list, "⇐ Wipe from Left", ANIM_WIPE_LEFT);
	obs_property_list_add_int(anim_list, "⇒ Wipe from Right", ANIM_WIPE_RIGHT);
	obs_property_list_add_int(anim_list, "⚡ Instant", ANIM_INSTANT);
	
	// TEXT STYLING
	obs_properties_add_font(advanced_group, "font_face", "Font");
	obs_properties_add_bool(advanced_group, "bold", "Bold");
	obs_properties_add_int_slider(advanced_group, "title_size", "Title Size (px)", 20, 120, 2);
	obs_properties_add_int_slider(advanced_group, "subtitle_size", "Subtitle Size (px)", 16, 100, 2);
	obs_properties_add_color(advanced_group, "text_color", "Text Color");
	
	// LOGO (LEFT SIDE - OPTIONAL)
	obs_properties_add_path(advanced_group, "logo_image", "Logo Image (Optional)", 
		OBS_PATH_FILE, "Image Files (*.png *.jpg *.jpeg *.bmp *.svg);;All Files (*.*)", NULL);
	obs_properties_add_int_slider(advanced_group, "logo_size", "Logo Size (px)", 30, 200, 5);
	obs_properties_add_int_slider(advanced_group, "logo_opacity", "Logo Opacity (%)", 0, 100, 1);
	obs_properties_add_int_slider(advanced_group, "logo_padding_horizontal", "Logo Padding (Left/Right)", 0, 200, 5);
	obs_properties_add_int_slider(advanced_group, "logo_padding_vertical", "Logo Padding (Top/Bottom)", 0, 100, 5);
	
	// BACKGROUND STYLING
	obs_properties_add_bool(advanced_group, "show_background", "☑️ Show Background (OFF = Text & Logo Only)");
	obs_properties_add_color(advanced_group, "bg_color", "Background Color");
	obs_properties_add_int_slider(advanced_group, "opacity", "Opacity (%)", 0, 100, 1);
	
	obs_property_t *gradient_list = obs_properties_add_list(advanced_group, "gradient_type", 
		"Gradient Style", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(gradient_list, "None", GRADIENT_NONE);
	obs_property_list_add_int(gradient_list, "Horizontal →", GRADIENT_HORIZONTAL);
	obs_property_list_add_int(gradient_list, "Vertical ↓", GRADIENT_VERTICAL);
	obs_property_list_add_int(gradient_list, "Diagonal ↘", GRADIENT_DIAGONAL_TL_BR);
	obs_property_list_add_int(gradient_list, "Diagonal ↗", GRADIENT_DIAGONAL_BL_TR);
	
	obs_properties_add_color(advanced_group, "gradient_color2", "Gradient End Color");
	obs_properties_add_path(advanced_group, "bg_image", "Background Image (Optional)", 
		OBS_PATH_FILE, "Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*.*)", NULL);
	
	// LAYOUT & POSITIONING
	obs_properties_add_int_slider(advanced_group, "bar_height", "Bar Height (px)", 80, 300, 10);
	obs_properties_add_int_slider(advanced_group, "padding_horizontal", "Text Padding (Left/Right)", 0, 300, 5);
	obs_properties_add_int_slider(advanced_group, "padding_vertical", "Text Padding (Top/Bottom)", 0, 150, 5);
	obs_properties_add_bool(advanced_group, "auto_scale", "Auto-Scale (OFF = Text Stays Original Size)");
	
	// Add the collapsible group to main properties
	obs_properties_add_group(props, "advanced_settings", "⚙️ Advanced Settings", 
		OBS_GROUP_NORMAL, advanced_group);
	
	return props;
}

// Constructor
lowerthirds_source::lowerthirds_source(obs_source_t *src, obs_data_t *settings)
	: source(src)
	, current_profile(0)
	, num_visible_tabs(1)
	, title_text_source(nullptr)
	, subtitle_text_source(nullptr)
	, title_right_text_source(nullptr)
	, subtitle_right_text_source(nullptr)
	, font_face(nullptr)
	, bg_image_path(nullptr)
	, bg_image(nullptr)
	, logo_image_path(nullptr)
	, logo_image(nullptr)
	, logo_size(140)  // Larger default logo
	, logo_opacity(100)
	, logo_padding_horizontal(20)  // More padding
	, logo_padding_vertical(0)
	, bg_color(0xFFF5A542)  // Light blue (ABGR)
	, text_color(0xFFFFFFFF)
	, opacity(100)  // 100% solid by default
	, title_size(72)  // Larger title
	, subtitle_size(48)  // Larger subtitle
	, padding_horizontal(60)  // More breathing room
	, padding_vertical(25)
	, bar_height_pixels(200)  // Taller box
	, show_background(true)
	, gradient_type(GRADIENT_NONE)
	, gradient_color2(0xFFD27619)
	, auto_scale(false)  // OFF by default - keeps consistent pixel sizes
	, scale_factor(1.0f)
	, is_visible(false)
	, animation_progress(0.0f)
	, animation_style(ANIM_SLIDE_LEFT)
	, auto_hide_enabled(true)
	, display_duration(5.0f)
	, display_timer(0.0f)
	, force_replay(false)
{
	// Create UNIQUE text sources for this instance (prevents conflicts when duplicating)
	// Use source pointer to ensure unique names for each duplicated instance
	char name_buffer[256];
	
	// Create text sources for rendering (left side) - UNIQUE per instance
	obs_data_t *title_settings = obs_data_create();
	obs_data_t *subtitle_settings = obs_data_create();
	
	snprintf(name_buffer, sizeof(name_buffer), "lt_title_%p", (void*)src);
	title_text_source = obs_source_create_private("text_ft2_source_v2", name_buffer, title_settings);
	
	snprintf(name_buffer, sizeof(name_buffer), "lt_subtitle_%p", (void*)src);
	subtitle_text_source = obs_source_create_private("text_ft2_source_v2", name_buffer, subtitle_settings);
	
	// If FreeType text not available, try GDI (macOS uses FreeType usually)
	if (!title_text_source) {
		snprintf(name_buffer, sizeof(name_buffer), "lt_title_%p", (void*)src);
		title_text_source = obs_source_create_private("text_gdiplus_v2", name_buffer, title_settings);
	}
	if (!subtitle_text_source) {
		snprintf(name_buffer, sizeof(name_buffer), "lt_subtitle_%p", (void*)src);
		subtitle_text_source = obs_source_create_private("text_gdiplus_v2", name_buffer, subtitle_settings);
	}
	
	obs_data_release(title_settings);
	obs_data_release(subtitle_settings);
	
	// Create text sources for right side (optional) - UNIQUE per instance
	obs_data_t *title_right_settings = obs_data_create();
	obs_data_t *subtitle_right_settings = obs_data_create();
	
	snprintf(name_buffer, sizeof(name_buffer), "lt_title_right_%p", (void*)src);
	title_right_text_source = obs_source_create_private("text_ft2_source_v2", name_buffer, title_right_settings);
	
	snprintf(name_buffer, sizeof(name_buffer), "lt_subtitle_right_%p", (void*)src);
	subtitle_right_text_source = obs_source_create_private("text_ft2_source_v2", name_buffer, subtitle_right_settings);
	
	if (!title_right_text_source) {
		snprintf(name_buffer, sizeof(name_buffer), "lt_title_right_%p", (void*)src);
		title_right_text_source = obs_source_create_private("text_gdiplus_v2", name_buffer, title_right_settings);
	}
	if (!subtitle_right_text_source) {
		snprintf(name_buffer, sizeof(name_buffer), "lt_subtitle_right_%p", (void*)src);
		subtitle_right_text_source = obs_source_create_private("text_gdiplus_v2", name_buffer, subtitle_right_settings);
	}
	
	obs_data_release(title_right_settings);
	obs_data_release(subtitle_right_settings);
	
	// Initialize all 5 profiles with nullptr
	for (int i = 0; i < 5; i++) {
		title[i] = nullptr;
		subtitle[i] = nullptr;
		title_right[i] = nullptr;
		subtitle_right[i] = nullptr;
	}
	
	update(settings);
}

lowerthirds_source::~lowerthirds_source()
{
	obs_source_release(title_text_source);
	obs_source_release(subtitle_text_source);
	obs_source_release(title_right_text_source);
	obs_source_release(subtitle_right_text_source);
	
	if (bg_image) {
		obs_enter_graphics();
		gs_image_file_free(bg_image);
		obs_leave_graphics();
		bfree(bg_image);
	}
	
	if (logo_image) {
		obs_enter_graphics();
		gs_image_file_free(logo_image);
		obs_leave_graphics();
		bfree(logo_image);
	}
	
	// Free all profiles
	for (int i = 0; i < 5; i++) {
		bfree(title[i]);
		bfree(subtitle[i]);
		bfree(title_right[i]);
		bfree(subtitle_right[i]);
	}
	
	bfree(font_face);
	bfree(bg_image_path);
	bfree(logo_image_path);
}

void lowerthirds_source::update(obs_data_t *settings)
{
	// Load all 5 profiles' text content
	const char *profile_names[] = {"profile1", "profile2", "profile3", "profile4", "profile5"};
	
	for (int i = 0; i < 5; i++) {
		char key[64];
		
		// Free old text
		bfree(title[i]);
		bfree(subtitle[i]);
		bfree(title_right[i]);
		bfree(subtitle_right[i]);
		
		// Load new text for this profile
		snprintf(key, sizeof(key), "%s_title", profile_names[i]);
		title[i] = bstrdup(obs_data_get_string(settings, key));
		
		snprintf(key, sizeof(key), "%s_subtitle", profile_names[i]);
		subtitle[i] = bstrdup(obs_data_get_string(settings, key));
		
		snprintf(key, sizeof(key), "%s_title_right", profile_names[i]);
		title_right[i] = bstrdup(obs_data_get_string(settings, key));
		
		snprintf(key, sizeof(key), "%s_subtitle_right", profile_names[i]);
		subtitle_right[i] = bstrdup(obs_data_get_string(settings, key));
	}
	
	// DON'T load current_profile from settings - it's managed by button clicks only
	// Loading from settings causes it to reset to Tab 1 during updates
	// current_profile is changed ONLY when clicking tab buttons
	// This prevents the "flash to Tab 1" issue when switching tabs
	
	// Load number of visible tabs
	num_visible_tabs = (int)obs_data_get_int(settings, "num_visible_tabs");
	
	// If not set (0 or invalid), default to 1 for clean interface
	if (num_visible_tabs < 1 || num_visible_tabs > 5) {
		num_visible_tabs = 1;
		// Save the corrected value
		obs_data_set_int(settings, "num_visible_tabs", num_visible_tabs);
	}
	
	// Get font from OBS font selector (returns JSON object with face, style, flags)
	obs_data_t *font_obj = obs_data_get_obj(settings, "font_face");
	const char *new_font = font_obj ? obs_data_get_string(font_obj, "face") : "Arial";
	if (!new_font || strlen(new_font) == 0) {
		new_font = "Arial";
	}
	obs_data_release(font_obj);
	
	const char *new_bg_image = obs_data_get_string(settings, "bg_image");
	
	bfree(font_face);
	font_face = bstrdup(new_font);
	
	// Load background image if path changed
	if (!bg_image_path || strcmp(bg_image_path, new_bg_image) != 0) {
		bfree(bg_image_path);
		bg_image_path = bstrdup(new_bg_image);
		
		// Clean up old image
		if (bg_image) {
			obs_enter_graphics();
			gs_image_file_free(bg_image);
			obs_leave_graphics();
			bfree(bg_image);
			bg_image = nullptr;
		}
		
		// Load new image if path is not empty
		if (bg_image_path && strlen(bg_image_path) > 0) {
			bg_image = (gs_image_file_t *)bzalloc(sizeof(gs_image_file_t));
			gs_image_file_init(bg_image, bg_image_path);
			
			obs_enter_graphics();
			gs_image_file_init_texture(bg_image);
			obs_leave_graphics();
		}
	}
	
	// Load logo image if path changed
	const char *new_logo_image = obs_data_get_string(settings, "logo_image");
	if (!logo_image_path || strcmp(logo_image_path, new_logo_image) != 0) {
		bfree(logo_image_path);
		logo_image_path = bstrdup(new_logo_image);
		
		// Clean up old logo
		if (logo_image) {
			obs_enter_graphics();
			gs_image_file_free(logo_image);
			obs_leave_graphics();
			bfree(logo_image);
			logo_image = nullptr;
		}
		
		// Load new logo if path is not empty
		if (logo_image_path && strlen(logo_image_path) > 0) {
			logo_image = (gs_image_file_t *)bzalloc(sizeof(gs_image_file_t));
			gs_image_file_init(logo_image, logo_image_path);
			
			obs_enter_graphics();
			gs_image_file_init_texture(logo_image);
			obs_leave_graphics();
		}
	}
	
	bg_color = (uint32_t)obs_data_get_int(settings, "bg_color");
	text_color = (uint32_t)obs_data_get_int(settings, "text_color");
	opacity = (int)obs_data_get_int(settings, "opacity");
	show_background = obs_data_get_bool(settings, "show_background");
	title_size = (int)obs_data_get_int(settings, "title_size");
	subtitle_size = (int)obs_data_get_int(settings, "subtitle_size");
	padding_horizontal = (int)obs_data_get_int(settings, "padding_horizontal");
	padding_vertical = (int)obs_data_get_int(settings, "padding_vertical");
	bar_height_pixels = (int)obs_data_get_int(settings, "bar_height");
	auto_scale = obs_data_get_bool(settings, "auto_scale");
	animation_style = (AnimationStyle)obs_data_get_int(settings, "animation_style");
	
	// Logo settings
	logo_size = (int)obs_data_get_int(settings, "logo_size");
	logo_opacity = (int)obs_data_get_int(settings, "logo_opacity");
	logo_padding_horizontal = (int)obs_data_get_int(settings, "logo_padding_horizontal");
	logo_padding_vertical = (int)obs_data_get_int(settings, "logo_padding_vertical");
	
	// Gradient settings
	gradient_type = (GradientType)obs_data_get_int(settings, "gradient_type");
	gradient_color2 = (uint32_t)obs_data_get_int(settings, "gradient_color2");
	
	bool new_visible = obs_data_get_bool(settings, "visible");
	auto_hide_enabled = obs_data_get_bool(settings, "auto_hide");
	display_duration = (float)obs_data_get_double(settings, "duration");
	
	// Check if force replay was requested (from eye icon or play button)
	if (force_replay && new_visible) {
		// Force a fresh start
		display_timer = 0.0f;
		animation_progress = 0.0f;
		is_visible = true;
		force_replay = false; // Clear the flag
	}
	// Normal case: transitioning from hidden to visible
	else if (new_visible && !is_visible) {
		display_timer = 0.0f;
		animation_progress = 0.0f;
		is_visible = true;
	}
	// Transitioning from visible to hidden
	else if (!new_visible && is_visible) {
		is_visible = false;
	}
	
	// Update text sources
	update_text_sources();
}

void lowerthirds_source::update_text_sources()
{
	// Use current profile's text content
	int profile = current_profile;
	if (profile < 0 || profile >= 5)
		profile = 0;
	
	if (title_text_source) {
		obs_data_t *text_settings = obs_data_create();
		obs_data_set_string(text_settings, "text", title[profile] ? title[profile] : "");
		obs_data_set_int(text_settings, "color1", text_color);
		obs_data_set_int(text_settings, "color2", text_color);
		
		// Font settings
		obs_data_t *font_obj = obs_data_create();
		obs_data_set_string(font_obj, "face", font_face);
		obs_data_set_int(font_obj, "size", title_size);
		obs_data_set_int(font_obj, "flags", OBS_FONT_BOLD);
		obs_data_set_obj(text_settings, "font", font_obj);
		obs_data_release(font_obj);
		
		obs_source_update(title_text_source, text_settings);
		obs_data_release(text_settings);
	}
	
	if (subtitle_text_source) {
		obs_data_t *text_settings = obs_data_create();
		obs_data_set_string(text_settings, "text", subtitle[profile] ? subtitle[profile] : "");
		obs_data_set_int(text_settings, "color1", text_color);
		obs_data_set_int(text_settings, "color2", text_color);
		
		// Font settings
		obs_data_t *font_obj = obs_data_create();
		obs_data_set_string(font_obj, "face", font_face);
		obs_data_set_int(font_obj, "size", subtitle_size);
		obs_data_set_int(font_obj, "flags", 0); // Normal weight
		obs_data_set_obj(text_settings, "font", font_obj);
		obs_data_release(font_obj);
		
		obs_source_update(subtitle_text_source, text_settings);
		obs_data_release(text_settings);
	}
	
	// Right side title (optional)
	if (title_right_text_source) {
		obs_data_t *text_settings = obs_data_create();
		obs_data_set_string(text_settings, "text", title_right[profile] ? title_right[profile] : "");
		obs_data_set_int(text_settings, "color1", text_color);
		obs_data_set_int(text_settings, "color2", text_color);
		
		// Font settings
		obs_data_t *font_obj = obs_data_create();
		obs_data_set_string(font_obj, "face", font_face);
		obs_data_set_int(font_obj, "size", title_size);
		obs_data_set_int(font_obj, "flags", OBS_FONT_BOLD);
		obs_data_set_obj(text_settings, "font", font_obj);
		obs_data_release(font_obj);
		
		obs_source_update(title_right_text_source, text_settings);
		obs_data_release(text_settings);
	}
	
	// Right side subtitle (optional)
	if (subtitle_right_text_source) {
		obs_data_t *text_settings = obs_data_create();
		obs_data_set_string(text_settings, "text", subtitle_right[profile] ? subtitle_right[profile] : "");
		obs_data_set_int(text_settings, "color1", text_color);
		obs_data_set_int(text_settings, "color2", text_color);
		
		// Font settings
		obs_data_t *font_obj = obs_data_create();
		obs_data_set_string(font_obj, "face", font_face);
		obs_data_set_int(font_obj, "size", subtitle_size);
		obs_data_set_int(font_obj, "flags", 0); // Normal weight
		obs_data_set_obj(text_settings, "font", font_obj);
		obs_data_release(font_obj);
		
		obs_source_update(subtitle_right_text_source, text_settings);
		obs_data_release(text_settings);
	}
}

void lowerthirds_source::tick(float seconds)
{
	// Update animation (smooth, modern timing: 1.0 second duration)
	if (is_visible && animation_progress < 1.0f) {
		animation_progress += seconds * 1.0f; // 1.0 second smooth animation
		if (animation_progress > 1.0f)
			animation_progress = 1.0f;
	} else if (!is_visible && animation_progress > 0.0f) {
		animation_progress -= seconds * 1.5f; // Slightly faster fade out
		if (animation_progress < 0.0f)
			animation_progress = 0.0f;
	}
	
	// Auto-hide timer
	if (is_visible && auto_hide_enabled) {
		display_timer += seconds;
		
		// When duration reached, automatically hide
		if (display_timer >= display_duration) {
			is_visible = false;
			display_timer = 0.0f;
			
			// Update the source settings to reflect the change
			obs_data_t *settings = obs_source_get_settings(source);
			obs_data_set_bool(settings, "visible", false);
			obs_source_update(source, settings);
			obs_data_release(settings);
		}
	}
}

void lowerthirds_source::render()
{
	if (animation_progress <= 0.0f)
		return;
	
	// Use FIXED internal width - never changes with OBS transforms
	// This ensures text and logo positions are always stable
	const uint32_t fixed_width = 1920;
	
	// Calculate scale factor
	// When auto_scale is OFF (default), text stays at exact pixel sizes you set
	// This means text won't shift/scale when you manually resize the source
	if (auto_scale) {
		obs_video_info ovi;
		obs_get_video_info(&ovi);
		// Scale based on canvas height (1080p = 1.0, 720p = 0.67, 4K = 2.0)
		scale_factor = ovi.base_height / 1080.0f;
		// Clamp between 0.5 and 2.5 for reasonable scaling
		if (scale_factor < 0.5f) scale_factor = 0.5f;
		if (scale_factor > 2.5f) scale_factor = 2.5f;
	} else {
		// Fixed 1.0 scale - text always stays at exact pixel sizes
		scale_factor = 1.0f;
	}
	
	// === MODERN SMOOTH EASING FUNCTIONS ===
	// Ease-out expo (very smooth, modern deceleration)
	auto ease_out_expo = [](float t) -> float {
		return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
	};
	
	// Ease-in-out quint (very smooth, polished feel)
	auto ease_in_out_quint = [](float t) -> float {
		return t < 0.5f
			? 16.0f * t * t * t * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;
	};
	
	// Ease-out back (subtle overshoot, not too bouncy)
	auto ease_out_back = [](float t) -> float {
		const float c1 = 1.70158f;
		const float c3 = c1 + 1.0f;
		return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
	};
	
	// Apply primary easing for background animation (smooth and modern)
	float eased_progress = ease_in_out_quint(animation_progress);
	
	// Calculate dimensions
	float bar_height = (float)bar_height_pixels * scale_factor;
	
	// === PROFESSIONAL LOGO ANIMATION ===
	// Logo appears at 15% (smooth entrance timing)
	float logo_progress = 0.0f;
	if (animation_progress > 0.15f) {
		logo_progress = (animation_progress - 0.15f) / 0.85f; // 0.15-1.0 → 0.0-1.0
		if (logo_progress > 1.0f) logo_progress = 1.0f;
	}
	
	// Apply smooth easing for modern, polished entrance
	float logo_eased = ease_out_back(logo_progress);
	
	// Logo fade + scale effect (starts at 70% size, smooth growth to 100%)
	float logo_alpha_animation = ease_out_expo(logo_progress); // Very smooth fade
	float logo_scale_animation = 0.7f + (0.3f * logo_eased); // 0.7 → 1.0 smooth with subtle overshoot
	
	// === MODERN TEXT ANIMATIONS ===
	// Title appears at 25% (earlier, more dynamic timing)
	float title_progress = 0.0f;
	if (animation_progress > 0.25f) {
		title_progress = (animation_progress - 0.25f) / 0.75f; // 0.25-1.0 → 0.0-1.0
		if (title_progress > 1.0f) title_progress = 1.0f;
	}
	
	// Subtitle appears at 35% (tighter stagger for modern flow)
	float subtitle_progress = 0.0f;
	if (animation_progress > 0.35f) {
		subtitle_progress = (animation_progress - 0.35f) / 0.65f; // 0.35-1.0 → 0.0-1.0
		if (subtitle_progress > 1.0f) subtitle_progress = 1.0f;
	}
	
	// Apply sophisticated easing for modern, dynamic feel
	// Title uses ease-out-back for subtle bounce/overshoot (attention-grabbing)
	float title_eased = ease_out_back(title_progress);
	// Subtitle uses ease-out-expo for ultra smooth elegance
	float subtitle_eased = ease_out_expo(subtitle_progress);
	
	// Text fade with smooth opacity curves
	float title_alpha = ease_out_expo(title_progress); // Smooth fade
	float subtitle_alpha = ease_out_expo(subtitle_progress); // Smooth fade
	
	// Convert background color with opacity
	float alpha = (opacity / 100.0f);
	
	// Apply animation fade for certain styles
	if (animation_style == ANIM_FADE) {
		alpha *= eased_progress; // Fade in effect
	}
	
	// === Draw Background ===
	// Apply transformation based on animation style
	gs_matrix_push();
	
	switch (animation_style) {
		case ANIM_SLIDE_LEFT:
			// Slide from left with overshoot effect
			{
				float slide_distance = -(float)fixed_width * (1.0f - eased_progress);
				gs_matrix_translate3f(slide_distance, 0.0f, 0.0f);
			}
			break;
			
		case ANIM_SLIDE_RIGHT:
			// Slide from right with overshoot effect
			{
				float slide_distance = (float)fixed_width * (1.0f - eased_progress);
				gs_matrix_translate3f(slide_distance, 0.0f, 0.0f);
			}
			break;
			
		case ANIM_SLIDE_BOTTOM:
			// Slide from bottom with bounce
			{
				float slide_distance = bar_height * (1.0f - eased_progress);
				gs_matrix_translate3f(0.0f, slide_distance, 0.0f);
			}
			break;
			
		case ANIM_FADE:
			// Pure fade in (no translation, alpha handles this)
			// Add subtle scale for depth effect
			{
				float fade_scale = 0.95f + (0.05f * eased_progress); // 95% to 100%
				float offset_x = (float)fixed_width * (1.0f - fade_scale) * 0.5f;
				float offset_y = bar_height * (1.0f - fade_scale) * 0.5f;
				gs_matrix_translate3f(offset_x, offset_y, 0.0f);
				gs_matrix_scale3f(fade_scale, fade_scale, 1.0f);
			}
			break;
			
		case ANIM_SLIDE_TOP:
			// Slide from top (smooth downward entrance)
			{
				float slide_distance = -bar_height * (1.0f - eased_progress);
				gs_matrix_translate3f(0.0f, slide_distance, 0.0f);
			}
			break;
			
		case ANIM_ZOOM:
			// Smooth zoom in from center (refined effect)
			{
				// Start from 50% size (elegant) and zoom to 100%
				float scale = 0.5f + (0.5f * eased_progress);
				float offset_x = (float)fixed_width * (1.0f - scale) * 0.5f;
				float offset_y = bar_height * (1.0f - scale) * 0.5f;
				gs_matrix_translate3f(offset_x, offset_y, 0.0f);
				gs_matrix_scale3f(scale, scale, 1.0f);
			}
			break;
			
		case ANIM_EXPAND_LEFT:
			// Expand from left edge (horizontal stretch effect)
			{
				float scale_x = eased_progress;
				gs_matrix_scale3f(scale_x, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_EXPAND_RIGHT:
			// Expand from right edge (horizontal stretch effect)
			{
				float scale_x = eased_progress;
				float offset_x = (float)fixed_width * (1.0f - scale_x);
				gs_matrix_translate3f(offset_x, 0.0f, 0.0f);
				gs_matrix_scale3f(scale_x, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_PUSH_LEFT:
			// Push from left (slide with scale combo)
			{
				float slide_distance = -(float)fixed_width * (1.0f - eased_progress);
				float scale_x = 0.8f + (0.2f * eased_progress); // 80% to 100%
				gs_matrix_translate3f(slide_distance, 0.0f, 0.0f);
				gs_matrix_scale3f(scale_x, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_PUSH_RIGHT:
			// Push from right (slide with scale combo)
			{
				float slide_distance = (float)fixed_width * (1.0f - eased_progress);
				float scale_x = 0.8f + (0.2f * eased_progress); // 80% to 100%
				gs_matrix_translate3f(slide_distance, 0.0f, 0.0f);
				gs_matrix_scale3f(scale_x, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_WIPE_LEFT:
			// Wipe from left (reveal effect)
			{
				float reveal = eased_progress;
				gs_matrix_scale3f(reveal, 1.0f, 1.0f);
				// Clip to visible area for wipe effect
			}
			break;
			
		case ANIM_WIPE_RIGHT:
			// Wipe from right (reveal effect)
			{
				float reveal = eased_progress;
				float offset_x = (float)fixed_width * (1.0f - reveal);
				gs_matrix_translate3f(offset_x, 0.0f, 0.0f);
				gs_matrix_scale3f(reveal, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_INSTANT:
			// No animation - instant appearance
			break;
	}
	
	// === Draw Background (optional - can be turned off) ===
	if (show_background) {
		if (bg_image && bg_image->texture) {
			// Draw background image
			gs_effect_t *image_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
			gs_eparam_t *image_param = gs_effect_get_param_by_name(image_effect, "image");
			
			gs_effect_set_texture(image_param, bg_image->texture);
			
			// Set opacity
			if (alpha < 1.0f) {
				gs_eparam_t *opacity_param = gs_effect_get_param_by_name(image_effect, "opacity");
				if (opacity_param)
					gs_effect_set_float(opacity_param, alpha);
			}
			
			gs_blend_state_push();
			gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
			
			while (gs_effect_loop(image_effect, "Draw")) {
				gs_draw_sprite(bg_image->texture, 0, fixed_width, (uint32_t)bar_height);
			}
			
			gs_blend_state_pop();
		} else {
		// Convert colors to vec4
		// OBS uses ABGR format: 0xAABBGGRR
		struct vec4 color1, color2;
		
		// First color (start color) - correct ABGR parsing
		color1.w = ((bg_color >> 24) & 0xFF) / 255.0f * alpha;  // Alpha
		color1.x = (bg_color & 0xFF) / 255.0f;                  // Red
		color1.y = ((bg_color >> 8) & 0xFF) / 255.0f;           // Green
		color1.z = ((bg_color >> 16) & 0xFF) / 255.0f;          // Blue
		
		// Second color (end color for gradient) - correct ABGR parsing
		color2.w = ((gradient_color2 >> 24) & 0xFF) / 255.0f * alpha;  // Alpha
		color2.x = (gradient_color2 & 0xFF) / 255.0f;                  // Red
		color2.y = ((gradient_color2 >> 8) & 0xFF) / 255.0f;           // Green
		color2.z = ((gradient_color2 >> 16) & 0xFF) / 255.0f;          // Blue
			
			// Draw gradient or solid color
			if (gradient_type != GRADIENT_NONE) {
				// Draw gradient rectangle
				draw_gradient_rect(0.0f, 0.0f, (float)fixed_width, bar_height, color1, color2, gradient_type);
			} else {
				// Draw solid color rectangle
				gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
				gs_eparam_t *param_color = gs_effect_get_param_by_name(solid, "color");
				gs_effect_set_vec4(param_color, &color1);
				
				while (gs_effect_loop(solid, "Solid")) {
					gs_render_start(true);
					gs_vertex2f(0.0f, 0.0f);              // Top-left
					gs_vertex2f((float)fixed_width, 0.0f);      // Top-right
					gs_vertex2f(0.0f, bar_height);        // Bottom-left
					gs_vertex2f((float)fixed_width, bar_height); // Bottom-right
					gs_render_stop(GS_TRISTRIP);
				}
			}
		}
	}
	
	gs_matrix_pop();
	
	// === Draw Logo (Left Side - Optional) ===
	// Logo has INDEPENDENT padding controls - does NOT affect text position
	float logo_width_with_padding = 0.0f;
	if (logo_image && logo_image->texture && logo_alpha_animation > 0.01f) {
		// Logo ALWAYS uses fixed pixel values (never scales with box size)
		float fixed_logo_size = (float)logo_size;
		float fixed_logo_pad_horizontal = (float)logo_padding_horizontal;
		float fixed_logo_pad_vertical = (float)logo_padding_vertical;
		
		// Apply animation scale to logo size (zoom effect)
		float animated_logo_size = fixed_logo_size * logo_scale_animation;
		
		// Logo position using INDEPENDENT padding controls
		// Horizontal padding applies to left side
		float logo_x = fixed_logo_pad_horizontal;
		float logo_y = fixed_logo_pad_vertical;
		
		// If logo_padding_vertical is 0, center vertically (default behavior)
		// Use bar_height_pixels (not scaled bar_height) for stable positioning
		// Adjust for animated size to keep center point stable
		if (logo_padding_vertical == 0) {
			logo_y = ((float)bar_height_pixels - animated_logo_size) / 2.0f;
		} else {
			// Adjust vertical position to compensate for scale animation
			float size_diff = (fixed_logo_size - animated_logo_size) / 2.0f;
			logo_y += size_diff;
		}
		
		// Adjust horizontal position to keep logo centered during scale
		float size_diff_x = (fixed_logo_size - animated_logo_size) / 2.0f;
		logo_x += size_diff_x;
		
		// User opacity control (independent from animation)
		float user_opacity = (logo_opacity / 100.0f);
		
		// Combine with animation fade
		float final_logo_alpha = user_opacity * logo_alpha_animation;
		
		// Only draw if there's some opacity
		if (final_logo_alpha > 0.001f) {
			// Push matrix for positioning
			gs_matrix_push();
			gs_matrix_translate3f(logo_x, logo_y, 0.0f);
			
			// Draw logo with custom alpha support
			draw_logo_with_alpha(logo_image->texture, animated_logo_size, animated_logo_size, final_logo_alpha);
			
			gs_matrix_pop();
		}
		
		// Calculate if text needs to be offset (only if logo would overlap text)
		// Use the BASE (non-animated) logo size for text positioning to avoid shifting text during animation
		float logo_right_edge = fixed_logo_pad_horizontal + fixed_logo_size + fixed_logo_pad_horizontal;
		float fixed_padding_horizontal = (float)padding_horizontal; // Use pixel value
		if (logo_right_edge > fixed_padding_horizontal) {
			// Logo extends past text position, push text right
			logo_width_with_padding = logo_right_edge - fixed_padding_horizontal;
		} else {
			// Logo doesn't overlap text position, no text offset needed
			logo_width_with_padding = 0.0f;
		}
	}
	
	// === Draw Text with Professional Animations ===
	gs_blend_state_push();
	gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
	
	// Calculate total text height for centering (always use pixel values for stability)
	float text_title_size = (float)title_size;
	float text_subtitle_size = (float)subtitle_size;
	float text_spacing = 15.0f;
	float total_text_height = text_title_size + text_spacing + text_subtitle_size;
	float center_offset = ((float)bar_height_pixels - total_text_height) / 2.0f;
	
	// Use fixed pixel padding (not scaled)
	float fixed_padding_horizontal = (float)padding_horizontal;
	
	// Modern dynamic text slide animation offsets
	// Title: More dramatic entrance with overshoot feel (40px slide-up, 25px slide-right)
	float fixed_title_offset_y = (1.0f - title_eased) * 40.0f;
	float fixed_title_offset_x = (1.0f - title_eased) * 25.0f;
	
	// Subtitle: Smooth flowing entrance (30px slide-up, 20px slide-right)
	float fixed_subtitle_offset_y = (1.0f - subtitle_eased) * 30.0f;
	float fixed_subtitle_offset_x = (1.0f - subtitle_eased) * 20.0f;
	
	// Add modern scale effect: text starts at 90% size and grows to 100%
	// Title has subtle scale for dynamic feel
	float title_scale = 0.90f + (0.10f * title_eased);
	// Subtitle has subtle scale for coordinated flow
	float subtitle_scale = 0.92f + (0.08f * subtitle_eased);
	
	// Draw title text (appears first with slide-up + slide-right + scale)
	if (title_text_source && title[current_profile] && strlen(title[current_profile]) > 0 && title_alpha > 0.01f) {
		// Center text vertically in the bar (uses pixel values for stable position)
		float title_y = center_offset + fixed_title_offset_y;
		float title_x = fixed_padding_horizontal + logo_width_with_padding + fixed_title_offset_x;
		
		gs_matrix_push();
		gs_matrix_translate3f(title_x, title_y, 0.0f);
		
		// Apply modern scale animation for dynamic entrance
		float final_scale = auto_scale ? (scale_factor * title_scale) : title_scale;
		gs_matrix_scale3f(final_scale, final_scale, 1.0f);
		
		// Apply professional fade with opacity
		if (title_alpha < 1.0f) {
			gs_effect_t *default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
			gs_eparam_t *opacity_param = gs_effect_get_param_by_name(default_effect, "opacity");
			if (opacity_param)
				gs_effect_set_float(opacity_param, title_alpha);
		}
		
		obs_source_video_render(title_text_source);
		gs_matrix_pop();
	}
	
	// Draw subtitle text (appears after title with slide-up + slide-right + scale - staggered effect)
	if (subtitle_text_source && subtitle[current_profile] && strlen(subtitle[current_profile]) > 0 && subtitle_alpha > 0.01f) {
		// Keep subtitle below title, centered in the bar
		float subtitle_y = center_offset + text_title_size + text_spacing + fixed_subtitle_offset_y;
		float subtitle_x = fixed_padding_horizontal + logo_width_with_padding + fixed_subtitle_offset_x;
		
		gs_matrix_push();
		gs_matrix_translate3f(subtitle_x, subtitle_y, 0.0f);
		
		// Apply modern scale animation for smooth entrance
		float final_scale = auto_scale ? (scale_factor * subtitle_scale) : subtitle_scale;
		gs_matrix_scale3f(final_scale, final_scale, 1.0f);
		
		// Apply professional fade with opacity
		if (subtitle_alpha < 1.0f) {
			gs_effect_t *default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
			gs_eparam_t *opacity_param = gs_effect_get_param_by_name(default_effect, "opacity");
			if (opacity_param)
				gs_effect_set_float(opacity_param, subtitle_alpha);
		}
		
		obs_source_video_render(subtitle_text_source);
		gs_matrix_pop();
	}
	
	// === Draw RIGHT SIDE Text (Optional) ===
	// Draw right title text (with modern scale animation)
	if (title_right_text_source && title_right[current_profile] && strlen(title_right[current_profile]) > 0 && title_alpha > 0.01f) {
		// Get text width for right alignment (pixel values for stable position)
		uint32_t title_right_width = obs_source_get_width(title_right_text_source);
		float title_right_x = (float)fixed_width - (float)title_right_width - fixed_padding_horizontal - fixed_title_offset_x;
		// Keep right text centered vertically too (use fixed offset)
		float title_right_y = center_offset + fixed_title_offset_y;
		
		gs_matrix_push();
		gs_matrix_translate3f(title_right_x, title_right_y, 0.0f);
		
		// Apply modern scale animation for dynamic entrance
		float final_scale = auto_scale ? (scale_factor * title_scale) : title_scale;
		gs_matrix_scale3f(final_scale, final_scale, 1.0f);
		
		// Apply professional fade with opacity
		if (title_alpha < 1.0f) {
			gs_effect_t *default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
			gs_eparam_t *opacity_param = gs_effect_get_param_by_name(default_effect, "opacity");
			if (opacity_param)
				gs_effect_set_float(opacity_param, title_alpha);
		}
		
		obs_source_video_render(title_right_text_source);
		gs_matrix_pop();
	}
	
	// Draw right subtitle text (with modern scale animation)
	if (subtitle_right_text_source && subtitle_right[current_profile] && strlen(subtitle_right[current_profile]) > 0 && subtitle_alpha > 0.01f) {
		// Get text width for right alignment (pixel values for stable position)
		uint32_t subtitle_right_width = obs_source_get_width(subtitle_right_text_source);
		float subtitle_right_x = (float)fixed_width - (float)subtitle_right_width - fixed_padding_horizontal - fixed_subtitle_offset_x;
		// Keep right subtitle centered vertically (use fixed values)
		float subtitle_right_y = center_offset + text_title_size + text_spacing + fixed_subtitle_offset_y;
		
		gs_matrix_push();
		gs_matrix_translate3f(subtitle_right_x, subtitle_right_y, 0.0f);
		
		// Apply modern scale animation for smooth entrance
		float final_scale = auto_scale ? (scale_factor * subtitle_scale) : subtitle_scale;
		gs_matrix_scale3f(final_scale, final_scale, 1.0f);
		
		// Apply professional fade with opacity
		if (subtitle_alpha < 1.0f) {
			gs_effect_t *default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
			gs_eparam_t *opacity_param = gs_effect_get_param_by_name(default_effect, "opacity");
			if (opacity_param)
				gs_effect_set_float(opacity_param, subtitle_alpha);
		}
		
		obs_source_video_render(subtitle_right_text_source);
		gs_matrix_pop();
	}
	
	gs_blend_state_pop();
}

uint32_t lowerthirds_source::get_width()
{
	return 1920;
}

uint32_t lowerthirds_source::get_height()
{
	return bar_height_pixels;
}

void lowerthirds_source::draw_gradient_rect(float x, float y, float width, float height, 
	struct vec4 color1, struct vec4 color2, GradientType type)
{
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *param_color = gs_effect_get_param_by_name(solid, "color");
	
	// For gradients, we need to draw with per-vertex colors
	// OBS doesn't have a built-in gradient effect, so we'll use multiple strips
	
	const int gradient_steps = 32; // Smooth gradient with 32 color steps
	
	switch (type) {
		case GRADIENT_HORIZONTAL:
			// Left to right gradient
			for (int i = 0; i < gradient_steps; i++) {
				float t1 = (float)i / (float)gradient_steps;
				float t2 = (float)(i + 1) / (float)gradient_steps;
				
				// Interpolate colors
				struct vec4 c1, c2;
				c1.x = color1.x + (color2.x - color1.x) * t1;
				c1.y = color1.y + (color2.y - color1.y) * t1;
				c1.z = color1.z + (color2.z - color1.z) * t1;
				c1.w = color1.w + (color2.w - color1.w) * t1;
				
				c2.x = color1.x + (color2.x - color1.x) * t2;
				c2.y = color1.y + (color2.y - color1.y) * t2;
				c2.z = color1.z + (color2.z - color1.z) * t2;
				c2.w = color1.w + (color2.w - color1.w) * t2;
				
				float x1 = x + width * t1;
				float x2 = x + width * t2;
				
				// Draw strip with first color
				gs_effect_set_vec4(param_color, &c1);
				while (gs_effect_loop(solid, "Solid")) {
					gs_render_start(true);
					gs_vertex2f(x1, y);
					gs_vertex2f(x2, y);
					gs_vertex2f(x1, y + height);
					gs_vertex2f(x2, y + height);
					gs_render_stop(GS_TRISTRIP);
				}
			}
			break;
			
		case GRADIENT_VERTICAL:
			// Top to bottom gradient
			for (int i = 0; i < gradient_steps; i++) {
				float t1 = (float)i / (float)gradient_steps;
				float t2 = (float)(i + 1) / (float)gradient_steps;
				
				// Interpolate colors
				struct vec4 c1, c2;
				c1.x = color1.x + (color2.x - color1.x) * t1;
				c1.y = color1.y + (color2.y - color1.y) * t1;
				c1.z = color1.z + (color2.z - color1.z) * t1;
				c1.w = color1.w + (color2.w - color1.w) * t1;
				
				c2.x = color1.x + (color2.x - color1.x) * t2;
				c2.y = color1.y + (color2.y - color1.y) * t2;
				c2.z = color1.z + (color2.z - color1.z) * t2;
				c2.w = color1.w + (color2.w - color1.w) * t2;
				
				float y1 = y + height * t1;
				float y2 = y + height * t2;
				
				// Draw strip with color
				gs_effect_set_vec4(param_color, &c1);
				while (gs_effect_loop(solid, "Solid")) {
					gs_render_start(true);
					gs_vertex2f(x, y1);
					gs_vertex2f(x + width, y1);
					gs_vertex2f(x, y2);
					gs_vertex2f(x + width, y2);
					gs_render_stop(GS_TRISTRIP);
				}
			}
			break;
			
		case GRADIENT_DIAGONAL_TL_BR:
			// Top-left to bottom-right diagonal
			for (int i = 0; i < gradient_steps; i++) {
				float t1 = (float)i / (float)gradient_steps;
				float t2 = (float)(i + 1) / (float)gradient_steps;
				
				// Interpolate colors
				struct vec4 c1;
				c1.x = color1.x + (color2.x - color1.x) * t1;
				c1.y = color1.y + (color2.y - color1.y) * t1;
				c1.z = color1.z + (color2.z - color1.z) * t1;
				c1.w = color1.w + (color2.w - color1.w) * t1;
				
				gs_effect_set_vec4(param_color, &c1);
				while (gs_effect_loop(solid, "Solid")) {
					// Simplified diagonal rendering using horizontal strips
					float y1 = y + height * t1;
					float y2 = y + height * t2;
					gs_render_start(true);
					gs_vertex2f(x, y1);
					gs_vertex2f(x + width, y1);
					gs_vertex2f(x, y2);
					gs_vertex2f(x + width, y2);
					gs_render_stop(GS_TRISTRIP);
				}
			}
			break;
			
		case GRADIENT_DIAGONAL_BL_TR:
			// Bottom-left to top-right diagonal
			for (int i = 0; i < gradient_steps; i++) {
				float t1 = (float)i / (float)gradient_steps;
				float t2 = (float)(i + 1) / (float)gradient_steps;
				
				// Interpolate colors (reversed)
				struct vec4 c1;
				c1.x = color1.x + (color2.x - color1.x) * t1;
				c1.y = color1.y + (color2.y - color1.y) * t1;
				c1.z = color1.z + (color2.z - color1.z) * t1;
				c1.w = color1.w + (color2.w - color1.w) * t1;
				
				gs_effect_set_vec4(param_color, &c1);
				while (gs_effect_loop(solid, "Solid")) {
					// Reversed vertical strips
					float y1 = y + height * (1.0f - t2);
					float y2 = y + height * (1.0f - t1);
					gs_render_start(true);
					gs_vertex2f(x, y1);
					gs_vertex2f(x + width, y1);
					gs_vertex2f(x, y2);
					gs_vertex2f(x + width, y2);
					gs_render_stop(GS_TRISTRIP);
				}
			}
			break;
			
		case GRADIENT_NONE:
		default:
			// Solid color
			gs_effect_t *solid_effect = obs_get_base_effect(OBS_EFFECT_SOLID);
			gs_eparam_t *color_param = gs_effect_get_param_by_name(solid_effect, "color");
			gs_effect_set_vec4(color_param, &color1);
			
			while (gs_effect_loop(solid_effect, "Solid")) {
				gs_render_start(true);
				gs_vertex2f(x, y);              // Top-left
				gs_vertex2f(x + width, y);      // Top-right
				gs_vertex2f(x, y + height);        // Bottom-left
				gs_vertex2f(x + width, y + height); // Bottom-right
				gs_render_stop(GS_TRISTRIP);
			}
			break;
	}
	}

// Helper function to draw logo/image with custom alpha/opacity
void lowerthirds_source::draw_logo_with_alpha(gs_texture_t *texture, float width, float height, float alpha)
{
	if (!texture || alpha < 0.001f)
		return;
	
	// Use default effect with texture
	gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	gs_eparam_t *image_param = gs_effect_get_param_by_name(effect, "image");
	gs_effect_set_texture(image_param, texture);
	
	// Enable color write for alpha modulation
	gs_enable_color(true, true, true, true);
	gs_enable_blending(true);
	gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
	
	// Calculate color with alpha (RGBA all set to create alpha effect)
	uint32_t color_with_alpha = ((uint32_t)(alpha * 255.0f) << 24) | 0x00FFFFFF;
	
	while (gs_effect_loop(effect, "Draw")) {
		// Manually render textured quad with UV coordinates
		// This allows us to control alpha through vertex colors
		gs_render_start(true);
		
		// Vertex 0: Top-left
		gs_vertex2f(0.0f, 0.0f);
		gs_texcoord(0.0f, 0.0f, 0);
		gs_color(color_with_alpha);
		
		// Vertex 1: Top-right
		gs_vertex2f(width, 0.0f);
		gs_texcoord(1.0f, 0.0f, 0);
		gs_color(color_with_alpha);
		
		// Vertex 2: Bottom-left
		gs_vertex2f(0.0f, height);
		gs_texcoord(0.0f, 1.0f, 0);
		gs_color(color_with_alpha);
		
		// Vertex 3: Bottom-right
		gs_vertex2f(width, height);
		gs_texcoord(1.0f, 1.0f, 0);
		gs_color(color_with_alpha);
		
		gs_render_stop(GS_TRISTRIP);
	}
}

void register_lowerthirds_source()
{
	obs_source_info info = {};
	info.id = "lowerthirds_plus_source";
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	
	info.get_name = lowerthirds_get_name;
	info.create = lowerthirds_create;
	info.destroy = lowerthirds_destroy;
	info.update = lowerthirds_update;
	info.get_defaults = lowerthirds_get_defaults;
	info.get_properties = lowerthirds_get_properties;
	info.video_tick = lowerthirds_video_tick;
	info.video_render = lowerthirds_video_render;
	info.get_width = lowerthirds_get_width;
	info.get_height = lowerthirds_get_height;
	
	// Add show/hide callbacks for eye icon "Play" functionality
	info.show = lowerthirds_show;
	info.hide = lowerthirds_hide;
	
	obs_register_source(&info);
}
