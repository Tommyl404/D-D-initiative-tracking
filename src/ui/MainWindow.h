#pragma once

#include <QMainWindow>
#include <QUndoStack>

#include "models/InitiativeModel.h"
#include "models/TurnManager.h"
#include "stores/EncounterStore.h"
#include "stores/RosterStore.h"
#include "undo/UndoCommands.h"
#include "utils/DiceRoller.h"
#include "utils/Settings.h"

class QDockWidget;
class QLineEdit;
class QSpinBox;
class QTextEdit;
class QTableView;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void handleNextTurn();
    void handlePreviousTurn();
    void handleRollNormal();
    void handleRollAdvantage();
    void handleRollDisadvantage();
    void updateStatusBar();

private:
    void setupUi();
    void setupMenus();
    void connectSignals();
    void populateSampleData();

    TurnManager m_turnManager;
    InitiativeModel m_model;
    QUndoStack m_undoStack;
    DiceRoller m_diceRoller;
    Settings m_settings;
    EncounterStore m_encounterStore;
    RosterStore m_rosterStore;

    QTableView *m_tableView = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QSpinBox *m_initiativeSpin = nullptr;
    QTextEdit *m_notesEdit = nullptr;
};

