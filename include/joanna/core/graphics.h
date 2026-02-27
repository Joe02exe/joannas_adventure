#pragma once

#ifdef MIYOO_BUILD
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace jo {

class RenderWindow;
using Window = RenderWindow;

class Time {
  public:
    Time() : m_microseconds(0) {}

    explicit Time(long long microseconds) : m_microseconds(microseconds) {}

    float asSeconds() const {
        return m_microseconds / 1000000.f;
    }

    int asMilliseconds() const {
        return m_microseconds / 1000;
    }

    long long asMicroseconds() const {
        return m_microseconds;
    }

  private:
    long long m_microseconds;
};

inline Time seconds(float amount) {
    return Time(static_cast<long long>(amount * 1000000));
}

inline Time milliseconds(int amount) {
    return Time(static_cast<long long>(amount) * 1000);
}

inline Time microseconds(long long amount) {
    return Time(amount);
}

class Clock {
  public:
    Clock() {
        restart();
    }

    Time getElapsedTime() const {
        return milliseconds(SDL_GetTicks() - m_startTime);
    }

    Time restart() {
        Time elapsed = getElapsedTime();
        m_startTime = SDL_GetTicks();
        return elapsed;
    }

  private:
    Uint32 m_startTime;
};

template <typename T> class Vector2 {
  public:
    T x, y;

    Vector2() : x(0), y(0) {}

    Vector2(T x, T y) : x(x), y(y) {}

    template <typename U>
    explicit Vector2(const Vector2<U>& vector)
        : x(static_cast<T>(vector.x)), y(static_cast<T>(vector.y)) {}

    bool operator==(const Vector2<T>& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vector2<T>& other) const {
        return !(*this == other);
    }

    Vector2<T> operator+(const Vector2<T>& other) const {
        return Vector2<T>(x + other.x, y + other.y);
    }

    Vector2<T> operator-(const Vector2<T>& other) const {
        return Vector2<T>(x - other.x, y - other.y);
    }

    Vector2<T> operator*(T scalar) const {
        return Vector2<T>(x * scalar, y * scalar);
    }

    Vector2<T> operator/(T scalar) const {
        return Vector2<T>(x / scalar, y / scalar);
    }

    Vector2<T>& operator+=(const Vector2<T>& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2<T>& operator-=(const Vector2<T>& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2<T>& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2<T>& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;

template <typename T> class Rect {
  public:
    Vector2<T> position;
    Vector2<T> size;

    Rect() {}

    Rect(T x, T y, T w, T h) : position(x, y), size(w, h) {}

    Rect(const Vector2<T>& pos, const Vector2<T>& sz)
        : position(pos), size(sz) {}

    bool contains(const Vector2<T>& point) const {
        return point.x >= position.x && point.x < position.x + size.x &&
               point.y >= position.y && point.y < position.y + size.y;
    }

    bool intersects(const Rect<T>& other) const {
        return position.x < other.position.x + other.size.x &&
               position.x + size.x > other.position.x &&
               position.y < other.position.y + other.size.y &&
               position.y + size.y > other.position.y;
    }
};

using FloatRect = Rect<float>;
using IntRect = Rect<int>;

class Color {
  public:
    Uint8 r, g, b, a;

    constexpr Color() : r(0), g(0), b(0), a(255) {}

    constexpr Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
        : r(r), g(g), b(b), a(a) {}

    static const Color Black, White, Red, Green, Blue, Yellow, Magenta, Cyan,
        Transparent;
};

class Font {
  public:
    Font() : m_font(nullptr) {}

    void setSmooth(bool smooth) const {}

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    Font(Font&& other) noexcept : m_font(other.m_font) {
        other.m_font = nullptr;
    }

    Font& operator=(Font&& other) noexcept {
        if (this != &other) {
            if (m_font)
                TTF_CloseFont(m_font);
            m_font = other.m_font;
            other.m_font = nullptr;
        }
        return *this;
    }

    ~Font() {
        if (m_font)
            TTF_CloseFont(m_font);
    }

    bool loadFromFile(const std::string& filename) {
        m_font = TTF_OpenFont(filename.c_str(), 24); // Default size 24
        return m_font != nullptr;
    }

    bool openFromFile(const std::string& filename) {
        return loadFromFile(filename);
    }

    TTF_Font* getNativeFont() const {
        return m_font;
    }

  private:
    TTF_Font* m_font;
};

class Image {
  public:
    Image() : m_surface(nullptr) {}

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&& other) noexcept : m_surface(other.m_surface) {
        other.m_surface = nullptr;
    }

    Image& operator=(Image&& other) noexcept {
        if (this != &other) {
            if (m_surface)
                SDL_FreeSurface(m_surface);
            m_surface = other.m_surface;
            other.m_surface = nullptr;
        }
        return *this;
    }

    ~Image() {
        if (m_surface)
            SDL_FreeSurface(m_surface);
    }

    bool loadFromFile(const std::string& filename) {
        m_surface = IMG_Load(filename.c_str());
        return m_surface != nullptr;
    }

    Color getPixel(const Vector2u& pos) const;

    SDL_Surface* getNativeSurface() const {
        return m_surface;
    }

  private:
    SDL_Surface* m_surface;
};

class View {
  public:
    View() : m_center(0, 0), m_size(0, 0), m_viewport(0, 0, 1, 1) {}

    View(const FloatRect& rect)
        : m_center(
              rect.position.x + rect.size.x / 2,
              rect.position.y + rect.size.y / 2
          ),
          m_size(rect.size), m_viewport(0, 0, 1, 1) {}

    View(const Vector2f& center, const Vector2f& size)
        : m_center(center), m_size(size), m_viewport(0, 0, 1, 1) {}

    void zoom(float factor) {
        m_size.x *= factor;
        m_size.y *= factor;
    }

    void move(const Vector2f& m) {
        m_center.x += m.x;
        m_center.y += m.y;
    }

    void setCenter(const Vector2f& center) {
        m_center = center;
    }

    void setSize(const Vector2f& size) {
        m_size = size;
    }

    void setViewport(const FloatRect& viewport) {
        m_viewport = viewport;
    }

    const Vector2f& getCenter() const {
        return m_center;
    }

    const Vector2f& getSize() const {
        return m_size;
    }

    const FloatRect& getViewport() const {
        return m_viewport;
    }

  private:
    Vector2f m_center;
    Vector2f m_size;
    FloatRect m_viewport;
};

class Texture {
  public:
    Texture() : m_surface(nullptr), m_width(0), m_height(0) {}

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept
        : m_surface(other.m_surface), m_width(other.m_width),
          m_height(other.m_height) {
        other.m_surface = nullptr;
        other.m_width = 0;
        other.m_height = 0;
    }

    Texture& operator=(Texture&& other) noexcept {
        if (this != &other) {
            if (m_surface)
                SDL_FreeSurface(m_surface);
            m_surface = other.m_surface;
            m_width = other.m_width;
            m_height = other.m_height;
            other.m_surface = nullptr;
            other.m_width = 0;
            other.m_height = 0;
        }
        return *this;
    }

    ~Texture() {
        if (m_surface)
            SDL_FreeSurface(m_surface);
    }

    bool loadFromFile(const std::string& filename); // defined in cpp

    void setSmooth(bool smooth) const {}

    Image copyToImage() const {
        return Image();
    }

    Color getPixel(const Vector2u& pos) const;

    Vector2u getSize() const {
        return Vector2u(m_width, m_height);
    }

    SDL_Surface* getNativeSurface() const {
        return m_surface;
    }

    void setNativeSurface(SDL_Surface* sfc, int w, int h) {
        if (m_surface)
            SDL_FreeSurface(m_surface);
        m_surface = sfc;
        m_width = w;
        m_height = h;
    }

  private:
    SDL_Surface* m_surface;
    int m_width;
    int m_height;
};

class Drawable;

class RenderTarget {
  public:
    virtual ~RenderTarget() = default;
    virtual void clear(const Color& color = Color(0, 0, 0, 255)) = 0;
    virtual void draw(const Drawable& drawable) = 0;
    virtual void draw(const Drawable& drawable, void* renderStates) = 0;
    virtual void setView(const View& view) = 0;
    virtual const View& getView() const = 0;

    virtual const View& getDefaultView() const {
        return getView();
    }

    virtual SDL_Surface* getRenderTargetSurface() const = 0;
};

class RenderWindow;

class Drawable {
  public:
    virtual ~Drawable() = default;
    // We pass RenderTarget to allow recursive drawing if needed, but in SDL we
    // need the Renderer.
    virtual void draw(RenderTarget& target, void* states = nullptr) const = 0;
};

// Shaders are not supported on Miyoo. Dummy types.
namespace Glsl {
struct Vec4 {
    Vec4(float, float, float, float) {}
};
} // namespace Glsl

class Shader {
  public:
    enum class Type { Fragment };

    bool loadFromFile(const std::string&, Type) {
        return true;
    }

    template <typename T> void setUniform(const std::string&, const T&) {}
};

class Sprite: public Drawable {
  public:
    Sprite() : m_texture(nullptr), m_color(Color::White) {}

    explicit Sprite(const Texture& texture)
        : m_texture(&texture), m_color(Color::White) {
        setTexture(
            texture, true
        ); // Set to true to initialize m_textureRect natively
    }

    void setTexture(const Texture& texture, bool resetRect = false) {
        m_texture = &texture;
        if (resetRect) {
            m_textureRect =
                IntRect(0, 0, texture.getSize().x, texture.getSize().y);
        }
    }

    void setTextureRect(const IntRect& rect) {
        m_textureRect = rect;
        m_flipHoriz = false;
        if (m_textureRect.size.x < 0) {
            m_textureRect.size.x = -m_textureRect.size.x;
            m_textureRect.position.x -= m_textureRect.size.x;
            m_flipHoriz = true;
        }
    }

    void setPosition(const Vector2f& position) {
        m_position = position;
    }

    void setScale(const Vector2f& scale) {
        m_scale = scale;
    }

    void setOrigin(const Vector2f& origin) {}

    FloatRect getLocalBounds() const {
        return FloatRect(0, 0, m_textureRect.size.x, m_textureRect.size.y);
    }

    void setColor(const Color& color) {
        m_color = color;
    }

    const Texture* getTexture() const {
        return m_texture;
    }

    const IntRect& getTextureRect() const {
        return m_textureRect;
    }

    const Vector2f& getPosition() const {
        return m_position;
    }

    const Vector2f& getScale() const {
        return m_scale;
    }

    const Color& getColor() const {
        return m_color;
    }

    void draw(RenderTarget& target, void* states = nullptr) const override;

  private:
    const Texture* m_texture;
    IntRect m_textureRect;
    Vector2f m_position;
    Vector2f m_scale = { 1.f, 1.f };
    Color m_color;
    bool m_flipHoriz = false;
};

class Text: public Drawable {
  public:
    enum Style {
        Regular = 0,
        Bold = 1 << 0,
        Italic = 1 << 1,
        Underlined = 1 << 2,
        StrikeThrough = 1 << 3
    };

    Text() : m_font(nullptr), m_characterSize(30), m_color(Color::White) {}

    explicit Text(const Font& font)
        : m_font(&font), m_characterSize(30), m_color(Color::White) {}

    Text(
        const std::string& string, const Font& font,
        unsigned int characterSize = 30
    )
        : m_string(string), m_font(&font), m_characterSize(characterSize),
          m_color(Color::White) {}

    Text(
        const Font& font, const std::string& string,
        unsigned int characterSize = 30
    )
        : m_string(string), m_font(&font), m_characterSize(characterSize),
          m_color(Color::White) {}

    void setString(const std::string& string) {
        m_string = string;
    }

    void setFont(const Font& font) {
        m_font = &font;
    }

    void setCharacterSize(unsigned int size) {
        m_characterSize = size;
    }

    void setLetterSpacing(float spacingFactor) {}

    void setLineSpacing(float spacingFactor) {}

    void setStyle(Uint32 style) {}

    void setFillColor(const Color& color) {
        m_color = color;
    }

    void setOutlineColor(const Color& color) {}

    void setOutlineThickness(float thickness) {}

    void setPosition(const Vector2f& position) {
        m_position = position;
    }

    void setOrigin(const Vector2f& origin) {}

    FloatRect getLocalBounds() const {
        // dummy bounds
        return FloatRect(
            0, 0, m_string.length() * m_characterSize, m_characterSize
        );
    }

    FloatRect getGlobalBounds() const {
        return getLocalBounds();
    }

    Vector2f getPosition() const {
        return m_position;
    }

    void draw(RenderTarget& target, void* states = nullptr) const override;

  private:
    std::string m_string;
    const Font* m_font;
    unsigned int m_characterSize;
    Color m_color;
    Vector2f m_position;
};

class RenderTexture: public RenderTarget {
  public:
    RenderTexture() {}

    RenderTexture(Vector2u size) {
        create(size.x, size.y);
    }

    RenderTexture(const RenderTexture&) = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;

    RenderTexture(RenderTexture&& other) noexcept
        : m_dummyTex(std::move(other.m_dummyTex)), m_view(other.m_view) {}

    RenderTexture& operator=(RenderTexture&& other) noexcept {
        if (this != &other) {
            m_dummyTex = std::move(other.m_dummyTex);
            m_view = other.m_view;
        }
        return *this;
    }

    ~RenderTexture() = default;

    bool create(unsigned int width, unsigned int height);
    void clear(const Color& color = Color(0, 0, 0, 255)) override;

    Vector2u getSize() const {
        return Vector2u(m_view.getSize().x, m_view.getSize().y);
    }

    void draw(const Drawable& drawable) override {
        drawable.draw(*this);
    }

    void draw(const Drawable& drawable, void* renderStates) override {
        drawable.draw(*this, renderStates);
    }

    void setView(const View& view) override {
        m_view = view;
    }

    const View& getView() const override {
        return m_view;
    }

    const Texture& getTexture() const {
        return m_dummyTex;
    } // Dummy texture wrapper

    void display() {}

    SDL_Surface* getNativeSurface() const {
        return m_dummyTex.getNativeSurface();
    }

    SDL_Surface* getRenderTargetSurface() const override {
        return m_dummyTex.getNativeSurface();
    }

  private:
    Texture m_dummyTex;
    View m_view;
};

class Shape: public Drawable {
  public:
    void setFillColor(const Color& color) {
        m_fillColor = color;
    }

    void setOutlineColor(const Color& color) {}

    void setOutlineThickness(float thickness) {}

    // basic dummy properties for Miyoo port
    Color m_fillColor;
};

class RectangleShape: public Shape {
  public:
    RectangleShape(const Vector2f& size = Vector2f(0, 0)) : m_size(size) {}

    void setSize(const Vector2f& size) {
        m_size = size;
    }

    Vector2f getSize() const {
        return m_size;
    }

    void setPosition(const Vector2f& position) {
        m_position = position;
    }

    Vector2f getPosition() const {
        return m_position;
    }

    void setOrigin(const Vector2f& origin) {}

    void draw(RenderTarget& target, void* states = nullptr) const override;

  private:
    Vector2f m_size;
    Vector2f m_position;
};

class ConvexShape: public Shape {
  public:
    ConvexShape(std::size_t pointCount = 0) {}

    void setPointCount(std::size_t count) {}

    void setPoint(std::size_t index, const Vector2f& point) {}

    void setPosition(const Vector2f& position) {}

    void draw(RenderTarget& target, void* states = nullptr) const override {}
};

class Keyboard {
  public:
    enum Key {
        Unknown = -1,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Escape,
        LControl,
        LShift,
        LAlt,
        LSystem,
        RControl,
        RShift,
        RAlt,
        RSystem,
        Menu,
        LBracket,
        RBracket,
        Semicolon,
        Comma,
        Period,
        Quote,
        Slash,
        Backslash,
        Tilde,
        Equal,
        Hyphen,
        Space,
        Enter,
        Backspace,
        Tab,
        PageUp,
        PageDown,
        End,
        Home,
        Insert,
        Delete,
        Add,
        Subtract,
        Multiply,
        Divide,
        Left,
        Right,
        Up,
        Down,
        Numpad0,
        Numpad1,
        Numpad2,
        Numpad3,
        Numpad4,
        Numpad5,
        Numpad6,
        Numpad7,
        Numpad8,
        Numpad9,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        Pause,
        // Miyoo face button aliases (to avoid conflicts with D-pad A/D keys)
        ButtonY, // Miyoo Y = LALT
        ButtonX, // Miyoo X = LSHIFT
        ButtonA, // Miyoo A = SPACE
        ButtonB, // Miyoo B = LCTRL
        KeyCount
    };

    static bool isKeyPressed(Key key);
};

class Mouse {
  public:
    enum Button { Left, Right, Middle, XButton1, XButton2, ButtonCount };

    static Vector2i getPosition(const RenderWindow& relativeTo);
    static bool isButtonPressed(Button button);
};

class Event {
  public:
    struct Closed {};

    struct Resized {
        struct Size {
            unsigned int x, y;
        } size;
    };

    struct KeyPressed {
        Keyboard::Key code;
        bool alt, control, shift, system;
    };

    struct KeyReleased {
        Keyboard::Key code;
        bool alt, control, shift, system;
    };

    struct MouseButtonPressed {
        Mouse::Button button;
        int x, y;
    };

    enum Type {
        None,
        TypeClosed,
        TypeResized,
        TypeKeyPressed,
        TypeKeyReleased,
        TypeMouseButtonPressed
    };

    Type type = None;

    Closed closed;
    Resized resized;
    KeyPressed keyPressed;
    KeyReleased keyReleased;
    MouseButtonPressed mouseButton;

    template <typename T> const T* getIf() const {
        if constexpr (std::is_same_v<T, Closed>)
            return type == TypeClosed ? &closed : nullptr;
        if constexpr (std::is_same_v<T, Resized>)
            return type == TypeResized ? &resized : nullptr;
        if constexpr (std::is_same_v<T, KeyPressed>)
            return type == TypeKeyPressed ? &keyPressed : nullptr;
        if constexpr (std::is_same_v<T, KeyReleased>)
            return type == TypeKeyReleased ? &keyReleased : nullptr;
        if constexpr (std::is_same_v<T, MouseButtonPressed>)
            return type == TypeMouseButtonPressed ? &mouseButton : nullptr;
        return nullptr;
    }

    template <typename T> bool is() const {
        return getIf<T>() != nullptr;
    }
};

class VideoMode {
  public:
    unsigned int width, height, bitsPerPixel;

    VideoMode(unsigned int w, unsigned int h, unsigned int bpp = 32)
        : width(w), height(h), bitsPerPixel(bpp) {}
};

class RenderWindow: public RenderTarget {
  public:
    RenderWindow(const VideoMode& mode, const std::string& title);
    ~RenderWindow();

    bool isOpen() const {
        return m_isOpen;
    }

    std::optional<Event> pollEvent();

    void close() {
        m_isOpen = false;
    }

    Vector2u getSize() const {
        return Vector2u(m_view.getSize().x, m_view.getSize().y);
    }

    Vector2f mapPixelToCoords(const Vector2i& point, const View& view) const {
        return Vector2f(
            static_cast<float>(point.x), static_cast<float>(point.y)
        );
    }

    void setMouseCursorVisible(bool visible) {}

    void setFramerateLimit(unsigned int limit) {}

    void clear(const Color& color = Color(0, 0, 0, 255)) override;

    void draw(const Drawable& drawable) override {
        drawable.draw(*this);
    }

    void draw(const Drawable& drawable, void* renderStates) override {
        drawable.draw(*this, renderStates);
    }

    void display();

    void setView(const View& view) override {
        m_view = view;
    }

    const View& getView() const override {
        return m_view;
    }

    const View& getDefaultView() const {
        return m_defaultView;
    }

    // Globals set in cpp to allow access to Renderer
    static SDL_Surface* getNativeScreen();

    static void setFlipped180(bool flipped);

    SDL_Surface* getRenderTargetSurface() const override {
        return getNativeScreen();
    }

  private:
    bool m_isOpen;
    View m_view;
    View m_defaultView;
};

class Exception: public std::runtime_error {
  public:
    Exception(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace jo

#else
// Proxy to SFML if MIYOO_BUILD is off
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

namespace jo {
using namespace sf;
}
#endif
