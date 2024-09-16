//#include <iostream>
//#include <vector>
//#include <cmath>
//#include <GLFW/glfw3.h>
//
//#ifndef M_PI
//#define M_PI 3.14159265358979323846
//#endif
//
//// Constants
//const float BALL_RADIUS = 10.0f;
//const int WINDOW_WIDTH = 1920;
//const int WINDOW_HEIGHT = 1080;
//const float GRAVITY = 0.0f;  // No gravity for this example
//const float DAMPING = 0.99f; // Frictional damping
//const int MAX_OBJECTS = 4;   // Max objects before subdivision in quadtree
//const int MAX_LEVELS = 5;    // Max depth of quadtree
//
//// Utility for vector operations
//struct Vec2 {
//    float x, y;
//
//    Vec2 operator+(const Vec2& v) const { return { x + v.x, y + v.y }; }
//    Vec2 operator-(const Vec2& v) const { return { x - v.x, y - v.y }; }
//    Vec2 operator*(float scalar) const { return { x * scalar, y * scalar }; }
//    float length() const { return std::sqrt(x * x + y * y); }
//    Vec2 normalize() const { float len = length(); return { x / len, y / len }; }
//};
//
//// Ball class to represent a ball object
//class Ball {
//public:
//    Vec2 position, velocity;
//    float radius;
//    float angularVelocity;  // New variable for simulating spin
//
//    Ball(Vec2 pos, float r) : position(pos), velocity({ 0, 0 }), radius(r), angularVelocity(0.0f) {}
//
//    void update(float dt) {
//        position = position + velocity * dt;
//
//        // Apply angular velocity (spin) effect by modifying velocity slightly based on spin
//        velocity.x += angularVelocity * dt * 0.1f;  // Example spin effect; adjust as needed
//    }
//
//    void render() {
//        glBegin(GL_TRIANGLE_FAN);
//        glVertex2f(position.x, position.y);
//        for (int i = 0; i <= 20; i++) {
//            float angle = i * 2.0f * M_PI / 20;
//            glVertex2f(position.x + radius * cosf(angle), position.y + radius * sinf(angle));
//        }
//        glEnd();
//    }
//};
//
//
//// AABB (Axis-Aligned Bounding Box) structure to define the boundaries of a quadtree node
//struct AABB {
//    float x, y, width, height;
//
//    bool contains(const Ball& ball) const {
//        return (ball.position.x - ball.radius >= x && ball.position.x + ball.radius <= x + width &&
//            ball.position.y - ball.radius >= y && ball.position.y + ball.radius <= y + height);
//    }
//
//    bool intersects(const AABB& range) const {
//        return !(range.x > x + width || range.x + range.width < x || range.y > y + height || range.y + range.height < y);
//    }
//};
//
//// Quadtree class for spatial partitioning
//class Quadtree {
//private:
//    int level;
//    std::vector<Ball*> objects;
//    AABB bounds;
//    Quadtree* nodes[4];
//
//    // Splits the current node into 4 subnodes
//    void split() {
//        float subWidth = bounds.width / 2.0f;
//        float subHeight = bounds.height / 2.0f;
//        float x = bounds.x;
//        float y = bounds.y;
//
//        nodes[0] = new Quadtree(level + 1, AABB{ x + subWidth, y, subWidth, subHeight });
//        nodes[1] = new Quadtree(level + 1, AABB{ x, y, subWidth, subHeight });
//        nodes[2] = new Quadtree(level + 1, AABB{ x, y + subHeight, subWidth, subHeight });
//        nodes[3] = new Quadtree(level + 1, AABB{ x + subWidth, y + subHeight, subWidth, subHeight });
//    }
//
//    // Determine which quadrant the object belongs to
//    int getIndex(const Ball& ball) const {
//        int index = -1;
//        float verticalMidpoint = bounds.x + bounds.width / 2.0f;
//        float horizontalMidpoint = bounds.y + bounds.height / 2.0f;
//
//        bool topQuadrant = (ball.position.y - ball.radius < horizontalMidpoint && ball.position.y + ball.radius < horizontalMidpoint);
//        bool bottomQuadrant = (ball.position.y - ball.radius > horizontalMidpoint);
//
//        if (ball.position.x - ball.radius < verticalMidpoint && ball.position.x + ball.radius < verticalMidpoint) {
//            if (topQuadrant) {
//                index = 1;
//            }
//            else if (bottomQuadrant) {
//                index = 2;
//            }
//        }
//        else if (ball.position.x - ball.radius > verticalMidpoint) {
//            if (topQuadrant) {
//                index = 0;
//            }
//            else if (bottomQuadrant) {
//                index = 3;
//            }
//        }
//        return index;
//    }
//
//public:
//    // Constructor
//    Quadtree(int level, const AABB& bounds) : level(level), bounds(bounds) {
//        for (int i = 0; i < 4; i++) nodes[i] = nullptr;
//    }
//
//    // Clears the quadtree and deallocates memory
//    void clear() {
//        objects.clear();
//        for (int i = 0; i < 4; i++) {
//            if (nodes[i] != nullptr) {
//                nodes[i]->clear();
//                delete nodes[i];
//                nodes[i] = nullptr;
//            }
//        }
//    }
//
//    // Insert an object into the quadtree
//    void insert(Ball& ball) {
//        if (nodes[0] != nullptr) {
//            int index = getIndex(ball);
//
//            if (index != -1) {
//                nodes[index]->insert(ball);
//                return;
//            }
//        }
//
//        objects.push_back(&ball);
//
//        // If the number of objects exceeds the limit and the node isn't at max depth, split
//        if (objects.size() > MAX_OBJECTS && level < MAX_LEVELS) {
//            if (nodes[0] == nullptr) {
//                split();
//            }
//
//            // Reassign objects to the appropriate child nodes
//            auto it = objects.begin();
//            while (it != objects.end()) {
//                int index = getIndex(**it);
//                if (index != -1) {
//                    nodes[index]->insert(**it);
//                    it = objects.erase(it);
//                }
//                else {
//                    ++it;
//                }
//            }
//        }
//    }
//
//    // Retrieve all objects that could collide with a given object
//    void retrieve(std::vector<Ball*>& returnObjects, const Ball& ball) {
//        int index = getIndex(ball);
//        if (index != -1 && nodes[0] != nullptr) {
//            nodes[index]->retrieve(returnObjects, ball);
//        }
//
//        returnObjects.insert(returnObjects.end(), objects.begin(), objects.end());
//    }
//};
//
//// Collision detection and response
//class Collision {
//public:
//    static bool detectBallBall(const Ball& a, const Ball& b) {
//        Vec2 delta = a.position - b.position;
//        float dist = delta.length();
//        return dist <= (a.radius + b.radius);
//    }
//
//    static void resolveBallBall(Ball& a, Ball& b) {
//        Vec2 delta = a.position - b.position;
//        float dist = delta.length();
//        Vec2 normal = delta.normalize();
//        Vec2 relativeVelocity = a.velocity - b.velocity;
//
//        // Check if the balls are moving towards each other
//        float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;
//        if (velocityAlongNormal > 0) return;  // If they're separating, no need to resolve
//
//        float restitution = 1.0f;  // Elastic collision
//        float impulseScalar = -(1 + restitution) * velocityAlongNormal;
//        impulseScalar /= (1.0f / a.radius) + (1.0f / b.radius);  // Adjust for mass (based on radius)
//
//        Vec2 impulse = normal * impulseScalar;
//
//        // Adjust velocities using impulse
//        a.velocity = a.velocity - impulse * (1.0f / a.radius);
//        b.velocity = b.velocity + impulse * (1.0f / b.radius);
//    }
//
//    static void resolveBallWall(Ball& ball, const Vec2& wallNormal) {
//        Vec2 normal = wallNormal;
//        float velocityAlongNormal = ball.velocity.x * normal.x + ball.velocity.y * normal.y;
//        if (velocityAlongNormal < 0) {
//            ball.velocity = ball.velocity - normal * 2.0f * velocityAlongNormal;
//        }
//    }
//};
//
//// Physics Engine class
//class PhysicsEngine {
//    std::vector<Ball> balls;
//    Quadtree quadtree;
//    AABB worldBounds;
//    bool gravityEnabled = false;  // Variable to track gravity state
//
//public:
//    PhysicsEngine() : quadtree(0, AABB{ 0, 0, 1920, 1080 }), worldBounds({ 0, 0, 1920, 1080 }) {}
//
//    void addBall(const Ball& ball) {
//        balls.push_back(ball);
//    }
//
//    void update(float dt) {
//        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
//            gravityEnabled = true;  // Enable gravity when the spacebar is pressed
//        }
//
//        quadtree.clear();
//
//        float rectWidth = 1600;
//        float rectHeight = 800;
//        float xOffset = (1920 - rectWidth) / 2;
//        float yOffset = (1080 - rectHeight) / 2;
//
//        for (Ball& ball : balls) {
//            if (gravityEnabled) {
//                ball.velocity.y -= 9.8f * dt;  // Apply gravity if enabled
//            }
//
//            quadtree.insert(ball);
//            ball.update(dt);
//
//            // Wall friction coefficient for simulating spin effect
//            float wallFriction = 0.98f;
//
//            // Check for collisions with the left and right walls
//            if (ball.position.x - ball.radius < xOffset) {
//                ball.position.x = xOffset + ball.radius;  // Reposition
//                ball.velocity.x = -ball.velocity.x * wallFriction;  // Invert x-velocity with friction
//                ball.angularVelocity += ball.velocity.y * 0.1f;  // Simulate spin when hitting wall
//            }
//            else if (ball.position.x + ball.radius > xOffset + rectWidth) {
//                ball.position.x = xOffset + rectWidth - ball.radius;  // Reposition
//                ball.velocity.x = -ball.velocity.x * wallFriction;  // Invert x-velocity with friction
//                ball.angularVelocity += ball.velocity.y * 0.1f;  // Simulate spin when hitting wall
//            }
//
//            // Check for collisions with the top and bottom walls
//            if (ball.position.y - ball.radius < yOffset) {
//                ball.position.y = yOffset + ball.radius;  // Reposition
//                ball.velocity.y = -ball.velocity.y * wallFriction;  // Invert y-velocity with friction
//                ball.angularVelocity += ball.velocity.x * 0.1f;  // Simulate spin when hitting wall
//            }
//            else if (ball.position.y + ball.radius > yOffset + rectHeight) {
//                ball.position.y = yOffset + rectHeight - ball.radius;  // Reposition
//                ball.velocity.y = -ball.velocity.y * wallFriction;  // Invert y-velocity with friction
//                ball.angularVelocity += ball.velocity.x * 0.1f;  // Simulate spin when hitting wall
//            }
//        }
//
//        // Check for collisions between balls
//        for (Ball& ball : balls) {
//            std::vector<Ball*> possibleCollisions;
//            quadtree.retrieve(possibleCollisions, ball);
//
//            // Check collisions with nearby balls
//            for (Ball* other : possibleCollisions) {
//                if (&ball != other && Collision::detectBallBall(ball, *other)) {
//                    Collision::resolveBallBall(ball, *other);
//                }
//            }
//        }
//    }
//
//
//    void render() {
//        // Define the size of the rectangle (centered on the screen)
//        float rectWidth = 1600;  // Set desired width
//        float rectHeight = 800;  // Set desired height
//        float xOffset = (1920 - rectWidth) / 2;
//        float yOffset = (1080 - rectHeight) / 2;
//
//        // Render the rectangle (walls) centered
//        glBegin(GL_LINE_LOOP);
//        glVertex2f(xOffset, yOffset);                           // Bottom-left
//        glVertex2f(xOffset + rectWidth, yOffset);               // Bottom-right
//        glVertex2f(xOffset + rectWidth, yOffset + rectHeight);  // Top-right
//        glVertex2f(xOffset, yOffset + rectHeight);              // Top-left
//        glEnd();
//
//        // Render the balls
//        for (Ball& ball : balls) {
//            ball.render();
//        }
//    }
//};
//
//
//// Main
//int main() {
//    if (!glfwInit()) {
//        std::cerr << "Failed to initialize GLFW\n";
//        return -1;
//    }
//
//    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "2D Ball Collision Simulation with Quadtree and Walls", nullptr, nullptr);
//    if (!window) {
//        std::cerr << "Failed to create GLFW window\n";
//        glfwTerminate();
//        return -1;
//    }
//
//    glfwMakeContextCurrent(window);
//
//    // Set up orthographic projection for a 1920x1080 window, with a centered rectangle
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(0, 1920, 0, 1080, -1, 1);  // Adjust for the new window size
//
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//
//    // In your main() function
//    PhysicsEngine engine;
//
//    // Add multiple balls with slower starting positions and velocities
//    engine.addBall(Ball({ 400, 500 }, BALL_RADIUS));
//    engine.addBall(Ball({ 600, 300 }, BALL_RADIUS));
//    engine.addBall(Ball({ 500, 400 }, BALL_RADIUS));
//    engine.addBall(Ball({ 300, 200 }, BALL_RADIUS));
//    engine.addBall(Ball({ 800, 600 }, BALL_RADIUS));
//    engine.addBall(Ball({ 700, 500 }, BALL_RADIUS));
//    engine.addBall(Ball({ 900, 700 }, BALL_RADIUS));
//
//    while (!glfwWindowShouldClose(window)) {
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        engine.update(0.01f);
//        engine.render();
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glfwDestroyWindow(window);
//    glfwTerminate();
//    return 0;
//}
