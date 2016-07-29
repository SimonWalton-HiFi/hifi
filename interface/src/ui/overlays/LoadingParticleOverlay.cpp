//
//  LoadingParticleOverlay.cpp
//
//  Created by Sam Gondelman on 7/27/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "LoadingParticleOverlay.h"

#include <Application.h>
#include "OctreeConstants.h"

// Overlays.addOverlay("particles", {"dimensions": {x: 32768, y: 32768, z: 32768}, "maxParticles": 50000, "userData": {"ProceduralParticles": {"shaderUrl": "https://hifi-content.s3.amazonaws.com/samuel/loadingParticles.fs", "uniforms": [{"numObjects":[2,0,0,0]}, {"objects":[[5, 5, 5, 0], [1, 1, 1, 0], [10, 10, 10, 0], [3, 3, 3, 0]]}]}}});
// Overlays.editOverlay(1, {"userData": {"ProceduralParticles": {"shaderUrl": "https://hifi-content.s3.amazonaws.com/samuel/loadingParticles.fs", "uniforms": [{"numObjects":[0,0,0,0]}]}}});

// Overlays.addOverlay("particles", {"dimensions": {x: 32768, y: 32768, z: 32768}, "maxParticles": 50000, "userData": {"ProceduralParticles": {"shaderUrl": "https://hifi-content.s3.amazonaws.com/samuel/loadingParticles.fs", "uniforms": []}}})

LoadingParticleOverlay::LoadingParticleOverlay() {
    QVariantMap properties;

    QVariantMap particleProperties;
    particleProperties.insert("shaderUrl", "https://hifi-content.s3.amazonaws.com/samuel/loadingParticles.fs");

    // for testing
    /*QList<QVariant> uniforms;

    QVariantMap numObjects;
    QList<QVariant> numObjectsVal = { 3, 0, 0, 0 };
    numObjects.insert("numObjects", numObjectsVal);
    uniforms.append(numObjects);

    QVariantMap objects;
    QList<QVariant> objectsVal = { 5, 5, 5, 0,
        0.01, 0.01, 0.01, 0,
        10, 10, 10, 0,
        0.1, 0.1, 0.1, 0,
        15, 15, 15, 0,
        1.0, 1.0, 1.0, 0 };
    QList<QVariant> objectsVal = { 0, 1, 0, 0,
        0.5, 0.5, 0.5, 0,
        0, 3, 0, 0,
        0.5, 0.5, 0.5, 0,
        0, 5, 0, 0,
        0.5, 0.5, 0.5, 0 };
    objects.insert("objects", objectsVal);
    uniforms.append(objects);

    particleProperties.insert("uniforms", uniforms);*/
    // end for testing

    QVariantMap userData;
    userData.insert("ProceduralParticles", particleProperties);

    // Set the dimensions to be as big as the domain so the effect is never frustum culled
    properties.insert("dimensions", QVector3D(TREE_SCALE, TREE_SCALE, TREE_SCALE));
    const int NUM_PARTICLES = 50000;
    properties.insert("maxParticles", NUM_PARTICLES);
    properties.insert("userData", userData);

    _overlayID = qApp->getOverlays().addOverlay("particles", properties);
}

void LoadingParticleOverlay::update() {
    quint64 unrezzedLastUpdatedTime = qApp->getMain3DScene()->getUnrezzedLastUpdatedTime();
    if (_particlesLastUpdatedTime < unrezzedLastUpdatedTime) {
        _particlesLastUpdatedTime = unrezzedLastUpdatedTime;
        auto objectList = qApp->getMain3DScene()->getUnrezzedObjects();

        QVariantMap properties;
        QVariantMap particleProperties;
        particleProperties.insert("shaderUrl", "https://hifi-content.s3.amazonaws.com/samuel/loadingParticles.fs");

        QList<QVariant> uniforms;

        QVariantMap numObjects;
        QList<QVariant> numObjectsVal = { objectList.size(), 0, 0, 0 };
        numObjects.insert("numObjects", numObjectsVal);
        uniforms.append(numObjects);

        QVariantMap objects;
        QList<QVariant> objectsVal;

        const int MAX_OBJECTS = 50;
        for(int i = 0; i < std::min(objectList.length(), MAX_OBJECTS); i++) {
            auto center = objectList[i].calcCenter();
            auto dim = objectList[i].getDimensions();

            objectsVal.push_back(center.x);
            objectsVal.push_back(center.y);
            objectsVal.push_back(center.z);
            objectsVal.push_back(dim.x);
            objectsVal.push_back(dim.y);
            objectsVal.push_back(dim.z);
        }

        objects.insert("objects", objectsVal);
        uniforms.append(objects);

        particleProperties.insert("uniforms", uniforms);

        QVariantMap userData;
        userData.insert("ProceduralParticles", particleProperties);
        properties.insert("userData", userData);

        qApp->getOverlays().editOverlay(_overlayID, properties);
    }
}
