#pragma once
#include <m2/sdl/Texture.h>
#include <string>

bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text);
std::optional<bool> ask_for_confirmation_with_cancellation(const std::string& question, const std::string& accept_text, const std::string& decline_text);
bool ask_for_confirmation_bottom(const std::string& question, const std::string& accept_text, const std::string& decline_text, m2::sdl::TextureUniquePtr background_texture);
