#pragma once

#include <QUndoCommand>

#include "models/TurnManager.h"

class AddCombatantCommand : public QUndoCommand {
public:
    AddCombatantCommand(TurnManager *manager, Combatant combatant, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    TurnManager *m_manager;
    Combatant m_combatant;
    bool m_done = false;
};

class EditCombatantCommand : public QUndoCommand {
public:
    EditCombatantCommand(TurnManager *manager, int combatantId, Combatant before, Combatant after, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    TurnManager *m_manager;
    int m_combatantId;
    Combatant m_before;
    Combatant m_after;
};

