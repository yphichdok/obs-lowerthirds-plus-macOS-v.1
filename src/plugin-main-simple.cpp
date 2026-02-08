/**
 * Lower Thirds Plus - OBS Studio Plugin
 * 
 * Author: Y Phic Hdok
 * Company: MTD Technologies
 * Version: 1.0
 * 
 * Professional lower thirds with tabbed profiles and modern animations
 */

#include <obs-module.h>
#include "lowerthirds-source-simple.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("LowerThirdsPlus", "en-US")

bool obs_module_load(void)
{
	blog(LOG_INFO, "LowerThirdsPlus v1.0 by Y Phic Hdok (MTD Technologies) - Plugin loaded");
	
	// Register the source type
	register_lowerthirds_source();
	
	return true;
}

void obs_module_unload(void)
{
	blog(LOG_INFO, "LowerThirdsPlus plugin unloaded");
}

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Professional lower thirds with 5 tabbed profiles, 13 animation styles, and extensive customization. Created by Y Phic Hdok, MTD Technologies.";
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return "Lower Thirds Plus";
}

MODULE_EXPORT const char *obs_module_author(void)
{
	return "Y Phic Hdok, MTD Technologies";
}
