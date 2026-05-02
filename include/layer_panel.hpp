#ifndef LAYER_PANEL_HPP
#define LAYER_PANEL_HPP

#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QPushButton;

class LayerPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LayerPanel(QWidget* parent = nullptr);
    ~LayerPanel() override;

signals:
    void layer_added(const QString& name);
    void layer_removed(const QString& name);
    void layer_selected(const QString& name);
    void layer_visibility_changed(const QString& name, bool visible);

private slots:
    void add_layer();
    void remove_selected_layer();
    void on_layer_item_changed(QListWidgetItem* item);
    void on_selection_changed();

private:
    void create_default_layer();
    void ensure_single_item_selection();

    QListWidget* m_layer_list;
    QPushButton* m_add_layer_button;
    QPushButton* m_remove_layer_button;
    bool m_ignore_item_changes;
};

#endif
