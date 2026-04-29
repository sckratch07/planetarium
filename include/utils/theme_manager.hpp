#ifndef THEME_MANAGER_HPP
#define THEME_MANAGER_HPP

class ThemeManager
{
public:
    enum Theme
    {
        Auto = 0,
        Light = 1,
        Dark = 2
    };

    ThemeManager(Theme theme = Theme::Auto);

    void apply_theme(const Theme& theme);
    void change_theme(const Theme& theme);
    const Theme& theme() const;

private:
    Theme current;
};

#endif