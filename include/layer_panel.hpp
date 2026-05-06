#ifndef LAYER_PANEL_HPP
#define LAYER_PANEL_HPP

#include <QWidget>
#include <QList>
#include <QStringList>
#include <QModelIndex>

class QListWidget;
class QListWidgetItem;
class QPushButton;

class LayerPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LayerPanel(QWidget* parent = nullptr);
    ~LayerPanel() override;

public slots:
    void set_layers(const QStringList& names, const QList<bool>& visibility, int selected_index);

signals:
    void layer_added(const QString& name);
    void layer_removed(int index);
    void layer_selected(int index);
    void layer_visibility_changed(int index, bool visible);
    void layer_moved(int from, int to);

private slots:
    void add_layer();
    void remove_selected_layer();
    void on_layer_item_changed(QListWidgetItem* item);
    void on_selection_changed();
    void on_rows_moved(const QModelIndex& parent, int start, int end, const QModelIndex& destination, int row);

private:
    void create_default_layer();
    void ensure_single_item_selection();

    QListWidget* m_layer_list;
    QPushButton* m_add_layer_button;
    QPushButton* m_remove_layer_button;
    bool m_ignore_item_changes;
};

#endif
