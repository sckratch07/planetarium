#ifndef TOOL_BAR_HPP
#define TOOL_BAR_HPP

#include <QToolBar>
#include <utils/theme_manager.hpp>

class ToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget* parent = nullptr);
    ~ToolBar() override;

protected:
    QMenu* create_menu(QString name);

protected slots:
    void theme_action_triggered(QAction* action);

private:
    ThemeManager theme_manager;
};

#endif