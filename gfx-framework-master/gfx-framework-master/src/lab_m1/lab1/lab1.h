#pragma once

#include "components/simple_scene.h"


namespace m1
{
    class Lab1 : public gfxc::SimpleScene
    {
    public:
        Lab1();
        ~Lab1();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        // TODO(student): Class variables go here
        glm::vec4 colorVecInit = glm::vec4(0, 0, 0, 1);
        glm::vec4 colorVec = glm::vec4(0.2f, 0.4f, 0.29f, 1);

        glm::vec3 cubePos = glm::vec3(1, 0.5f, 0);

        Mesh* m;
    };
}