#include <iostream>
#include <vector>
#include <cmath>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <unordered_map>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constants
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float PLAYER_SPEED = 300.0f;
const float ENEMY_SPEED = 100.0f;
const float BALL_RADIUS = 20.0f;
const float PROJECTILE_RADIUS = 5.0f;
const float PROJECTILE_SPEED = 500.0f;

// Vec2 structure for 2D vector operations
struct Vec2 {
    float x, y;
    Vec2 operator+(const Vec2& v) const { return { x + v.x, y + v.y }; }
    Vec2 operator-(const Vec2& v) const { return { x - v.x, y - v.y }; }
    Vec2 operator*(float scalar) const { return { x * scalar, y * scalar }; }
    float length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalize() const { float len = length(); return { x / len, y / len }; }
};

// Component structures
struct Position {
    Vec2 value;
};

struct Velocity {
    Vec2 value;
};

struct Health {
    int current, max;
};

// Entity class
class Entity {
public:
    int id;
    Entity(int id) : id(id) {}
};

// Component containers
std::unordered_map<int, Position> positions;
std::unordered_map<int, Velocity> velocities;
std::unordered_map<int, Health> healths;
std::vector<Entity> projectiles;

// Function to convert integer to string (replacement for std::to_string)
std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// System functions
void MovementSystem(float dt) {
    for (auto& entry : positions) {
        int entityId = entry.first;
        Position& pos = entry.second;
        if (velocities.find(entityId) != velocities.end()) {
            pos.value = pos.value + velocities[entityId].value * dt;

            // Keep entities within window bounds
            if (pos.value.x < BALL_RADIUS) pos.value.x = BALL_RADIUS;
            if (pos.value.y < BALL_RADIUS) pos.value.y = BALL_RADIUS;
            if (pos.value.x > WINDOW_WIDTH - BALL_RADIUS) pos.value.x = WINDOW_WIDTH - BALL_RADIUS;
            if (pos.value.y > WINDOW_HEIGHT - BALL_RADIUS) pos.value.y = WINDOW_HEIGHT - BALL_RADIUS;
        }
    }
}

void PlayerInputSystem(int playerId, float dt) {
    if (positions.find(playerId) != positions.end()) {
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS) {
            velocities[playerId].value.x = -PLAYER_SPEED;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS) {
            velocities[playerId].value.x = PLAYER_SPEED;
        }
        else {
            velocities[playerId].value.x = 0;
        }

        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS) {
            velocities[playerId].value.y = PLAYER_SPEED;
        }
        else if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS) {
            velocities[playerId].value.y = -PLAYER_SPEED;
        }
        else {
            velocities[playerId].value.y = 0;
        }
    }
}

void ProjectileSystem(int playerId) {
    if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double mouseX, mouseY;
        glfwGetCursorPos(glfwGetCurrentContext(), &mouseX, &mouseY);
        mouseY = WINDOW_HEIGHT - mouseY;  // Invert Y for OpenGL coordinates

        if (positions.find(playerId) != positions.end()) {
            Position& playerPos = positions[playerId];
            Vec2 direction = { static_cast<float>(mouseX) - playerPos.value.x, static_cast<float>(mouseY) - playerPos.value.y };
            direction = direction.normalize();

            int projectileId = projectiles.size() + 100; // Start projectile IDs at 100
            projectiles.emplace_back(projectileId);
            positions[projectileId] = { playerPos.value };
            velocities[projectileId] = { direction * PROJECTILE_SPEED };
        }
    }
}

void ProjectileMovementSystem(float dt) {
    for (auto& projectile : projectiles) {
        if (positions.find(projectile.id) != positions.end()) {
            positions[projectile.id].value = positions[projectile.id].value + velocities[projectile.id].value * dt;

            // Remove projectile if it goes out of bounds
            if (positions[projectile.id].value.x < 0 || positions[projectile.id].value.x > WINDOW_WIDTH ||
                positions[projectile.id].value.y < 0 || positions[projectile.id].value.y > WINDOW_HEIGHT) {
                positions.erase(projectile.id);
                velocities.erase(projectile.id);
            }
        }
    }
}

