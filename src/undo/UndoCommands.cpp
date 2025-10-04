#include "UndoCommands.h"

#include <algorithm>

AddCombatantCommand::AddCombatantCommand(TurnManager *manager, Combatant combatant, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_manager(manager)
    , m_combatant(std::move(combatant)) {
    setText(QObject::tr("Add %1").arg(m_combatant.name));
}

void AddCombatantCommand::undo() {
    if (!m_manager) {
        return;
    }
    m_manager->removeCombatant(m_combatant.id);
    m_done = false;
}

void AddCombatantCommand::redo() {
    if (!m_manager) {
        return;
    }
    if (!m_done) {
        m_manager->addCombatant(m_combatant);
        m_done = true;
    }
}

EditCombatantCommand::EditCombatantCommand(TurnManager *manager, int combatantId, Combatant before, Combatant after, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_manager(manager)
    , m_combatantId(combatantId)
    , m_before(std::move(before))
    , m_after(std::move(after)) {
    setText(QObject::tr("Edit %1").arg(m_after.name));
}

static Combatant *findCombatant(TurnManager *manager, int id) {
    auto &list = manager->combatants();
    const auto it = std::find_if(list.begin(), list.end(), [id](const Combatant &c) { return c.id == id; });
    if (it == list.end()) {
        return nullptr;
    }
    return &(*it);
}

void EditCombatantCommand::undo() {
    if (auto *combatant = findCombatant(m_manager, m_combatantId)) {
        *combatant = m_before;
        m_manager->sortCombatants();
    }
}

void EditCombatantCommand::redo() {
    if (auto *combatant = findCombatant(m_manager, m_combatantId)) {
        *combatant = m_after;
        m_manager->sortCombatants();
    }
}

