//
//  Created by Sam Gondelman on 7/20/16
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ProceduralParticles.h"

#include "glm/glm.hpp"

#include <gpu/Batch.h>
#include <gpu/Framebuffer.h>
#include <FramebufferCache.h>

#include <DependencyManager.h>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>

#include "procedural_particle_update_frag.h"
#include "draw_procedural_particle_vert.h"
#include "untextured_procedural_particle_frag.h"
#include "textured_procedural_particle_frag.h"
#include <gpu/DrawUnitQuadTexcoord_vert.h>

static const QString PROCEDURAL_USER_DATA_KEY = "ProceduralParticle";
static const QString URL_KEY = "shaderUrl";
static const QString UNIFORMS_KEY = "uniforms";

static int UPDATE_PARTICLES_BUFFER = -1;
static int UPDATE_HIFI_BUFFER = -1;
static int UPDATE_PARTICLES = -1;
static int NOTEX_DRAW_BUFFER = -1;
static int NOTEX_DRAW_PARTICLES = -1;
static int DRAW_BUFFER = -1;
static int DRAW_PARTICLES = 0;
static int DRAW_TEXTURE = 1;

ProceduralParticles::ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM) {
    _particleUniforms = ParticleUniforms();
    _particleUniforms.color = color;
    _particleUniforms.radius = radius;
    _particleUniforms.iResolution.z = maxParticles;

    // Create the FBOs
    for (int i = 0; i < 2; i++) {
        _particleBuffers.append(gpu::FramebufferPointer(gpu::Framebuffer::create(gpu::Format(gpu::Dimension::VEC4,
                                gpu::Type::FLOAT,
                                gpu::Semantic::RGBA), 1, 1)));
    }

    // Resize the FBOs
    setMaxParticles(maxParticles, MAX_DIM);

    _uniformBuffer = std::make_shared<Buffer>(sizeof(ParticleUniforms), (const gpu::Byte*) &_particleUniforms);

    _proceduralDataDirty = false;
}

ProceduralParticles::ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM, const QString& userDataJson) :
    ProceduralParticles(color, radius, maxParticles, MAX_DIM)
{
    parse(userDataJson);
    _proceduralDataDirty = true;
}

void ProceduralParticles::parse(const QString& userDataJson) {
    auto proceduralData = getProceduralData(userDataJson);
    // Instead of parsing, prep for a parse on the rendering thread
    // This will be called by ProceduralParticles::ready
    std::lock_guard<std::mutex> lock(_proceduralDataMutex);
    _proceduralData = proceduralData.toObject();

    // Mark as dirty after modifying _proceduralData, but before releasing lock
    // to avoid setting it after parsing has begun
    _proceduralDataDirty = true;
}

// Example
//{
//    "ProceduralParticles": {
//        "uniforms": {},
//    }
//}
QJsonValue ProceduralParticles::getProceduralData(const QString& proceduralJson) {
    if (proceduralJson.isEmpty()) {
        return QJsonValue();
    }

    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(proceduralJson.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return QJsonValue();
    }

    return doc.object()[PROCEDURAL_USER_DATA_KEY];
}

bool ProceduralParticles::parseUrl(const QUrl& shaderUrl) {
    if (!shaderUrl.isValid()) {
        if (!shaderUrl.isEmpty()) {
            qWarning() << "Invalid shader URL: " << shaderUrl;
        }
        _networkShader.reset();
        return false;
    }

    _shaderUrl = shaderUrl;
    _shaderDirty = true;

    if (_shaderUrl.isLocalFile()) {
        _shaderPath = _shaderUrl.toLocalFile();
        qDebug() << "Shader path: " << _shaderPath;
        if (!QFile(_shaderPath).exists()) {
            _networkShader.reset();
            return false;;
        }
    }
    else {
        qDebug() << "Shader url: " << _shaderUrl;
        _networkShader = ShaderCache::instance().getShader(_shaderUrl);
    }

    return true;
}

bool ProceduralParticles::parseUniforms(const QJsonArray& uniforms) {
    if (_parsedUniforms != uniforms) {
        _parsedUniforms = uniforms;
        _uniformsDirty = true;
    }

    return true;
}

void ProceduralParticles::parse(const QJsonObject& proceduralData) {
    _enabled = false;

    auto shaderUrl = proceduralData[URL_KEY].toString();
    shaderUrl = ResourceManager::normalizeURL(shaderUrl);
    auto uniforms = proceduralData[UNIFORMS_KEY].toArray();

    bool isValid = true;

    // Run through parsing regardless of validity to clear old cached resources
    isValid = parseUrl(shaderUrl) && isValid;
    isValid = parseUniforms(uniforms) && isValid;

    if (!proceduralData.isEmpty() && isValid) {
        _enabled = true;
    }
}

