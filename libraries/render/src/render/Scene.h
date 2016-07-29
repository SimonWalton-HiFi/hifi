//
//  Scene.h
//  render/src/render
//
//  Created by Sam Gateau on 1/11/15.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_Scene_h
#define hifi_render_Scene_h

#include "Item.h"
#include "SpatialTree.h"

namespace render {

class Engine;

class PendingChanges {
public:
    PendingChanges() {}
    ~PendingChanges() {}

    void resetItem(ItemID id, const PayloadPointer& payload);
    void removeItem(ItemID id);

    template <class T> void updateItem(ItemID id, std::function<void(T&)> func) {
        updateItem(id, std::make_shared<UpdateFunctor<T>>(func));
    }

    void updateItem(ItemID id, const UpdateFunctorPointer& functor);
    void updateItem(ItemID id) { updateItem(id, nullptr); }

    void merge(PendingChanges& changes);

    ItemIDs _resetItems; 
    Payloads _resetPayloads;
    ItemIDs _removedItems;
    ItemIDs _updatedItems;
    UpdateFunctors _updateFunctors;

protected:
};
typedef std::queue<PendingChanges> PendingChangesQueue;


// Scene is a container for Items
// Items are introduced, modified or erased in the scene through PendingChanges
// Once per Frame, the PendingChanges are all flushed
// During the flush the standard buckets are updated
// Items are notified accordingly on any update message happening
class Scene {
public:
    Scene(glm::vec3 origin, float size);
    ~Scene() {}

    // This call is thread safe, can be called from anywhere to allocate a new ID
    ItemID allocateID();

    // Check that the ID is valid and allocated for this scene, this a threadsafe call
    bool isAllocatedID(const ItemID& id);

    // THis is the total number of allocated items, this a threadsafe call
    size_t getNumItems() const { return _numAllocatedItems.load(); }

    // Enqueue change batch to the scene
    void enqueuePendingChanges(const PendingChanges& pendingChanges);

    // Process the penging changes equeued
    void processPendingChangesQueue();

    // Keep track of any objects that are in the processing of rezzing, but can't be displayed yet
    void clearUnrezzedObjects() {
        _unrezzedObjectsPriority = std::priority_queue<QPair<float, QUuid>, std::vector<QPair<float, QUuid>>, PriorityComparator>();
        _unrezzedObjects.clear();
        _unrezzedLastUpdatedTime = usecTimestampNow();
    }
    QList<AABox> getUnrezzedObjects() {
        auto objectsPriorityCopy = _unrezzedObjectsPriority;
        QList<AABox> unrezzedObjects;
        while (objectsPriorityCopy.size() > 0) {
            auto objectsPair = objectsPriorityCopy.top();
            if (_unrezzedObjects.contains(objectsPair.second)) {
                unrezzedObjects.append(_unrezzedObjects[objectsPair.second]);
            }
            objectsPriorityCopy.pop();
        }
        return unrezzedObjects;
    }
    void updateUnrezzedObject(float priority, QUuid entityID, AABox bounds) {
        _unrezzedObjectsPriority.push(QPair<float, QUuid>(priority, entityID));
        _unrezzedObjects.insert(entityID, bounds);
        _unrezzedLastUpdatedTime = usecTimestampNow();
    }
    void removeUnrezzedObject(QUuid entityID) {
        // TODO: remove from priority queue
        int numRemoved = _unrezzedObjects.remove(entityID);
        if (numRemoved > 0) {
            _unrezzedLastUpdatedTime = usecTimestampNow();
        }
    }
    quint64 getUnrezzedLastUpdatedTime() { return _unrezzedLastUpdatedTime; }

    // This next call are  NOT threadsafe, you have to call them from the correct thread to avoid any potential issues

    // Access a particular item form its ID
    // WARNING, There is No check on the validity of the ID, so this could return a bad Item
    const Item& getItem(const ItemID& id) const { return _items[id]; }

    // Access the spatialized items
    const ItemSpatialTree& getSpatialTree() const { return _masterSpatialTree; }

    // Access non-spatialized items (overlays, backgrounds)
    const ItemIDSet& getNonspatialSet() const { return _masterNonspatialSet; }

protected:
    // Thread safe elements that can be accessed from anywhere
    std::atomic<unsigned int> _IDAllocator{ 1 }; // first valid itemID will be One
    std::atomic<unsigned int> _numAllocatedItems{ 1 }; // num of allocated items, matching the _items.size()
    std::mutex _changeQueueMutex;
    PendingChangesQueue _changeQueue;

    // The actual database
    // database of items is protected for editing by a mutex
    std::mutex _itemsMutex;
    Item::Vector _items;
    ItemSpatialTree _masterSpatialTree;
    ItemIDSet _masterNonspatialSet;

    void resetItems(const ItemIDs& ids, Payloads& payloads);
    void removeItems(const ItemIDs& ids);
    void updateItems(const ItemIDs& ids, UpdateFunctors& functors);

    struct PriorityComparator {
        bool operator()(QPair<float, QUuid> a, QPair<float, QUuid> b) {
            return a.first > b.first;
        }
    };

    std::priority_queue<QPair<float, QUuid>, std::vector<QPair<float, QUuid>>, PriorityComparator> _unrezzedObjectsPriority;
    QHash<QUuid, AABox> _unrezzedObjects;
    quint64 _unrezzedLastUpdatedTime { 0 };

    friend class Engine;
};

typedef std::shared_ptr<Scene> ScenePointer;
typedef std::vector<ScenePointer> Scenes;

}

#endif // hifi_render_Scene_h
