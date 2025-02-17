#include "lab_m1/Tema1/tema1.h"
#include <vector>
#include <iostream>
#include <cmath>

using namespace std;
using namespace m1;

Tema1::Tema1()
    : amplitude1(30.0f), amplitude2(20.0f), frequency1(0.02f), frequency2(0.05f) // pozitia de start a tancului in stanga ecranului
{
    projectiles.clear(); // golim vectorul de proiectile
    // pozitioneaza tancul la începutul ecranului
    tankPosX = 50.0f; // tank 1 la stanga ecranului
    
    // calculezm pozitia y a tancului pe baza conturului terenului
    tankPosY = heightMultiplier * Height(tankPosX / xOffset) + yOffset;

    float dx = 0.1f;
    float nextHeight = heightMultiplier * Height((tankPosX + dx) / xOffset) + yOffset;
    float slope = (nextHeight - tankPosY) / dx;
    initialTankAngle = atan(slope);
    turretAngle = 0.0f;

    //tank 2
    tank2PosX = 1100.0f;  // tank 2 la dreapta ecranului
    tank2PosY = heightMultiplier * Height(tank2PosX / xOffset) + yOffset;

    float nextHeight2 = heightMultiplier * Height((tank2PosX + dx) / xOffset) + yOffset;
    float slope2 = (nextHeight2 - tank2PosY) / dx;
    tank2Angle = atan(slope2);
    tank2TurretAngle = glm::pi<float>();

    terrainMesh = nullptr;
    tankBaseMesh1 = nullptr;
    tankBaseMesh2 = nullptr;
    tankTurretMesh = nullptr;
    tankBarrelMesh = nullptr;

    InitializeHeightGrid();
}

Tema1::~Tema1()
{
    if (terrainMesh != nullptr) {
        delete terrainMesh;
    }

    DeleteTankMeshes();
}

// stergem mesh-urile vechi ale tancului
// pentru optimizarea memoriei
void Tema1::DeleteTankMeshes() {
    if (tankBaseMesh1 != nullptr) {
        delete tankBaseMesh1;
        tankBaseMesh1 = nullptr;
    }
    if (tankBaseMesh2 != nullptr) {
        delete tankBaseMesh2;
        tankBaseMesh2 = nullptr;
    }
    if (tankTurretMesh != nullptr) {
        delete tankTurretMesh;
        tankTurretMesh = nullptr;
    }
    if (tankBarrelMesh != nullptr) {
        delete tankBarrelMesh;
        tankBarrelMesh = nullptr;
    }
}

void Tema1::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

}

// transformarile pe matrici

// Translation matrix
inline glm::mat3 Translate(float translateX, float translateY)
{
    return glm::transpose(
        glm::mat3(1, 0, translateX,
            0, 1, translateY,
            0, 0, 1)
    );
}

// Scale matrix
inline glm::mat3 Scale(float scaleX, float scaleY)
{
    return glm::transpose(
        glm::mat3(scaleX, 0, 0,
            0, scaleY, 0,
            0, 0, 1)
    );
}

