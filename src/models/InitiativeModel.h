#pragma once

#include <QAbstractTableModel>

#include "TurnManager.h"

class InitiativeModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Columns {
        ColumnIndex,
        ColumnName,
        ColumnInitiative,
        ColumnDex,
        ColumnType,
        ColumnStatus,
        ColumnHP,
        ColumnAC,
        ColumnConditions,
        ColumnNotes,
        ColumnCount
    };

    explicit InitiativeModel(TurnManager *manager, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void refresh();

private:
    TurnManager *m_manager;
};

