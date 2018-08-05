//
//  Created by Sam Gondelman on 7/31/2018
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ProceduralMaterial_h
#define hifi_ProceduralMaterial_h

#include <model-networking/ModelCache.h>
#include "Procedural.h"

namespace graphics {

class ProceduralMaterial : public NetworkMaterial {
public:
    ProceduralMaterial() : NetworkMaterial() { initializeProcedural(); }
    ProceduralMaterial(const NetworkMaterial& material) : NetworkMaterial(material) { initializeProcedural(); }
    ProceduralMaterial(const ProceduralMaterial& material) : NetworkMaterial(material), _procedural(material._procedural) {}

    void setProceduralData(const QString& data) {
        _proceduralString = data;
        _procedural.setProceduralData(ProceduralData::parse(data));
    }
    Procedural& editProcedural() { return _procedural; }
    const Procedural& getProcedural() { return _procedural; }

    const QString& getProceduralString() { return _proceduralString; }

    void initializeProcedural();

private:
    QString _proceduralString;
    Procedural _procedural;
};
typedef std::shared_ptr<ProceduralMaterial> ProceduralMaterialPointer;

class MaterialLayer {
public:
    MaterialLayer(ProceduralMaterialPointer material, quint16 priority) : material(material), priority(priority) {}

    ProceduralMaterialPointer material{ nullptr };
    quint16 priority{ 0 };
};

class MaterialLayerCompare {
public:
    bool operator()(MaterialLayer left, MaterialLayer right) { return left.priority < right.priority; }
};

class MultiMaterial : public std::priority_queue<MaterialLayer, std::vector<MaterialLayer>, MaterialLayerCompare> {
public:
    bool remove(const MaterialPointer& value) {
        auto it = c.begin();
        while (it != c.end()) {
            if (it->material == value) {
                break;
            }
            it++;
        }
        if (it != c.end()) {
            c.erase(it);
            std::make_heap(c.begin(), c.end(), comp);
            return true;
        } else {
            return false;
        }
    }
};

};  // namespace graphics

#endif