void AISystem(int enemyId, int playerId, float dt) {
    if (positions.find(enemyId) != positions.end() && positions.find(playerId) != positions.end()) {
        Position& enemyPos = positions[enemyId];
        Position& playerPos = positions[playerId];

        Vec2 direction = { playerPos.value.x - enemyPos.value.x, playerPos.value.y - enemyPos.value.y };
        direction = direction.normalize();
        velocities[enemyId].value = { direction.x * ENEMY_SPEED, direction.y * ENEMY_SPEED };
    }
}

void CollisionSystem(int playerId) {
    // Check projectile collisions with enemies only
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        bool destroyed = false;
        for (auto& entry : positions) {
            int enemyId = entry.first;
            if (enemyId != playerId && enemyId >= 2 && enemyId < 100) { // Check if it's an enemy
                Position& projPos = positions[it->id];
                Position& enemyPos = positions[enemyId];
                Vec2 delta = { projPos.value.x - enemyPos.value.x, projPos.value.y - enemyPos.value.y };
                if (delta.length() < BALL_RADIUS + PROJECTILE_RADIUS) {
                    // Destroy enemy and remove projectile
                    positions.erase(enemyId);
                    velocities.erase(enemyId);
                    healths.erase(enemyId);

                    positions.erase(it->id);
                    velocities.erase(it->id);
                    it = projectiles.erase(it);
                    destroyed = true;
                    break;
                }
            }
        }
        if (!destroyed) ++it;
    }
}

void RenderSystem(int playerId) {
    // Draw boundary box
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();

    // Draw trajectory line
    double mouseX, mouseY;
    glfwGetCursorPos(glfwGetCurrentContext(), &mouseX, &mouseY);
    mouseY = WINDOW_HEIGHT - mouseY;

    if (positions.find(playerId) != positions.end()) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0xAAAA);
        glBegin(GL_LINES);
        glVertex2f(positions[playerId].value.x, positions[playerId].value.y);
        glVertex2f(mouseX, mouseY);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }

    // Draw entities
    for (const auto& entry : positions) {
        const Position& pos = entry.second;

        if (entry.first == playerId) {
            glColor3f(1.0f, 1.0f, 0.0f); // Yellow for player
        }
        else if (entry.first >= 2 && entry.first < 100) {
            glColor3f(1.0f, 0.0f, 0.0f); // Red for enemies
        }
        else {
            glColor3f(1.0f, 1.0f, 1.0f); // White for projectiles
        }

        float radius = (entry.first >= 100) ? PROJECTILE_RADIUS : BALL_RADIUS; // Projectiles are smaller
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(pos.value.x, pos.value.y);
        for (int i = 0; i <= 20; i++) {
            float angle = i * 2.0f * M_PI / 20;
            glVertex2f(pos.value.x + radius * cosf(angle), pos.value.y + radius * sinf(angle));
        }
        glEnd();
    }
}

// Main function
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "2D ECS Game Demo with Shooting", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    int playerId = 1;
    positions[playerId] = { {960, 540} };
    velocities[playerId] = { {0, 0} };
    healths[playerId] = { 100, 100 };

    // Add enemy entities
    for (int i = 2; i <= 5; ++i) {
        positions[i] = { {static_cast<float>(rand() % WINDOW_WIDTH), static_cast<float>(rand() % WINDOW_HEIGHT)} };
        velocities[i] = { {0, 0} };
        healths[i] = { 50, 50 }; // Enemies have 50 health points
    }

    double previousTime = glfwGetTime();
    const double targetFrameTime = 1.0 / 60.0;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - previousTime;

        if (deltaTime >= targetFrameTime) {
            float dt = static_cast<float>(deltaTime);
            glClear(GL_COLOR_BUFFER_BIT);

            PlayerInputSystem(playerId, dt);
            for (int i = 2; i <= 5; ++i) {
                AISystem(i, playerId, dt);
            }
            ProjectileSystem(playerId);
            MovementSystem(dt);
            ProjectileMovementSystem(dt);
            CollisionSystem(playerId);
            RenderSystem(playerId);

            glfwSwapBuffers(window);
            glfwPollEvents();
            previousTime = currentTime;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