void ProceduralParticles::setMaxParticles(quint32 maxParticles, const quint32 MAX_DIM) {
    _particleUniforms.iResolution.z = maxParticles;

    int width = std::min(MAX_DIM, maxParticles);
    int height = ((int)(maxParticles / MAX_DIM) + 1) * 2;
    for (auto& buffer : _particleBuffers) {
        buffer->resize(width, height);
    }

    _particleUniforms.iResolution.x = width;
    _particleUniforms.iResolution.y = height;

    // Restart the simulation when the number of particles changes
    _particleUniforms.firstPass = true;
}

void ProceduralParticles::setTextures(const QString& textures) {
    if (textures.isEmpty()) {
        _texture.clear();
    } else {
        _texture = DependencyManager::get<TextureCache>()->getTexture(textures);
    }
}

void ProceduralParticles::update(float iGlobalTime, float iDeltaTime) {
    _particleUniforms.iGlobalTime = iGlobalTime;
    _particleUniforms.iDeltaTime = iDeltaTime;

    // TODO: remove (for testing)
    if (!_texture) {
        _texture = DependencyManager::get<TextureCache>()->getTexture(QString("https://hifi-public.s3.amazonaws.com/alan/Particles/Particle-Sprite-Smoke-1.png"));
    }
}

bool ProceduralParticles::ready() {
    // Load any changes to the procedural
    // Check for changes atomically, in case they are currently being made
    if (_proceduralDataDirty) {
        std::lock_guard<std::mutex> lock(_proceduralDataMutex);
        parse(_proceduralData);

        // Reset dirty flag after reading _proceduralData, but before releasing lock
        // to avoid resetting it after more data is set
        _proceduralDataDirty = false;
    }

    if (!_enabled) {
        return false;
    }

    // Do we have a network or local shader, and if so, is it loaded?
    if (_shaderPath.isEmpty() && (!_networkShader || !_networkShader->isLoaded())) {
        return false;
    }

    return true;
}

void ProceduralParticles::setupUniforms() {
    // setup the particle buffer
    memcpy(&editParticleUniforms(), &_particleUniforms, sizeof(ParticleUniforms));

    // setup the user uniform buffer
    std::vector<float> data;

    // Set any userdata specified uniforms 
    foreach(QJsonValue key, _parsedUniforms) {
        std::string uniformName = key.toString().toLocal8Bit().data();
        int32_t slot = UPDATE_HIFI_BUFFER;
        if (gpu::Shader::INVALID_LOCATION == slot) {
            continue;
        }
        QJsonValue value = _parsedUniforms[key];
        if (value.isDouble()) {
            float v = value.toDouble();
            _uniforms.push_back([=](gpu::Batch& batch) {
                batch._glUniform1f(slot, v);
            });
        }
        else if (value.isArray()) {
            auto valueArray = value.toArray();
            switch (valueArray.size()) {
            case 0:
                break;

            case 1: {
                float v = valueArray[0].toDouble();
                _uniforms.push_back([=](gpu::Batch& batch) {
                    batch._glUniform1f(slot, v);
                });
                break;
            }

            case 2: {
                glm::vec2 v{ valueArray[0].toDouble(), valueArray[1].toDouble() };
                _uniforms.push_back([=](gpu::Batch& batch) {
                    batch._glUniform2f(slot, v.x, v.y);
                });
                break;
            }

            case 3: {
                glm::vec3 v{
                    valueArray[0].toDouble(),
                    valueArray[1].toDouble(),
                    valueArray[2].toDouble(),
                };
                _uniforms.push_back([=](gpu::Batch& batch) {
                    batch._glUniform3f(slot, v.x, v.y, v.z);
                });
                break;
            }

            default:
            case 4: {
                glm::vec4 v{
                    valueArray[0].toDouble(),
                    valueArray[1].toDouble(),
                    valueArray[2].toDouble(),
                    valueArray[3].toDouble(),
                };
                _uniforms.push_back([=](gpu::Batch& batch) {
                    batch._glUniform4f(slot, v.x, v.y, v.z, v.w);
                });
                break;
            }
            }
        }
    }
}

