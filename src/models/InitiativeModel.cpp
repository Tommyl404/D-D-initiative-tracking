#include "InitiativeModel.h"

#include <QBrush>

InitiativeModel::InitiativeModel(TurnManager *manager, QObject *parent)
    : QAbstractTableModel(parent)
    , m_manager(manager) {}

int InitiativeModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid() || !m_manager) {
        return 0;
    }
    return m_manager->combatants().size();
}

int InitiativeModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant InitiativeModel::data(const QModelIndex &index, int role) const {
    if (!m_manager || !index.isValid()) {
        return QVariant();
    }
    const auto &combatants = m_manager->combatants();
    if (index.row() >= combatants.size()) {
        return QVariant();
    }
    const auto &combatant = combatants.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColumnIndex:
            return index.row() + 1;
        case ColumnName:
            return combatant.name;
        case ColumnInitiative:
            return combatant.initiative;
        case ColumnDex:
            return combatant.dexMod;
        case ColumnType:
            return combatant.isPC ? tr("PC") : tr("NPC");
        case ColumnStatus:
            return combatant.conscious ? tr("OK") : tr("Down");
        case ColumnHP:
            return combatant.hp;
        case ColumnAC:
            return combatant.ac;
        case ColumnConditions: {
            QStringList names;
            for (const auto &condition : combatant.conditions) {
                names << QStringLiteral("%1 (%2)").arg(condition.name).arg(condition.remainingRounds);
            }
            return names.join(", ");
        }
        case ColumnNotes:
            return combatant.notes;
        default:
            break;
        }
    }

    if (role == Qt::ToolTipRole && index.column() == ColumnNotes) {
        return combatant.notes;
    }

    if (role == Qt::ForegroundRole && !combatant.conscious) {
        return QBrush(Qt::gray);
    }
    return QVariant();
}

QVariant InitiativeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnIndex:
            return tr("#");
        case ColumnName:
            return tr("Name");
        case ColumnInitiative:
            return tr("Init");
        case ColumnDex:
            return tr("Dex");
        case ColumnType:
            return tr("Type");
        case ColumnStatus:
            return tr("Status");
        case ColumnHP:
            return tr("HP");
        case ColumnAC:
            return tr("AC");
        case ColumnConditions:
            return tr("Conditions");
        case ColumnNotes:
            return tr("Notes");
        default:
            break;
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags InitiativeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    switch (index.column()) {
    case ColumnName:
    case ColumnInitiative:
    case ColumnDex:
    case ColumnHP:
    case ColumnAC:
    case ColumnNotes:
        itemFlags |= Qt::ItemIsEditable;
        break;
    default:
        break;
    }
    return itemFlags;
}

bool InitiativeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!m_manager || role != Qt::EditRole || !index.isValid()) {
        return false;
    }
    auto &combatant = m_manager->combatants()[index.row()];
    switch (index.column()) {
    case ColumnName:
        combatant.name = value.toString();
        break;
    case ColumnInitiative:
        combatant.initiative = value.toInt();
        m_manager->sortCombatants();
        break;
    case ColumnDex:
        combatant.dexMod = value.toInt();
        m_manager->sortCombatants();
        break;
    case ColumnHP:
        combatant.hp = value.toInt();
        break;
    case ColumnAC:
        combatant.ac = value.toInt();
        break;
    case ColumnNotes:
        combatant.notes = value.toString();
        break;
    default:
        return false;
    }
    emit dataChanged(index, index);
    return true;
}

void InitiativeModel::refresh() {
    beginResetModel();
    endResetModel();
}

