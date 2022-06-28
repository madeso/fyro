#pragma once

#include "tred/render/geom.h"

#include "tred/render/material.h"


namespace render
{

struct Mesh
{
    Material material;
    Geom geom;
};

}
