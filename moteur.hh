#pragma once

#define NB_RAYS 1
#define NB_RAYS_REFLECTED 3
#define MAX_DEPTH 5

#include <algorithm>
#include <cmath>
#include <ctime>
#include <memory>
#include <random>

#include "image/image.hh"
#include "light/circle_light.hh"
#include "light/point_light.hh"
#include "objects/mesh.hh"
#include "objects/sphere.hh"
#include "objects/triangle.hh"
#include "objects/blob.hh"
#include "texture/ImageTexture.hh"
#include "texture/LightTexture.hh"
#include "texture/UniformTexture.hh"
#include "utils/console.hh"
#include "utils/scene.hh"
#include "utils/vector4.hh"

template <typename T, typename U>
using pair = std::pair<T, U>;

template <typename T>
using optional = std::optional<T>;

template <typename T>
using vector = std::vector<T>;

struct BRDF
{
    Vector4 V; // View vector
    Vector4 N; // Normal vector
    Vector4 L; // Light vector
    Vector4 H; // Half between V and L
    Vector4 R; // Perfect Reflected ray
};

struct LightComputation
{
    Color* color = nullptr;
    float kd = 0; // Diffuse reflection
    float ks = 0; // Specular reflection
    float li = 0; // Light power
    float ns = 0; // Shininess factor
    float nl = 0; // Dot Product between light and normal vector
    float li_red = 0; // Light power for the RED Canal
    float li_blue = 0; // Light power for the BLUE Canal
    float li_green = 0; // Light power for the GREEN Canal
    float products = 0; // Dot Product between light and reflected vector
    float contribution_red =
        0; // Contribution for the RED canal of other objects
    float contribution_blue =
        0; // Contribution for the BLUE canal of other objects
    float contribution_green =
        0; // Contribution for the GREEN canal of other objects
};

int val = -1;
