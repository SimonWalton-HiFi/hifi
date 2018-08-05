//
//  Created by Sam Gondelman on 8/4/2018
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ProceduralMaterial.h"

#include <StencilMaskPass.h>
#include "render-utils/simple_vert.h"
#include "render-utils/simple_frag.h"
#include "render-utils/simple_transparent_frag.h"


void graphics::ProceduralMaterial::initializeProcedural() {
    _procedural._vertexSource = simple_vert::getSource();
    // FIXME: Setup proper uniform slots and use correct pipelines for forward rendering
    _procedural._opaquefragmentSource = simple_frag::getSource();
    // FIXME: Transparent procedural entities only seem to work if they use the opaque pipelines
    //_procedural._transparentfragmentSource = simple_transparent_frag::getSource();
    _procedural._transparentfragmentSource = simple_frag::getSource();
    _procedural._opaqueState->setCullMode(gpu::State::CULL_NONE);
    _procedural._opaqueState->setDepthTest(true, true, gpu::LESS_EQUAL);
    PrepareStencil::testMaskDrawShape(*_procedural._opaqueState);
    _procedural._opaqueState->setBlendFunction(false,
        gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA,
        gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);
    _procedural._transparentState->setCullMode(gpu::State::CULL_BACK);
    _procedural._transparentState->setDepthTest(true, true, gpu::LESS_EQUAL);
    PrepareStencil::testMask(*_procedural._transparentState);
    _procedural._transparentState->setBlendFunction(true,
        gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA,
        gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);
}