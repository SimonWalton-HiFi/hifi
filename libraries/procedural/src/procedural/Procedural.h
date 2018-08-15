//
//  Created by Bradley Austin Davis on 2015/09/05
//  Copyright 2013-2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Procedural_h
#define hifi_Procedural_h

#include <atomic>

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

#include <gpu/Shader.h>
#include <gpu/Pipeline.h>
#include <gpu/Batch.h>
#include <model-networking/ShaderCache.h>
#include <model-networking/TextureCache.h>

using UniformLambdas = std::list<std::function<void(gpu::Batch& batch)>>;
const size_t MAX_PROCEDURAL_TEXTURE_CHANNELS{ 4 };

/**jsdoc
 * An object containing user-defined uniforms for communicating data to shaders.
 * @typedef {object} ProceduralUniforms
 */

/**jsdoc
 * The data used to define a Procedural shader material.
 * @typedef {object} ProceduralData
 * @property {number} version=1 - The version of the procedural shader.
 * @property {string} shaderUrl - A link to a shader.  Currently, only GLSL shaders are supported.  The shader must implement a different method depending on the version.
 * @property {string[]} channels=[] - An array of input texture URLs.  Currently, up to 4 are supported.
 * @property {ProceduralUniforms} uniforms={} - A {@link ProceduralUniforms} object containing all the custom uniforms to be passed to the shader.
 */
struct ProceduralData {
    static QJsonValue getProceduralData(const QString& proceduralJson);
    static ProceduralData parse(const QString& userDataJson);
    // This should only be called from the render thread, as it shares data with Procedural::prepare
    void parse(const QJsonObject&);

    // Rendering object descriptions, from userData
    uint8_t version { 0 };
    QUrl shaderUrl;
    QJsonObject uniforms;
    QJsonArray channels;
};

class ProceduralProgramKey {
public:
    enum FlagBit {
        IS_TRANSPARENT_FLAG = 0,
        IS_SKINNED_FLAG,
        IS_SKINNED_DQ_FLAG,

        NUM_FLAGS
    };

    enum Flag {
        IS_TRANSPARENT = (1 << IS_TRANSPARENT_FLAG),
        IS_SKINNED = (1 << IS_SKINNED_FLAG),
        IS_SKINNED_DQ = (1 << IS_SKINNED_DQ_FLAG),
    };
    typedef unsigned short Flags;

    bool isFlag(short flagNum) const { return bool((_flags & flagNum) != 0); }

    bool isTransparent() const { return isFlag(IS_TRANSPARENT); }
    bool isSkinned() const { return isFlag(IS_SKINNED); }
    bool isSkinnedDQ() const { return isFlag(IS_SKINNED_DQ); }

    Flags _flags = 0;
    short _spare = 0;

    int getRaw() const { return *reinterpret_cast<const int*>(this); }

    ProceduralProgramKey(bool transparent = false, bool isSkinned = false, bool isSkinnedDQ = false) {
        _flags = (transparent ? IS_TRANSPARENT : 0) | (isSkinned ? IS_SKINNED : 0) | (isSkinnedDQ ? IS_SKINNED_DQ : 0);
    }

    ProceduralProgramKey(int bitmask) : _flags(bitmask) {}
};

inline uint qHash(const ProceduralProgramKey& key, uint seed) {
    return qHash(key.getRaw(), seed);
}

inline bool operator==(const ProceduralProgramKey& a, const ProceduralProgramKey& b) {
    return a.getRaw() == b.getRaw();
}

inline bool operator!=(const ProceduralProgramKey& a, const ProceduralProgramKey& b) {
    return a.getRaw() != b.getRaw();
}

// WARNING with threaded rendering it is the RESPONSIBILITY OF THE CALLER to ensure that 
// calls to `setProceduralData` happen on the main thread and that calls to `ready` and `prepare` 
// are treated atomically, and that they cannot happen concurrently with calls to `setProceduralData`
// FIXME better encapsulation
// FIXME better mechanism for extending to things rendered using shaders other than simple.slv
struct Procedural {
public:
    Procedural();
    void setProceduralData(const ProceduralData& proceduralData);

    bool isReady() const;
    bool isEnabled() const { return _enabled; }
    void prepare(gpu::Batch& batch, const glm::vec3& position, const glm::vec3& size, const glm::quat& orientation, const ProceduralProgramKey key = ProceduralProgramKey());

    glm::vec4 getColor(const glm::vec4& entityColor) const;
    quint64 getFadeStartTime() const { return _fadeStartTime; }
    bool isFading() const { return _doesFade && _isFading; }
    void setIsFading(bool isFading) { _isFading = isFading; }
    void setDoesFade(bool doesFade) { _doesFade = doesFade; }

    // Vertex shaders
    gpu::Shader::Source _vertexSource;
    gpu::Shader::Source _vertexSourceSkin;
    gpu::Shader::Source _vertexSourceSkinDQ;

    // Fragment shaders
    gpu::Shader::Source _opaqueFragmentSource;
    gpu::Shader::Source _transparentFragmentSource;

    gpu::StatePointer _opaqueState { std::make_shared<gpu::State>() };
    gpu::StatePointer _transparentState { std::make_shared<gpu::State>() };

protected:
    // Procedural metadata
    ProceduralData _data;

    bool _enabled { false };
    uint64_t _start { 0 };
    int32_t _frameCount { 0 };

    // Rendering object descriptions, from userData
    QString _shaderSource;
    QString _shaderPath;
    quint64 _shaderModified { 0 };
    NetworkShaderPointer _networkShader;
    bool _dirty { false };
    bool _shaderDirty { true };
    bool _uniformsDirty { true };
    bool _channelsDirty { true };

    // Rendering objects
    UniformLambdas _uniforms;
    NetworkTexturePointer _channels[MAX_PROCEDURAL_TEXTURE_CHANNELS];

    QHash<ProceduralProgramKey, gpu::ShaderPointer> _proceduralShaders;
    QHash<ProceduralProgramKey, gpu::PipelinePointer> _proceduralPrograms;

    // Entity metadata
    glm::vec3 _entityDimensions;
    glm::vec3 _entityPosition;
    glm::mat3 _entityOrientation;

private:
    // This should only be called from the render thread, as it shares data with Procedural::prepare
    void setupUniforms(ProceduralProgramKey key);
    void setupChannels(bool shouldCreate, ProceduralProgramKey key);

    std::string replaceProceduralBlock(const std::string& fragmentSource);

    mutable quint64 _fadeStartTime { 0 };
    mutable bool _hasStartedFade { false };
    mutable bool _isFading { false };
    bool _doesFade { true };
    ProceduralProgramKey _prevKey;
};

#endif
