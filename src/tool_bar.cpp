#include <tool_bar.hpp>

#include <QString>
#include <QMenu>
#include <QToolButton>
#include <QActionGroup>
#include <QSettings>

ToolBar::ToolBar(QWidget* parent) :
    QToolBar(parent)
{
    QToolBar::setObjectName("tool_bar");
    QToolBar::setFloatable(false);
    QToolBar::setMovable(false);

    QSettings settings("Game Academy", "EarthPower");
    
    settings.beginGroup("tool_bar");
    ThemeManager::Theme current_theme = static_cast<ThemeManager::Theme>(settings.value("theme", 0).value<int>());
    settings.endGroup();

    theme_manager.change_theme(current_theme);

    auto* theme_menu = create_menu("Theme");
    auto* theme_action_group = new QActionGroup(this);
    theme_action_group->setExclusive(true);

    QAction* computer_theme_action = new QAction("Computer Theme", this);
    computer_theme_action->setCheckable(true);
    theme_action_group->addAction(computer_theme_action);
    theme_menu->addAction(computer_theme_action);

    QAction* light_theme_action = new QAction("Light Theme", this);
    light_theme_action->setCheckable(true);
    theme_action_group->addAction(light_theme_action);
    theme_menu->addAction(light_theme_action);

    QAction* dark_theme_action = new QAction("Dark Theme", this);
    dark_theme_action->setCheckable(true);
    theme_action_group->addAction(dark_theme_action);
    theme_menu->addAction(dark_theme_action);
    
    connect(theme_action_group, &QActionGroup::triggered, this, &ToolBar::theme_action_triggered);
    
    if (current_theme == ThemeManager::Theme::Auto)
        computer_theme_action->setChecked(true);
    else if (current_theme == ThemeManager::Theme::Light)
        light_theme_action->setChecked(true);
    else if (current_theme == ThemeManager::Theme::Dark)
        dark_theme_action->setChecked(true);
}

ToolBar::~ToolBar()
{
    QSettings settings("Game Academy", "EarthPower");
    
    settings.beginGroup("tool_bar");
    settings.setValue("theme", static_cast<int>(theme_manager.theme()));
    settings.endGroup();
}

QMenu* ToolBar::create_menu(QString name)
{
    auto* menu = new QMenu(this);
    auto* button = new QToolButton(this);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setText(name);
    button->setMenu(menu);

    addWidget(button);
    return menu;
}

void ToolBar::theme_action_triggered(QAction* action)
{
    QString theme_name = action->text();
    if (theme_name == "Light Theme")
        theme_manager.change_theme(ThemeManager::Theme::Light);
    else if (theme_name == "Dark Theme")
        theme_manager.change_theme(ThemeManager::Theme::Dark);
    else
        theme_manager.change_theme(ThemeManager::Theme::Auto);
}