#include <utils/theme_manager.hpp>

#include <QFile>
#include <QApplication>
#include <QGuiApplication>
#include <QStyleHints>

ThemeManager::ThemeManager(Theme theme) :
    current(theme)
{
    apply_theme(current);
}

void ThemeManager::apply_theme(const Theme& theme)
{
    QString path;
    if (theme == Theme::Auto)
        path = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) ? ":/theme/dark" : ":/theme/light";
    else
        path = (theme == Theme::Dark) ? ":/theme/dark" : ":/theme/light";

    QFile file(path);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qWarning() << "Failed to load theme:" << path;
        qApp->setStyleSheet({});
        return;
    }

    qApp->setStyleSheet(file.readAll());
}

void ThemeManager::change_theme(const Theme& theme)
{
    if (current == theme) return;
    current = theme;
    apply_theme(theme);
}

const ThemeManager::Theme& ThemeManager::theme() const 
{
    return current;
}