void ProceduralParticles::render(RenderArgs* args) {
    if (_shaderUrl.isLocalFile()) {
        auto lastModified = (quint64)QFileInfo(_shaderPath).lastModified().toMSecsSinceEpoch();
        if (lastModified > _shaderModified) {
            QFile file(_shaderPath);
            file.open(QIODevice::ReadOnly);
            _shaderSource = QTextStream(&file).readAll();
            _shaderDirty = true;
            _shaderModified = lastModified;
        }
    } else if (_networkShader && _networkShader->isLoaded()) {
        _shaderSource = _networkShader->_source;
    }

    // lazy creation of particle system pipeline
    if (!_updatePipeline || !_untexturedPipeline || !_texturedPipeline || _shaderDirty) {
        createPipelines();
    }

    auto mainViewport = args->_viewport;
    gpu::Batch& batch = *args->_batch;

    if (_shaderDirty || _uniformsDirty) {
        setupUniforms();
    }

    _shaderDirty = _uniformsDirty = false;

    // Update the particles in the other FBO based on the current FBO's texture
    batch.setPipeline(_updatePipeline);
    batch.setUniformBuffer(UPDATE_PARTICLES_BUFFER, _uniformBuffer);
    batch.setUniformBuffer(UPDATE_HIFI_BUFFER, _hifiBuffer);
    batch.setFramebuffer(_particleBuffers[(int)!_evenPass]);
    glm::ivec4 viewport = glm::ivec4(0, 0, _particleBuffers[(int)!_evenPass]->getWidth(), _particleBuffers[(int)!_evenPass]->getHeight());
    batch.setViewportTransform(viewport);
    batch.setResourceTexture(UPDATE_PARTICLES, _particleBuffers[(int)_evenPass]->getRenderBuffer(0));
    batch.draw(gpu::TRIANGLE_STRIP, 4);

    // Render using the updated FBO's texture
    auto lightingFramebuffer = DependencyManager::get<FramebufferCache>()->getLightingFramebuffer();
    batch.setFramebuffer(lightingFramebuffer);
    batch.setViewportTransform(mainViewport);
    if (_texture && _texture->isLoaded()) {
        batch.setPipeline(_texturedPipeline);
        batch.setUniformBuffer(DRAW_BUFFER, _uniformBuffer);
        batch.setResourceTexture(DRAW_PARTICLES, _particleBuffers[(int)!_evenPass]->getRenderBuffer(0));
        batch.setResourceTexture(DRAW_TEXTURE, _texture->getGPUTexture());
    }
    else {
        batch.setPipeline(_untexturedPipeline);
        batch.setUniformBuffer(NOTEX_DRAW_BUFFER, _uniformBuffer);
        batch.setResourceTexture(NOTEX_DRAW_PARTICLES, _particleBuffers[(int)!_evenPass]->getRenderBuffer(0));
    }

    batch.setModelTransform(Transform());

    batch.draw(gpu::TRIANGLES, 3 * _particleUniforms.iResolution.z);
    //batch.drawInstanced((gpu::uint32)_particleUniforms.iResolution.z, gpu::TRIANGLES, (gpu::uint32)VERTEX_PER_PARTICLE);

    _particleUniforms.firstPass = false;
    _evenPass = !_evenPass;
}

void ProceduralParticles::createPipelines() {
    std::string particleBuffer = "particleBuffer";
    std::string hifiBuffer = "hifiBuffer";
    std::string particlesTex = "particlesTex";
    std::string colorMap = "colorMap";
    if (!_updatePipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);

        auto vertShader = gpu::Shader::createVertex(std::string(DrawUnitQuadTexcoord_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(procedural_particle_update_frag));

        auto program = gpu::Shader::createProgram(vertShader, fragShader);

        gpu::Shader::BindingSet slotBindings;
        gpu::Shader::makeProgram(*program, slotBindings);

        UPDATE_PARTICLES_BUFFER = program->getBuffers().findLocation(particleBuffer);
        UPDATE_HIFI_BUFFER = program->getBuffers().findLocation(hifiBuffer);
        UPDATE_PARTICLES = program->getTextures().findLocation(particlesTex);

        _updatePipeline = gpu::Pipeline::create(program, state);
    }
    if (!_untexturedPipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);
        state->setDepthTest(true, false, gpu::LESS_EQUAL);
        state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE,
            gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);

        auto vertShader = gpu::Shader::createVertex(std::string(draw_procedural_particle_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(untextured_procedural_particle_frag));
        auto program = gpu::Shader::createProgram(vertShader, fragShader);

        gpu::Shader::BindingSet slotBindings;
        gpu::Shader::makeProgram(*program, slotBindings);

        NOTEX_DRAW_BUFFER = program->getBuffers().findLocation(particleBuffer);
        NOTEX_DRAW_PARTICLES = program->getTextures().findLocation(particlesTex);

        _untexturedPipeline = gpu::Pipeline::create(program, state);
    }
    if (!_texturedPipeline) {
        auto state = std::make_shared<gpu::State>();
        state->setCullMode(gpu::State::CULL_BACK);
        state->setDepthTest(true, false, gpu::LESS_EQUAL);
        state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE,
            gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);

        auto vertShader = gpu::Shader::createVertex(std::string(draw_procedural_particle_vert));
        auto fragShader = gpu::Shader::createPixel(std::string(textured_procedural_particle_frag));
        auto program = gpu::Shader::createProgram(vertShader, fragShader);

        // Request these specifically because it doesn't work otherwise
        gpu::Shader::BindingSet slotBindings;
        slotBindings.insert(gpu::Shader::Binding(particlesTex, DRAW_PARTICLES));
        slotBindings.insert(gpu::Shader::Binding(colorMap, DRAW_TEXTURE));
        gpu::Shader::makeProgram(*program, slotBindings);

        DRAW_BUFFER = program->getBuffers().findLocation(particleBuffer);
        DRAW_PARTICLES = program->getTextures().findLocation(particlesTex);
        DRAW_TEXTURE = program->getTextures().findLocation(colorMap);

        _texturedPipeline = gpu::Pipeline::create(program, state);
    }
}