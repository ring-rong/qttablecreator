#include "TableCreator.h"
#include <QPushButton>
#include <QInputDialog>
#include <QStringList>
#include <QMessageBox>
#include <QIcon>
#include <QAction>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QHeaderView>
#include <QFileDialog>
#include <QDomDocument>
#include <QTextStream>

TableCreator::TableCreator()
{
    // Задаем фиксированный размер окна
    this->setFixedSize(1200, 800);
    createToolBars();
    tabWidget = new QTabWidget(this);
    layout.addWidget(tabWidget, 1);
     createTabNavigationToolBar();
    setLayout(&layout);
}


void TableCreator::createToolBars()
{
    fileToolBar = new QToolBar(tr("Работа с файлом"));
    fileToolBar->setIconSize(QSize(40, 40));

    QAction *createTableAction = new QAction(QIcon(":/icons/table.png"), tr("Создать таблицу"), this);
    connect(createTableAction, &QAction::triggered, this, &TableCreator::createTable);
    fileToolBar->addAction(createTableAction);

    QAction *loadFromXmlAction = new QAction(QIcon(":/icons/icon_open_file.png"), tr("Загрузить из XML"), this);
    connect(loadFromXmlAction, &QAction::triggered, this, &TableCreator::loadFromXml);
    fileToolBar->addAction(loadFromXmlAction);

    QAction *saveToXmlAction = new QAction(QIcon(":/icons/icon_save_file.png"), tr("Сохранить как XML"), this);
    connect(saveToXmlAction, &QAction::triggered, this, &TableCreator::saveToXml);
    fileToolBar->addAction(saveToXmlAction);

    QAction *clearTableAction = new QAction(QIcon(":/icons/icon_close.png"), tr("Очистить таблицу"), this);
    connect(clearTableAction, &QAction::triggered, this, &TableCreator::clearTable);
    fileToolBar->addAction(clearTableAction);

    QAction *searchAction = new QAction(QIcon(":/icons/icon_find.png"), tr("Поиск"), this);
    connect(searchAction, &QAction::triggered, this, &TableCreator::searchTable);
    fileToolBar->addAction(searchAction);



    layout.addWidget(fileToolBar, 0, Qt::AlignTop | Qt::AlignLeft);
    setLayout(&layout);
}

void TableCreator::createTabNavigationToolBar()
{
    QToolBar *tabNavigationToolBar = new QToolBar(tr("Навигация по таблицам"));
    tabNavigationToolBar->setIconSize(QSize(40, 40));

    QAction *prevTabAction = new QAction(QIcon(":/icons/arrow_left.png"), tr("Предыдущая таблица"), this);
    connect(prevTabAction, &QAction::triggered, this, &TableCreator::prevTab);
    tabNavigationToolBar->addAction(prevTabAction);

    QAction *nextTabAction = new QAction(QIcon(":/icons/arrow_right.png"), tr("Следующая таблица"), this);
    connect(nextTabAction, &QAction::triggered, this, &TableCreator::nextTab);
    tabNavigationToolBar->addAction(nextTabAction);

    layout.addWidget(tabNavigationToolBar, 0, Qt::AlignBottom | Qt::AlignRight);
}

void TableCreator::prevTab()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex > 0) {
        tabWidget->setCurrentIndex(currentIndex - 1);
    }
}

void TableCreator::nextTab()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < tabWidget->count() - 1) {
        tabWidget->setCurrentIndex(currentIndex + 1);
    }
}

void TableCreator::createTable()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);

    form.addRow(new QLabel("Введите размеры таблицы:"));

    QSpinBox *rowCountBox = new QSpinBox(&dialog);
    rowCountBox->setRange(1, 100);
    form.addRow("Количество строк:", rowCountBox);

    QSpinBox *columnCountBox = new QSpinBox(&dialog);
    columnCountBox->setRange(1, 100);
    form.addRow("Количество столбцов:", columnCountBox);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        int rowCount = rowCountBox->value();
        int columnCount = columnCountBox->value();

        if(rowCount > 10 || columnCount > 10) {
            QMessageBox::critical(this, "Ошибка", "Размер таблицы не должен превышать 10x10");
            return;
        }

        QTableWidget *newTable = new QTableWidget(rowCount, columnCount);
        newTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        newTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        // Здесь настраиваем таблицу и ее сигналы
        newTable->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
        newTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(newTable->verticalHeader(), &QHeaderView::customContextMenuRequested, this, &TableCreator::onHeaderContextMenu);
        connect(newTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &TableCreator::onHeaderContextMenu);
        connect(newTable, &QTableWidget::cellClicked, this, &TableCreator::clearHighlightedCells);
        tabWidget->addTab(newTable, tr("Таблица %1").arg(tabWidget->count() + 1));
    }
}

void TableCreator::clearHighlightedCells()
{
    foreach (QTableWidgetItem* item, highlightedItems) {
        item->setBackground(Qt::white);
    }
    highlightedItems.clear();
}


void TableCreator::onHeaderContextMenu(const QPoint &pos)
{
    QTableWidget* currentTable = qobject_cast<QTableWidget*>(tabWidget->currentWidget());
    if (!currentTable) return;

    QHeaderView *header = qobject_cast<QHeaderView*>(sender());
    if (!header) return;

    int index = header->logicalIndexAt(pos);
    if (index < 0) return;

    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit Header"),
                                         tr("Header:"), QLineEdit::Normal,
                                         header->model()->headerData(index, header->orientation()).toString(), &ok);
    if (ok && !text.isEmpty()) {
        if (header == currentTable->horizontalHeader()) {
            QTableWidgetItem *newItem = new QTableWidgetItem(text);
            currentTable->setHorizontalHeaderItem(index, newItem);
        } else if (header == currentTable->verticalHeader()) {
            QTableWidgetItem *newItem = new QTableWidgetItem(text);
            currentTable->setVerticalHeaderItem(index, newItem);
        }
    }
}

