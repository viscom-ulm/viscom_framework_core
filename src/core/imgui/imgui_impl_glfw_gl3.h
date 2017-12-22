// ImGui GLFW binding with OpenGL3 + shaders
// In this binding, ImTextureID is used to store an OpenGL 'GLuint' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include <glm/fwd.hpp>
#include <imgui.h>

struct GLFWwindow;

IMGUI_API bool        ImGui_ImplGlfwGL3_Init(GLFWwindow* window, bool drawCursor);
IMGUI_API void        ImGui_ImplGlfwGL3_Shutdown();
IMGUI_API void        ImGui_ImplGlfwGL3_NewFrame(const glm::ivec2& viewportOrigin, const glm::ivec2& viewportSize, const glm::vec2& scaling, double currentTime, double deltaTime);
IMGUI_API void        ImGui_ImplGlfwGL3_FinishAllFrames();

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void        ImGui_ImplGlfwGL3_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_ImplGlfwGL3_CreateDeviceObjects();

// GLFW callbacks (installed by default if you enable 'install_callbacks' during initialization)
// Provided here if you want to chain callbacks.
// You can also handle inputs yourself and use those as a reference.
IMGUI_API void        ImGui_ImplGlfwGL3_MouseButtonCallback(int button, int action, int mods);
IMGUI_API void        ImGui_ImplGlfwGL3_MousePositionCallback(double x, double y);
IMGUI_API void        ImGui_ImplGlfwGL3_ScrollCallback(double xoffset, double yoffset);
IMGUI_API void        ImGui_ImplGlfwGL3_KeyCallback(int key, int scancode, int action, int mods);
IMGUI_API void        ImGui_ImplGlfwGL3_CharCallback(unsigned int c);
