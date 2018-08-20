//
//  Created by Sam Gondelman on 8/4/2018
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ProceduralMaterial.h"

#include <shaders/Shaders.h>

void graphics::ProceduralMaterial::initializeProcedural() {
    _procedural._vertexSource = gpu::Shader::getVertexShaderSource(shader::render_utils::vertex::simple);
    _procedural._vertexSourceSkin = gpu::Shader::getVertexShaderSource(shader::render_utils::vertex::simple_skin);
    _procedural._vertexSourceSkinDQ = gpu::Shader::getVertexShaderSource(shader::render_utils::vertex::simple_skin_dq);
    // FIXME: Setup proper uniform slots and use correct pipelines for forward rendering
    _procedural._opaqueFragmentSource = gpu::Shader::getFragmentShaderSource(shader::render_utils::fragment::simple);
    _procedural._transparentFragmentSource = gpu::Shader::getFragmentShaderSource(shader::render_utils::fragment::simple_transparent);
}