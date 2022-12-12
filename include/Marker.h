#ifndef COMPILER_MARKER_H
#define COMPILER_MARKER_H

#include <iomanip>

class Marker final {
public:
    enum class Color : uint8_t {
        WHITE = 0b00,
        GREY = 0b01,
        BLACK = 0b10
    };

    Marker() = default;

    // Set color
    Marker &operator|=(const Color &c) {
        color_ |= static_cast<uint8_t>(c);
        return *this;
    }

    // Unset color
    Marker &operator&=(const Color &c) {
        // reversed color bits
        switch (c) {
            case Color::GREY:
                color_ &= 0b10;
                break;
            case Color::BLACK:
                color_ &= 0b01;
                break;
            default:
                color_ &= 0b11;
                break;
        }
        return *this;
    }

    [[nodiscard]] bool IsWhite() const {
        return (color_ == static_cast<uint8_t>(Color::WHITE));
    }

    [[nodiscard]] bool HasBlack() const {
        return ((color_ & 0b10) == static_cast<uint8_t>(Color::BLACK));
    }

    [[nodiscard]] bool HasGrey() const {
        return ((color_ & 0b01) == static_cast<uint8_t>(Color::GREY));
    }

private:
    uint8_t color_ = static_cast<uint8_t>(Color::WHITE);
};

#endif //COMPILER_MARKER_H
