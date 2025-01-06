#define SK_GL

#include "seek_bar.h"

#include "GLFW/glfw3.h"
#include "include/core/SkSurface.h"

#include <cstring>
#include <iostream>
#include <cstdlib>

constexpr int windowWidth = 960;
constexpr int windowHeight = 640;
constexpr double moveOffset = 20.0;

void errorCallback(int error, const char* description) {
    std::cerr << "Error " << error << " occured: " << description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
    SeekBar* bar = reinterpret_cast<SeekBar*>(glfwGetWindowUserPointer(window));

    if (!bar) {
        std::cerr << "keyCallback: seek bar pointer after casting is null!" << std::endl;
        return;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        bar->updateCursorPosition(bar->getCursorX() + moveOffset);
    } else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        bar->updateCursorPosition(bar->getCursorX() - moveOffset);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, [[maybe_unused]] int mods) {
    SeekBar* bar = reinterpret_cast<SeekBar*>(glfwGetWindowUserPointer(window));

    if (!bar) {
        std::cerr << "mouseButtonCallback: seek bar pointer after casting is null!" << std::endl;
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            if (bar->isMouseWithinBar(xpos, ypos)) {
                bar->updateCursorPosition(xpos);
                bar->startCursorDragging();
            }
            bar->handleButtonClick(xpos, ypos);
        } else if (action == GLFW_RELEASE) {
            bar->stopCursorDragging();
        }
    }

    if (bar->isCursorDragging()) {
        bar->updateCursorPosition(xpos);
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    SeekBar* bar = reinterpret_cast<SeekBar*>(glfwGetWindowUserPointer(window));

    if (!bar) {
        std::cerr << "cursorPosCallback: seek bar pointer after casting is null!" << std::endl;
        return;
    }

    const auto cursorShape =
        bar->isCursorDragging() || bar->isMouseWithinBar(xpos, ypos) || bar->isMouseWithinIcons(xpos, ypos)
            ? GLFW_HAND_CURSOR
            : GLFW_ARROW_CURSOR;
    glfwSetCursor(window, glfwCreateStandardCursor(cursorShape));

    if (bar->isCursorDragging()) {
        bar->updateCursorPosition(xpos);
        bar->setHoverForChapter(xpos);
    } else if (bar->isMouseWithinBar(xpos, ypos)) {
        bar->setCursorVisibility(true);
        bar->setHoverForChapter(xpos);
    } else {
        bar->setCursorVisibility(false);
        bar->resetHover();
    }
}

void dropCallback(GLFWwindow* window, int count, const char** paths) {
    SeekBar* bar = reinterpret_cast<SeekBar*>(glfwGetWindowUserPointer(window));

    if (!bar) {
        std::cerr << "cursorPosCallback: seek bar pointer after casting is null!" << std::endl;
        return;
    }

    for (int i = 0; i < count; i++) {
        std::cout << "Dropped file: " << paths[i] << std::endl;
    }

    // Simulate indeterminate loading for few seconds
    if (!bar->isLoading()) {
        auto start = std::chrono::steady_clock::now();
        bar->load(start);
    }
}

void render(const sk_sp<SkSurface>& surface, GLFWwindow* window) {
    SeekBar* bar = reinterpret_cast<SeekBar*>(glfwGetWindowUserPointer(window));

    if (!bar) {
        std::cerr << "render: seek bar pointer after casting is null!" << std::endl;
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    bar->draw();

    SkImageInfo info = surface->imageInfo();
    SkPixmap pixmap;

    if (surface->peekPixels(&pixmap)) {
        glDrawPixels(info.width(), info.height(), GL_RGBA, GL_UNSIGNED_BYTE, pixmap.addr());
    }

    glfwSwapBuffers(window);
}

void initOpenGL() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(void) {
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Custom Seek Bar using Skia", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    initOpenGL();

    SkImageInfo imageInfo = SkImageInfo::Make(windowWidth, windowHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::Raster(imageInfo);

    SeekBar bar{surface->getCanvas(), windowWidth, windowHeight};
    glfwSetWindowUserPointer(window, &bar);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetDropCallback(window, dropCallback);

    while (!glfwWindowShouldClose(window)) {
        render(surface, window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
