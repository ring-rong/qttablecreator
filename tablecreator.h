#ifndef TABLECREATOR_H
#define TABLECREATOR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QToolBar>

class TableCreator : public QWidget
{
    Q_OBJECT

public:
    TableCreator();

public slots:
    void createTable();
    void saveToXml();
    void loadFromXml();
    void clearTable();
    void searchTable();
    void clearHighlightedCells();

private:
    void createToolBars();
    void createTabNavigationToolBar();
    void prevTab();
    void nextTab();
    QVBoxLayout layout;
    QTableWidget *table = nullptr;
    QToolBar *fileToolBar;
    QTabWidget *tabWidget;
    void onHeaderContextMenu(const QPoint &pos);
      QList<QTableWidgetItem*> highlightedItems;
};

#endif // TABLECREATOR_H
