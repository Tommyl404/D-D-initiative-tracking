#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMenuBar>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableView>
#include <QTextEdit>
#include <QToolBar>

#include "undo/UndoCommands.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_model(&m_turnManager, this) {
    setupUi();
    setupMenus();
    connectSignals();
    populateSampleData();
    updateStatusBar();
}

void MainWindow::setupUi() {
    m_tableView = new QTableView(this);
    m_tableView->setModel(&m_model);
    setCentralWidget(m_tableView);

    auto *editorDock = new QDockWidget(tr("Editor"), this);
    auto *editorWidget = new QWidget(editorDock);
    auto *form = new QFormLayout(editorWidget);
    m_nameEdit = new QLineEdit(editorWidget);
    m_initiativeSpin = new QSpinBox(editorWidget);
    m_initiativeSpin->setRange(-10, 40);
    m_notesEdit = new QTextEdit(editorWidget);
    form->addRow(tr("Name"), m_nameEdit);
    form->addRow(tr("Initiative"), m_initiativeSpin);
    form->addRow(tr("Notes"), m_notesEdit);
    editorWidget->setLayout(form);
    editorDock->setWidget(editorWidget);
    addDockWidget(Qt::RightDockWidgetArea, editorDock);

    auto *toolBar = addToolBar(tr("Turns"));
    toolBar->addAction(tr("Prev"), this, &MainWindow::handlePreviousTurn);
    toolBar->addAction(tr("Next"), this, &MainWindow::handleNextTurn);

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupMenus() {
    auto *turnMenu = menuBar()->addMenu(tr("Encounter"));
    turnMenu->addAction(tr("Previous Turn"), this, &MainWindow::handlePreviousTurn, QKeySequence(Qt::Key_PageUp));
    turnMenu->addAction(tr("Next Turn"), this, &MainWindow::handleNextTurn, QKeySequence(Qt::Key_PageDown));

    auto *rollMenu = menuBar()->addMenu(tr("Roll"));
    rollMenu->addAction(tr("Normal"), this, &MainWindow::handleRollNormal, QKeySequence(tr("Ctrl+R")));
    rollMenu->addAction(tr("Advantage"), this, &MainWindow::handleRollAdvantage);
    rollMenu->addAction(tr("Disadvantage"), this, &MainWindow::handleRollDisadvantage);
}

void MainWindow::connectSignals() {
    connect(m_tableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &current) {
        if (!current.isValid()) {
            return;
        }
        const auto &combatant = m_turnManager.combatants().at(current.row());
        m_nameEdit->setText(combatant.name);
        m_initiativeSpin->setValue(combatant.initiative);
        m_notesEdit->setPlainText(combatant.notes);
    });

    connect(m_nameEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        const auto index = m_tableView->currentIndex();
        if (index.isValid()) {
            m_model.setData(m_model.index(index.row(), InitiativeModel::ColumnName), text, Qt::EditRole);
        }
    });

    connect(m_initiativeSpin, &QSpinBox::valueChanged, this, [this](int value) {
        const auto index = m_tableView->currentIndex();
        if (index.isValid()) {
            m_model.setData(m_model.index(index.row(), InitiativeModel::ColumnInitiative), value, Qt::EditRole);
        }
    });

    connect(m_notesEdit, &QTextEdit::textChanged, this, [this]() {
        const auto index = m_tableView->currentIndex();
        if (index.isValid()) {
            m_model.setData(m_model.index(index.row(), InitiativeModel::ColumnNotes), m_notesEdit->toPlainText(), Qt::EditRole);
        }
    });

    connect(m_tableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &) {
        updateStatusBar();
    });
}

void MainWindow::populateSampleData() {
    TurnManager::CombatantList list;
    for (int i = 0; i < 3; ++i) {
        Combatant combatant;
        combatant.id = i + 1;
        combatant.name = QStringLiteral("Hero %1").arg(i + 1);
        combatant.initiative = 15 - i;
        combatant.dexMod = 3 - i;
        combatant.isPC = true;
        combatant.hp = 20 + i * 5;
        list.push_back(combatant);
    }
    for (int i = 0; i < 3; ++i) {
        Combatant combatant;
        combatant.id = i + 100;
        combatant.name = QStringLiteral("Bandit %1").arg(i + 1);
        combatant.initiative = 12 - i;
        combatant.dexMod = 2;
        combatant.isPC = false;
        combatant.hp = 12;
        list.push_back(combatant);
    }
    m_turnManager.setCombatants(list);
    m_model.refresh();
    if (m_model.rowCount() > 0) {
        m_tableView->selectRow(0);
    }
}

void MainWindow::handleNextTurn() {
    m_turnManager.advanceTurn();
    updateStatusBar();
}

void MainWindow::handlePreviousTurn() {
    m_turnManager.rewindTurn();
    updateStatusBar();
}

void MainWindow::handleRollNormal() {
    const auto index = m_tableView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    auto &combatant = m_turnManager.combatants()[index.row()];
    combatant.initiative = m_diceRoller.rollD20(RollMode::Normal, combatant.dexMod);
    m_turnManager.sortCombatants();
    m_model.refresh();
}

void MainWindow::handleRollAdvantage() {
    const auto index = m_tableView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    auto &combatant = m_turnManager.combatants()[index.row()];
    combatant.initiative = m_diceRoller.rollD20(RollMode::Advantage, combatant.dexMod);
    m_turnManager.sortCombatants();
    m_model.refresh();
}

void MainWindow::handleRollDisadvantage() {
    const auto index = m_tableView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    auto &combatant = m_turnManager.combatants()[index.row()];
    combatant.initiative = m_diceRoller.rollD20(RollMode::Disadvantage, combatant.dexMod);
    m_turnManager.sortCombatants();
    m_model.refresh();
}

void MainWindow::updateStatusBar() {
    if (m_turnManager.combatants().isEmpty()) {
        statusBar()->showMessage(tr("Round 0 • Turn 0/0"));
        return;
    }
    const auto &current = m_turnManager.combatants().at(m_turnManager.turnIndex());
    statusBar()->showMessage(tr("Round %1 • Turn %2/%3 • Current: %4")
                                 .arg(m_turnManager.round())
                                 .arg(m_turnManager.turnIndex() + 1)
                                 .arg(m_turnManager.combatants().size())
                                 .arg(current.name));
}