// Rotate matrix
inline glm::mat3 Rotate(float radians)
{
    return glm::transpose(
        glm::mat3(cos(radians), -sin(radians), 0,
            sin(radians), cos(radians), 0,
            0, 0, 1)
    );
}
void Tema1::InitializeTankMeshes(glm::vec3 colorBase, glm::vec3 colorTurret, glm::vec3 colorBarrel) {
    // sterg mesh-urile vechi daca exista
    DeleteTankMeshes();

    // trapez mic - baza inferioara
    std::vector<VertexFormat> baseVertices1 = {
        VertexFormat(glm::vec3(-0.5f, 0.0f, 0), colorBase),
        VertexFormat(glm::vec3(0.5f, 0.0f, 0), colorBase),
        VertexFormat(glm::vec3(1.0f, 0.3f, 0), colorBase),
        VertexFormat(glm::vec3(-1.0f, 0.3f, 0), colorBase)
    };
    std::vector<unsigned int> baseIndices1 = { 0, 1, 2, 0, 2, 3 };
    tankBaseMesh1 = new Mesh("baseTrapezoid1");
    tankBaseMesh1->InitFromData(baseVertices1, baseIndices1);

    // trapez mare - baza superioara
    std::vector<VertexFormat> baseVertices2 = {
        VertexFormat(glm::vec3(-1.5f, 0.0f, 0), colorBase),
        VertexFormat(glm::vec3(1.5f, 0.0f, 0), colorBase),
        VertexFormat(glm::vec3(1.0f, 0.6f, 0), colorBase),
        VertexFormat(glm::vec3(-1.0f, 0.6f, 0), colorBase)
    };
    std::vector<unsigned int> baseIndices2 = { 0, 1, 2, 0, 2, 3 };
    tankBaseMesh2 = new Mesh("baseTrapezoid2");
    tankBaseMesh2->InitFromData(baseVertices2, baseIndices2);

    // turela - semicerc
    std::vector<VertexFormat> turretVertices;
    std::vector<unsigned int> turretIndices;
    int segments = 20;
    float radius = 0.6f;
    float angleStep = glm::pi<float>() / segments;

    turretVertices.emplace_back(glm::vec3(0, 0, 0), colorTurret);
    for (int i = 0; i <= segments; i++) {
        float angle = i * angleStep;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        turretVertices.emplace_back(glm::vec3(x, y, 0), colorTurret);
        if (i > 0) {
            turretIndices.push_back(0);
            turretIndices.push_back(i);
            turretIndices.push_back(i + 1);
        }
    }
    tankTurretMesh = new Mesh("turret");
    tankTurretMesh->InitFromData(turretVertices, turretIndices);

    // teava - dreptunghi
    std::vector<VertexFormat> barrelVertices = {
        VertexFormat(glm::vec3(0.0f, -0.1f, 0), colorBarrel),
        VertexFormat(glm::vec3(2.0f, -0.1f, 0), colorBarrel),
        VertexFormat(glm::vec3(2.0f, 0.1f, 0), colorBarrel),
        VertexFormat(glm::vec3(0.0f, 0.1f, 0), colorBarrel)
    };
    std::vector<unsigned int> barrelIndices = { 0, 1, 2, 0, 2, 3 };
    tankBarrelMesh = new Mesh("barrel");
    tankBarrelMesh->InitFromData(barrelVertices, barrelIndices);
}

void Tema1::CreateTank(float posX, float posY, float rotation, glm::vec3 colorBase, glm::vec3 colorTurret, glm::vec3 colorBarrel, float turretRotation) {
    // initializez mesh-urile daca nu au fost create
    if (tankBaseMesh1 == nullptr) {
        InitializeTankMeshes(colorBase, colorTurret, colorBarrel);
    }

    glm::mat3 modelMatrix = glm::mat3(1);
    modelMatrix *= Translate(posX, posY);
    modelMatrix *= Rotate(rotation);
    modelMatrix *= Scale(scaleFactor, scaleFactor);

    // render baza inferioara
    RenderMesh2D(tankBaseMesh1, shaders["VertexColor"], modelMatrix);

    // render baza superioara
    RenderMesh2D(tankBaseMesh2, shaders["VertexColor"], modelMatrix * Translate(0, 0.3f));

    // render turela
    RenderMesh2D(tankTurretMesh, shaders["VertexColor"], modelMatrix * Translate(0, 0.9f));

    // render teava
    RenderMesh2D(tankBarrelMesh, shaders["VertexColor"], modelMatrix * Translate(0, 0.9f) * Rotate(turretRotation));
}

// calculez inaltimea terenului la un punct x
float Tema1::Height(float x) {
    return amplitude1 * sin(frequency1 * x) + amplitude2 * cos(frequency2 * x); // formula din cerinta
}

