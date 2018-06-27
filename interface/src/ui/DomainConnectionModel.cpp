//
//  DomainConnectionModel.cpp
//
//  Created by Vlad Stelmahovsky
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "DomainConnectionModel.h"
#include <QLoggingCategory>

#include <NodeList.h>
#include <NumericalConstants.h>

Q_LOGGING_CATEGORY(dcmodel, "hifi.dcmodel")

DomainConnectionModel::DomainConnectionModel(QAbstractItemModel* parent) :
    QAbstractItemModel(parent)
{}

DomainConnectionModel::~DomainConnectionModel() {
}

QVariant DomainConnectionModel::data(const QModelIndex& index, int role) const {
    //sanity

    const LimitedNodeList::ConnectionTimesMap& times =
              DependencyManager::get<NodeList>()->getLastConnectionTimes();

    if (!index.isValid() || index.row() >= times.size())
        return QVariant();

    
    using Steps = std::vector<std::pair<LimitedNodeList::ConnectionStep, quint64>>;
    Steps steps;
    steps.reserve(times.size());

    steps.insert(steps.begin(), times.cbegin(), times.cend());
    std::sort(steps.begin(), steps.end(),
        [](Steps::value_type& s1, Steps::value_type& s2) { return s1.second < s2.second; });
    
    /// setup our data with the values from the NodeList
    quint64 firstStepTime = steps[0].second / USECS_PER_MSEC;
    quint64 timestamp = steps[index.row()].second;

    quint64 stepTime = timestamp / USECS_PER_MSEC;
    quint64 delta = 0;//(stepTime - lastStepTime);
    quint64 elapsed = 0;//stepTime - firstStepTime;

    if (index.row() > 0) {
        quint64 prevstepTime = steps[index.row() - 1].second / USECS_PER_MSEC;
        delta = stepTime - prevstepTime;
        elapsed = stepTime - firstStepTime;
    }

    if (role == Qt::DisplayRole || role == DisplayNameRole) {
        const QMetaObject &nodeListMeta = NodeList::staticMetaObject;
        QMetaEnum stepEnum = nodeListMeta.enumerator(nodeListMeta.indexOfEnumerator("ConnectionStep"));
        int stepIndex = (int) steps[index.row()].first;
        return stepEnum.valueToKey(stepIndex);
    } else if (role == DeltaRole) {
        return delta;
    } else if (role == TimestampRole) {
        return stepTime;
    } else if (role == TimeElapsedRole) {
        return elapsed;
    }
    return QVariant();
}

int DomainConnectionModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    const LimitedNodeList::ConnectionTimesMap& times =
            DependencyManager::get<NodeList>()->getLastConnectionTimes();
    return (int) times.size();
}

QHash<int, QByteArray> DomainConnectionModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles.insert(DisplayNameRole, "name");
    roles.insert(TimestampRole, "timestamp");
    roles.insert(DeltaRole, "delta");
    roles.insert(TimeElapsedRole, "timeelapsed");
    return roles;
}

QModelIndex DomainConnectionModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, column);
}

QModelIndex DomainConnectionModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int DomainConnectionModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

void DomainConnectionModel::refresh() {
    //inform view that we want refresh data
    beginResetModel();
    endResetModel();
}
