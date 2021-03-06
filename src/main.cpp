#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include "engine/separate_application.h"
#include "engine/MyECS.h"
#include "engine/utils.h"
#include "movement.h"
#include "engine/window_glfw.h"
#include "engine/scene.h"
#include "engine/renderer2d.h"
using namespace Escape;
using namespace Escape::Render;

class Logic : public SystemManager
{
    class PostInit : public ECSSystem
    {
        Logic *logic;

    public:
        PostInit(Logic *logic) : logic(logic) {}
        void initialize() override
        {
            ECSSystem::initialize();
            logic->player = world->create();
            logic->player->assign<Position>(Position(0, 0));
            logic->player->assign<Name>("agent");
        }
    };

public:
    Entity *player;
    TimeServer *timeserver;
    void initialize() override
    {
        SystemManager::initialize();
        timeserver = findSystem<Application>()->timeserver;
    }
    Logic()
    {
        addSubSystem(new PostInit(this));
    }

    void fire(Entity *ent, float angle)
    {
        // weapon_system->fire(ent, angle);
    }
    void move(Entity *ent, const glm::vec2 &vel)
    {
        // movement_system->move(ent, vel);
    }
};

class Display : public WindowGLFW
{
    Scene scene;
    Renderer2D renderer;
    Logic *logic;
    World *world;

public:
    Display() : WindowGLFW("Escape", 800, 600)
    {
        windowResized(800, 600);
    }
    void initialize() override
    {
        WindowGLFW::initialize();
        logic = findSystem<Logic>();
        world = logic->getWorld();
    }
    void windowResized(int width, int height) override
    {
        WindowGLFW::windowResized(width, height);
        scene.mat = glm::ortho<float>(-width / 2, width / 2, -height / 2, height / 2);
    }
    virtual void processInput() override
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (logic->player == nullptr)
            return;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            x -= width / 2;
            y = height / 2 - y;
            // std::cerr << "Cursor: " << x << " " << y << std::endl;
            auto pos = logic->player->get<Position>();
            assert(pos.isValid());
            float angle = atan2(y - pos->y, x - pos->x);
            logic->fire(logic->player, angle);
        }

        glm::vec2 vel(0, 0);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            vel.y += 1;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            vel.y += -1;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            vel.x += -1;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            vel.x += 1;
        float spd = glm::length(*(glm::vec2 *)&vel);
        if (spd > 0)
        {
            if (spd > 1)
            {
                vel /= spd;
            }
            vel *= 64.0f;
        }
        logic->move(logic->player, vel);
    }
    void render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.applyScene(scene);
        world->each<Name>([&, this](Entity *ent, ComponentHandle<Name> name) {
            if (name.get() == "agent")
            {
                renderAgent(ent);
            }
            if (name.get() == "bullet")
            {
                renderBullet(ent);
            }
        });
    }
    void renderAgent(Entity *ent)
    {
        auto pos = ent->get<Position>();
        renderer.drawRect(Rectangle(pos->x, pos->y, 32, 32, 1, 1, 1));
    }
    void renderBullet(Entity *ent)
    {
        auto &&pos = ent->get<Position>();
        assert(pos.isValid());
        renderer.drawRect(Rectangle(pos->x, pos->y, 2, 2, 1, 0, 0));
    }
};

int main()
{
    Escape::SeparateApplication app(new Display(), new Logic());
    app.loop();
    return 0;
}