void Tema1::RenderTerrainMethod() {
    glm::ivec2 resolution = window->GetResolution();
    float spacing = 5.0f;
    glm::vec3 terrainColor = glm::vec3(0.91f, 0.84f, 0.23f); // culoarea galbena a terenului

    bool needsUpdate = false;
    for (const auto& row : heightGrid) {
        for (const auto& point : row) {
            if (point.needsUpdate) {
                needsUpdate = true;
                break;
            }
        }
        if (needsUpdate) break;
    }

    if (terrainMesh != nullptr && !needsUpdate) {
        RenderMesh2D(terrainMesh, shaders["VertexColor"], glm::mat3(1.0f));
        return;
    }

    // Dacă avem un mesh vechi, îl ștergem
    if (terrainMesh != nullptr) {
        delete terrainMesh;
        terrainMesh = nullptr;
    }

    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    for (float x = 0; x <= resolution.x; x += spacing) {
        float y = GetInterpolatedHeight(x);
        vertices.emplace_back(glm::vec3(x, y, 0), glm::vec3(0.91f, 0.84f, 0.23f));  // vertex superior (teren)
        vertices.emplace_back(glm::vec3(x, 0.0f, 0), glm::vec3(0.91f, 0.84f, 0.23f)); // vertex inferior (de jos)
    }

    for (size_t i = 0; i < vertices.size() - 2; i += 2) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);

        indices.push_back(i + 1);
        indices.push_back(i + 3);
        indices.push_back(i + 2);
    }

    Mesh* terrainMesh = new Mesh("terrainStrip");
    terrainMesh->InitFromData(vertices, indices);
    terrainMesh->SetDrawMode(GL_TRIANGLE_STRIP);

    for (auto& row : heightGrid) {
        for (auto& point : row) {
            point.needsUpdate = false;
        }
    }

    RenderMesh2D(terrainMesh, shaders["VertexColor"], glm::mat3(1.0f));
}

void Tema1::RenderProjectileTrajectoryPreview(float startX, float startY, float angle, float magnitude)
{
    std::vector<glm::vec2> trajectoryPoints;
    float t = 0.0f;
    float timeStep = 0.1f;
    float velX = magnitude * cos(angle);
    float velY = magnitude * sin(angle);

    while (true) {
        float x = startX + velX * t;
        float y = startY + velY * t - 0.5f * gravity * t * t;

        // verific coliziunea cu terenul folosind GetInterpolatedHeight
        if (y < GetInterpolatedHeight(x)) {
            break;
        }

        trajectoryPoints.emplace_back(x, y);
        t += timeStep;
    }

    for (size_t i = 1; i < trajectoryPoints.size(); i++) {
        DrawLineSegmentWithAngle(trajectoryPoints[i - 1], trajectoryPoints[i]);
    }
}

void Tema1::DrawLineSegment(glm::vec2 p1, glm::vec2 p2, float angle)
{
    std::vector<VertexFormat> lineVertices = {
        VertexFormat(glm::vec3(p1.x, p1.y, 0), glm::vec3(1, 1, 1)),  // de culoare alba
        VertexFormat(glm::vec3(p2.x, p2.y, 0), glm::vec3(1, 1, 1))
    };

    std::vector<unsigned int> lineIndices = { 0, 1 };

    Mesh* line = new Mesh("trajectoryLine");
    line->InitFromData(lineVertices, lineIndices);
    line->SetDrawMode(GL_LINES);
    RenderMesh2D(line, shaders["VertexColor"], glm::mat3(1.0f));
}

void Tema1::DrawLineSegmentWithAngle(glm::vec2 p1, glm::vec2 p2)
{
    std::vector<VertexFormat> lineVertices = {
        VertexFormat(glm::vec3(p1.x, p1.y, 0), glm::vec3(1, 1, 1)),
        VertexFormat(glm::vec3(p2.x, p2.y, 0), glm::vec3(1, 1, 1))
    };

    std::vector<unsigned int> lineIndices = { 0, 1 };

    Mesh* line = new Mesh("trajectoryLine");
    line->InitFromData(lineVertices, lineIndices);
    line->SetDrawMode(GL_LINES);
    RenderMesh2D(line, shaders["VertexColor"], glm::mat3(1.0f));
}

void Tema1::RenderProjectile(float posX, float posY) {
    glm::mat3 modelMatrix = glm::mat3(1);
    modelMatrix *= Translate(posX, posY);
    modelMatrix *= Scale(5.0f, 5.0f);  // marime ajustabila pentru vizibilitate

    // setez culoarea proiectilului, gri inchis
    glm::vec3 darkGrayColor = glm::vec3(0.2f, 0.2f, 0.2f);

    // cercul proiectilului
    std::vector<VertexFormat> projectileVertices;
    std::vector<unsigned int> projectileIndices;

    int segments = 30;  // nr de segmente pentru un cerc destul de rotund
    float radius = 1.0f;
    float angleStep = 2 * glm::pi<float>() / segments;

    // adaug centrul cercului
    projectileVertices.emplace_back(glm::vec3(0, 0, 0), darkGrayColor);

    // adaug punctele pe marginea cercului
    for (int i = 0; i <= segments; i++) {
        float angle = i * angleStep;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        projectileVertices.emplace_back(glm::vec3(x, y, 0), darkGrayColor);

        if (i > 0) {
            // adaug triunghiurile pentru fiecare segment
            projectileIndices.push_back(0);       // centrul cercului
            projectileIndices.push_back(i);       // vertex pe margine
            projectileIndices.push_back(i + 1);   // urmatorul vertex pe margine
        }
    }

    // cream si redam mesh-ul pentru proiectil
    Mesh* projectileMesh = new Mesh("circularProjectile");
    projectileMesh->InitFromData(projectileVertices, projectileIndices);
    RenderMesh2D(projectileMesh, shaders["VertexColor"], modelMatrix);
}

