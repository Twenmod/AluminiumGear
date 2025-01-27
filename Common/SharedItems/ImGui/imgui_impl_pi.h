// dear imgui: Platform Backend for PiTmpl
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)

//Modified from imgui_impl_glfw

#pragma once
#include "IInput.h"
#include "ImGui/imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE


IMGUI_IMPL_API bool     ImGui_ImplPi_InitForOpenGL();

IMGUI_IMPL_API void     ImGui_ImplPi_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplPi_NewFrame();

// GLFW callbacks (individual callbacks to call yourself if you didn't install callbacks)
IMGUI_IMPL_API void     ImGui_ImplPi_WindowFocusCallback(int focused);        // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplPi_CursorEnterCallback(int entered);        // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplPi_CursorPosCallback(double x, double y);   // Since 1.87
IMGUI_IMPL_API void     ImGui_ImplPi_MouseButtonCallback(int button, bool down);
IMGUI_IMPL_API void     ImGui_ImplPi_ScrollCallback(double xoffset, double yoffset);
IMGUI_IMPL_API void     ImGui_ImplPi_KeyCallback(real::Key key, int scancode, bool value);
IMGUI_IMPL_API void     ImGui_ImplPi_CharCallback(unsigned int c);

#endif // #ifndef IMGUI_DISABLE
