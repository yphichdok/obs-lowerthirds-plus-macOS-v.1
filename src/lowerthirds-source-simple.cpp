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
	obs_data_set_default_int(settings, "logo_animation_style", ANIM_SLIDE_LEFT);
	obs_data_set_default_int(settings, "text_animation_style", ANIM_SLIDE_LEFT);
	
	// Logo defaults
	obs_data_set_default_string(settings, "logo_image", "");
	obs_data_set_default_int(settings, "logo_size", 140);  // Larger logo to match bigger box
	obs_data_set_default_int(settings, "logo_opacity", 100);
	obs_data_set_default_int(settings, "logo_padding_horizontal", 20);  // More padding
	obs_data_set_default_int(settings, "logo_padding_vertical", 0);
	
	// Gradient defaults
	obs_data_set_default_int(settings, "gradient_type", GRADIENT_NONE);
	obs_data_set_default_int(settings, "gradient_color2", 0xFFD27619); // Darker blue for gradient (ABGR format)
	
	// Text shadow defaults
	obs_data_set_default_bool(settings, "text_shadow_enabled", false);
	obs_data_set_default_int(settings, "text_shadow_color", 0xFF000000); // Black (ABGR format)
	obs_data_set_default_int(settings, "text_shadow_opacity", 75);
	obs_data_set_default_int(settings, "text_shadow_offset_x", 3);
	obs_data_set_default_int(settings, "text_shadow_offset_y", 3);
	
	// Logo shadow defaults
	obs_data_set_default_bool(settings, "logo_shadow_enabled", false);
	obs_data_set_default_int(settings, "logo_shadow_color", 0xFF000000); // Black (ABGR format)
	obs_data_set_default_int(settings, "logo_shadow_opacity", 75);
	obs_data_set_default_int(settings, "logo_shadow_offset_x", 3);
	obs_data_set_default_int(settings, "logo_shadow_offset_y", 3);
	
	// Text highlight defaults
	obs_data_set_default_bool(settings, "text_highlight_enabled", false);
	obs_data_set_default_int(settings, "text_highlight_color", 0xFF000000); // Black (ABGR format)
	obs_data_set_default_int(settings, "text_highlight_opacity", 80);
	obs_data_set_default_int(settings, "text_highlight_corner_radius", 8);
	obs_data_set_default_int(settings, "text_highlight_padding_horizontal", 20);
	obs_data_set_default_int(settings, "text_highlight_padding_vertical", 10);
	
	// Background art effect defaults
	obs_data_set_default_int(settings, "art_effect", ART_PARTICLES); // Particles by default
	obs_data_set_default_int(settings, "art_color", 0xFFFFFFFF); // White (ABGR format) - glowing effect
	obs_data_set_default_int(settings, "art_opacity", 60); // Clearly visible
	obs_data_set_default_double(settings, "art_intensity", 1.0); // Normal intensity
	obs_data_set_default_double(settings, "art_speed", 30.0); // Moderate movement
	obs_data_set_default_bool(settings, "art_animate", true); // Animated by default
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
	
	// When preview is enabled, force the lower third to show and reset animation
	bool preview_enabled = obs_data_get_bool(settings, "preview_mode");
	obs_data_set_bool(settings, "visible", preview_enabled);
	
	// If enabling preview, trigger a fresh animation
	if (preview_enabled) {
		// This will be handled by the update() function to show fresh content
		// Preview mode keeps it visible indefinitely for configuration
	}
	
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
	
	// === TAB PROFILES (Professional Organization) ===
	
	// TAB 1 GROUP
	obs_properties_t *tab1_group = obs_properties_create();
	obs_properties_add_text(tab1_group, "profile1_title", "Left Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab1_group, "profile1_subtitle", "Left Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab1_group, "profile1_title_right", "Right Title (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab1_group, "profile1_subtitle_right", "Right Subtitle (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_group(props, "tab1_content", "📝 Tab 1 Content", 
		OBS_GROUP_NORMAL, tab1_group);
	
	// TAB 2 GROUP
	obs_properties_t *tab2_group = obs_properties_create();
	obs_properties_add_text(tab2_group, "profile2_title", "Left Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab2_group, "profile2_subtitle", "Left Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab2_group, "profile2_title_right", "Right Title (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab2_group, "profile2_subtitle_right", "Right Subtitle (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_group(props, "tab2_content", "📝 Tab 2 Content", 
		OBS_GROUP_NORMAL, tab2_group);
	
	// TAB 3 GROUP
	obs_properties_t *tab3_group = obs_properties_create();
	obs_properties_add_text(tab3_group, "profile3_title", "Left Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab3_group, "profile3_subtitle", "Left Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab3_group, "profile3_title_right", "Right Title (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab3_group, "profile3_subtitle_right", "Right Subtitle (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_group(props, "tab3_content", "📝 Tab 3 Content", 
		OBS_GROUP_NORMAL, tab3_group);
	
	// TAB 4 GROUP
	obs_properties_t *tab4_group = obs_properties_create();
	obs_properties_add_text(tab4_group, "profile4_title", "Left Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab4_group, "profile4_subtitle", "Left Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab4_group, "profile4_title_right", "Right Title (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab4_group, "profile4_subtitle_right", "Right Subtitle (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_group(props, "tab4_content", "📝 Tab 4 Content", 
		OBS_GROUP_NORMAL, tab4_group);
	
	// TAB 5 GROUP
	obs_properties_t *tab5_group = obs_properties_create();
	obs_properties_add_text(tab5_group, "profile5_title", "Left Title", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab5_group, "profile5_subtitle", "Left Subtitle", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab5_group, "profile5_title_right", "Right Title (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_text(tab5_group, "profile5_subtitle_right", "Right Subtitle (Optional)", OBS_TEXT_DEFAULT);
	obs_properties_add_group(props, "tab5_content", "📝 Tab 5 Content", 
		OBS_GROUP_NORMAL, tab5_group);
	
	// === ADVANCED SETTINGS (COLLAPSIBLE) ===
	obs_properties_t *advanced_group = obs_properties_create();
	
	// ═══════════════════════════════════════════════════════════════════════════
	// CATEGORY 1: ANIMATION CONTROLS
	// ═══════════════════════════════════════════════════════════════════════════
	obs_properties_t *animation_group = obs_properties_create();
	
	// Playback settings
	obs_properties_add_bool(animation_group, "auto_hide", "Auto-Hide After Duration");
	obs_properties_add_float_slider(animation_group, "duration", "Duration (seconds)", 1.0, 30.0, 0.5);
	
	// Background animation
	obs_property_t *anim_list = obs_properties_add_list(animation_group, "animation_style", 
		"Background Animation", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
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
	obs_property_list_add_int(anim_list, "↻ Spin", ANIM_SPIN);
	obs_property_list_add_int(anim_list, "⇢ Scroll", ANIM_SCROLL);
	obs_property_list_add_int(anim_list, "⟲ Roll", ANIM_ROLL);
	obs_property_list_add_int(anim_list, "⚡ Instant", ANIM_INSTANT);
	
	// Logo animation
	obs_property_t *logo_anim_list = obs_properties_add_list(animation_group, "logo_animation_style", 
		"Logo Animation", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(logo_anim_list, "↖ Slide from Left", ANIM_SLIDE_LEFT);
	obs_property_list_add_int(logo_anim_list, "↗ Slide from Right", ANIM_SLIDE_RIGHT);
	obs_property_list_add_int(logo_anim_list, "↑ Slide from Bottom", ANIM_SLIDE_BOTTOM);
	obs_property_list_add_int(logo_anim_list, "↓ Slide from Top", ANIM_SLIDE_TOP);
	obs_property_list_add_int(logo_anim_list, "⊙ Fade In", ANIM_FADE);
	obs_property_list_add_int(logo_anim_list, "⊕ Zoom In", ANIM_ZOOM);
	obs_property_list_add_int(logo_anim_list, "⇤ Expand from Left", ANIM_EXPAND_LEFT);
	obs_property_list_add_int(logo_anim_list, "⇥ Expand from Right", ANIM_EXPAND_RIGHT);
	obs_property_list_add_int(logo_anim_list, "⟵ Push from Left", ANIM_PUSH_LEFT);
	obs_property_list_add_int(logo_anim_list, "⟶ Push from Right", ANIM_PUSH_RIGHT);
	obs_property_list_add_int(logo_anim_list, "⇐ Wipe from Left", ANIM_WIPE_LEFT);
	obs_property_list_add_int(logo_anim_list, "⇒ Wipe from Right", ANIM_WIPE_RIGHT);
	obs_property_list_add_int(logo_anim_list, "↻ Spin", ANIM_SPIN);
	obs_property_list_add_int(logo_anim_list, "⇢ Scroll", ANIM_SCROLL);
	obs_property_list_add_int(logo_anim_list, "⟲ Roll", ANIM_ROLL);
	obs_property_list_add_int(logo_anim_list, "⚡ Instant", ANIM_INSTANT);
	
	// Text animation
	obs_property_t *text_anim_list = obs_properties_add_list(animation_group, "text_animation_style", 
		"Text Animation", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(text_anim_list, "↖ Slide from Left", ANIM_SLIDE_LEFT);
	obs_property_list_add_int(text_anim_list, "↗ Slide from Right", ANIM_SLIDE_RIGHT);
	obs_property_list_add_int(text_anim_list, "↑ Slide from Bottom", ANIM_SLIDE_BOTTOM);
	obs_property_list_add_int(text_anim_list, "↓ Slide from Top", ANIM_SLIDE_TOP);
	obs_property_list_add_int(text_anim_list, "⊙ Fade In", ANIM_FADE);
	obs_property_list_add_int(text_anim_list, "⊕ Zoom In", ANIM_ZOOM);
	obs_property_list_add_int(text_anim_list, "⇤ Expand from Left", ANIM_EXPAND_LEFT);
	obs_property_list_add_int(text_anim_list, "⇥ Expand from Right", ANIM_EXPAND_RIGHT);
	obs_property_list_add_int(text_anim_list, "⟵ Push from Left", ANIM_PUSH_LEFT);
	obs_property_list_add_int(text_anim_list, "⟶ Push from Right", ANIM_PUSH_RIGHT);
	obs_property_list_add_int(text_anim_list, "⇐ Wipe from Left", ANIM_WIPE_LEFT);
	obs_property_list_add_int(text_anim_list, "⇒ Wipe from Right", ANIM_WIPE_RIGHT);
	obs_property_list_add_int(text_anim_list, "↻ Spin", ANIM_SPIN);
	obs_property_list_add_int(text_anim_list, "⇢ Scroll", ANIM_SCROLL);
	obs_property_list_add_int(text_anim_list, "⟲ Roll", ANIM_ROLL);
	obs_property_list_add_int(text_anim_list, "⚡ Instant", ANIM_INSTANT);
	
	obs_properties_add_group(advanced_group, "animation_controls", "🎬 Animation Controls", 
		OBS_GROUP_NORMAL, animation_group);
	
	// ═══════════════════════════════════════════════════════════════════════════
	// CATEGORY 2: TEXT STYLING
	// ═══════════════════════════════════════════════════════════════════════════
	obs_properties_t *text_style_group = obs_properties_create();
	
	obs_properties_add_font(text_style_group, "font_face", "Font");
	obs_properties_add_bool(text_style_group, "bold", "Bold");
	obs_properties_add_int_slider(text_style_group, "title_size", "Title Size (px)", 20, 120, 2);
	obs_properties_add_int_slider(text_style_group, "subtitle_size", "Subtitle Size (px)", 16, 100, 2);
	obs_properties_add_color(text_style_group, "text_color", "Text Color");
	
	obs_properties_add_group(advanced_group, "text_styling", "✏️ Text Styling", 
		OBS_GROUP_NORMAL, text_style_group);
	
	// ═══════════════════════════════════════════════════════════════════════════
	// CATEGORY 3: TEXT EFFECTS
	// ═══════════════════════════════════════════════════════════════════════════
	obs_properties_t *text_effects_group = obs_properties_create();
	
	// Shadow
	obs_properties_add_bool(text_effects_group, "text_shadow_enabled", "🌑 Enable Text Shadow");
	obs_properties_add_color(text_effects_group, "text_shadow_color", "   Shadow Color");
	obs_properties_add_int_slider(text_effects_group, "text_shadow_opacity", "   Shadow Opacity (%)", 0, 100, 1);
	obs_properties_add_int_slider(text_effects_group, "text_shadow_offset_x", "   Shadow Offset X (px)", -20, 20, 1);
	obs_properties_add_int_slider(text_effects_group, "text_shadow_offset_y", "   Shadow Offset Y (px)", -20, 20, 1);
	
	// Highlight/Background
	obs_properties_add_bool(text_effects_group, "text_highlight_enabled", "🎨 Enable Text Highlight (Background Box)");
	obs_properties_add_color(text_effects_group, "text_highlight_color", "   Highlight Color");
	obs_properties_add_int_slider(text_effects_group, "text_highlight_opacity", "   Highlight Opacity (%)", 0, 100, 1);
	obs_properties_add_int_slider(text_effects_group, "text_highlight_corner_radius", "   Corner Radius (px)", 0, 50, 1);
	obs_properties_add_int_slider(text_effects_group, "text_highlight_padding_horizontal", "   Padding Horizontal (px)", 0, 100, 2);
	obs_properties_add_int_slider(text_effects_group, "text_highlight_padding_vertical", "   Padding Vertical (px)", 0, 50, 2);
	
	obs_properties_add_group(advanced_group, "text_effects", "✨ Text Effects", 
		OBS_GROUP_NORMAL, text_effects_group);
	
	// ═══════════════════════════════════════════════════════════════════════════
	// CATEGORY 4: LOGO SETTINGS
	// ═══════════════════════════════════════════════════════════════════════════
	obs_properties_t *logo_group = obs_properties_create();
	
	obs_properties_add_path(logo_group, "logo_image", "Logo Image (Optional)", 
		OBS_PATH_FILE, "Image Files (*.png *.jpg *.jpeg *.bmp *.svg);;All Files (*.*)", NULL);
	obs_properties_add_int_slider(logo_group, "logo_size", "Logo Size (px)", 30, 200, 5);
	obs_properties_add_int_slider(logo_group, "logo_opacity", "Logo Opacity (%)", 0, 100, 1);
	obs_properties_add_int_slider(logo_group, "logo_padding_horizontal", "Logo Padding (Left/Right)", 0, 200, 5);
	obs_properties_add_int_slider(logo_group, "logo_padding_vertical", "Logo Padding (Top/Bottom)", 0, 100, 5);
	
	// Logo shadow
	obs_properties_add_bool(logo_group, "logo_shadow_enabled", "🌑 Enable Logo Shadow");
	obs_properties_add_color(logo_group, "logo_shadow_color", "   Shadow Color");
	obs_properties_add_int_slider(logo_group, "logo_shadow_opacity", "   Shadow Opacity (%)", 0, 100, 1);
	obs_properties_add_int_slider(logo_group, "logo_shadow_offset_x", "   Shadow Offset X (px)", -20, 20, 1);
	obs_properties_add_int_slider(logo_group, "logo_shadow_offset_y", "   Shadow Offset Y (px)", -20, 20, 1);
	
	obs_properties_add_group(advanced_group, "logo_settings", "🖼️ Logo Settings", 
		OBS_GROUP_NORMAL, logo_group);
	
	// ═══════════════════════════════════════════════════════════════════════════
	// CATEGORY 5: BACKGROUND STYLING
	// ═══════════════════════════════════════════════════════════════════════════
	obs_properties_t *background_group = obs_properties_create();
	
	obs_properties_add_bool(background_group, "show_background", "☑️ Show Background");
	obs_properties_add_color(background_group, "bg_color", "Background Color");
	obs_properties_add_int_slider(background_group, "opacity", "Opacity (%)", 0, 100, 1);
	
	obs_property_t *gradient_list = obs_properties_add_list(background_group, "gradient_type", 
		"Gradient Style", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(gradient_list, "None", GRADIENT_NONE);
	obs_property_list_add_int(gradient_list, "Horizontal →", GRADIENT_HORIZONTAL);
	obs_property_list_add_int(gradient_list, "Vertical ↓", GRADIENT_VERTICAL);
	obs_property_list_add_int(gradient_list, "Diagonal ↘", GRADIENT_DIAGONAL_TL_BR);
	obs_property_list_add_int(gradient_list, "Diagonal ↗", GRADIENT_DIAGONAL_BL_TR);
	
	obs_properties_add_color(background_group, "gradient_color2", "Gradient End Color");
	obs_properties_add_path(background_group, "bg_image", "Background Image (Optional)", 
		OBS_PATH_FILE, "Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*.*)", NULL);
	
	// Background art effects
	obs_property_t *art_list = obs_properties_add_list(background_group, "art_effect", 
		"✨ Art Effect", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(art_list, "None", ART_NONE);
	obs_property_list_add_int(art_list, "✦ Particles (Floating)", ART_PARTICLES);
	obs_property_list_add_int(art_list, "☀ Light Rays", ART_LIGHT_RAYS);
	obs_property_list_add_int(art_list, "◉ Bokeh Blur", ART_BOKEH);
	obs_property_list_add_int(art_list, "✨ Sparkles", ART_SPARKLES);
	obs_property_list_add_int(art_list, "⊙ Glow Orbs", ART_GLOW_ORBS);
	obs_property_list_add_int(art_list, "━ Light Streaks", ART_LIGHT_STREAKS);
	obs_property_list_add_int(art_list, "◈ Shimmer", ART_SHIMMER);
	obs_property_list_add_int(art_list, "⚡ Energy Flow", ART_ENERGY_FLOW);
	
	obs_properties_add_color(background_group, "art_color", "   Effect Color");
	obs_properties_add_int_slider(background_group, "art_opacity", "   Effect Opacity (%)", 0, 100, 1);
	obs_properties_add_float_slider(background_group, "art_intensity", "   Effect Intensity", 0.5, 2.0, 0.1);
	obs_properties_add_bool(background_group, "art_animate", "   🎬 Animate Effect (Live)");
	obs_properties_add_float_slider(background_group, "art_speed", "   Animation Speed", 5.0, 100.0, 5.0);
	
	obs_properties_add_group(advanced_group, "background_styling", "🎨 Background Styling", 
		OBS_GROUP_NORMAL, background_group);
	
	// ═══════════════════════════════════════════════════════════════════════════
	// CATEGORY 6: LAYOUT & POSITIONING
	// ═══════════════════════════════════════════════════════════════════════════
	obs_properties_t *layout_group = obs_properties_create();
	
	obs_properties_add_int_slider(layout_group, "bar_height", "Bar Height (px)", 80, 300, 10);
	obs_properties_add_int_slider(layout_group, "padding_horizontal", "Text Padding (Left/Right)", 0, 300, 5);
	obs_properties_add_int_slider(layout_group, "padding_vertical", "Text Padding (Top/Bottom)", 0, 150, 5);
	obs_properties_add_bool(layout_group, "auto_scale", "Auto-Scale (OFF = Text Stays Original Size)");
	
	obs_properties_add_group(advanced_group, "layout_settings", "📐 Layout & Positioning", 
		OBS_GROUP_NORMAL, layout_group);
	
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
	, text_shadow_enabled(false)
	, text_shadow_color(0xFF000000)  // Black
	, text_shadow_opacity(75)
	, text_shadow_offset_x(3)
	, text_shadow_offset_y(3)
	, logo_shadow_enabled(false)
	, logo_shadow_color(0xFF000000)  // Black
	, logo_shadow_opacity(75)
	, logo_shadow_offset_x(3)
	, logo_shadow_offset_y(3)
	, text_highlight_enabled(false)
	, text_highlight_color(0xFF000000)  // Black
	, text_highlight_opacity(80)
	, text_highlight_corner_radius(8)
	, text_highlight_padding_horizontal(20)
	, text_highlight_padding_vertical(10)
	, gradient_type(GRADIENT_NONE)
	, gradient_color2(0xFFD27619)
	, art_effect(ART_PARTICLES)
	, art_color(0xFFFFFFFF)  // White - glowing effect
	, art_opacity(60)
	, art_intensity(1.0f)
	, art_speed(30.0f)
	, art_animate(true)
	, art_animation_offset(0.0f)
	, auto_scale(false)  // OFF by default - keeps consistent pixel sizes
	, scale_factor(1.0f)
	, is_visible(false)
	, animation_progress(0.0f)
	, animation_style(ANIM_SLIDE_LEFT)
	, logo_animation_style(ANIM_SLIDE_LEFT)
	, text_animation_style(ANIM_SLIDE_LEFT)
	, auto_hide_enabled(true)
	, display_duration(5.0f)
	, display_timer(0.0f)
	, force_replay(false)
	, preview_mode(false)
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
	logo_animation_style = (AnimationStyle)obs_data_get_int(settings, "logo_animation_style");
	text_animation_style = (AnimationStyle)obs_data_get_int(settings, "text_animation_style");
	
	// Logo settings
	logo_size = (int)obs_data_get_int(settings, "logo_size");
	logo_opacity = (int)obs_data_get_int(settings, "logo_opacity");
	logo_padding_horizontal = (int)obs_data_get_int(settings, "logo_padding_horizontal");
	logo_padding_vertical = (int)obs_data_get_int(settings, "logo_padding_vertical");
	
	// Gradient settings
	gradient_type = (GradientType)obs_data_get_int(settings, "gradient_type");
	gradient_color2 = (uint32_t)obs_data_get_int(settings, "gradient_color2");
	
	// Background art effect settings
	art_effect = (BackgroundArtEffect)obs_data_get_int(settings, "art_effect");
	art_color = (uint32_t)obs_data_get_int(settings, "art_color");
	art_opacity = (int)obs_data_get_int(settings, "art_opacity");
	art_intensity = (float)obs_data_get_double(settings, "art_intensity");
	art_speed = (float)obs_data_get_double(settings, "art_speed");
	art_animate = obs_data_get_bool(settings, "art_animate");
	
	// Text shadow settings
	text_shadow_enabled = obs_data_get_bool(settings, "text_shadow_enabled");
	text_shadow_color = (uint32_t)obs_data_get_int(settings, "text_shadow_color");
	text_shadow_opacity = (int)obs_data_get_int(settings, "text_shadow_opacity");
	text_shadow_offset_x = (int)obs_data_get_int(settings, "text_shadow_offset_x");
	text_shadow_offset_y = (int)obs_data_get_int(settings, "text_shadow_offset_y");
	
	// Logo shadow settings
	logo_shadow_enabled = obs_data_get_bool(settings, "logo_shadow_enabled");
	logo_shadow_color = (uint32_t)obs_data_get_int(settings, "logo_shadow_color");
	logo_shadow_opacity = (int)obs_data_get_int(settings, "logo_shadow_opacity");
	logo_shadow_offset_x = (int)obs_data_get_int(settings, "logo_shadow_offset_x");
	logo_shadow_offset_y = (int)obs_data_get_int(settings, "logo_shadow_offset_y");
	
	// Text highlight settings
	text_highlight_enabled = obs_data_get_bool(settings, "text_highlight_enabled");
	text_highlight_color = (uint32_t)obs_data_get_int(settings, "text_highlight_color");
	text_highlight_opacity = (int)obs_data_get_int(settings, "text_highlight_opacity");
	text_highlight_corner_radius = (int)obs_data_get_int(settings, "text_highlight_corner_radius");
	text_highlight_padding_horizontal = (int)obs_data_get_int(settings, "text_highlight_padding_horizontal");
	text_highlight_padding_vertical = (int)obs_data_get_int(settings, "text_highlight_padding_vertical");
	
	// Load preview mode
	preview_mode = obs_data_get_bool(settings, "preview_mode");
	
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
	// Update animation (enhanced modern timing: 1.4 second duration for smooth, professional feel)
	if (is_visible && animation_progress < 1.0f) {
		animation_progress += seconds * 0.714f; // 1.4 second smooth animation (1/1.4 = 0.714)
		if (animation_progress > 1.0f)
			animation_progress = 1.0f;
	} else if (!is_visible && animation_progress > 0.0f) {
		animation_progress -= seconds * 1.25f; // Faster fade out (0.8s)
		if (animation_progress < 0.0f)
			animation_progress = 0.0f;
	}
	
	// Auto-hide timer (but NOT in preview mode - preview stays visible for configuration)
	if (is_visible && auto_hide_enabled && !preview_mode) {
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
	
	// Reset timer when in preview mode to ensure smooth restart when preview is disabled
	if (preview_mode && display_timer > 0.0f) {
		display_timer = 0.0f;
	}
	
	// Update art effect animation offset for live effect
	if (art_animate && art_effect != ART_NONE) {
		art_animation_offset += seconds * art_speed;
		// Wrap around to prevent overflow
		if (art_animation_offset > 10000.0f)
			art_animation_offset = fmodf(art_animation_offset, 1000.0f);
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
	
	// === PROFESSIONAL CHOREOGRAPHED ANIMATIONS ===
	// Enhanced timing with better staggering for premium feel
	
	// Logo appears at 12% (earlier, leads the entrance)
	float logo_progress = 0.0f;
	if (animation_progress > 0.12f) {
		logo_progress = (animation_progress - 0.12f) / 0.88f; // 0.12-1.0 → 0.0-1.0
		if (logo_progress > 1.0f) logo_progress = 1.0f;
	}
	
	// Apply premium easing curves for sophisticated movement
	float logo_eased = ease_out_expo(logo_progress); // Smooth deceleration
	float logo_alpha_animation = logo_eased; // Synchronized fade
	
	// === LEFT TEXT ANIMATIONS (Cascading entrance) ===
	// Title appears at 35% (delayed after background and logo are visible)
	float title_progress = 0.0f;
	if (animation_progress > 0.35f) {
		title_progress = (animation_progress - 0.35f) / 0.65f; // 0.35-1.0 → 0.0-1.0
		if (title_progress > 1.0f) title_progress = 1.0f;
	}
	
	// Subtitle appears at 42% (staggered after title with more delay)
	float subtitle_progress = 0.0f;
	if (animation_progress > 0.42f) {
		subtitle_progress = (animation_progress - 0.42f) / 0.58f; // 0.42-1.0 → 0.0-1.0
		if (subtitle_progress > 1.0f) subtitle_progress = 1.0f;
	}
	
	// === RIGHT SIDE TEXT ANIMATIONS (Delayed elegance) ===
	// Right title appears at 55% (creates visual hierarchy)
	float title_right_progress = 0.0f;
	if (animation_progress > 0.55f) {
		title_right_progress = (animation_progress - 0.55f) / 0.45f; // 0.55-1.0 → 0.0-1.0
		if (title_right_progress > 1.0f) title_right_progress = 1.0f;
	}
	
	// Right subtitle appears at 63% (final flourish)
	float subtitle_right_progress = 0.0f;
	if (animation_progress > 0.63f) {
		subtitle_right_progress = (animation_progress - 0.63f) / 0.37f; // 0.63-1.0 → 0.0-1.0
		if (subtitle_right_progress > 1.0f) subtitle_right_progress = 1.0f;
	}
	
	// Apply premium easing curves for each element
	// Use ease_in_out_quint for ultra-smooth, broadcast-quality motion
	float title_eased = ease_in_out_quint(title_progress);
	float subtitle_eased = ease_in_out_quint(subtitle_progress);
	float title_right_eased = ease_in_out_quint(title_right_progress);
	float subtitle_right_eased = ease_in_out_quint(subtitle_right_progress);
	
	// Smooth opacity curves with gentle acceleration
	float title_alpha = powf(title_progress, 0.6f); // Subtle ease-out
	float subtitle_alpha = powf(subtitle_progress, 0.6f);
	float title_right_alpha = powf(title_right_progress, 0.6f);
	float subtitle_right_alpha = powf(subtitle_right_progress, 0.6f);
	
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
			// Pure slide from left - NO scaling, just horizontal movement
			{
				float slide_distance = -(float)fixed_width * (1.0f - eased_progress);
				gs_matrix_translate3f(slide_distance, 0.0f, 0.0f);
			}
			break;
			
		case ANIM_SLIDE_RIGHT:
			// Pure slide from right - NO scaling, just horizontal movement
			{
				float slide_distance = (float)fixed_width * (1.0f - eased_progress);
				gs_matrix_translate3f(slide_distance, 0.0f, 0.0f);
			}
			break;
			
		case ANIM_SLIDE_BOTTOM:
			// Pure slide from bottom - NO scaling, just vertical movement up
			{
				float slide_distance = bar_height * (1.0f - eased_progress);
				gs_matrix_translate3f(0.0f, slide_distance, 0.0f);
			}
			break;
			
		case ANIM_FADE:
			// Pure fade - minimal scale, centered, opacity-driven
			{
				float fade_scale = 0.98f + (0.02f * eased_progress);
				float offset_x = (float)fixed_width * (1.0f - fade_scale) * 0.5f;
				float offset_y = bar_height * (1.0f - fade_scale) * 0.5f;
				gs_matrix_translate3f(offset_x, offset_y, 0.0f);
				gs_matrix_scale3f(fade_scale, fade_scale, 1.0f);
			}
			break;
			
		case ANIM_SLIDE_TOP:
			// Pure slide from top - NO scaling, just vertical movement down
			{
				float slide_distance = -bar_height * (1.0f - eased_progress);
				gs_matrix_translate3f(0.0f, slide_distance, 0.0f);
			}
			break;
			
		case ANIM_ZOOM:
			// Dramatic zoom - scales from 30% to 100%, centered
			{
				float scale = 0.3f + (0.7f * eased_progress);
				float offset_x = (float)fixed_width * (1.0f - scale) * 0.5f;
				float offset_y = bar_height * (1.0f - scale) * 0.5f;
				gs_matrix_translate3f(offset_x, offset_y, 0.0f);
				gs_matrix_scale3f(scale, scale, 1.0f);
			}
			break;
			
		case ANIM_EXPAND_LEFT:
			// Horizontal stretch from left - width grows, height stays constant
			{
				float scale_x = eased_progress;
				gs_matrix_scale3f(scale_x, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_EXPAND_RIGHT:
			// Horizontal stretch from right - width grows, height stays constant
			{
				float scale_x = eased_progress;
				float offset_x = (float)fixed_width * (1.0f - scale_x);
				gs_matrix_translate3f(offset_x, 0.0f, 0.0f);
				gs_matrix_scale3f(scale_x, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_PUSH_LEFT:
			// Push from left - combines slide + perspective squeeze (vertical scale too)
			{
				float slide_distance = -(float)fixed_width * (1.0f - eased_progress);
				float scale_x = 0.7f + (0.3f * eased_progress);
				float scale_y = 0.85f + (0.15f * eased_progress);
				float offset_y = bar_height * (1.0f - scale_y) * 0.5f;
				gs_matrix_translate3f(slide_distance, offset_y, 0.0f);
				gs_matrix_scale3f(scale_x, scale_y, 1.0f);
			}
			break;
			
		case ANIM_PUSH_RIGHT:
			// Push from right - combines slide + perspective squeeze (vertical scale too)
			{
				float slide_distance = (float)fixed_width * (1.0f - eased_progress);
				float scale_x = 0.7f + (0.3f * eased_progress);
				float scale_y = 0.85f + (0.15f * eased_progress);
				float offset_y = bar_height * (1.0f - scale_y) * 0.5f;
				gs_matrix_translate3f(slide_distance, offset_y, 0.0f);
				gs_matrix_scale3f(scale_x, scale_y, 1.0f);
			}
			break;
			
		case ANIM_WIPE_LEFT:
			// Wipe from left - horizontal reveal with diagonal crop effect
			{
				float reveal = eased_progress;
				float diagonal_offset = bar_height * 0.2f * (1.0f - eased_progress);
				gs_matrix_translate3f(0.0f, diagonal_offset, 0.0f);
				gs_matrix_scale3f(reveal, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_WIPE_RIGHT:
			// Wipe from right - horizontal reveal with diagonal crop effect
			{
				float reveal = eased_progress;
				float offset_x = (float)fixed_width * (1.0f - reveal);
				float diagonal_offset = bar_height * 0.2f * (1.0f - eased_progress);
				gs_matrix_translate3f(offset_x, diagonal_offset, 0.0f);
				gs_matrix_scale3f(reveal, 1.0f, 1.0f);
			}
			break;
			
		case ANIM_SPIN:
			// Spin with rotation - background fades and slightly rotates
			{
				float rotation = (1.0f - eased_progress) * 45.0f * (3.14159265f / 180.0f);
				float scale = 0.88f + (0.12f * eased_progress);
				gs_matrix_translate3f((float)fixed_width * 0.5f, bar_height * 0.5f, 0.0f);
				gs_matrix_rotaa4f(0.0f, 0.0f, 1.0f, rotation);
				gs_matrix_scale3f(scale, scale, 1.0f);
				gs_matrix_translate3f(-(float)fixed_width * 0.5f, -bar_height * 0.5f, 0.0f);
			}
			break;
			
		case ANIM_SCROLL:
			// Smooth scroll from right with slight bounce at end
			{
				float scroll_distance = (float)fixed_width * 1.2f * (1.0f - eased_progress);
				gs_matrix_translate3f(scroll_distance, 0.0f, 0.0f);
			}
			break;
			
		case ANIM_ROLL:
			// Roll tumble - full 360° rotation while sliding in
			{
				float roll_rotation = (1.0f - eased_progress) * 360.0f * (3.14159265f / 180.0f);
				float roll_distance = (float)fixed_width * 1.3f * (1.0f - eased_progress);
				gs_matrix_translate3f(roll_distance + (float)fixed_width * 0.5f, bar_height * 0.5f, 0.0f);
				gs_matrix_rotaa4f(0.0f, 0.0f, 1.0f, roll_rotation);
				gs_matrix_translate3f(-(float)fixed_width * 0.5f, -bar_height * 0.5f, 0.0f);
			}
			break;
			
		case ANIM_INSTANT:
		default:
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
	
	// Draw art pattern overlay AFTER background (if enabled)
	// Pattern renders on top of all background elements
	if (show_background && art_effect != ART_NONE && art_opacity > 0) {
		// Draw art effect within the current transformation matrix
		draw_art_effect(0.0f, 0.0f, (float)fixed_width, bar_height, 
			art_effect, art_color, (art_opacity / 100.0f) * alpha, 
			art_intensity, art_animation_offset);
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
		
		// Logo position using INDEPENDENT padding controls
		// Horizontal padding applies to left side
		float base_logo_x = fixed_logo_pad_horizontal;
		float base_logo_y = fixed_logo_pad_vertical;
		
		// If logo_padding_vertical is 0, center vertically (default behavior)
		if (logo_padding_vertical == 0) {
			base_logo_y = ((float)bar_height_pixels - fixed_logo_size) / 2.0f;
		}
		
		// User opacity control (independent from animation)
		float user_opacity = (logo_opacity / 100.0f);
		
		// Combine with animation fade
		float final_logo_alpha = user_opacity * logo_alpha_animation;
		
		// Only draw if there's some opacity
		if (final_logo_alpha > 0.001f) {
			// Apply animation transformation based on LOGO animation style
			gs_matrix_push();
			
			// Apply style-specific transformation for logo
			switch (logo_animation_style) {
				case ANIM_SLIDE_LEFT:
					{
						// Enhanced slide with subtle scale
						float slide_offset = -(float)fixed_width * (1.0f - logo_eased);
						float scale = 0.85f + (0.15f * logo_eased);
						gs_matrix_translate3f(base_logo_x + slide_offset + (fixed_logo_size * (1.0f - scale) / 2.0f), 
						                     base_logo_y + (fixed_logo_size * (1.0f - scale) / 2.0f), 0.0f);
						gs_matrix_scale3f(scale, scale, 1.0f);
					}
					break;
					
				case ANIM_SLIDE_RIGHT:
					{
						// Enhanced slide with subtle scale
						float slide_offset = (float)fixed_width * (1.0f - logo_eased);
						float scale = 0.85f + (0.15f * logo_eased);
						gs_matrix_translate3f(base_logo_x + slide_offset + (fixed_logo_size * (1.0f - scale) / 2.0f), 
						                     base_logo_y + (fixed_logo_size * (1.0f - scale) / 2.0f), 0.0f);
						gs_matrix_scale3f(scale, scale, 1.0f);
					}
					break;
					
				case ANIM_SLIDE_BOTTOM:
					{
						// Slide with anticipation and scale
						float slide_offset = (float)bar_height_pixels * 0.9f * (1.0f - logo_eased);
						float scale = 0.80f + (0.20f * logo_eased);
						gs_matrix_translate3f(base_logo_x + (fixed_logo_size * (1.0f - scale) / 2.0f), 
						                     base_logo_y + slide_offset + (fixed_logo_size * (1.0f - scale) / 2.0f), 0.0f);
						gs_matrix_scale3f(scale, scale, 1.0f);
					}
					break;
					
				case ANIM_SLIDE_TOP:
					{
						// Slide with anticipation and scale
						float slide_offset = -(float)bar_height_pixels * 0.9f * (1.0f - logo_eased);
						float scale = 0.80f + (0.20f * logo_eased);
						gs_matrix_translate3f(base_logo_x + (fixed_logo_size * (1.0f - scale) / 2.0f), 
						                     base_logo_y + slide_offset + (fixed_logo_size * (1.0f - scale) / 2.0f), 0.0f);
						gs_matrix_scale3f(scale, scale, 1.0f);
					}
					break;
					
				case ANIM_ZOOM:
					{
						// Dramatic zoom with smooth deceleration
						float zoom_scale = 0.15f + (0.85f * logo_eased); // 0.15 → 1.0 for impact
						gs_matrix_translate3f(base_logo_x + (fixed_logo_size * (1.0f - zoom_scale) / 2.0f), 
						                     base_logo_y + (fixed_logo_size * (1.0f - zoom_scale) / 2.0f), 0.0f);
						gs_matrix_scale3f(zoom_scale, zoom_scale, 1.0f);
					}
					break;
					
				case ANIM_FADE:
					{
						// Minimal scale for elegant fade
						float scale = 0.95f + (0.05f * logo_eased);
						gs_matrix_translate3f(base_logo_x + (fixed_logo_size * (1.0f - scale) / 2.0f), 
						                     base_logo_y + (fixed_logo_size * (1.0f - scale) / 2.0f), 0.0f);
						gs_matrix_scale3f(scale, scale, 1.0f);
					}
					break;
					
				case ANIM_EXPAND_LEFT:
				case ANIM_EXPAND_RIGHT:
				case ANIM_PUSH_LEFT:
				case ANIM_PUSH_RIGHT:
				case ANIM_WIPE_LEFT:
				case ANIM_WIPE_RIGHT:
					{
						// Elastic scale with push effect
						float scale_amount = 0.5f + (0.5f * logo_eased);
						gs_matrix_translate3f(base_logo_x + (fixed_logo_size * (1.0f - scale_amount) / 2.0f), 
						                     base_logo_y + (fixed_logo_size * (1.0f - scale_amount) / 2.0f), 0.0f);
						gs_matrix_scale3f(scale_amount, scale_amount, 1.0f);
					}
					break;
					
				case ANIM_SPIN:
					{
						// Spinning rotation with scale (720 degrees = 2 full rotations)
						float rotation_angle = (1.0f - logo_eased) * 720.0f * (3.14159265f / 180.0f); // Convert to radians
						float scale = 0.3f + (0.7f * logo_eased); // Scale from 30% to 100%
						
						// Move to center of logo
						gs_matrix_translate3f(base_logo_x + (fixed_logo_size / 2.0f), 
						                     base_logo_y + (fixed_logo_size / 2.0f), 0.0f);
						// Rotate around center
						gs_matrix_rotaa4f(0.0f, 0.0f, 1.0f, rotation_angle);
						// Scale
						gs_matrix_scale3f(scale, scale, 1.0f);
						// Move back to corner for drawing
						gs_matrix_translate3f(-(fixed_logo_size / 2.0f), -(fixed_logo_size / 2.0f), 0.0f);
					}
					break;
					
				case ANIM_SCROLL:
					{
						// Horizontal scroll from left with fade
						float scroll_offset = -(float)fixed_width * 0.5f * (1.0f - logo_eased);
						gs_matrix_translate3f(base_logo_x + scroll_offset, base_logo_y, 0.0f);
					}
					break;
					
				case ANIM_ROLL:
					{
						// Rolling tumble effect
						float roll_angle = (1.0f - logo_eased) * 180.0f * (3.14159265f / 180.0f);
						float roll_offset = -(float)fixed_width * 0.3f * (1.0f - logo_eased);
						
						gs_matrix_translate3f(base_logo_x + roll_offset + (fixed_logo_size / 2.0f), 
						                     base_logo_y + (fixed_logo_size / 2.0f), 0.0f);
						gs_matrix_rotaa4f(0.0f, 0.0f, 1.0f, roll_angle);
						gs_matrix_translate3f(-(fixed_logo_size / 2.0f), -(fixed_logo_size / 2.0f), 0.0f);
					}
					break;
					
				case ANIM_INSTANT:
				default:
					// Instant appearance
					gs_matrix_translate3f(base_logo_x, base_logo_y, 0.0f);
					break;
			}
			
			// Draw logo shadow first (if enabled) - simple darkened copy with offset
			if (logo_shadow_enabled) {
				gs_matrix_push();
				
				// Apply shadow offset
				gs_matrix_translate3f((float)logo_shadow_offset_x, (float)logo_shadow_offset_y, 0.0f);
				
				// Calculate shadow alpha (combination of shadow opacity and animation)
				float shadow_alpha = (logo_shadow_opacity / 100.0f) * final_logo_alpha;
				
				// Draw logo as shadow (darkened with shadow color)
				draw_logo_with_alpha(logo_image->texture, fixed_logo_size, fixed_logo_size, shadow_alpha * 0.5f);
				
				gs_matrix_pop();
			}
			
			// Draw logo with custom alpha support
			draw_logo_with_alpha(logo_image->texture, fixed_logo_size, fixed_logo_size, final_logo_alpha);
			
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
	
	// Calculate animation-specific text transformations based on TEXT animation style
	float fixed_title_offset_y = 0.0f;
	float fixed_title_offset_x = 0.0f;
	float fixed_subtitle_offset_y = 0.0f;
	float fixed_subtitle_offset_x = 0.0f;
	float title_scale = 1.0f;
	float subtitle_scale = 1.0f;
	
	switch (text_animation_style) {
		case ANIM_SLIDE_LEFT:
			// Enhanced slide from left with subtle scale
			fixed_title_offset_x = -(float)fixed_width * (1.0f - title_eased);
			fixed_subtitle_offset_x = -(float)fixed_width * (1.0f - subtitle_eased);
			title_scale = 0.92f + (0.08f * title_eased); // Subtle grow
			subtitle_scale = 0.94f + (0.06f * subtitle_eased);
			break;
			
		case ANIM_SLIDE_RIGHT:
			// Enhanced slide from right with subtle scale
			fixed_title_offset_x = (float)fixed_width * (1.0f - title_eased);
			fixed_subtitle_offset_x = (float)fixed_width * (1.0f - subtitle_eased);
			title_scale = 0.92f + (0.08f * title_eased);
			subtitle_scale = 0.94f + (0.06f * subtitle_eased);
			break;
			
		case ANIM_SLIDE_BOTTOM:
			// Slide from bottom with anticipation
			fixed_title_offset_y = (float)bar_height_pixels * 0.8f * (1.0f - title_eased);
			fixed_subtitle_offset_y = (float)bar_height_pixels * 0.8f * (1.0f - subtitle_eased);
			title_scale = 0.90f + (0.10f * title_eased);
			subtitle_scale = 0.92f + (0.08f * subtitle_eased);
			break;
			
		case ANIM_SLIDE_TOP:
			// Slide from top with anticipation
			fixed_title_offset_y = -(float)bar_height_pixels * 0.8f * (1.0f - title_eased);
			fixed_subtitle_offset_y = -(float)bar_height_pixels * 0.8f * (1.0f - subtitle_eased);
			title_scale = 0.90f + (0.10f * title_eased);
			subtitle_scale = 0.92f + (0.08f * subtitle_eased);
			break;
			
		case ANIM_ZOOM:
			// Dramatic zoom with smooth deceleration
			title_scale = 0.2f + (0.8f * title_eased);
			subtitle_scale = 0.25f + (0.75f * subtitle_eased);
			break;
			
		case ANIM_FADE:
			// Pure fade with minimal scale
			title_scale = 0.98f + (0.02f * title_eased);
			subtitle_scale = 0.98f + (0.02f * subtitle_eased);
			break;
			
		case ANIM_EXPAND_LEFT:
			// Horizontal stretch from left - NO movement, just width expansion
			title_scale = 0.5f + (0.5f * title_eased);
			subtitle_scale = 0.5f + (0.5f * subtitle_eased);
			// Offset to keep left edge anchored
			fixed_title_offset_x = -(float)fixed_width * 0.25f * (1.0f - title_eased);
			fixed_subtitle_offset_x = -(float)fixed_width * 0.25f * (1.0f - subtitle_eased);
			break;
			
		case ANIM_EXPAND_RIGHT:
			// Horizontal stretch from right - NO movement, just width expansion
			title_scale = 0.5f + (0.5f * title_eased);
			subtitle_scale = 0.5f + (0.5f * subtitle_eased);
			// Offset to keep right edge anchored
			fixed_title_offset_x = (float)fixed_width * 0.25f * (1.0f - title_eased);
			fixed_subtitle_offset_x = (float)fixed_width * 0.25f * (1.0f - subtitle_eased);
			break;
			
		case ANIM_PUSH_LEFT:
			// Push from left - slide + perspective squeeze (both X and Y scale)
			fixed_title_offset_x = -(float)fixed_width * 0.5f * (1.0f - title_eased);
			fixed_subtitle_offset_x = -(float)fixed_width * 0.5f * (1.0f - subtitle_eased);
			title_scale = 0.65f + (0.35f * title_eased);
			subtitle_scale = 0.68f + (0.32f * subtitle_eased);
			// Add vertical offset for depth
			fixed_title_offset_y = -15.0f * (1.0f - title_eased);
			fixed_subtitle_offset_y = -12.0f * (1.0f - subtitle_eased);
			break;
			
		case ANIM_PUSH_RIGHT:
			// Push from right - slide + perspective squeeze (both X and Y scale)
			fixed_title_offset_x = (float)fixed_width * 0.5f * (1.0f - title_eased);
			fixed_subtitle_offset_x = (float)fixed_width * 0.5f * (1.0f - subtitle_eased);
			title_scale = 0.65f + (0.35f * title_eased);
			subtitle_scale = 0.68f + (0.32f * subtitle_eased);
			// Add vertical offset for depth
			fixed_title_offset_y = -15.0f * (1.0f - title_eased);
			fixed_subtitle_offset_y = -12.0f * (1.0f - subtitle_eased);
			break;
			
		case ANIM_WIPE_LEFT:
			// Wipe from left - smaller slide with diagonal feel
			fixed_title_offset_x = -(float)fixed_width * 0.35f * (1.0f - title_eased);
			fixed_subtitle_offset_x = -(float)fixed_width * 0.35f * (1.0f - subtitle_eased);
			fixed_title_offset_y = 25.0f * (1.0f - title_eased);
			fixed_subtitle_offset_y = 20.0f * (1.0f - subtitle_eased);
			title_scale = 0.75f + (0.25f * title_eased);
			subtitle_scale = 0.78f + (0.22f * subtitle_eased);
			break;
			
		case ANIM_WIPE_RIGHT:
			// Wipe from right - smaller slide with diagonal feel
			fixed_title_offset_x = (float)fixed_width * 0.35f * (1.0f - title_eased);
			fixed_subtitle_offset_x = (float)fixed_width * 0.35f * (1.0f - subtitle_eased);
			fixed_title_offset_y = 25.0f * (1.0f - title_eased);
			fixed_subtitle_offset_y = 20.0f * (1.0f - subtitle_eased);
			title_scale = 0.75f + (0.25f * title_eased);
			subtitle_scale = 0.78f + (0.22f * subtitle_eased);
			break;
			
		case ANIM_SPIN:
			// Spin - dramatic zoom with rotation feel
			title_scale = 0.4f + (0.6f * title_eased);
			subtitle_scale = 0.45f + (0.55f * subtitle_eased);
			// Slight circular offset
			fixed_title_offset_x = -30.0f * (1.0f - title_eased);
			fixed_title_offset_y = -20.0f * (1.0f - title_eased);
			fixed_subtitle_offset_x = -28.0f * (1.0f - subtitle_eased);
			fixed_subtitle_offset_y = -18.0f * (1.0f - subtitle_eased);
			break;
			
		case ANIM_SCROLL:
			// Smooth horizontal scroll - NO scale, pure movement
			fixed_title_offset_x = -(float)fixed_width * 0.7f * (1.0f - title_eased);
			fixed_subtitle_offset_x = -(float)fixed_width * 0.7f * (1.0f - subtitle_eased);
			title_scale = 1.0f;
			subtitle_scale = 1.0f;
			break;
			
		case ANIM_ROLL:
			// Rolling tumble - slide with scale variation
			fixed_title_offset_x = -(float)fixed_width * 0.6f * (1.0f - title_eased);
			fixed_subtitle_offset_x = -(float)fixed_width * 0.6f * (1.0f - subtitle_eased);
			// Bouncing scale effect
			title_scale = 0.6f + (0.4f * title_eased);
			subtitle_scale = 0.62f + (0.38f * subtitle_eased);
			// Slight vertical bobbing
			fixed_title_offset_y = 12.0f * sinf((1.0f - title_eased) * 3.14159f);
			fixed_subtitle_offset_y = 10.0f * sinf((1.0f - subtitle_eased) * 3.14159f);
			break;
			
		case ANIM_INSTANT:
		default:
			// Instant appearance
			title_scale = 1.0f;
			subtitle_scale = 1.0f;
			break;
	}
	
	// Draw title text (appears first with animation)
	if (title_text_source && title[current_profile] && strlen(title[current_profile]) > 0 && title_alpha > 0.01f) {
		// Center text vertically in the bar (uses pixel values for stable position)
		float title_y = center_offset + fixed_title_offset_y;
		float title_x = fixed_padding_horizontal + logo_width_with_padding + fixed_title_offset_x;
		
		// Draw text highlight/background box (if enabled) - BEFORE text
		if (text_highlight_enabled) {
			uint32_t text_width = obs_source_get_width(title_text_source);
			uint32_t text_height = obs_source_get_height(title_text_source);
			
			if (text_width > 0 && text_height > 0) {
				float box_x = title_x - (float)text_highlight_padding_horizontal;
				float box_y = title_y - (float)text_highlight_padding_vertical;
				float box_width = (float)text_width + 2.0f * (float)text_highlight_padding_horizontal;
				float box_height = (float)text_height + 2.0f * (float)text_highlight_padding_vertical;
				float highlight_opacity = (text_highlight_opacity / 100.0f) * title_alpha;
				
				draw_rounded_rect(box_x, box_y, box_width, box_height, 
					(float)text_highlight_corner_radius, text_highlight_color, highlight_opacity);
			}
		}
		
		gs_matrix_push();
		gs_matrix_translate3f(title_x, title_y, 0.0f);
		
		// Apply modern scale animation for dynamic entrance
		float final_scale = auto_scale ? (scale_factor * title_scale) : title_scale;
		gs_matrix_scale3f(final_scale, final_scale, 1.0f);
		
		// Render text shadow first (if enabled)
		if (text_shadow_enabled) {
			gs_matrix_push();
			gs_matrix_translate3f((float)text_shadow_offset_x, (float)text_shadow_offset_y, 0.0f);
			
			// Apply shadow opacity combined with text alpha
			float shadow_opacity = (text_shadow_opacity / 100.0f) * title_alpha;
			
			// Apply shadow color (convert ABGR to packed RGBA uint32)
			uint8_t shadow_r = (text_shadow_color >> 0) & 0xFF;
			uint8_t shadow_g = (text_shadow_color >> 8) & 0xFF;
			uint8_t shadow_b = (text_shadow_color >> 16) & 0xFF;
			uint8_t shadow_a = (uint8_t)(shadow_opacity * 255.0f);
			
			// Pack into RGBA format (R in lowest byte)
			uint32_t shadow_color_packed = (shadow_a << 24) | (shadow_b << 16) | (shadow_g << 8) | shadow_r;
			
			gs_enable_color(true, true, true, true);
			gs_color(shadow_color_packed);
			
			obs_source_video_render(title_text_source);
			
			// Reset color modulation to white (fully opaque)
			gs_color(0xFFFFFFFF);
			
			gs_matrix_pop();
		}
		
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
	
	// Draw subtitle text (appears after title - staggered effect)
	if (subtitle_text_source && subtitle[current_profile] && strlen(subtitle[current_profile]) > 0 && subtitle_alpha > 0.01f) {
		// Keep subtitle below title, centered in the bar
		float subtitle_y = center_offset + text_title_size + text_spacing + fixed_subtitle_offset_y;
		float subtitle_x = fixed_padding_horizontal + logo_width_with_padding + fixed_subtitle_offset_x;
		
		// Draw text highlight/background box (if enabled) - BEFORE text
		if (text_highlight_enabled) {
			uint32_t text_width = obs_source_get_width(subtitle_text_source);
			uint32_t text_height = obs_source_get_height(subtitle_text_source);
			
			if (text_width > 0 && text_height > 0) {
				float box_x = subtitle_x - (float)text_highlight_padding_horizontal;
				float box_y = subtitle_y - (float)text_highlight_padding_vertical;
				float box_width = (float)text_width + 2.0f * (float)text_highlight_padding_horizontal;
				float box_height = (float)text_height + 2.0f * (float)text_highlight_padding_vertical;
				float highlight_opacity = (text_highlight_opacity / 100.0f) * subtitle_alpha;
				
				draw_rounded_rect(box_x, box_y, box_width, box_height, 
					(float)text_highlight_corner_radius, text_highlight_color, highlight_opacity);
			}
		}
		
		gs_matrix_push();
		gs_matrix_translate3f(subtitle_x, subtitle_y, 0.0f);
		
		// Apply modern scale animation for smooth entrance
		float final_scale = auto_scale ? (scale_factor * subtitle_scale) : subtitle_scale;
		gs_matrix_scale3f(final_scale, final_scale, 1.0f);
		
		// Render text shadow first (if enabled)
		if (text_shadow_enabled) {
			gs_matrix_push();
			gs_matrix_translate3f((float)text_shadow_offset_x, (float)text_shadow_offset_y, 0.0f);
			
			// Apply shadow opacity combined with text alpha
			float shadow_opacity = (text_shadow_opacity / 100.0f) * subtitle_alpha;
			
			// Apply shadow color (convert ABGR to packed RGBA uint32)
			uint8_t shadow_r = (text_shadow_color >> 0) & 0xFF;
			uint8_t shadow_g = (text_shadow_color >> 8) & 0xFF;
			uint8_t shadow_b = (text_shadow_color >> 16) & 0xFF;
			uint8_t shadow_a = (uint8_t)(shadow_opacity * 255.0f);
			
			// Pack into RGBA format (R in lowest byte)
			uint32_t shadow_color_packed = (shadow_a << 24) | (shadow_b << 16) | (shadow_g << 8) | shadow_r;
			
			gs_enable_color(true, true, true, true);
			gs_color(shadow_color_packed);
			
			obs_source_video_render(subtitle_text_source);
			
			// Reset color modulation to white (fully opaque)
			gs_color(0xFFFFFFFF);
			
			gs_matrix_pop();
		}
		
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
	
	// === Draw RIGHT SIDE Text (Optional) - DELAYED APPEARANCE ===
	// Calculate delayed animation offsets for right-side text (matches TEXT animation style)
	float fixed_title_right_offset_y = 0.0f;
	float fixed_title_right_offset_x = 0.0f;
	float fixed_subtitle_right_offset_y = 0.0f;
	float fixed_subtitle_right_offset_x = 0.0f;
	float title_right_scale = 1.0f;
	float subtitle_right_scale = 1.0f;
	
	switch (text_animation_style) {
		case ANIM_SLIDE_LEFT:
			fixed_title_right_offset_x = -(float)fixed_width * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = -(float)fixed_width * (1.0f - subtitle_right_eased);
			title_right_scale = 0.92f + (0.08f * title_right_eased);
			subtitle_right_scale = 0.94f + (0.06f * subtitle_right_eased);
			break;
			
		case ANIM_SLIDE_RIGHT:
			fixed_title_right_offset_x = (float)fixed_width * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = (float)fixed_width * (1.0f - subtitle_right_eased);
			title_right_scale = 0.92f + (0.08f * title_right_eased);
			subtitle_right_scale = 0.94f + (0.06f * subtitle_right_eased);
			break;
			
		case ANIM_SLIDE_BOTTOM:
			fixed_title_right_offset_y = (float)bar_height_pixels * 0.8f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_y = (float)bar_height_pixels * 0.8f * (1.0f - subtitle_right_eased);
			title_right_scale = 0.90f + (0.10f * title_right_eased);
			subtitle_right_scale = 0.92f + (0.08f * subtitle_right_eased);
			break;
			
		case ANIM_SLIDE_TOP:
			fixed_title_right_offset_y = -(float)bar_height_pixels * 0.8f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_y = -(float)bar_height_pixels * 0.8f * (1.0f - subtitle_right_eased);
			title_right_scale = 0.90f + (0.10f * title_right_eased);
			subtitle_right_scale = 0.92f + (0.08f * subtitle_right_eased);
			break;
			
		case ANIM_ZOOM:
			title_right_scale = 0.2f + (0.8f * title_right_eased);
			subtitle_right_scale = 0.25f + (0.75f * subtitle_right_eased);
			break;
			
		case ANIM_FADE:
			title_right_scale = 0.98f + (0.02f * title_right_eased);
			subtitle_right_scale = 0.98f + (0.02f * subtitle_right_eased);
			break;
			
		case ANIM_EXPAND_LEFT:
			// Horizontal stretch from left - right text mimics left behavior
			title_right_scale = 0.5f + (0.5f * title_right_eased);
			subtitle_right_scale = 0.5f + (0.5f * subtitle_right_eased);
			fixed_title_right_offset_x = -(float)fixed_width * 0.25f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = -(float)fixed_width * 0.25f * (1.0f - subtitle_right_eased);
			break;
			
		case ANIM_EXPAND_RIGHT:
			// Horizontal stretch from right - mirrors left text expansion
			title_right_scale = 0.5f + (0.5f * title_right_eased);
			subtitle_right_scale = 0.5f + (0.5f * subtitle_right_eased);
			fixed_title_right_offset_x = (float)fixed_width * 0.25f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = (float)fixed_width * 0.25f * (1.0f - subtitle_right_eased);
			break;
			
		case ANIM_PUSH_LEFT:
			// Push from left with perspective
			fixed_title_right_offset_x = -(float)fixed_width * 0.5f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = -(float)fixed_width * 0.5f * (1.0f - subtitle_right_eased);
			title_right_scale = 0.65f + (0.35f * title_right_eased);
			subtitle_right_scale = 0.68f + (0.32f * subtitle_right_eased);
			fixed_title_right_offset_y = -15.0f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_y = -12.0f * (1.0f - subtitle_right_eased);
			break;
			
		case ANIM_PUSH_RIGHT:
			// Push from right with perspective
			fixed_title_right_offset_x = (float)fixed_width * 0.5f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = (float)fixed_width * 0.5f * (1.0f - subtitle_right_eased);
			title_right_scale = 0.65f + (0.35f * title_right_eased);
			subtitle_right_scale = 0.68f + (0.32f * subtitle_right_eased);
			fixed_title_right_offset_y = -15.0f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_y = -12.0f * (1.0f - subtitle_right_eased);
			break;
			
		case ANIM_WIPE_LEFT:
			// Wipe from left with diagonal
			fixed_title_right_offset_x = -(float)fixed_width * 0.35f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = -(float)fixed_width * 0.35f * (1.0f - subtitle_right_eased);
			fixed_title_right_offset_y = 25.0f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_y = 20.0f * (1.0f - subtitle_right_eased);
			title_right_scale = 0.75f + (0.25f * title_right_eased);
			subtitle_right_scale = 0.78f + (0.22f * subtitle_right_eased);
			break;
			
		case ANIM_WIPE_RIGHT:
			// Wipe from right with diagonal
			fixed_title_right_offset_x = (float)fixed_width * 0.35f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = (float)fixed_width * 0.35f * (1.0f - subtitle_right_eased);
			fixed_title_right_offset_y = 25.0f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_y = 20.0f * (1.0f - subtitle_right_eased);
			title_right_scale = 0.75f + (0.25f * title_right_eased);
			subtitle_right_scale = 0.78f + (0.22f * subtitle_right_eased);
			break;
			
		case ANIM_SPIN:
			// Spin with dramatic zoom
			title_right_scale = 0.4f + (0.6f * title_right_eased);
			subtitle_right_scale = 0.45f + (0.55f * subtitle_right_eased);
			fixed_title_right_offset_x = 30.0f * (1.0f - title_right_eased);
			fixed_title_right_offset_y = -20.0f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = 28.0f * (1.0f - subtitle_right_eased);
			fixed_subtitle_right_offset_y = -18.0f * (1.0f - subtitle_right_eased);
			break;
			
		case ANIM_SCROLL:
			// Smooth horizontal scroll from right
			fixed_title_right_offset_x = (float)fixed_width * 0.7f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = (float)fixed_width * 0.7f * (1.0f - subtitle_right_eased);
			title_right_scale = 1.0f;
			subtitle_right_scale = 1.0f;
			break;
			
		case ANIM_ROLL:
			// Rolling tumble from right
			fixed_title_right_offset_x = (float)fixed_width * 0.6f * (1.0f - title_right_eased);
			fixed_subtitle_right_offset_x = (float)fixed_width * 0.6f * (1.0f - subtitle_right_eased);
			title_right_scale = 0.6f + (0.4f * title_right_eased);
			subtitle_right_scale = 0.62f + (0.38f * subtitle_right_eased);
			fixed_title_right_offset_y = 12.0f * sinf((1.0f - title_right_eased) * 3.14159f);
			fixed_subtitle_right_offset_y = 10.0f * sinf((1.0f - subtitle_right_eased) * 3.14159f);
			break;
			
		case ANIM_INSTANT:
		default:
			title_right_scale = 1.0f;
			subtitle_right_scale = 1.0f;
			break;
	}
	
	// Draw right title text (with modern scale animation - DELAYED)
	if (title_right_text_source && title_right[current_profile] && strlen(title_right[current_profile]) > 0 && title_right_alpha > 0.01f) {
		// Get text width for right alignment (pixel values for stable position)
		uint32_t title_right_width = obs_source_get_width(title_right_text_source);
		uint32_t title_right_height = obs_source_get_height(title_right_text_source);
		float title_right_x = (float)fixed_width - (float)title_right_width - fixed_padding_horizontal - fixed_title_right_offset_x;
		// Keep right text centered vertically too (use fixed offset)
		float title_right_y = center_offset + fixed_title_right_offset_y;
		
		// Draw text highlight/background box (if enabled) - BEFORE text
		if (text_highlight_enabled && title_right_width > 0 && title_right_height > 0) {
			float box_x = title_right_x - (float)text_highlight_padding_horizontal;
			float box_y = title_right_y - (float)text_highlight_padding_vertical;
			float box_width = (float)title_right_width + 2.0f * (float)text_highlight_padding_horizontal;
			float box_height = (float)title_right_height + 2.0f * (float)text_highlight_padding_vertical;
			float highlight_opacity = (text_highlight_opacity / 100.0f) * title_right_alpha;
			
			draw_rounded_rect(box_x, box_y, box_width, box_height, 
				(float)text_highlight_corner_radius, text_highlight_color, highlight_opacity);
		}
		
		gs_matrix_push();
		gs_matrix_translate3f(title_right_x, title_right_y, 0.0f);
		
		// Apply modern scale animation for dynamic entrance
		float final_scale = auto_scale ? (scale_factor * title_right_scale) : title_right_scale;
		gs_matrix_scale3f(final_scale, final_scale, 1.0f);
		
		// Render text shadow first (if enabled)
		if (text_shadow_enabled) {
			gs_matrix_push();
			gs_matrix_translate3f((float)text_shadow_offset_x, (float)text_shadow_offset_y, 0.0f);
			
			// Apply shadow opacity combined with text alpha
			float shadow_opacity = (text_shadow_opacity / 100.0f) * title_right_alpha;
			
			// Apply shadow color (convert ABGR to packed RGBA uint32)
			uint8_t shadow_r = (text_shadow_color >> 0) & 0xFF;
			uint8_t shadow_g = (text_shadow_color >> 8) & 0xFF;
			uint8_t shadow_b = (text_shadow_color >> 16) & 0xFF;
			uint8_t shadow_a = (uint8_t)(shadow_opacity * 255.0f);
			
			// Pack into RGBA format (R in lowest byte)
			uint32_t shadow_color_packed = (shadow_a << 24) | (shadow_b << 16) | (shadow_g << 8) | shadow_r;
			
			gs_enable_color(true, true, true, true);
			gs_color(shadow_color_packed);
			
			obs_source_video_render(title_right_text_source);
			
			// Reset color modulation to white (fully opaque)
			gs_color(0xFFFFFFFF);
			
			gs_matrix_pop();
		}
		
		// Apply professional fade with opacity
		if (title_right_alpha < 1.0f) {
			gs_effect_t *default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
			gs_eparam_t *opacity_param = gs_effect_get_param_by_name(default_effect, "opacity");
			if (opacity_param)
				gs_effect_set_float(opacity_param, title_right_alpha);
		}
		
		obs_source_video_render(title_right_text_source);
		gs_matrix_pop();
	}
	
	// Draw right subtitle text (with modern scale animation - MORE DELAYED)
	if (subtitle_right_text_source && subtitle_right[current_profile] && strlen(subtitle_right[current_profile]) > 0 && subtitle_right_alpha > 0.01f) {
		// Get text width for right alignment (pixel values for stable position)
		uint32_t subtitle_right_width = obs_source_get_width(subtitle_right_text_source);
		uint32_t subtitle_right_height = obs_source_get_height(subtitle_right_text_source);
		float subtitle_right_x = (float)fixed_width - (float)subtitle_right_width - fixed_padding_horizontal - fixed_subtitle_right_offset_x;
		// Keep right subtitle centered vertically (use fixed values)
		float subtitle_right_y = center_offset + text_title_size + text_spacing + fixed_subtitle_right_offset_y;
		
		// Draw text highlight/background box (if enabled) - BEFORE text
		if (text_highlight_enabled && subtitle_right_width > 0 && subtitle_right_height > 0) {
			float box_x = subtitle_right_x - (float)text_highlight_padding_horizontal;
			float box_y = subtitle_right_y - (float)text_highlight_padding_vertical;
			float box_width = (float)subtitle_right_width + 2.0f * (float)text_highlight_padding_horizontal;
			float box_height = (float)subtitle_right_height + 2.0f * (float)text_highlight_padding_vertical;
			float highlight_opacity = (text_highlight_opacity / 100.0f) * subtitle_right_alpha;
			
			draw_rounded_rect(box_x, box_y, box_width, box_height, 
				(float)text_highlight_corner_radius, text_highlight_color, highlight_opacity);
		}
		
		gs_matrix_push();
		gs_matrix_translate3f(subtitle_right_x, subtitle_right_y, 0.0f);
		
		// Apply modern scale animation for smooth entrance
		float final_scale = auto_scale ? (scale_factor * subtitle_right_scale) : subtitle_right_scale;
		gs_matrix_scale3f(final_scale, final_scale, 1.0f);
		
		// Render text shadow first (if enabled)
		if (text_shadow_enabled) {
			gs_matrix_push();
			gs_matrix_translate3f((float)text_shadow_offset_x, (float)text_shadow_offset_y, 0.0f);
			
			// Apply shadow opacity combined with text alpha
			float shadow_opacity = (text_shadow_opacity / 100.0f) * subtitle_right_alpha;
			
			// Apply shadow color (convert ABGR to packed RGBA uint32)
			uint8_t shadow_r = (text_shadow_color >> 0) & 0xFF;
			uint8_t shadow_g = (text_shadow_color >> 8) & 0xFF;
			uint8_t shadow_b = (text_shadow_color >> 16) & 0xFF;
			uint8_t shadow_a = (uint8_t)(shadow_opacity * 255.0f);
			
			// Pack into RGBA format (R in lowest byte)
			uint32_t shadow_color_packed = (shadow_a << 24) | (shadow_b << 16) | (shadow_g << 8) | shadow_r;
			
			gs_enable_color(true, true, true, true);
			gs_color(shadow_color_packed);
			
			obs_source_video_render(subtitle_right_text_source);
			
			// Reset color modulation to white (fully opaque)
			gs_color(0xFFFFFFFF);
			
			gs_matrix_pop();
		}
		
		// Apply professional fade with opacity
		if (subtitle_right_alpha < 1.0f) {
			gs_effect_t *default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
			gs_eparam_t *opacity_param = gs_effect_get_param_by_name(default_effect, "opacity");
			if (opacity_param)
				gs_effect_set_float(opacity_param, subtitle_right_alpha);
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

void lowerthirds_source::draw_rounded_rect(float x, float y, float width, float height, float radius, uint32_t color, float opacity)
{
	if (opacity < 0.001f || width <= 0.0f || height <= 0.0f)
		return;
	
	// Clamp radius to valid range
	float max_radius = fminf(width / 2.0f, height / 2.0f);
	radius = fminf(radius, max_radius);
	
	// Convert ABGR color to vec4 (RGBA float 0-1)
	struct vec4 color_vec;
	vec4_set(&color_vec,
		((color >> 0) & 0xFF) / 255.0f,   // R
		((color >> 8) & 0xFF) / 255.0f,   // G
		((color >> 16) & 0xFF) / 255.0f,  // B
		opacity                             // A
	);
	
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *color_param = gs_effect_get_param_by_name(solid, "color");
	gs_effect_set_vec4(color_param, &color_vec);
	
	gs_matrix_push();
	gs_matrix_translate3f(x, y, 0.0f);
	
	while (gs_effect_loop(solid, "Solid")) {
		if (radius <= 0.5f) {
			// Draw simple rectangle (no rounded corners)
			gs_render_start(true);
			gs_vertex2f(0.0f, 0.0f);
			gs_vertex2f(width, 0.0f);
			gs_vertex2f(0.0f, height);
			gs_vertex2f(width, height);
			gs_render_stop(GS_TRISTRIP);
		} else {
			// Draw rounded rectangle using segments
			int segments = 8; // Number of segments per corner
			
			// Center rectangle (main body)
			gs_render_start(true);
			gs_vertex2f(radius, 0.0f);
			gs_vertex2f(width - radius, 0.0f);
			gs_vertex2f(radius, height);
			gs_vertex2f(width - radius, height);
			gs_render_stop(GS_TRISTRIP);
			
			// Left rectangle
			gs_render_start(true);
			gs_vertex2f(0.0f, radius);
			gs_vertex2f(radius, radius);
			gs_vertex2f(0.0f, height - radius);
			gs_vertex2f(radius, height - radius);
			gs_render_stop(GS_TRISTRIP);
			
			// Right rectangle
			gs_render_start(true);
			gs_vertex2f(width - radius, radius);
			gs_vertex2f(width, radius);
			gs_vertex2f(width - radius, height - radius);
			gs_vertex2f(width, height - radius);
			gs_render_stop(GS_TRISTRIP);
			
			// Top-left corner (triangle strip)
			for (int i = 0; i < segments; i++) {
				float angle1 = (float)M_PI + (float)i / (float)segments * (float)M_PI / 2.0f;
				float angle2 = (float)M_PI + (float)(i + 1) / (float)segments * (float)M_PI / 2.0f;
				
				gs_render_start(true);
				gs_vertex2f(radius, radius); // Center
				gs_vertex2f(radius + radius * cosf(angle1), radius + radius * sinf(angle1));
				gs_vertex2f(radius + radius * cosf(angle2), radius + radius * sinf(angle2));
				gs_render_stop(GS_TRIS);
			}
			
			// Top-right corner (triangle strip)
			for (int i = 0; i < segments; i++) {
				float angle1 = 1.5f * (float)M_PI + (float)i / (float)segments * (float)M_PI / 2.0f;
				float angle2 = 1.5f * (float)M_PI + (float)(i + 1) / (float)segments * (float)M_PI / 2.0f;
				
				gs_render_start(true);
				gs_vertex2f(width - radius, radius); // Center
				gs_vertex2f(width - radius + radius * cosf(angle1), radius + radius * sinf(angle1));
				gs_vertex2f(width - radius + radius * cosf(angle2), radius + radius * sinf(angle2));
				gs_render_stop(GS_TRIS);
			}
			
			// Bottom-left corner (triangle strip)
			for (int i = 0; i < segments; i++) {
				float angle1 = 0.5f * (float)M_PI + (float)i / (float)segments * (float)M_PI / 2.0f;
				float angle2 = 0.5f * (float)M_PI + (float)(i + 1) / (float)segments * (float)M_PI / 2.0f;
				
				gs_render_start(true);
				gs_vertex2f(radius, height - radius); // Center
				gs_vertex2f(radius + radius * cosf(angle1), height - radius + radius * sinf(angle1));
				gs_vertex2f(radius + radius * cosf(angle2), height - radius + radius * sinf(angle2));
				gs_render_stop(GS_TRIS);
			}
			
			// Bottom-right corner (triangle strip)
			for (int i = 0; i < segments; i++) {
				float angle1 = 0.0f + (float)i / (float)segments * (float)M_PI / 2.0f;
				float angle2 = 0.0f + (float)(i + 1) / (float)segments * (float)M_PI / 2.0f;
				
				gs_render_start(true);
				gs_vertex2f(width - radius, height - radius); // Center
				gs_vertex2f(width - radius + radius * cosf(angle1), height - radius + radius * sinf(angle1));
				gs_vertex2f(width - radius + radius * cosf(angle2), height - radius + radius * sinf(angle2));
				gs_render_stop(GS_TRIS);
			}
		}
	}
	
	gs_matrix_pop();
}

// Draw background pattern with live animation effects
void lowerthirds_source::draw_art_effect(float x, float y, float width, float height, 
	BackgroundArtEffect effect, uint32_t color, float opacity, float intensity, float animation_offset)
{
	if (opacity < 0.001f || width <= 0.0f || height <= 0.0f || effect == ART_NONE)
		return;
	
	// Convert color to vec4 (ABGR to RGBA)
	struct vec4 art_color_vec;
	vec4_set(&art_color_vec,
		((color >> 0) & 0xFF) / 255.0f,   // R
		((color >> 8) & 0xFF) / 255.0f,   // G
		((color >> 16) & 0xFF) / 255.0f,  // B
		opacity                             // A
	);
	
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *color_param = gs_effect_get_param_by_name(solid, "color");
	
	// Push blend state - use soft blending for smooth integration
	gs_blend_state_push();
	gs_enable_color(true, true, true, true);
	gs_enable_blending(true);
	// Use soft additive blending for better background integration
	gs_blend_function_separate(
		GS_BLEND_SRCALPHA, GS_BLEND_ONE,      // RGB: soft additive
		GS_BLEND_ZERO, GS_BLEND_ONE           // Alpha: preserve
	);
	
	gs_matrix_push();
	gs_matrix_translate3f(x, y, 0.0f);
	
	switch (effect) {
		case ART_PARTICLES: {
			// Floating particle system with smooth radial gradient falloff
			int num_particles = (int)(60 * intensity);
			float base_particle_size = 4.0f * intensity;
			
			for (int i = 0; i < num_particles; i++) {
				// Create pseudo-random but deterministic particle positions
				float seed = (float)i * 12.9898f;
				float px = fmodf(sinf(seed) * 43758.5453f, width);
				float py = fmodf(cosf(seed * 1.414f) * 43758.5453f, height);
				
				// Smooth floating animation with ease curves
				float speed = 0.3f + fmodf(sinf(seed * 2.718f), 0.4f);
				float drift_phase = animation_offset * 0.3f + seed;
				float drift = sinf(drift_phase) * 25.0f * cosf(drift_phase * 0.7f);
				py = fmodf(py - animation_offset * speed * 8.0f + height, height);
				px = px + drift;
				
				// Smooth pulsating with sine easing
				float pulse_phase = animation_offset * 1.5f + seed * 3.14159f;
				float pulse = (sinf(pulse_phase) * 0.4f + 0.6f);
				
				// Varying particle sizes for depth
				float size_variation = 0.7f + sinf(seed * 7.0f) * 0.5f;
				float particle_size = base_particle_size * size_variation;
				
				// Multi-layer radial gradient for smooth glow
				while (gs_effect_loop(solid, "Solid")) {
					for (int layer = 0; layer < 5; layer++) {
						float layer_size = particle_size * (1.0f + layer * 0.6f);
						float layer_alpha = pulse * opacity * powf(1.0f - (float)layer / 5.0f, 2.0f);
						
						struct vec4 particle_color = art_color_vec;
						particle_color.w = layer_alpha * 0.25f; // Softer overall
						gs_effect_set_vec4(color_param, &particle_color);
						
						// Draw as circle approximation with more segments for smoothness
						gs_render_start(true);
						int segments = 8;
						for (int s = 0; s <= segments; s++) {
							float angle = ((float)s / (float)segments) * 2.0f * (float)M_PI;
							float cx = px + cosf(angle) * layer_size;
							float cy = py + sinf(angle) * layer_size;
							gs_vertex2f(px, py);
							gs_vertex2f(cx, cy);
						}
						gs_render_stop(GS_TRISTRIP);
					}
				}
			}
			break;
		}
		
		case ART_LIGHT_RAYS: {
			// Diagonal light rays with smooth gradient falloff
			int num_rays = (int)(10 * intensity);
			float base_ray_width = 35.0f * intensity;
			
			for (int i = 0; i < num_rays; i++) {
				// Distribute rays evenly across angle range
				float angle = ((float)i / (float)num_rays) * (float)M_PI * 0.6f - (float)M_PI * 0.1f;
				
				// Smooth, slower animation
				float offset = fmodf(animation_offset * 15.0f, 250.0f);
				float start_dist = -250.0f + offset + i * 70.0f;
				
				// Calculate ray direction
				float dx = cosf(angle + (float)M_PI * 0.25f);
				float dy = sinf(angle + (float)M_PI * 0.25f);
				
				// Ray fades smoothly along its length with exponential falloff
				int num_segments = 8;
				for (int seg = 0; seg < num_segments; seg++) {
					float t1 = (float)seg / (float)num_segments;
					float t2 = (float)(seg + 1) / (float)num_segments;
					
					float dist1 = start_dist + t1 * 400.0f;
					float dist2 = start_dist + t2 * 400.0f;
					
					// Exponential fade for smoother appearance
					float alpha1 = opacity * 0.2f * powf(1.0f - t1, 1.5f);
					float alpha2 = opacity * 0.2f * powf(1.0f - t2, 1.5f);
					
					// Ray width also tapers for more natural look
					float width1 = base_ray_width * (1.0f - t1 * 0.3f);
					float width2 = base_ray_width * (1.0f - t2 * 0.3f);
					
					// Draw ray with soft edges (center bright, edges fade)
					while (gs_effect_loop(solid, "Solid")) {
						// Center core of ray
						struct vec4 ray_color_center = art_color_vec;
						ray_color_center.w = alpha1 * 1.5f;
						gs_effect_set_vec4(color_param, &ray_color_center);
						
						gs_render_start(true);
						gs_vertex2f(dist1 * dx, dist1 * dy);
						gs_vertex2f(dist2 * dx, dist2 * dy);
						
						// Soft outer glow layers
						for (int edge = 0; edge < 2; edge++) {
							float edge_mult = 1.0f + (edge + 1) * 0.7f;
							float edge_alpha1 = alpha1 / (edge + 2);
							float edge_alpha2 = alpha2 / (edge + 2);
							
							struct vec4 edge_color1 = art_color_vec;
							struct vec4 edge_color2 = art_color_vec;
							edge_color1.w = edge_alpha1;
							edge_color2.w = edge_alpha2;
							
							gs_effect_set_vec4(color_param, &edge_color1);
							gs_vertex2f(dist1 * dx - width1 * edge_mult * dy, dist1 * dy + width1 * edge_mult * dx);
							gs_effect_set_vec4(color_param, &edge_color2);
							gs_vertex2f(dist2 * dx - width2 * edge_mult * dy, dist2 * dy + width2 * edge_mult * dx);
						}
						gs_render_stop(GS_TRISTRIP);
					}
				}
			}
			break;
		}
		
		case ART_BOKEH: {
			// Bokeh blur circles with soft radial gradient
			int num_bokeh = (int)(20 * intensity);
			
			for (int i = 0; i < num_bokeh; i++) {
				// Pseudo-random positions
				float seed = (float)i * 17.951f;
				float px = fmodf(sinf(seed) * 43758.5453f + width * 0.5f, width);
				float py = fmodf(cosf(seed * 1.732f) * 43758.5453f + height * 0.5f, height);
				
				// Very slow, smooth drift animation
				float drift_phase_x = animation_offset * 0.2f + seed;
				float drift_phase_y = animation_offset * 0.15f + seed * 1.5f;
				px += sinf(drift_phase_x) * 35.0f * cosf(drift_phase_x * 0.5f);
				py += cosf(drift_phase_y) * 25.0f * sinf(drift_phase_y * 0.7f);
				
				// Varying sizes for depth perception
				float base_size = (18.0f + sinf(seed * 3.14f) * 12.0f) * intensity;
				
				// Gentle pulsating with smooth sine
				float pulse_phase = animation_offset * 0.8f + seed;
				float pulse = (sinf(pulse_phase) * 0.25f + 0.75f);
				
				// Multi-layer soft bokeh with radial gradient
				while (gs_effect_loop(solid, "Solid")) {
					for (int layer = 0; layer < 4; layer++) {
						float layer_size = base_size * (1.0f + layer * 0.5f);
						float layer_alpha = pulse * opacity * 0.15f * powf(1.0f - (float)layer / 4.0f, 2.5f);
						
						struct vec4 bokeh_color = art_color_vec;
						bokeh_color.w = layer_alpha;
						gs_effect_set_vec4(color_param, &bokeh_color);
						
						// Draw smooth circle with more segments
						gs_render_start(true);
						int segments = 12;
						for (int j = 0; j <= segments; j++) {
							float angle = ((float)j / (float)segments) * 2.0f * (float)M_PI;
							float cx = px + cosf(angle) * layer_size;
							float cy = py + sinf(angle) * layer_size;
							gs_vertex2f(px, py);
							gs_vertex2f(cx, cy);
						}
						gs_render_stop(GS_TRISTRIP);
					}
				}
			}
			break;
		}
		
		case ART_SPARKLES: {
			// Sparkling stars with smooth twinkling and soft glow
			int num_sparkles = (int)(50 * intensity);
			
			for (int i = 0; i < num_sparkles; i++) {
				// Fixed sparkle positions
				float seed = (float)i * 23.140f;
				float px = fmodf(sinf(seed) * 43758.5453f + width * 0.5f, width);
				float py = fmodf(cosf(seed * 1.618f) * 43758.5453f + height * 0.5f, height);
				
				// Smooth twinkling with varied timing
				float twinkle_speed = 2.0f + sinf(seed) * 1.5f;
				float twinkle_phase = animation_offset * twinkle_speed + seed * 10.0f;
				float twinkle_raw = sinf(twinkle_phase) * 0.5f + 0.5f;
				// Smooth curve for elegant twinkling
				float twinkle = powf(twinkle_raw, 3.0f);
				
				float alpha = twinkle * opacity * 0.6f;
				
				if (alpha > 0.05f) {
					// Varying sparkle sizes
					float base_size = (2.0f + sinf(seed * 7.0f) * 1.5f) * intensity;
					float size = base_size * (0.5f + twinkle * 0.5f);
					
					// Draw star with soft glow layers
					while (gs_effect_loop(solid, "Solid")) {
						for (int layer = 0; layer < 3; layer++) {
							float layer_mult = 1.0f + layer * 0.8f;
							float layer_alpha = alpha * powf(1.0f - (float)layer / 3.0f, 2.0f);
							
							struct vec4 sparkle_color = art_color_vec;
							sparkle_color.w = layer_alpha * 0.4f;
							gs_effect_set_vec4(color_param, &sparkle_color);
							
							float cross_length = size * 3.0f * layer_mult;
							float cross_width = size * 0.4f * layer_mult;
							
							// Horizontal beam with soft taper
							gs_render_start(true);
							gs_vertex2f(px - cross_length, py - cross_width);
							gs_vertex2f(px + cross_length, py - cross_width);
							gs_vertex2f(px - cross_length, py + cross_width);
							gs_vertex2f(px + cross_length, py + cross_width);
							gs_render_stop(GS_TRISTRIP);
							
							// Vertical beam with soft taper
							gs_render_start(true);
							gs_vertex2f(px - cross_width, py - cross_length);
							gs_vertex2f(px + cross_width, py - cross_length);
							gs_vertex2f(px - cross_width, py + cross_length);
							gs_vertex2f(px + cross_width, py + cross_length);
							gs_render_stop(GS_TRISTRIP);
							
							// Add center bright point
							if (layer == 0) {
								float point_size = size * 1.5f;
								sparkle_color.w = alpha * 0.8f;
								gs_effect_set_vec4(color_param, &sparkle_color);
								
								gs_render_start(true);
								gs_vertex2f(px - point_size, py - point_size);
								gs_vertex2f(px + point_size, py - point_size);
								gs_vertex2f(px - point_size, py + point_size);
								gs_vertex2f(px + point_size, py + point_size);
								gs_render_stop(GS_TRISTRIP);
							}
						}
					}
				}
			}
			break;
		}
		
		case ART_GLOW_ORBS: {
			// Glowing orbs with smooth radial gradient and gentle motion
			int num_orbs = (int)(12 * intensity);
			
			for (int i = 0; i < num_orbs; i++) {
				// Pseudo-random positions
				float seed = (float)i * 31.415f;
				float base_px = fmodf(sinf(seed) * 43758.5453f + width * 0.5f, width);
				float base_py = fmodf(cosf(seed * 2.236f) * 43758.5453f + height * 0.5f, height);
				
				// Smooth floating animation with figure-eight pattern
				float orbit_radius = 35.0f;
				float orbit_speed = 0.3f + sinf(seed) * 0.2f;
				float phase = animation_offset * orbit_speed + seed;
				float px = base_px + cosf(phase) * orbit_radius + sinf(phase * 2.0f) * 15.0f;
				float py = base_py + sinf(phase * 1.3f) * orbit_radius * 0.8f;
				
				// Smooth pulsating with sine easing
				float pulse_phase = animation_offset * 1.0f + seed * 5.0f;
				float pulse = (sinf(pulse_phase) * 0.2f + 0.8f);
				
				// Varying orb sizes
				float base_size = (22.0f + sinf(seed * 5.0f) * 8.0f) * intensity;
				
				// Multi-layer soft glow with exponential falloff
				while (gs_effect_loop(solid, "Solid")) {
					for (int layer = 0; layer < 6; layer++) {
						float layer_size = base_size * (1.0f + layer * 0.5f);
						float layer_opacity = opacity * pulse * 0.12f * powf(1.0f - (float)layer / 6.0f, 2.5f);
						
						struct vec4 orb_color = art_color_vec;
						orb_color.w = layer_opacity;
						gs_effect_set_vec4(color_param, &orb_color);
						
						gs_render_start(true);
						int segments = 16;
						for (int j = 0; j <= segments; j++) {
							float angle = ((float)j / (float)segments) * 2.0f * (float)M_PI;
							float cx = px + cosf(angle) * layer_size;
							float cy = py + sinf(angle) * layer_size;
							gs_vertex2f(px, py);
							gs_vertex2f(cx, cy);
						}
						gs_render_stop(GS_TRISTRIP);
					}
				}
			}
			break;
		}
		
		case ART_LIGHT_STREAKS: {
			// Smooth horizontal light streaks with soft gradients
			int num_streaks = (int)(8 * intensity);
			
			for (int i = 0; i < num_streaks; i++) {
				// Staggered vertical positions
				float seed = (float)i * 19.739f;
				float py = (height / (float)num_streaks) * (float)i + sinf(seed) * 15.0f;
				
				// Smooth animation with ease
				float speed = 1.0f + sinf(seed) * 0.4f;
				float streak_pos = fmodf(animation_offset * speed * 25.0f + seed * 100.0f, width + 250.0f) - 125.0f;
				
				// Streak dimensions with variation
				float streak_length = 90.0f + sinf(seed * 3.0f) * 50.0f;
				float streak_height = 1.8f + sinf(seed * 7.0f) * 1.2f;
				
				// Smooth fade in/out at edges with smooth curve
				float streak_alpha = opacity * 0.5f;
				float fade_distance = 120.0f;
				if (streak_pos < 0.0f) {
					streak_alpha *= powf((fade_distance + streak_pos) / fade_distance, 2.0f);
				} else if (streak_pos > width - streak_length) {
					float over = streak_pos - (width - streak_length);
					streak_alpha *= powf(1.0f - over / fade_distance, 2.0f);
				}
				
				if (streak_alpha > 0.03f) {
					// Multi-layer gradient for ultra-smooth appearance
					while (gs_effect_loop(solid, "Solid")) {
						for (int layer = 0; layer < 4; layer++) {
							float layer_height = streak_height * (1.0f + layer * 0.8f);
							float layer_alpha = streak_alpha * powf(1.0f - (float)layer / 4.0f, 2.0f);
							
							struct vec4 streak_color = art_color_vec;
							streak_color.w = layer_alpha;
							gs_effect_set_vec4(color_param, &streak_color);
							
							// Draw streak with tapered ends
							gs_render_start(true);
							// Leading edge fade
							streak_color.w = layer_alpha * 0.1f;
							gs_effect_set_vec4(color_param, &streak_color);
							gs_vertex2f(streak_pos - 20.0f, py - layer_height);
							gs_vertex2f(streak_pos - 20.0f, py + layer_height);
							
							// Main bright body
							streak_color.w = layer_alpha;
							gs_effect_set_vec4(color_param, &streak_color);
							gs_vertex2f(streak_pos + streak_length * 0.3f, py - layer_height);
							gs_vertex2f(streak_pos + streak_length * 0.3f, py + layer_height);
							
							// Trailing edge fade
							streak_color.w = layer_alpha * 0.3f;
							gs_effect_set_vec4(color_param, &streak_color);
							gs_vertex2f(streak_pos + streak_length, py - layer_height);
							gs_vertex2f(streak_pos + streak_length, py + layer_height);
							gs_render_stop(GS_TRISTRIP);
						}
					}
				}
			}
			break;
		}
		
		case ART_SHIMMER: {
			// Smooth shimmering effect like light on water surface
			int num_waves = (int)(15 * intensity);
			
			for (int i = 0; i < num_waves; i++) {
				float seed = (float)i * 13.579f;
				float base_y = (height / (float)num_waves) * (float)i;
				
				// Smooth wave parameters
				float wave_frequency = 0.04f + sinf(seed) * 0.02f;
				float wave_amplitude = 12.0f + cosf(seed * 2.0f) * 8.0f;
				float wave_speed = 0.4f + sinf(seed * 3.0f) * 0.25f;
				
				// Multi-layer shimmer for depth
				while (gs_effect_loop(solid, "Solid")) {
					for (int layer = 0; layer < 3; layer++) {
						float layer_mult = 1.0f + layer * 0.3f;
						
						gs_render_start(true);
						for (float px = 0; px < width; px += 6.0f) {
							float wave_phase = px * wave_frequency + animation_offset * wave_speed + seed;
							float py = base_y + wave_amplitude * sinf(wave_phase) * layer_mult;
							
							// Smooth intensity variation with double sine for complexity
							float shimmer_phase1 = px * 0.08f + animation_offset * 1.5f + seed * 10.0f;
							float shimmer_phase2 = px * 0.05f - animation_offset * 1.0f + seed * 7.0f;
							float shimmer_raw = (sinf(shimmer_phase1) + sinf(shimmer_phase2)) * 0.5f;
							float shimmer_intensity = shimmer_raw * 0.5f + 0.5f;
							
							// Smooth alpha with layer falloff
							float layer_alpha = opacity * shimmer_intensity * 0.35f * powf(1.0f - (float)layer / 3.0f, 1.5f);
							
							struct vec4 shimmer_color = art_color_vec;
							shimmer_color.w = layer_alpha;
							gs_effect_set_vec4(color_param, &shimmer_color);
							
							// Variable thickness based on intensity
							float thickness = (1.5f + shimmer_intensity * 2.5f) * layer_mult;
							
							gs_vertex2f(px, py - thickness);
							gs_vertex2f(px, py + thickness);
						}
						gs_render_stop(GS_TRISTRIP);
					}
				}
			}
			break;
		}
		
		case ART_ENERGY_FLOW: {
			// Smooth flowing energy with pulsing gradients
			int num_flows = (int)(6 * intensity);
			
			for (int i = 0; i < num_flows; i++) {
				float seed = (float)i * 27.183f;
				float lane_y = (height / (float)num_flows) * (float)i + sinf(seed) * 8.0f;
				
				// Energy pulses traveling along flow lines
				int num_pulses = 2;
				for (int p = 0; p < num_pulses; p++) {
					float pulse_seed = seed + (float)p * 100.0f;
					float pulse_speed = 0.8f + sinf(pulse_seed) * 0.4f;
					float pulse_pos = fmodf(animation_offset * pulse_speed * 35.0f + pulse_seed * 200.0f, width + 180.0f) - 90.0f;
					
					// Pulse dimensions
					float pulse_length = 70.0f + sinf(pulse_seed * 2.0f) * 25.0f;
					float base_thickness = 2.5f * intensity;
					
					// Draw smooth gradient pulse with multiple layers
					int num_segments = 8;
					for (int seg = 0; seg < num_segments; seg++) {
						float t1 = (float)seg / (float)num_segments;
						float t2 = (float)(seg + 1) / (float)num_segments;
						
						float seg_start = pulse_pos + t1 * pulse_length;
						float seg_end = pulse_pos + t2 * pulse_length;
						
						// Smooth intensity curve - peaks in the middle, fades at ends
						float peak_pos = 0.3f; // Peak near front
						float intensity1 = 1.0f - powf(fabsf(t1 - peak_pos) / (1.0f - peak_pos), 1.5f);
						float intensity2 = 1.0f - powf(fabsf(t2 - peak_pos) / (1.0f - peak_pos), 1.5f);
						
						// Smooth sine wave perturbation
						float wave_phase1 = seg_start * 0.08f + animation_offset * 0.7f + seed;
						float wave_phase2 = seg_end * 0.08f + animation_offset * 0.7f + seed;
						float wave_offset1 = sinf(wave_phase1) * 6.0f;
						float wave_offset2 = sinf(wave_phase2) * 6.0f;
						
						// Multi-layer for soft glow
						while (gs_effect_loop(solid, "Solid")) {
							for (int layer = 0; layer < 3; layer++) {
								float layer_mult = 1.0f + layer * 0.6f;
								float layer_thickness = base_thickness * layer_mult;
								
								float alpha1 = opacity * intensity1 * 0.3f / (1.0f + layer * 0.5f);
								float alpha2 = opacity * intensity2 * 0.3f / (1.0f + layer * 0.5f);
								
								struct vec4 flow_color1 = art_color_vec;
								struct vec4 flow_color2 = art_color_vec;
								flow_color1.w = alpha1;
								flow_color2.w = alpha2;
								
								gs_render_start(true);
								gs_effect_set_vec4(color_param, &flow_color1);
								gs_vertex2f(seg_start, lane_y + wave_offset1 - layer_thickness);
								gs_vertex2f(seg_start, lane_y + wave_offset1 + layer_thickness);
								gs_effect_set_vec4(color_param, &flow_color2);
								gs_vertex2f(seg_end, lane_y + wave_offset2 - layer_thickness);
								gs_vertex2f(seg_end, lane_y + wave_offset2 + layer_thickness);
								gs_render_stop(GS_TRISTRIP);
							}
						}
					}
				}
			}
			break;
		}
		
		default:
			break;
	}
	
	gs_matrix_pop();
	
	// Restore blend state
	gs_blend_state_pop();
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