void Tema1::LaunchProjectile(float startX, float startY, float angle, float magnitude, int launcherId) {
    // creez un proiectil nou
    Projectile projectile;
    projectile.posX = startX;
    projectile.posY = startY;
    projectile.velX = cos(angle) * magnitude;
    projectile.velY = sin(angle) * magnitude;
    projectile.active = true;
    projectile.launcherId = launcherId;
    // adaug proiectilul in vectorul de projectiles
    projectiles.push_back(projectile);
}

bool Tema1::CheckProjectileCollision(float posX, float posY) {
    // calculez intaltimea terenului la pozitia X a proiectilului
    float terrainHeight = GetInterpolatedHeight(posX);
    return posY <= terrainHeight;
}

// coliziune intre proiectil si tanc
bool Tema1::CheckCollision(float projX, float projY, float tankX, float tankY, float projRadius, float tankRadius) {
    float distanceX = projX - tankX;
    float distanceY = projY - tankY;
    float distanceSquared = distanceX * distanceX + distanceY * distanceY;
    float radiusSum = projRadius + tankRadius;
    return distanceSquared <= radiusSum * radiusSum;
}

void Tema1::RenderLifeBar(float tankPosX, float tankPosY, int health) {
    float barWidth = 40.0f;  // latimea life bar-ului
    float barHeight = 10.0f;  // inaltimea life bar-ului
    float offsetY = 70.0f;   // distanta life bar-ului de deasupra tank-ului
    float healthRatio = health / 4.0;

    glm::mat3 modelMatrix = glm::mat3(1);
    modelMatrix *= Translate(tankPosX - barWidth / 2, tankPosY + offsetY);

    // dreptunghi wireframe
    std::vector<VertexFormat> borderVertices = {
        VertexFormat(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)),
        VertexFormat(glm::vec3(barWidth, 0, 0), glm::vec3(1, 1, 1)),
        VertexFormat(glm::vec3(barWidth, barHeight, 0), glm::vec3(1, 1, 1)),
        VertexFormat(glm::vec3(0, barHeight, 0), glm::vec3(1, 1, 1))
    };
    std::vector<unsigned int> borderIndices = { 0, 1, 2, 2, 3, 0 };
    Mesh* borderMesh = new Mesh("border");
    borderMesh->InitFromData(borderVertices, borderIndices);
    borderMesh->SetDrawMode(GL_LINE_LOOP);
    RenderMesh2D(borderMesh, shaders["VertexColor"], modelMatrix);

    // dreptunghi umplut indicand cu sanatatea ramasa
    glm::mat3 filledModelMatrix = modelMatrix * Scale(healthRatio, 1.0f);
    std::vector<VertexFormat> fillVertices = {
        VertexFormat(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0)),
        VertexFormat(glm::vec3(barWidth, 0, 0), glm::vec3(1, 0, 0)),
        VertexFormat(glm::vec3(barWidth, barHeight, 0), glm::vec3(1, 0, 0)),
        VertexFormat(glm::vec3(0, barHeight, 0), glm::vec3(1, 0, 0))
    };
    std::vector<unsigned int> fillIndices = { 0, 1, 2, 2, 3, 0 };
    Mesh* fillMesh = new Mesh("fill");
    fillMesh->InitFromData(fillVertices, fillIndices);
    RenderMesh2D(fillMesh, shaders["VertexColor"], filledModelMatrix);
}

void Tema1::InitializeHeightGrid() {
    glm::ivec2 resolution = window->GetResolution();
    int gridWidth = resolution.x / 5;
    int gridHeight = 1;  // doar o singura linie de inaltimi

    heightGrid.resize(gridWidth);
    for (int x = 0; x < gridWidth; x++) {
        heightGrid[x].resize(gridHeight);
        float worldX = x * 5.0f;
        float baseHeight = heightMultiplier * Height(worldX / xOffset) + yOffset;
        heightGrid[x][0].height = baseHeight;
        heightGrid[x][0].needsUpdate = false;
    }
}

