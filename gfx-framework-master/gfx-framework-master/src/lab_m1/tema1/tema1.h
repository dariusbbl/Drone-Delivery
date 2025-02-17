#pragma once

#include "components/simple_scene.h"

namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;
        std::vector<float> heightMap;
        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void RenderProjectileTrajectoryPreview(float startX, float startY, float angle, float magnitude);
        void DrawLineSegment(glm::vec2 p1, glm::vec2 p2, float angle);
        void DrawLineSegmentWithAngle(glm::vec2 p1, glm::vec2 p2);
        void RenderProjectile(float posX, float posY);
        void LaunchProjectile(float startX, float startY, float angle, float magnitude, int launcherId);
        bool CheckProjectileCollision(float posX, float posY);
        bool CheckCollision(float projX, float projY, float tankX, float tankY, float projRadius, float tankRadius);

        // Functii auxiliare pentru crearea elementelor grafice
        void CreateTank(float posX, float posY, float rotation, glm::vec3 colorBase, glm::vec3 colorTurret, glm::vec3 colorBarrel, float turretRotation);
        float Height(float x);
        void RenderTerrainMethod();
        void RenderLifeBar(float tankX, float tankY, int life);
        // Variabile membre pentru parametrizarea terenului si pozitia tancului
        float amplitude1, amplitude2, frequency1, frequency2;
        float tankPosX; // Pozitia pe axa x a tancului
        float scaleFactor = 30.0f; // factor de scalare(vizibilitate) pentru tank (de cate ori il maresc)
        float tankHeight = 0.9f * scaleFactor;
        float xOffset = 4.0f;
        float yOffset = 200.0f;
        float tankPosY; // Pozitia pe axa y a tancului
        float turretAngle = 0.0f; // unghiul de rotatie al tunului
        float heightMultiplier = 3.0f; // multiplicator pentru inaltimea terenului
        float barrelAngle = 0.0f; // unghiul de inclinare al tunului
        float barrelLength = 2.0f; // lungimea tunului

        // tank 2
        float tank2PosX;
        float tank2PosY;
        float tank2TurretAngle = glm::pi<float>(); // tank 2 sa fie orientat inspre tank 1 la initializare
        float tank2Angle = 0.0f;

        // variabile pentru controlul jocului
        int tank1Life = 4; // viata tanc 1, maxim 3 lovituri
        int tank2Life = 4; // viata tanc 2, maxim 3 lovituri
        bool tank1Active = true; // tank1 este in viata
        bool tank2Active = true; // tank2 este in viata
        const float projectileRadius = 5.0f; // raza cercului de coliziune pentru proiectil
        const float tankRadius = 30.0f;      // raza cercului de coliziune pentru tanc
        // structura de proiectil
        struct Projectile {
            float posX, posY;
            float velX, velY;
            bool active;
            int launcherId;
        };
        // vector de proiectile pentru a putea trasa mai multe proiectile simultan
        std::vector<Projectile> projectiles;
        //std::vector<Projectile> projectiles2;
        float gravity = 9.81f; // constanta de gravitatie
        float initialTankAngle; // unghiul initial al tancului

        // pentru deformarea terenului
        // structua pentru stocarea datelor de inaltime
        struct HeightPoint {
            float height;
            bool needsUpdate;
        };

        // vector pentru stocarea intaltimilor
        std::vector<std::vector<HeightPoint>> heightGrid;
        float deformationRadius = 100.0f; // raza de efect a exploziei
        float transferThreshold = 10.0f;  // pragul pentru transferul de inaltime
        float transferAmount = 5.0f;     // cantitatea de transfer

        void InitializeHeightGrid();
        void DeformTerrain(float posX, float posY);
        void UpdateTerrainSlide(float deltaTime);
        float GetInterpolatedHeight(float x);
        void SetHeight(int x, int y, float height);

        // pentru optimizarea memoriei
        Mesh* terrainMesh;
        Mesh* tankBaseMesh1;
        Mesh* tankBaseMesh2;
        Mesh* tankTurretMesh;
        Mesh* tankBarrelMesh;

        // functie pentru initializarea mesh-urilor tancului
        void InitializeTankMeshes(glm::vec3 colorBase, glm::vec3 colorTurret, glm::vec3 colorBarrel);
        void DeleteTankMeshes();
    };
}