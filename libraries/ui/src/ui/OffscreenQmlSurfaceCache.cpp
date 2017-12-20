//
//  Created by Bradley Austin Davis on 2017-01-11
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "OffscreenQmlSurfaceCache.h"

#include <QtGui/QOpenGLContext>
#include <QtQml/QQmlContext>

#include <PathUtils.h>
#include "OffscreenQmlSurface.h"

OffscreenQmlSurfaceCache::OffscreenQmlSurfaceCache() {
}

OffscreenQmlSurfaceCache::~OffscreenQmlSurfaceCache() {
    _cache.clear();
}

QSharedPointer<OffscreenQmlSurface> OffscreenQmlSurfaceCache::acquire(const QString& rootSource) {
    auto& list = _cache[rootSource].first;
    if (list.empty()) {
        list.push_back(buildSurface(rootSource));
    }
    auto result = list.front();
    list.pop_front();
    return result;
}

void OffscreenQmlSurfaceCache::reserve(const QString& rootSource, int count) {
    auto& listAndReserved = _cache[rootSource];
    while (listAndReserved.first.size() < count) {
        listAndReserved.first.push_back(buildSurface(rootSource));
    }
    listAndReserved.second += count;
}

void OffscreenQmlSurfaceCache::release(const QString& rootSource, const QSharedPointer<OffscreenQmlSurface>& surface) {
    auto& listAndReserved = _cache[rootSource];
    if (listAndReserved.first.length() < listAndReserved.second) {
        listAndReserved.first.push_back(surface);
    }
}

QSharedPointer<OffscreenQmlSurface> OffscreenQmlSurfaceCache::buildSurface(const QString& rootSource) {
    auto surface = QSharedPointer<OffscreenQmlSurface>(new OffscreenQmlSurface());
    surface->create();
    surface->setBaseUrl(QUrl::fromLocalFile(PathUtils::resourcesPath() + "/qml/"));
    surface->load(rootSource);
    return surface;
}