void Tema1::DeformTerrain(float posX, float posY) {
    int gridX = posX / 5.0f;
    int radius = deformationRadius / 5.0f;

    // aplic deformarea intr-o zona circulara
    for (int x = gridX - radius; x <= gridX + radius; x++) {
        if (x < 0 || x >= heightGrid.size()) continue;

        // calculez inaltimea initiala a terenului la aceasta pozitie
        float baseHeight = heightMultiplier * Height((x * 5.0f) / xOffset) + yOffset;
        float dx = (x * 5.0f) - posX;
        float distance = abs(dx);

        if (distance <= deformationRadius) {
            float deformFactor = 1.0f - (distance / deformationRadius);
            float deformAmount = deformFactor * 50.0f;  // Adâncimea craterului

            // inaltimea curenta dupa deformarile anterioare
            float currentHeight = heightGrid[x][0].height;

            // aplic deformarea doar daca noua inaltime este mai mica decat cea curenta
            float newHeight = currentHeight - deformAmount;

            // deformarea este cumulativa (doar scade inaltimea)
            heightGrid[x][0].height = std::min(currentHeight, newHeight);
            heightGrid[x][0].needsUpdate = true;
        }
    }
}

void Tema1::UpdateTerrainSlide(float deltaTime) {
    for (int x = 1; x < heightGrid.size() - 1; x++) {
        float currentHeight = heightGrid[x][0].height;
        float leftHeight = heightGrid[x - 1][0].height;
        float rightHeight = heightGrid[x + 1][0].height;
        float baseHeight = heightMultiplier * Height((x * 5.0f) / xOffset) + yOffset;

        if (abs(currentHeight - leftHeight) > transferThreshold) {
            float transfer = transferAmount * deltaTime;
            // transfer doar dacă diferenta este mai mare decat pragul
            if (currentHeight > leftHeight) {
                // ma asigur ca nu depaseste inaltimea originala
                float newCurrentHeight = std::min(currentHeight - transfer, baseHeight);
                float newLeftHeight = std::min(leftHeight + transfer, baseHeight);

                heightGrid[x][0].height = newCurrentHeight;
                heightGrid[x - 1][0].height = newLeftHeight;
            }
        }

        if (abs(currentHeight - rightHeight) > transferThreshold) {
            float transfer = transferAmount * deltaTime;
            if (currentHeight > rightHeight) {
                // ma asigur ca nu depaseste inaltimea originala
                float newCurrentHeight = std::min(currentHeight - transfer, baseHeight);
                float newRightHeight = std::min(rightHeight + transfer, baseHeight);

                heightGrid[x][0].height = newCurrentHeight;
                heightGrid[x + 1][0].height = newRightHeight;
            }
        }
    }
}

float Tema1::GetInterpolatedHeight(float x) {
    int gridX = x / 5.0f;
    if (gridX < 0 || gridX >= heightGrid.size() - 1) {
        return heightMultiplier * Height(x / xOffset) + yOffset;
    }

    float fractX = (x / 5.0f) - gridX;
    float h1 = heightGrid[gridX][0].height;
    float h2 = heightGrid[gridX + 1][0].height;

    return h1 * (1.0f - fractX) + h2 * fractX;
}

