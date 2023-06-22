#include <string>

class ExtraText {
    private:
        float m_x = 0, m_y = 0, m_font_size = 8;
        std::string m_text;
        std::string m_style;

    public:
        ExtraText(std::string text);
        ~ExtraText();

        float x(void);
        float y(void);
        float font_size(void);
        std::string text(void);
        std::string style(void);
};
