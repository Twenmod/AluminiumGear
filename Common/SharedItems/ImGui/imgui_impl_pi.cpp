// dear imgui: Platform Backend for PiTmpl
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)

//Modified from imgui_impl_glfw
//However almost everything is deleted since it was simpler than expected



#include "precomp.h"



#ifndef IMGUI_DISABLE
#include "imgui_impl_pi.h"

struct ImGui_ImplPi_Data
{
    double                  Time;
    ImVec2                  LastValidMousePos;
    bool                    InstalledCallbacks;
    bool                    CallbacksChainForAllWindows;
    ImGui_ImplPi_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// - Because glfwPollEvents() process all windows and some events may be called outside of it, you will need to register your own callbacks
//   (passing install_callbacks=false in ImGui_ImplPi_InitXXX functions), set the current dear imgui context and then call our callbacks.
// - Otherwise we may need to store a GLFWWindow* -> ImGuiContext* map and handle this in the backend, adding a little bit of extra complexity to it.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
static ImGui_ImplPi_Data* ImGui_ImplPi_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplPi_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

// Functions

// Not static to allow third-party code to use that if they want to (but undocumented)
ImGuiKey ImGui_ImplPi_KeyToImGuiKey(real::Key keycode, int scancode);
ImGuiKey ImGui_ImplPi_KeyToImGuiKey(real::Key keycode, int scancode)
{
    IM_UNUSED(scancode);
    switch (keycode)
    {
		//Wont be using text input so only filling in usefull stuff (numbers and enter)

		case real::Key::TAB : return ImGuiKey_Tab;
  //      case real::Key::ARROW_LEFT : return ImGuiKey_LeftArrow;
  //      case real::Key::ARROW_RIGHT : return ImGuiKey_RightArrow;
  //      case real::Key::ARROW_UP : return ImGuiKey_UpArrow;
  //      case real::Key:: : return ImGuiKey_DownArrow;
  //      case real::Key:: : return ImGuiKey_PageUp;
  //      case real::Key:: : return ImGuiKey_PageDown;
  //      case real::Key:: : return ImGuiKey_Home;
  //      case real::Key:: : return ImGuiKey_End;
  //      case real::Key:: : return ImGuiKey_Insert;
  //      case real::Key:: : return ImGuiKey_Delete;
//    	case real::Key:: : return ImGuiKey_Backspace;
        case real::Key::SPACE : return ImGuiKey_Space;
        case real::Key::ENTER : return ImGuiKey_Enter;
  //      case real::Key:: : return ImGuiKey_Escape;
  //      case real::Key:: : return ImGuiKey_Apostrophe;
  //      case real::Key:: : return ImGuiKey_Comma;
  //      case real::Key:: : return ImGuiKey_Minus;
  //      case real::Key:: : return ImGuiKey_Period;
  //      case real::Key:: : return ImGuiKey_Slash;
  //      case real::Key:: : return ImGuiKey_Semicolon;
  //      case real::Key:: : return ImGuiKey_Equal;
  //      case real::Key:: : return ImGuiKey_LeftBracket;
  //      case real::Key:: : return ImGuiKey_Backslash;
  //      case real::Key:: : return ImGuiKey_RightBracket;
  //      case real::Key:: : return ImGuiKey_GraveAccent;
  //      case real::Key:: : return ImGuiKey_CapsLock;
  //      case real::Key:: : return ImGuiKey_ScrollLock;
  //      case real::Key:: : return ImGuiKey_NumLock;
  //      case real::Key:: : return ImGuiKey_PrintScreen;
  //      case real::Key:: : return ImGuiKey_Pause;
  //      case real::Key:: : return ImGuiKey_Keypad0;
  //      case real::Key:: : return ImGuiKey_Keypad1;
  //      case real::Key:: : return ImGuiKey_Keypad2;
  //      case real::Key:: : return ImGuiKey_Keypad3;
  //      case real::Key:: : return ImGuiKey_Keypad4;
  //      case real::Key:: : return ImGuiKey_Keypad5;
  //      case real::Key:: : return ImGuiKey_Keypad6;
  //      case real::Key:: : return ImGuiKey_Keypad7;
  //      case real::Key:: : return ImGuiKey_Keypad8;
  //      case real::Key:: : return ImGuiKey_Keypad9;
  //      case real::Key:: : return ImGuiKey_KeypadDecimal;
  //      case real::Key:: : return ImGuiKey_KeypadDivide;
  //      case real::Key:: : return ImGuiKey_KeypadMultiply;
  //      case real::Key:: : return ImGuiKey_KeypadSubtract;
  //      case real::Key:: : return ImGuiKey_KeypadAdd;
  //      case real::Key:: : return ImGuiKey_KeypadEnter;
  //      case real::Key:: : return ImGuiKey_KeypadEqual;
  //      case real::Key:: : return ImGuiKey_LeftShift;
  //      case real::Key:: : return ImGuiKey_LeftCtrl;
  //      case real::Key:: : return ImGuiKey_LeftAlt;
  //      case real::Key:: : return ImGuiKey_LeftSuper;
  //      case real::Key:: : return ImGuiKey_RightShift;
  //      case real::Key:: : return ImGuiKey_RightCtrl;
  //      case real::Key:: : return ImGuiKey_RightAlt;
  //      case real::Key:: : return ImGuiKey_RightSuper;
  //      case real::Key:: : return ImGuiKey_Menu;
		case real::Key::NUM_0 : return ImGuiKey_0;
        case real::Key::NUM_1 : return ImGuiKey_1;
        case real::Key::NUM_2 : return ImGuiKey_2;
        case real::Key::NUM_3 : return ImGuiKey_3;
        case real::Key::NUM_4 : return ImGuiKey_4;
        case real::Key::NUM_5 : return ImGuiKey_5;
        case real::Key::NUM_6 : return ImGuiKey_6;
        case real::Key::NUM_7 : return ImGuiKey_7;
        case real::Key::NUM_8 : return ImGuiKey_8;
        case real::Key::NUM_9 : return ImGuiKey_9;
  //      case real::Key:: : return ImGuiKey_A;
  //      case real::Key:: : return ImGuiKey_B;
  //      case real::Key:: : return ImGuiKey_C;
  //      case real::Key:: : return ImGuiKey_D;
  //      case real::Key:: : return ImGuiKey_E;
  //      case real::Key:: : return ImGuiKey_F;
  //      case real::Key:: : return ImGuiKey_G;
  //      case real::Key:: : return ImGuiKey_H;
  //      case real::Key:: : return ImGuiKey_I;
  //      case real::Key:: : return ImGuiKey_J;
  //      case real::Key:: : return ImGuiKey_K;
  //      case real::Key:: : return ImGuiKey_L;
  //      case real::Key:: : return ImGuiKey_M;
  //      case real::Key:: : return ImGuiKey_N;
  //      case real::Key:: : return ImGuiKey_O;
  //      case real::Key:: : return ImGuiKey_P;
  //      case real::Key:: : return ImGuiKey_Q;
  //      case real::Key:: : return ImGuiKey_R;
  //      case real::Key:: : return ImGuiKey_S;
  //      case real::Key:: : return ImGuiKey_T;
  //      case real::Key:: : return ImGuiKey_U;
  //      case real::Key:: : return ImGuiKey_V;
  //      case real::Key:: : return ImGuiKey_W;
  //      case real::Key:: : return ImGuiKey_X;
  //      case real::Key:: : return ImGuiKey_Y;
  //      case real::Key:: : return ImGuiKey_Z;
  //      case real::Key:: : return ImGuiKey_F1;
  //      case real::Key:: : return ImGuiKey_F2;
  //      case real::Key:: : return ImGuiKey_F3;
  //      case real::Key:: : return ImGuiKey_F4;
  //      case real::Key:: : return ImGuiKey_F5;
  //      case real::Key:: : return ImGuiKey_F6;
  //      case real::Key:: : return ImGuiKey_F7;
  //      case real::Key:: : return ImGuiKey_F8;
  //      case real::Key:: : return ImGuiKey_F9;
  //      case real::Key:: : return ImGuiKey_F10;
  //      case real::Key:: : return ImGuiKey_F11;
  //      case real::Key:: : return ImGuiKey_F12;
  //      case real::Key:: : return ImGuiKey_F13;
  //      case real::Key:: : return ImGuiKey_F14;
  //      case real::Key:: : return ImGuiKey_F15;
  //      case real::Key:: : return ImGuiKey_F16;
  //      case real::Key:: : return ImGuiKey_F17;
  //      case real::Key:: : return ImGuiKey_F18;
  //      case real::Key:: : return ImGuiKey_F19;
  //      case real::Key:: : return ImGuiKey_F20;
  //      case real::Key:: : return ImGuiKey_F21;
  //      case real::Key:: : return ImGuiKey_F22;
  //      case real::Key:: : return ImGuiKey_F23;
  //      case real::Key:: : return ImGuiKey_F24;
        default: return ImGuiKey_None;
    }
}

//Dont care about modifiers

//// X11 does not include current pressed/released modifier key in 'mods' flags submitted by GLFW
//// See https://github.com/ocornut/imgui/issues/6034 and https://github.com/glfw/glfw/issues/1630
//static void ImGui_ImplPi_UpdateKeyModifiers(GLFWwindow* window)
//{
//    ImGuiIO& io = ImGui::GetIO();
//    io.AddKeyEvent(ImGuiMod_Ctrl,  (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
//    io.AddKeyEvent(ImGuiMod_Shift, (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)   == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)   == GLFW_PRESS));
//    io.AddKeyEvent(ImGuiMod_Alt,   (glfwGetKey(window, GLFW_KEY_LEFT_ALT)     == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT)     == GLFW_PRESS));
//    io.AddKeyEvent(ImGuiMod_Super, (glfwGetKey(window, GLFW_KEY_LEFT_SUPER)   == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER)   == GLFW_PRESS));
//}


void ImGui_ImplPi_MouseButtonCallback(int button, bool down)
{
	//Doesnt seem important
    //ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();
    //if (bd->PrevUserCallbackMousebutton != nullptr && ImGui_ImplPi_ShouldChainCallback(window))
    //    bd->PrevUserCallbackMousebutton(window, button, action, mods);

    //ImGui_ImplPi_UpdateKeyModifiers(window);

    ImGuiIO& io = ImGui::GetIO();
    if (button >= 0 && button < ImGuiMouseButton_COUNT)
        io.AddMouseButtonEvent(button, down);
}

void ImGui_ImplPi_ScrollCallback(double xoffset, double yoffset)
{
    //ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
}


void ImGui_ImplPi_KeyCallback(real::Key keycode, int scancode, bool value)
{
    //ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();

    ImGuiIO& io = ImGui::GetIO();
    ImGuiKey imgui_key = ImGui_ImplPi_KeyToImGuiKey(keycode, scancode);
    io.AddKeyEvent(imgui_key, value);
    //io.SetKeyEventNativeData(imgui_key, imgui_key, scancode); // To support legacy indexing (<1.87 user code)
}

void ImGui_ImplPi_WindowFocusCallback(int focused)
{
    //ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();

    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(focused != 0);
}

void ImGui_ImplPi_CursorPosCallback(double x, double y)
{
    ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
    bd->LastValidMousePos = ImVec2((float)x, (float)y);
}

//This seems useless with docked windows
void ImGui_ImplPi_CursorEnterCallback(int)
{

}

void ImGui_ImplPi_CharCallback(unsigned int c)
{
    //ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

//Dont care

//void ImGui_ImplPi_InstallCallbacks(GLFWwindow* window)
//{
//    ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();
//    IM_ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
//    IM_ASSERT(bd->Window == window);
//
//    bd->PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, ImGui_ImplPi_WindowFocusCallback);
//    bd->PrevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, ImGui_ImplPi_CursorEnterCallback);
//    bd->PrevUserCallbackCursorPos = glfwSetCursorPosCallback(window, ImGui_ImplPi_CursorPosCallback);
//    bd->PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplPi_MouseButtonCallback);
//    bd->PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplPi_ScrollCallback);
//    bd->PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplPi_KeyCallback);
//    bd->PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplPi_CharCallback);
//    bd->PrevUserCallbackMonitor = glfwSetMonitorCallback(ImGui_ImplPi_MonitorCallback);
//    bd->InstalledCallbacks = true;
//}
//
//void ImGui_ImplPi_RestoreCallbacks(GLFWwindow* window)
//{
//    ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();
//    IM_ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
//    IM_ASSERT(bd->Window == window);
//
//    glfwSetWindowFocusCallback(window, bd->PrevUserCallbackWindowFocus);
//    glfwSetCursorEnterCallback(window, bd->PrevUserCallbackCursorEnter);
//    glfwSetCursorPosCallback(window, bd->PrevUserCallbackCursorPos);
//    glfwSetMouseButtonCallback(window, bd->PrevUserCallbackMousebutton);
//    glfwSetScrollCallback(window, bd->PrevUserCallbackScroll);
//    glfwSetKeyCallback(window, bd->PrevUserCallbackKey);
//    glfwSetCharCallback(window, bd->PrevUserCallbackChar);
//    glfwSetMonitorCallback(bd->PrevUserCallbackMonitor);
//    bd->InstalledCallbacks = false;
//    bd->PrevUserCallbackWindowFocus = nullptr;
//    bd->PrevUserCallbackCursorEnter = nullptr;
//    bd->PrevUserCallbackCursorPos = nullptr;
//    bd->PrevUserCallbackMousebutton = nullptr;
//    bd->PrevUserCallbackScroll = nullptr;
//    bd->PrevUserCallbackKey = nullptr;
//    bd->PrevUserCallbackChar = nullptr;
//    bd->PrevUserCallbackMonitor = nullptr;
//}

// Set to 'true' to enable chaining installed callbacks for all windows (including secondary viewports created by backends or by user.
// This is 'false' by default meaning we only chain callbacks for the main viewport.
// We cannot set this to 'true' by default because user callbacks code may be not testing the 'window' parameter of their callback.
// If you set this to 'true' your user callback code will need to make sure you are testing the 'window' parameter.
void ImGui_ImplPi_SetCallbacksChainForAllWindows(bool chain_for_all_windows)
{
    ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();
    bd->CallbacksChainForAllWindows = chain_for_all_windows;
}

static bool ImGui_ImplPi_Init()
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    //printf("GLFW_VERSION: %d.%d.%d (%d)", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION, GLFW_VERSION_COMBINED);

    // Setup backend capabilities flags
    ImGui_ImplPi_Data* bd = IM_NEW(ImGui_ImplPi_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_pi";

    bd->Time = 0.0;

    //ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

    // Set platform dependent data in viewport
    //ImGuiViewport* main_viewport = ImGui::GetMainViewport();
   
	//Maybe important???
	//main_viewport->PlatformHandle = (void*)bd->Window;

    return true;
}

bool ImGui_ImplPi_InitForOpenGL()
{
    return ImGui_ImplPi_Init();
}


void ImGui_ImplPi_Shutdown()
{
    ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = nullptr;
	io.BackendPlatformUserData = nullptr;
    IM_DELETE(bd);
}

//static void ImGui_ImplPi_UpdateMouseCursor()
//{
//    ImGuiIO& io = ImGui::GetIO();
//    ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();
//    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(bd->Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
//        return;
//
//    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
//    // (those braces are here to reduce diff with multi-viewports support in 'docking' branch)
//    {
//        GLFWwindow* window = bd->Window;
//        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
//        {
//            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
//            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//        }
//        else
//        {
//            // Show OS mouse cursor
//            // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
//            glfwSetCursor(window, bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow]);
//            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//        }
//    }
//}
//
// 
// Gamepads?!?!?!?! no way
// 
// 
//// Update gamepad inputs
//static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v; }
//static void ImGui_ImplPi_UpdateGamepads()
//{
//    ImGuiIO& io = ImGui::GetIO();
//    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0) // FIXME: Technically feeding gamepad shouldn't depend on this now that they are regular inputs.
//        return;
//
//    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
//#if GLFW_HAS_GAMEPAD_API && !defined(EMSCRIPTEN_USE_EMBEDDED_GLFW3)
//    GLFWgamepadstate gamepad;
//    if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
//        return;
//    #define MAP_BUTTON(KEY_NO, BUTTON_NO, _UNUSED)          do { io.AddKeyEvent(KEY_NO, gamepad.buttons[BUTTON_NO] != 0); } while (0)
//    #define MAP_ANALOG(KEY_NO, AXIS_NO, _UNUSED, V0, V1)    do { float v = gamepad.axes[AXIS_NO]; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
//#else
//    int axes_count = 0, buttons_count = 0;
//    const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
//    const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
//    if (axes_count == 0 || buttons_count == 0)
//        return;
//    #define MAP_BUTTON(KEY_NO, _UNUSED, BUTTON_NO)          do { io.AddKeyEvent(KEY_NO, (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS)); } while (0)
//    #define MAP_ANALOG(KEY_NO, _UNUSED, AXIS_NO, V0, V1)    do { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
//#endif
//    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
//    MAP_BUTTON(ImGuiKey_GamepadStart,       GLFW_GAMEPAD_BUTTON_START,          7);
//    MAP_BUTTON(ImGuiKey_GamepadBack,        GLFW_GAMEPAD_BUTTON_BACK,           6);
//    MAP_BUTTON(ImGuiKey_GamepadFaceLeft,    GLFW_GAMEPAD_BUTTON_X,              2);     // Xbox X, PS Square
//    MAP_BUTTON(ImGuiKey_GamepadFaceRight,   GLFW_GAMEPAD_BUTTON_B,              1);     // Xbox B, PS Circle
//    MAP_BUTTON(ImGuiKey_GamepadFaceUp,      GLFW_GAMEPAD_BUTTON_Y,              3);     // Xbox Y, PS Triangle
//    MAP_BUTTON(ImGuiKey_GamepadFaceDown,    GLFW_GAMEPAD_BUTTON_A,              0);     // Xbox A, PS Cross
//    MAP_BUTTON(ImGuiKey_GamepadDpadLeft,    GLFW_GAMEPAD_BUTTON_DPAD_LEFT,      13);
//    MAP_BUTTON(ImGuiKey_GamepadDpadRight,   GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,     11);
//    MAP_BUTTON(ImGuiKey_GamepadDpadUp,      GLFW_GAMEPAD_BUTTON_DPAD_UP,        10);
//    MAP_BUTTON(ImGuiKey_GamepadDpadDown,    GLFW_GAMEPAD_BUTTON_DPAD_DOWN,      12);
//    MAP_BUTTON(ImGuiKey_GamepadL1,          GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,    4);
//    MAP_BUTTON(ImGuiKey_GamepadR1,          GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,   5);
//    MAP_ANALOG(ImGuiKey_GamepadL2,          GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,     4,      -0.75f,  +1.0f);
//    MAP_ANALOG(ImGuiKey_GamepadR2,          GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,    5,      -0.75f,  +1.0f);
//    MAP_BUTTON(ImGuiKey_GamepadL3,          GLFW_GAMEPAD_BUTTON_LEFT_THUMB,     8);
//    MAP_BUTTON(ImGuiKey_GamepadR3,          GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,    9);
//    MAP_ANALOG(ImGuiKey_GamepadLStickLeft,  GLFW_GAMEPAD_AXIS_LEFT_X,           0,      -0.25f,  -1.0f);
//    MAP_ANALOG(ImGuiKey_GamepadLStickRight, GLFW_GAMEPAD_AXIS_LEFT_X,           0,      +0.25f,  +1.0f);
//    MAP_ANALOG(ImGuiKey_GamepadLStickUp,    GLFW_GAMEPAD_AXIS_LEFT_Y,           1,      -0.25f,  -1.0f);
//    MAP_ANALOG(ImGuiKey_GamepadLStickDown,  GLFW_GAMEPAD_AXIS_LEFT_Y,           1,      +0.25f,  +1.0f);
//    MAP_ANALOG(ImGuiKey_GamepadRStickLeft,  GLFW_GAMEPAD_AXIS_RIGHT_X,          2,      -0.25f,  -1.0f);
//    MAP_ANALOG(ImGuiKey_GamepadRStickRight, GLFW_GAMEPAD_AXIS_RIGHT_X,          2,      +0.25f,  +1.0f);
//    MAP_ANALOG(ImGuiKey_GamepadRStickUp,    GLFW_GAMEPAD_AXIS_RIGHT_Y,          3,      -0.25f,  -1.0f);
//    MAP_ANALOG(ImGuiKey_GamepadRStickDown,  GLFW_GAMEPAD_AXIS_RIGHT_Y,          3,      +0.25f,  +1.0f);
//    #undef MAP_BUTTON
//    #undef MAP_ANALOG
//}

//Not actually needed rn
void ImGui_ImplPi_NewFrame()
{
    //ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplPi_Data* bd = ImGui_ImplPi_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplPi_InitForXXX()?");

    // Setup display size (every frame to accommodate for window resizing)
	// 
	// Is this important????
	// 
    //int w, h;
    //int display_w, display_h; 
    //io.DisplaySize = ImVec2((float)w, (float)h);
    //if (w > 0 && h > 0)
    //    io.DisplayFramebufferScale = ImVec2((float)display_w / (float)w, (float)display_h / (float)h);
}


#endif // #ifndef IMGUI_DISABLE