void Tema1::Update(float deltaTimeSeconds)
{
    // deseneaza terenul
    RenderTerrainMethod();

    // --------------------- amplasarea tancului pe teren -------------------

    // pas foarte mic care va ajuta la calcularea tangentei curbei
    float dx = 0.1f;

    // tank 1
    if (tank1Active) {

        // calculez pozitia y a tancului pe conturul terenului la pozitia tankPosX
        tankPosY = GetInterpolatedHeight(tankPosX);

        float nextHeight = GetInterpolatedHeight(tankPosX + dx);
        // derivata curbei terenului in punctul tankPosX folosind un pas mic dx
        float slope = (nextHeight - tankPosY) / dx;

        // unghiul de rotatie în funcție de panta curbei
        float tankAngle = atan(slope);

        // desenez tank 1
        CreateTank(tankPosX, tankPosY, tankAngle, glm::vec3(0.2f, 0.35f, 0.2f), glm::vec3(0.3f, 0.6f, 0.2f), glm::vec3(0.2f, 0.2f, 0.2f), turretAngle);

        // calculez și afișez traiectoria pentru tank1
        float barrelTipX = tankPosX + tankHeight * (-sin(tankAngle)) + barrelLength * cos(tankAngle + turretAngle);
        float barrelTipY = tankPosY + tankHeight * cos(tankAngle) + barrelLength * sin(tankAngle + turretAngle);
        RenderProjectileTrajectoryPreview(barrelTipX, barrelTipY, tankAngle + turretAngle, 70.0f);
        RenderLifeBar(tankPosX, tankPosY, tank1Life);
    }
    // tank 2
    if (tank2Active) {
        // calculare pozitie si unghi
        tank2PosY = GetInterpolatedHeight(tank2PosX);
        float nextHeight2 = GetInterpolatedHeight(tank2PosX + dx);
        float slope2 = (nextHeight2 - tank2PosY) / dx;
        float tank2Angle = atan(slope2);

        // desenez tank 2
        CreateTank(tank2PosX, tank2PosY, tank2Angle, glm::vec3(0.9f, 0.2f, 0.2f), glm::vec3(0.6f, 0.2f, 0.3f), glm::vec3(0.2f, 0.2f, 0.2f), tank2TurretAngle);

        // calculez si afisez traiectoria pentru tank2
        float barrelTipX2 = tank2PosX + tankHeight * (-sin(tank2Angle)) + barrelLength * cos(tank2Angle + tank2TurretAngle);
        float barrelTipY2 = tank2PosY + tankHeight * cos(tank2Angle) + barrelLength * sin(tank2Angle + tank2TurretAngle);
        RenderProjectileTrajectoryPreview(barrelTipX2, barrelTipY2, tank2Angle + tank2TurretAngle, 70.0f);
        RenderLifeBar(tank2PosX, tank2PosY, tank2Life);
    }
    // -------------- amplasarea/ miscarea proiectilelor din vector ------------
    for (auto& projectile : projectiles) {
        if (projectile.active) {
            // gravitatia pe axa y
            projectile.velY -= 18 * gravity * deltaTimeSeconds;

            // actualizare pozitie proiectil
            projectile.posX += projectile.velX * deltaTimeSeconds;
            projectile.posY += projectile.velY * deltaTimeSeconds;

            RenderProjectile(projectile.posX, projectile.posY);

            // coliziunea cu terenul
            if (projectile.active && CheckProjectileCollision(projectile.posX, projectile.posY)) {
                DeformTerrain(projectile.posX, projectile.posY);
                projectile.active = false; // dispare proiectilul cand loveste terenul
            }

            // coliziune cu tank 1
            // verific si ca nu cumva tank 1 sa traga in el insusi
            if (projectile.launcherId != 1 && CheckCollision(projectile.posX, projectile.posY, tankPosX, tankPosY, projectileRadius, tankRadius)) {
                projectile.active = false;
                tank1Life--;
                if (tank1Life <= 0) {
                    // tank1 este eliminat
                    tank1Active = false;
                    tankPosX = -1000.0f; // mut tankul in afara ecranului dupa ce a fost eliminat
                }
            }

            // coliziune cu tank 2
            // verific si ca nu cumva tank 2 sa traga in el insusi
            if (projectile.launcherId != 2 && CheckCollision(projectile.posX, projectile.posY, tank2PosX, tank2PosY, projectileRadius, tankRadius)) {
                projectile.active = false;
                tank2Life--;
                if (tank2Life <= 0) {
                    // tank2 este eliminat
                    tank2Active = false;
                    tank2PosX = -1000.0f; // mut tankul in afara ecranului dupa ce a fost eliminat
                }
            }
        }
    }

    // elimin proiectilele inactive din vector
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const Projectile& p) { return !p.active; }), projectiles.end());

    UpdateTerrainSlide(deltaTimeSeconds);

    // Folosim noua inaltime pentru render
    RenderTerrainMethod();
}