void TableCreator::clearTable()
{
    QTableWidget* currentTable = qobject_cast<QTableWidget*>(tabWidget->currentWidget());
    if (currentTable) {
        currentTable->clearContents();
        currentTable->setRowCount(0);
        currentTable->setColumnCount(0);
    }
}

void TableCreator::searchTable()
{
    QTableWidget* currentTable = qobject_cast<QTableWidget*>(tabWidget->currentWidget());
    if (!currentTable)
        return;

    bool ok;
    QString keyword = QInputDialog::getText(this, tr("Поиск"), tr("Введите ключевую фразу:"), QLineEdit::Normal, "", &ok);
    if (!ok || keyword.isEmpty())
        return;

    currentTable->clearSelection();
    highlightedItems = currentTable->findItems(keyword, Qt::MatchContains);

    if (!highlightedItems.isEmpty()) {
        foreach (QTableWidgetItem* item, highlightedItems) {
            item->setBackground(Qt::yellow);
        }
    } else {
        QMessageBox::information(this, tr("Поиск"), tr("Ключевая фраза не найдена в таблице."));
    }
}

void TableCreator::loadFromXml() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Открыть XML"), "", tr("XML Files (*.xml)"));
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }

        QDomDocument doc;
        if (!doc.setContent(&file)) {
            file.close();
            return;
        }
        file.close();

        QDomElement root = doc.documentElement();

        QDomElement columnHeadersElement = root.firstChildElement("column_headers");
        QDomNodeList columnHeaders = columnHeadersElement.elementsByTagName("column_header");

        QDomNodeList rows = root.elementsByTagName("row");

        if (rows.count() > 10 || columnHeaders.count() > 10) {
            QMessageBox::critical(this, tr("Ошибка"), tr("Таблица не может содержать более 10 строк или столбцов."));
            return;
        }

        QTableWidget* currentTable = new QTableWidget(rows.count(), columnHeaders.count());
         connect(currentTable, &QTableWidget::cellClicked, this, &TableCreator::clearHighlightedCells);
        currentTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        currentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        // Здесь настраиваем таблицу и ее сигналы
        currentTable->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
        currentTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(currentTable->verticalHeader(), &QHeaderView::customContextMenuRequested, this, &TableCreator::onHeaderContextMenu);
        connect(currentTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &TableCreator::onHeaderContextMenu);
        tabWidget->addTab(currentTable, tr("Таблица %1").arg(tabWidget->count() + 1));

        currentTable->blockSignals(true);

        for (int j = 0; j < columnHeaders.count(); ++j) {
            QDomElement columnHeader = columnHeaders.at(j).toElement();
            QTableWidgetItem *newItem = new QTableWidgetItem(columnHeader.text());
            currentTable->setHorizontalHeaderItem(j, newItem);
        }

        for (int i = 0; i < rows.count(); ++i) {
            QDomElement row = rows.at(i).toElement();

            QDomElement rowHeaderElement = row.firstChildElement("row_header");
            QTableWidgetItem *newRowHeaderItem = new QTableWidgetItem(rowHeaderElement.text());
            currentTable->setVerticalHeaderItem(i, newRowHeaderItem);

            QDomNodeList columns = row.elementsByTagName("column");

            for (int j = 0; j < columns.count(); ++j) {
                QDomElement column = columns.at(j).toElement();
                QTableWidgetItem *newItem = new QTableWidgetItem(column.text());
                currentTable->setItem(i, j, newItem);

            }
        }

        currentTable->blockSignals(false);
    }
}


void TableCreator::saveToXml()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Сохранить как XML"), "", tr("XML Files (*.xml)"));
    if (!filename.isEmpty()) {
        QDomDocument doc;

        // Создаем корневой элемент
        QDomElement root = doc.createElement("table");
        doc.appendChild(root);

        QTableWidget* currentTable = qobject_cast<QTableWidget*>(tabWidget->currentWidget());
        if (!currentTable) return;

        // Создаем элементы заголовков столбцов
        QDomElement columnHeaders = doc.createElement("column_headers");
        root.appendChild(columnHeaders);

        for (int j = 0; j < currentTable->columnCount(); ++j) {
            QDomElement columnHeader = doc.createElement("column_header");
            QTableWidgetItem *headerItem = currentTable->horizontalHeaderItem(j);
            QDomText headerText = doc.createTextNode(headerItem ? headerItem->text() : QString("Column %1").arg(j+1));
            columnHeader.appendChild(headerText);
            columnHeaders.appendChild(columnHeader);
        }

        // Создаем элементы заголовков строк и ячеек
        for (int i = 0; i < currentTable->rowCount(); ++i) {
            QDomElement row = doc.createElement("row");
            root.appendChild(row);

            // Записываем заголовок строки
            QDomElement rowHeader = doc.createElement("row_header");
            QTableWidgetItem *rowHeaderItem = currentTable->verticalHeaderItem(i);
            QDomText rowHeaderText = doc.createTextNode(rowHeaderItem ? rowHeaderItem->text() : QString("Row %1").arg(i+1));
            rowHeader.appendChild(rowHeaderText);
            row.appendChild(rowHeader);

            for (int j = 0; j < currentTable->columnCount(); ++j) {
                QDomElement column = doc.createElement("column");
                QTableWidgetItem *item = currentTable->item(i, j);
                QDomText cellText = doc.createTextNode(item ? item->text() : "");
                column.appendChild(cellText);
                row.appendChild(column);
            }
        }

        // Сохраняем в файл
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }
        QTextStream stream(&file);
        stream << doc.toString();
        file.close();
    }
}
