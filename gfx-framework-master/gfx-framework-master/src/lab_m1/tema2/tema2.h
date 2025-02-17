#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Tema2/camera.h"


namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        struct ViewportArea
        {
            ViewportArea() : x(0), y(0), width(1), height(1) {}
            ViewportArea(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        struct Package {
            glm::vec3 position; // pozitia pachetului
            bool isPickedUp; // daca pachetul a fost ridicat de drona
            bool isVisible; // daca pachetul este vizibil
        };

        struct Destination {
            glm::vec3 position; // pozitia destinatiei
            bool isVisible; // daca destinatia este vizibila
            float radius; // raza destinatiei
        };

        Tema2();
        ~Tema2();

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
        void RenderDrone();
        void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix);
        Mesh* CreateTerrain(const std::string& name, int gridSize);
        Mesh* CreateTree(const std::string& name);
        Mesh* CreateBuilding(const std::string& name);
        void CheckDroneGroundCollision();
        bool CollisionWithTree(const glm::vec3& center1, float radius1, const glm::vec3& center2, float radius2);
        bool CollisionWithBuilding(const glm::vec3& sphereCenter, float radius, const glm::vec3& boxMin, const glm::vec3& boxMax);
        void UpdatePackageSystem(float deltaTimeSeconds);
        void RenderPackageSystem();
        bool IsValidLocation(const glm::vec3& position, float radius);
        void RenderScene(const glm::mat4& viewMatrix, float deltaTimeSeconds);
        void RenderDirectionArrow();
        glm::vec3 GenerateValidPosition();
        Mesh* CreateCloud(const std::string& name);
        void UpdateClouds(float deltaTimeSeconds);
    protected:
        glm::mat4 modelMatrix;
        float translateX, translateY, translateZ;
        float scaleX, scaleY, scaleZ;
        float angularStepOX, angularStepOY, angularStepOZ;
        GLenum polygonMode;
        ViewportArea miniViewportArea;
        glm::mat4 projectionMatrix;
        bool hasPickedUpPackage; // pt afisarea mesajului de colectare a pachetului

        // drona
        class Drone {
        public:
            glm::vec3 position; // pozitia dronei
            glm::vec3 rotation; // rotatia dronei pe Ox, Oy, Oz
            glm::vec3 scale; // dimensiunea dronei
            float bladeSpeed; // viteza de rotatie a elicei
            float bladeAngle; // unghiul de rotatie al elicei
            glm::vec3 droneColor; // culoarea dronei
            glm::vec3 bladeColor; // culoarea elicei

            Drone() {
                position = glm::vec3(0, 2, 0);
                rotation = glm::vec3(0);
                scale = glm::vec3(1);
                bladeSpeed = 10.0f;
                bladeAngle = 0;
                droneColor = glm::vec3(0.7f, 0.7f, 0.7f); // gri
                bladeColor = glm::vec3(0.1f, 0.1f, 0.1f); // negru
            }

            void UpdateBlade(float deltaTime) {
                bladeAngle += deltaTime * bladeSpeed; // elicele se rotesc constant
            }
        };
        Drone* drone;

        implemented::Camera* camera;
        std::vector<glm::vec3> treePositions;
        std::vector<float> treeSizes;
        std::vector<glm::vec3> buildingPositions;
        std::vector<glm::vec3> buildingSizes;

        Package* currentPackage;
        Destination* currentDestination;
        int collectedPackages; // contor pentru pachetele colectate

        struct Cloud {
            glm::vec3 position; // pozitia norului
            glm::vec3 scale; // dimensiunea norului
            float speed; // viteza de deplasare a norului
            float rotationAngle; // unghiul de rotatie al norului
        };

        std::vector<Cloud> clouds;
        const int NUM_CLOUDS = 20; // numarul de nori
        const float CLOUD_HEIGHT = 20.0f; // inaltimea la care se randeaza norii
        const float CLOUD_AREA = 100.0f; // suprafata in care se deplaseaza norii
    };
}   // namespace m1

