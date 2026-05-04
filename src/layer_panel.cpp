#include <layer_panel.hpp>

#include <QBoxLayout>
#include <QDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>

LayerPanel::LayerPanel(QWidget* parent) : QWidget(parent), m_ignore_item_changes(false)
{
    m_layer_list = new QListWidget(this);
    m_layer_list->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    m_layer_list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_layer_list->setAlternatingRowColors(true);
    m_layer_list->setDragDropMode(QAbstractItemView::NoDragDrop);

    m_add_layer_button = new QPushButton("Add Layer", this);
    m_remove_layer_button = new QPushButton("Remove Layer", this);

    auto* button_layout = new QHBoxLayout();
    button_layout->addWidget(m_add_layer_button);
    button_layout->addWidget(m_remove_layer_button);

    auto* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(button_layout);
    main_layout->addWidget(m_layer_list);

    connect(m_add_layer_button, &QPushButton::pressed, this, &LayerPanel::add_layer);
    connect(m_remove_layer_button, &QPushButton::pressed, this, &LayerPanel::remove_selected_layer);
    connect(m_layer_list, &QListWidget::itemChanged, this, &LayerPanel::on_layer_item_changed);
    connect(m_layer_list, &QListWidget::itemSelectionChanged, this, &LayerPanel::on_selection_changed);

    create_default_layer();
}

LayerPanel::~LayerPanel()
{
}

void LayerPanel::create_default_layer()
{
    m_ignore_item_changes = true;
    auto* item = new QListWidgetItem("Layer 1", m_layer_list);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Checked);
    m_layer_list->setCurrentItem(item);
    m_ignore_item_changes = false;
    emit layer_added(item->text());
    emit layer_selected(0);
}

void LayerPanel::add_layer()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, "Add Layer", "Layer name:", QLineEdit::Normal, QString(), &ok);
    if (!ok || name.isEmpty()) return;

    for (int i = 0; i < m_layer_list->count(); ++i)
    {
        if (m_layer_list->item(i)->text() == name)
        {
            name = QString("%1 (%2)").arg(name).arg(i + 1);
            break;
        }
    }

    m_ignore_item_changes = true;
    auto* item = new QListWidgetItem(name, m_layer_list);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Checked);
    m_layer_list->setCurrentItem(item);
    int row = m_layer_list->row(item);
    m_ignore_item_changes = false;

    emit layer_added(item->text());
    emit layer_selected(row);
}

void LayerPanel::remove_selected_layer()
{
    if (m_layer_list->count() <= 1) return;

    const auto selected = m_layer_list->selectedItems();
    if (selected.isEmpty()) return;

    auto* item = selected[0];
    int row = m_layer_list->row(item);
    delete m_layer_list->takeItem(row);

    ensure_single_item_selection();
    emit layer_removed(row);
}

void LayerPanel::on_layer_item_changed(QListWidgetItem* item)
{
    if (m_ignore_item_changes || !item) return;
    int row = m_layer_list->row(item);
    if (row < 0) return;
    emit layer_visibility_changed(row, item->checkState() == Qt::Checked);
}

void LayerPanel::on_selection_changed()
{
    const auto selected = m_layer_list->selectedItems();
    if (selected.isEmpty()) return;
    int row = m_layer_list->row(selected[0]);
    if (row < 0) return;
    emit layer_selected(row);
}

void LayerPanel::ensure_single_item_selection()
{
    if (m_layer_list->selectedItems().isEmpty() && m_layer_list->count() > 0)
        m_layer_list->setCurrentRow(0);
}
