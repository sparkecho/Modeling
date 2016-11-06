/*******************************************************************
 ** This code is part of Breakout.
 **
 ** Breakout is free software: you can redistribute it and/or modify
 ** it under the terms of the CC BY 4.0 license as published by
 ** Creative Commons, either version 4 of the License, or (at your
 ** option) any later version.
 ******************************************************************/
#include "game.hpp"
#include "resource_manager.hpp"
#include "sprite_renderer.hpp"
#include "game_object.hpp"
#include "ball_object.hpp"
#include "particle_generator.hpp"


// Game-related State data
SpriteRenderer    *Renderer;
GameObject        *Player;
BallObject        *Ball;
ParticleGenerator *Particles;
ParticleGenerator *Particlesl;
ParticleGenerator *Particlesr;


Game::Game(GLuint width, GLuint height) 
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height) 
{ 

}

Game::~Game()
{
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    delete Particlesl;
    delete Particlesr;
}

void Game::Init()
{
    // Load shaders
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag", nullptr, "particle");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
    // Load textures
    ResourceManager::LoadTexture("textures/background.png", GL_FALSE, "background");
    ResourceManager::LoadTexture("textures/fire.png", GL_TRUE, "fire");
    ResourceManager::LoadTexture("textures/paddle.png", GL_TRUE, "paddle");
    ResourceManager::LoadTexture("textures/particle.png", GL_TRUE, "particle");
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Particles  = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 9000);
    Particlesl = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 9000);
    Particlesr = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 9000);
    // Configure game objects
    glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("fire"));
}

void Game::Update(GLfloat dt)
{
    // Update particles
    Particles->Update (dt, *Ball, 40, glm::vec2(Ball->Radius, Ball->Radius + 20), glm::vec2(0.0f, -30.0f)); // radius / 2  Ball->Radius / 2
    Particlesl->Update(dt, *Ball, 40, glm::vec2(Ball->Radius / 4 * 3, Ball->Radius + 20), glm::vec2(5.0f, -20.0f));
    Particlesr->Update(dt, *Ball, 40, glm::vec2(Ball->Radius / 4 * 5, Ball->Radius + 20), glm::vec2(-5.0f, -20.0f));
    // // Check loss condition
    // if (Ball->Position.y >= this->Height) // Did ball reach bottom edge?
    // {
    //     this->ResetPlayer();
    // }
}


void Game::ProcessInput(GLfloat dt)
{
    if (this->State == GAME_ACTIVE)
    {
        // GLfloat velocity = PLAYER_VELOCITY * dt;
        GLfloat velocity = 200.0f * dt;
        // Move playerboard
        if (this->Keys[GLFW_KEY_A] && Ball->Stuck == false)
        {
            if (Ball->Position.x >= 0)
                Ball->Position.x -= velocity;
        }
        if (this->Keys[GLFW_KEY_D] && Ball->Stuck == false)
        {
            if (Ball->Position.x <= this->Width - Ball->Size.x)
                Ball->Position.x += velocity;
        }
        if (this->Keys[GLFW_KEY_W] && Ball->Stuck == false)
        {
            if (Ball->Position.y >= 0)
                Ball->Position.y -= velocity;
        }
        if (this->Keys[GLFW_KEY_S] && Ball->Stuck == false)
        {
            if (Ball->Position.y <= this->Height - Ball->Size.y)
                Ball->Position.y += velocity;
        }
        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
    }

}

void Game::Render()
{
    if (this->State == GAME_ACTIVE)
    {
        // Draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
        // Draw player
        // Player->Draw(*Renderer);
        // Draw ball
        Ball->Draw(*Renderer);
        // Draw particles
        if(Ball->Stuck == false)
        {
            Particles->Draw();
            Particlesl->Draw();
            Particlesr->Draw();
        }
    }
}



void Game::ResetPlayer()
{
    // Reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);
}


// Calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    GLfloat max = 0.0f;
    GLuint best_match = -1;
    for (GLuint i = 0; i < 4; i++)
    {
        GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}   