void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    float speed = 200.0f; // viteza de deplasare a tankului
    float rotationSpeed = glm::radians(90.0f); // viteza de rotatie a tunului
    float maxRotation = glm::radians(180.0f); // unghiul maxim +- de rotatie al tunului
    float minRotation = glm::radians(0.0f); // unghiul minim +- de rotatie al tunului
    // sa nu poata sa iasa tank ul din cadru
    glm::ivec2 resolution = window->GetResolution();
    float minX = 0.0f; // sa nu poata iesi in stanga din cadru
    float maxX = resolution.x; // sa nu poata iesi in dreapta din cadru

    // tank 1
    //miscarea tankului 1 in dreapta
    if (tank1Active) {
        if (window->KeyHold(GLFW_KEY_D)) {
            if (tankPosX + speed * deltaTime < maxX) {
                tankPosX += speed * deltaTime;
            }
        }

        // miscarea tankului 1 in stanga
        if (window->KeyHold(GLFW_KEY_A)) {
            if (tankPosX - speed * deltaTime > minX) {
                tankPosX -= speed * deltaTime;
            }
        }

        // miscarea tunului tankului 1 in sens anti ceasornic
        if (window->KeyHold(GLFW_KEY_W)) {
            turretAngle += rotationSpeed * deltaTime;
            if (turretAngle > maxRotation) {
                turretAngle = maxRotation;
            }
        }

        // miscarea tunului tankului 1 in sens ceasornic
        if (window->KeyHold(GLFW_KEY_S)) {
            turretAngle -= rotationSpeed * deltaTime;
            if (turretAngle < minRotation) {
                turretAngle = minRotation;
            }
        }
    }
    // tank 2
    if (tank2Active) {
        //miscarea tankului 2 in dreapta
        if (window->KeyHold(GLFW_KEY_RIGHT)) {
            if (tank2PosX + speed * deltaTime < maxX) {
                tank2PosX += speed * deltaTime;
            }
        }

        // miscarea tankului 2 in stanga
        if (window->KeyHold(GLFW_KEY_LEFT)) {
            if (tank2PosX - speed * deltaTime > minX) {
                tank2PosX -= speed * deltaTime;
            }
        }

        // miscarea tunului tankului 2 in sens anti ceasornic
        if (window->KeyHold(GLFW_KEY_UP)) {
            tank2TurretAngle += rotationSpeed * deltaTime;
            if (tank2TurretAngle > maxRotation) {
                tank2TurretAngle = maxRotation;
            }
        }

        // miscarea tunului tankului 2 in sens ceasornic
        if (window->KeyHold(GLFW_KEY_DOWN)) {
            tank2TurretAngle -= rotationSpeed * deltaTime;
            if (tank2TurretAngle < minRotation) {
                tank2TurretAngle = minRotation;
            }
        }
    }
}

void Tema1::FrameStart()
{
    glClearColor(0.5f, 0.75f, 0.90f, 1.0f); // culoarea cerului
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::FrameEnd()
{
}

void Tema1::OnKeyPress(int key, int mods) {
    float magnitude = 300.0f;
    float dx = 0.1f;

    // tank 1
    if (key == GLFW_KEY_SPACE && tank1Active) {
        float nextHeight = GetInterpolatedHeight(tankPosX + dx);
        float slope = (nextHeight - tankPosY) / dx;
        float tankAngle = atan(slope);

        float barrelTipX = tankPosX + tankHeight * (-sin(tankAngle)) + barrelLength * cos(tankAngle + turretAngle);
        float barrelTipY = tankPosY + tankHeight * cos(tankAngle) + barrelLength * sin(tankAngle + turretAngle);

        float launchAngle = tankAngle + turretAngle;
        LaunchProjectile(barrelTipX, barrelTipY, launchAngle, magnitude, 1);
    }

    // tank 2
    if (key == GLFW_KEY_ENTER && tank2Active) {
        float nextHeight2 = GetInterpolatedHeight(tank2PosX + dx);
        float slope2 = (nextHeight2 - tank2PosY) / dx;
        float tank2Angle = atan(slope2);

        float barrelTipX2 = tank2PosX + tankHeight * (-sin(tank2Angle)) + barrelLength * cos(tank2Angle + tank2TurretAngle);
        float barrelTipY2 = tank2PosY + tankHeight * cos(tank2Angle) + barrelLength * sin(tank2Angle + tank2TurretAngle);

        float launchAngle = tank2Angle + tank2TurretAngle;
        LaunchProjectile(barrelTipX2, barrelTipY2, launchAngle, magnitude, 2);
    }
}

void Tema1::OnKeyRelease(int key, int mods) {}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {}

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}

void Tema1::OnWindowResize(int width, int height) {}