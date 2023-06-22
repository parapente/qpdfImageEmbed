#include "extraText.h"
#include "logger.h"
#include <regex>

ExtraText::ExtraText(std::string text) {
    const std::regex re(":");

    std::sregex_token_iterator it{text.begin(), text.end(), re, -1};
    std::vector<std::string> tokenized{it, {}};

    // Additional check to remove empty strings
    tokenized.erase(
        std::remove_if(tokenized.begin(), tokenized.end(),
                       [](std::string const &s) { return s.size() == 0; }),
        tokenized.end());

    const std::regex pos_r("^(\\d+\\.*\\d*),(\\d+\\.*\\d*)$");
    const std::regex size_r("^(\\d+\\.*\\d*)$");
    const std::regex style_r("^([i,b]{1,2})$");

    for (auto token : tokenized) {
        std::smatch match;

        // If position match is found
        if (std::regex_match(token, match, pos_r)) {
            m_x = std::stof(match[1].str());
            m_y = std::stof(match[2].str());

            logger << "ExtraText.(x,y) = (" << m_x << "," << m_y << ")\n";
        }

        if (std::regex_match(token, match, size_r)) {
            m_font_size = std::stof(match[1]);

            logger << "ExtraText.font_size = " << m_font_size << "\n";
        }

        if (std::regex_match(token, match, style_r)) {
            if (match[1].str() == "i") {
                m_style = "Italic";
            }

            if (match[1].str() == "b") {
                m_style = "Bold";
            }

            if (match[1].str() == "bi" || match[1].str() == "ib") {
                m_style = "BoldItalic";
            }

            logger << "ExtraText.style = " << m_style << "\n";
        }
    }

    m_text = tokenized.back();
    logger << "ExtraText.text = " << m_text << "\n";
}

ExtraText::~ExtraText() {}

float ExtraText::x(void) { return m_x; }

float ExtraText::y(void) { return m_y; }

float ExtraText::font_size(void) { return m_font_size; }

std::string ExtraText::style(void) { return m_style; }

std::string ExtraText::text(void) { return m_text; }