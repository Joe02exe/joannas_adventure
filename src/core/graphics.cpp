#include "joanna/core/graphics.h"

#ifdef MIYOO_BUILD
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <iostream>
#include <utility>

namespace jo {

// Colors
const Color Color::Black(0, 0, 0, 255);
const Color Color::White(255, 255, 255, 255);
const Color Color::Red(255, 0, 0, 255);
const Color Color::Green(0, 255, 0, 255);
const Color Color::Blue(0, 0, 255, 255);
const Color Color::Yellow(255, 255, 0, 255);
const Color Color::Magenta(255, 0, 255, 255);
const Color Color::Cyan(0, 255, 255, 255);
const Color Color::Transparent(0, 0, 0, 0);

static SDL_Surface* g_screen = nullptr;
static bool g_flipped180 = false; // Toggle this if screen is upside down

// Shadow surface for 180-degree rotation (allocated once at startup)
static SDL_Surface* g_shadow = nullptr;

// Fast 180-degree rotation using 32-bit word copy.
// Copies src into dst rotated 180 degrees.
// Both surfaces must be the same size and 32bpp.
static void rotate180Fast(SDL_Surface* dst, SDL_Surface* src) {
    if (SDL_MUSTLOCK(src) && SDL_LockSurface(src) < 0)
        return;
    if (SDL_MUSTLOCK(dst) && SDL_LockSurface(dst) < 0) {
        if (SDL_MUSTLOCK(src))
            SDL_UnlockSurface(src);
        return;
    }
    const int total = src->w * src->h;
    const Uint32* s = static_cast<const Uint32*>(src->pixels);
    Uint32* d = static_cast<Uint32*>(dst->pixels) + total - 1;
    // Walk source forward, write destination backward
    for (int i = 0; i < total; ++i) {
        *d-- = *s++;
    }
    if (SDL_MUSTLOCK(dst))
        SDL_UnlockSurface(dst);
    if (SDL_MUSTLOCK(src))
        SDL_UnlockSurface(src);
}

void RenderWindow::setFlipped180(bool flipped) {
    g_flipped180 = flipped;
}

SDL_Surface* RenderWindow::getNativeScreen() {
    return g_screen;
}

RenderWindow::RenderWindow(const VideoMode& mode, const std::string& title)
    : m_isOpen(true), m_view(FloatRect(0, 0, mode.width, mode.height)),
      m_defaultView(FloatRect(0, 0, mode.width, mode.height)) {
    // Miyoo Mini screen is physically mounted upside-down; flip to correct it.
    g_flipped180 = true;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        throw Exception(
            "SDL could not initialize! SDL_Error: " +
            std::string(SDL_GetError())
        );
    }

    // SDL_SWSURFACE: use software surface without vsync/hardware flip.
    // SDL_DOUBLEBUF on this Miyoo framebuffer driver caused FlipHWSurface
    // to block at ~10 Hz (the hardware vsync rate), hard-capping FPS at 10.
    g_screen = SDL_SetVideoMode(mode.width, mode.height, 32, SDL_SWSURFACE);
    if (!g_screen) {
        throw Exception(
            "Window could not be created! SDL_Error: " +
            std::string(SDL_GetError())
        );
    }

    // Allocate shadow surface for 180-degree rotation (only used if
    // g_flipped180)
    g_shadow = SDL_CreateRGBSurface(
        SDL_SWSURFACE, mode.width, mode.height, g_screen->format->BitsPerPixel,
        g_screen->format->Rmask, g_screen->format->Gmask,
        g_screen->format->Bmask, g_screen->format->Amask
    );

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        throw Exception(
            "SDL_image could not initialize! SDL_image Error: " +
            std::string(IMG_GetError())
        );
    }

    if (TTF_Init() == -1) {
        throw Exception(
            "SDL_ttf could not initialize! SDL_ttf Error: " +
            std::string(TTF_GetError())
        );
    }
}

RenderWindow::~RenderWindow() {
    if (g_shadow) {
        SDL_FreeSurface(g_shadow);
        g_shadow = nullptr;
    }
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Maps an SDL keysym to a jo::Keyboard::Key.
// Miyoo Mini SDL1.2 button mappings:
//   A=SPACE, B=LCTRL, X=LSHIFT, Y=LALT
//   START=RETURN, SELECT=ESCAPE
//   L1=E, R1=T, L2=TAB, R2=BACKSPACE
static Keyboard::Key sdlKeysymToJoKey(SDLKey sym) {
    switch (sym) {
        case SDLK_a:
            return Keyboard::Key::A;
        case SDLK_b:
            return Keyboard::Key::B;
        case SDLK_c:
            return Keyboard::Key::C;
        case SDLK_d:
            return Keyboard::Key::D;
        case SDLK_e:
            return Keyboard::Key::E;
        case SDLK_f:
            return Keyboard::Key::F;
        case SDLK_g:
            return Keyboard::Key::G;
        case SDLK_h:
            return Keyboard::Key::H;
        case SDLK_i:
            return Keyboard::Key::I;
        case SDLK_j:
            return Keyboard::Key::J;
        case SDLK_k:
            return Keyboard::Key::K;
        case SDLK_l:
            return Keyboard::Key::L;
        case SDLK_m:
            return Keyboard::Key::M;
        case SDLK_n:
            return Keyboard::Key::N;
        case SDLK_o:
            return Keyboard::Key::O;
        case SDLK_p:
            return Keyboard::Key::P;
        case SDLK_q:
            return Keyboard::Key::Q;
        case SDLK_r:
            return Keyboard::Key::R;
        case SDLK_s:
            return Keyboard::Key::S;
        case SDLK_t:
            return Keyboard::Key::T;
        case SDLK_u:
            return Keyboard::Key::U;
        case SDLK_v:
            return Keyboard::Key::V;
        case SDLK_w:
            return Keyboard::Key::W;
        case SDLK_x:
            return Keyboard::Key::X;
        case SDLK_y:
            return Keyboard::Key::Y;
        case SDLK_z:
            return Keyboard::Key::Z;
        case SDLK_SPACE:
            return Keyboard::Key::Space;
        case SDLK_RETURN:
            return Keyboard::Key::Enter;
        case SDLK_ESCAPE:
            return Keyboard::Key::Escape;
        case SDLK_TAB:
            return Keyboard::Key::Tab;
        case SDLK_BACKSPACE:
            return Keyboard::Key::Backspace;
        case SDLK_UP:
            return Keyboard::Key::Up;
        case SDLK_DOWN:
            return Keyboard::Key::Down;
        case SDLK_LEFT:
            return Keyboard::Key::Left;
        case SDLK_RIGHT:
            return Keyboard::Key::Right;
        // Miyoo face buttons
        case SDLK_LALT:
            return Keyboard::Key::ButtonY; // Miyoo Y = attack
        case SDLK_LSHIFT:
            return Keyboard::Key::ButtonX; // Miyoo X = roll/counter
        case SDLK_LCTRL:
            return Keyboard::Key::ButtonB; // Miyoo B
        default:
            return Keyboard::Key::Unknown;
    }
}

std::optional<Event> RenderWindow::pollEvent() {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        Event event;
        if (e.type == SDL_QUIT) {
            event.type = Event::TypeClosed;
            return event;
        }
        if (e.type == SDL_KEYDOWN) {
            event.type = Event::TypeKeyPressed;
            event.keyPressed.code = sdlKeysymToJoKey(e.key.keysym.sym);
            event.keyPressed.alt = false;
            event.keyPressed.control = false;
            event.keyPressed.shift = false;
            event.keyPressed.system = false;
            return event;
        }
        if (e.type == SDL_KEYUP) {
            event.type = Event::TypeKeyReleased;
            event.keyReleased.code = sdlKeysymToJoKey(e.key.keysym.sym);
            event.keyReleased.alt = false;
            event.keyReleased.control = false;
            event.keyReleased.shift = false;
            event.keyReleased.system = false;
            return event;
        }
    }
    return std::nullopt;
}

void RenderWindow::clear(const Color& color) {
    if (g_screen) {
        Uint32 col = SDL_MapRGB(g_screen->format, color.r, color.g, color.b);
        SDL_FillRect(g_screen, nullptr, col);
    }
}

void RenderWindow::display() {
    if (!g_screen)
        return;

    if (g_flipped180 && g_shadow) {
        // Blit the back-buffer we've been drawing into (g_screen) into shadow,
        // then copy shadow rotated 180 degrees back into g_screen.
        SDL_BlitSurface(g_screen, nullptr, g_shadow, nullptr);
        rotate180Fast(g_screen, g_shadow);
    }

    // SDL_UpdateRect instead of SDL_Flip: no hardware vsync block.
    SDL_UpdateRect(g_screen, 0, 0, 0, 0);
}

// Coordinate mapping helper
static Vector2i transformCoords(Vector2i pos) {
    if (g_flipped180 && g_screen) {
        return Vector2i(g_screen->w - 1 - pos.x, g_screen->h - 1 - pos.y);
    }
    return pos;
}

bool Texture::loadFromFile(const std::string& filename) {
    if (!g_screen)
        return false;
    SDL_Surface* loadedSurface = IMG_Load(filename.c_str());
    if (!loadedSurface) {
        std::cerr << "Unable to load image " << filename
                  << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    SDL_Surface* finalSurface = nullptr;

    if (loadedSurface->format->BitsPerPixel == 8 &&
        loadedSurface->format->palette != nullptr) {
        // ---- 8-bit indexed sprite (entity/NPC sprites) ----
        // Keep as paletted surface — 4x less memory, fast SDL palette blit.
        // Palette index 0 is transparent (standard convention for our assets).
        finalSurface = loadedSurface;
        SDL_SetColorKey(finalSurface, SDL_SRCCOLORKEY, 0);
        std::cout << "[Tex] 8-bit palette: " << filename << " ("
                  << finalSurface->w << "x" << finalSurface->h << ")\n";
    } else {
        // ---- True-color image (tileset, backgrounds, UI) ----
        // Convert to exact screen format to avoid per-blit pixel conversion.
        SDL_Surface* optimizedSurface = loadedSurface;
        if (g_screen && g_screen->format) {
            SDL_PixelFormat* fmt = g_screen->format;
            optimizedSurface = SDL_CreateRGBSurface(
                SDL_SWSURFACE, loadedSurface->w, loadedSurface->h, 32,
                fmt->Rmask, fmt->Gmask, fmt->Bmask,
                ~(fmt->Rmask | fmt->Gmask | fmt->Bmask)
            );
            if (optimizedSurface) {
                SDL_SetAlpha(loadedSurface, 0, 0);
                SDL_BlitSurface(
                    loadedSurface, nullptr, optimizedSurface, nullptr
                );
                SDL_FreeSurface(loadedSurface);
            } else {
                optimizedSurface = loadedSurface; // fallback
            }
        }
        finalSurface = optimizedSurface;
        std::cout << "[Tex] 32-bit: " << filename << " (" << finalSurface->w
                  << "x" << finalSurface->h << ")\n";
    }

    m_width = finalSurface->w;
    m_height = finalSurface->h;

    if (m_surface)
        SDL_FreeSurface(m_surface);
    m_surface = finalSurface;
    return true;
}

Color Texture::getPixel(const Vector2u& pos) const {
    if (!m_surface || (int)pos.x >= m_surface->w || (int)pos.y >= m_surface->h)
        return Color::Transparent;

    Uint32 pixel = 0;
    Uint8 r, g, b, a;

    if (SDL_MUSTLOCK(m_surface))
        SDL_LockSurface(m_surface);

    int bpp = m_surface->format->BytesPerPixel;
    Uint8* p =
        (Uint8*)m_surface->pixels + pos.y * m_surface->pitch + pos.x * bpp;

    switch (bpp) {
        case 1:
            pixel = *p;
            break;
        case 2:
            pixel = *(Uint16*)p;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel = p[0] << 16 | p[1] << 8 | p[2];
            else
                pixel = p[0] | p[1] << 8 | p[2] << 16;
            break;
        case 4:
            pixel = *(Uint32*)p;
            break;
    }

    if (SDL_MUSTLOCK(m_surface))
        SDL_UnlockSurface(m_surface);

    SDL_GetRGBA(pixel, m_surface->format, &r, &g, &b, &a);
    return Color(r, g, b, a);
}

void Sprite::draw(RenderTarget& target, void* /*states*/) const {
    const View& view = target.getView();

    SDL_Surface* dest = target.getRenderTargetSurface();
    if (!m_texture || !m_texture->getNativeSurface() || !dest)
        return;

    SDL_Surface* src = m_texture->getNativeSurface();

    // View coordinate mapping calculations
    float viewportX = view.getViewport().position.x * dest->w;
    float viewportY = view.getViewport().position.y * dest->h;
    float viewportW = view.getViewport().size.x * dest->w;
    float viewportH = view.getViewport().size.y * dest->h;

    float scaleX = viewportW / view.getSize().x;
    float scaleY = viewportH / view.getSize().y;

    float offsetX =
        m_position.x - (view.getCenter().x - view.getSize().x / 2.f);
    float offsetY =
        m_position.y - (view.getCenter().y - view.getSize().y / 2.f);

    SDL_Rect srcRect = { static_cast<Sint16>(m_textureRect.position.x),
                         static_cast<Sint16>(m_textureRect.position.y),
                         static_cast<Uint16>(m_textureRect.size.x),
                         static_cast<Uint16>(m_textureRect.size.y) };

    // Use rounding instead of truncating cast to avoid 1px black gaps between
    // tiles when fractional scale produces values like 15.9999.
    // iround: portable integer round without needing lroundf.
    auto iround = [](float v) { return static_cast<int>(v + 0.5f); };
    SDL_Rect dstRect = {
        static_cast<Sint16>(iround(viewportX + offsetX * scaleX)),
        static_cast<Sint16>(iround(viewportY + offsetY * scaleY)),
        static_cast<Uint16>(iround(m_textureRect.size.x * m_scale.x * scaleX)),
        static_cast<Uint16>(iround(m_textureRect.size.y * m_scale.y * scaleY))
    };

    SDL_Rect clipRect = { static_cast<Sint16>(viewportX),
                          static_cast<Sint16>(viewportY),
                          static_cast<Uint16>(viewportW),
                          static_cast<Uint16>(viewportH) };
    SDL_SetClipRect(dest, &clipRect);

    if (dstRect.w > 0 && dstRect.h > 0) {
        // Fast path: no scaling needed and no flip
        if (dstRect.w == srcRect.w && dstRect.h == srcRect.h && !m_flipHoriz) {
            SDL_BlitSurface(src, &srcRect, dest, &dstRect);
        } else if (src->format->BitsPerPixel == 8) {
            // ---- 8-bit paletted sprite blit (entity sprites) ----
            // Convert the cropped source region to 32-bit first so we
            // can scale and optionally flip it via the pixel loop below.
            if (srcRect.w <= 0 || srcRect.h <= 0)
                return;

            SDL_PixelFormat* fmt = dest->format;
            SDL_Surface* tmp = SDL_CreateRGBSurface(
                SDL_SWSURFACE, srcRect.w, srcRect.h, 32, fmt->Rmask, fmt->Gmask,
                fmt->Bmask, ~(fmt->Rmask | fmt->Gmask | fmt->Bmask)
            );
            if (!tmp)
                return;

            // Blit the cropped 8-bit region → 32-bit (SDL converts palette)
            SDL_BlitSurface(src, &srcRect, tmp, nullptr);

            // Now scale (and optionally flip) from tmp → dest
            SDL_Rect tmpSrc = { 0, 0, (Uint16)srcRect.w, (Uint16)srcRect.h };
            SDL_LockSurface(tmp);
            if (SDL_MUSTLOCK(dest))
                SDL_LockSurface(dest);

            Uint32 stepX_fixed = (tmpSrc.w << 16) / dstRect.w;
            Uint32 stepY_fixed = (tmpSrc.h << 16) / dstRect.h;
            Uint32 amask = tmp->format->Amask;
            Uint32 curY = 0;

            for (int y = 0; y < dstRect.h; ++y) {
                int dy = dstRect.y + y;
                if (dy < 0) {
                    curY += stepY_fixed;
                    continue;
                }
                if (dy >= dest->h)
                    break;
                int sy_idx = curY >> 16;
                curY += stepY_fixed;
                if (sy_idx >= tmp->h)
                    continue;
                Uint32* srcRow =
                    (Uint32*)((Uint8*)tmp->pixels + sy_idx * tmp->pitch);
                Uint32* dstRow =
                    (Uint32*)((Uint8*)dest->pixels + dy * dest->pitch);
                Uint32 curX = 0;
                for (int x = 0; x < dstRect.w; ++x) {
                    int dx = dstRect.x + x;
                    if (dx < 0) {
                        curX += stepX_fixed;
                        continue;
                    }
                    if (dx >= dest->w)
                        break;
                    int sx_idx = m_flipHoriz ? (tmpSrc.w - 1 - (curX >> 16))
                                             : (curX >> 16);
                    curX += stepX_fixed;
                    if (sx_idx < 0 || sx_idx >= tmp->w)
                        continue;
                    Uint32 pixel = srcRow[sx_idx];
                    if (!amask || (pixel & amask))
                        dstRow[dx] = pixel;
                }
            }

            SDL_UnlockSurface(tmp);
            if (SDL_MUSTLOCK(dest))
                SDL_UnlockSurface(dest);
            SDL_FreeSurface(tmp);
        } else {
            // Guard against zero-size rectangles
            if (srcRect.w <= 0 || srcRect.h <= 0)
                return;

            // Simple nearest-neighbor scaling with binary alpha support for
            // Miyoo
            if (dest->format->BytesPerPixel == 4 &&
                src->format->BytesPerPixel == 4) {
                if (SDL_MUSTLOCK(dest))
                    SDL_LockSurface(dest);
                if (SDL_MUSTLOCK(src))
                    SDL_LockSurface(src);

                // Fixed point scaling (16.16 format) to avoid slow floating
                // point operations
                Uint32 stepX_fixed = (srcRect.w << 16) / dstRect.w;
                Uint32 stepY_fixed = (srcRect.h << 16) / dstRect.h;

                Uint32 amask = src->format->Amask;
                Uint32 curY = 0;

                for (int y = 0; y < dstRect.h; ++y) {
                    int dy = dstRect.y + y;
                    if (dy < 0) {
                        curY += stepY_fixed;
                        continue;
                    }
                    if (dy >= dest->h)
                        break; // Out of bounds, rest are as well

                    int sy_idx = srcRect.y + (curY >> 16);
                    curY += stepY_fixed;

                    if (sy_idx < 0 || sy_idx >= src->h)
                        continue;

                    Uint32* srcRow =
                        (Uint32*)((Uint8*)src->pixels + sy_idx * src->pitch);
                    Uint32* dstRow =
                        (Uint32*)((Uint8*)dest->pixels + dy * dest->pitch);

                    Uint32 curX = 0;
                    for (int x = 0; x < dstRect.w; ++x) {
                        int dx = dstRect.x + x;
                        if (dx < 0) {
                            curX += stepX_fixed;
                            continue;
                        }
                        if (dx >= dest->w)
                            break; // Out of bounds, rest are as well

                        int sx_idx = srcRect.x + (curX >> 16);
                        if (m_flipHoriz) {
                            sx_idx = srcRect.x + srcRect.w - 1 - (curX >> 16);
                        }
                        curX += stepX_fixed;

                        if (sx_idx < 0 || sx_idx >= src->w)
                            continue;

                        Uint32 pixel = srcRow[sx_idx];
                        // Binary alpha test: if not fully transparent, draw it
                        if (!amask || (pixel & amask)) {
                            dstRow[dx] = pixel;
                        }
                    }
                }

                if (SDL_MUSTLOCK(src))
                    SDL_UnlockSurface(src);
                if (SDL_MUSTLOCK(dest))
                    SDL_UnlockSurface(dest);
            } else {
                // Fallback for non-32bit surfaces (loses alpha, but works)
                if (srcRect.x < 0)
                    srcRect.x = 0;
                if (srcRect.y < 0)
                    srcRect.y = 0;
                if (srcRect.x + srcRect.w > src->w)
                    srcRect.w = src->w - srcRect.x;
                if (srcRect.y + srcRect.h > src->h)
                    srcRect.h = src->h - srcRect.y;
                SDL_SoftStretch(src, &srcRect, dest, &dstRect);
            }
        }
    }
}

void Text::draw(RenderTarget& target, void* /*states*/) const {
    if (!m_font || !m_font->getNativeFont() || m_string.empty() || !g_screen)
        return;

    SDL_Color sdlColor = { m_color.r, m_color.g, m_color.b,
                           m_color.a }; // Unused in SDL1 Blended
    SDL_Surface* surface = TTF_RenderText_Blended(
        m_font->getNativeFont(), m_string.c_str(), sdlColor
    );
    if (!surface)
        return;

    const View& view = target.getView();
    SDL_Surface* dest = target.getRenderTargetSurface();
    if (!dest) {
        SDL_FreeSurface(surface);
        return;
    }

    float viewportX = view.getViewport().position.x * dest->w;
    float viewportY = view.getViewport().position.y * dest->h;
    float viewportW = view.getViewport().size.x * dest->w;
    float viewportH = view.getViewport().size.y * dest->h;

    float scaleX = viewportW / view.getSize().x;
    float scaleY = viewportH / view.getSize().y;

    float offsetX =
        m_position.x - (view.getCenter().x - view.getSize().x / 2.f);
    float offsetY =
        m_position.y - (view.getCenter().y - view.getSize().y / 2.f);

    SDL_Rect dstRect = { static_cast<Sint16>(viewportX + offsetX * scaleX),
                         static_cast<Sint16>(viewportY + offsetY * scaleY), 0,
                         0 };

    SDL_Rect clipRect = { static_cast<Sint16>(viewportX),
                          static_cast<Sint16>(viewportY),
                          static_cast<Uint16>(viewportW),
                          static_cast<Uint16>(viewportH) };
    SDL_SetClipRect(dest, &clipRect);

    SDL_BlitSurface(surface, nullptr, dest, &dstRect);
    SDL_FreeSurface(surface);
    SDL_SetClipRect(dest, nullptr); // Reset clip rect
}

bool RenderTexture::create(unsigned int width, unsigned int height) {
    if (!g_screen)
        return false;

    SDL_Surface* surface = SDL_CreateRGBSurface(
        SDL_SWSURFACE, width, height, g_screen->format->BitsPerPixel,
        g_screen->format->Rmask, g_screen->format->Gmask,
        g_screen->format->Bmask, 0
    );
    if (surface) {
        m_dummyTex.setNativeSurface(surface, width, height);
        m_view.setSize(
            Vector2f(static_cast<float>(width), static_cast<float>(height))
        );
        m_view.setCenter(Vector2f(
            static_cast<float>(width) / 2.f, static_cast<float>(height) / 2.f
        ));
        return true;
    }
    return false;
}

void RenderTexture::clear(const Color& color) {
    SDL_Surface* sfc = m_dummyTex.getNativeSurface();
    if (!sfc)
        return;
    Uint32 col = SDL_MapRGB(sfc->format, color.r, color.g, color.b);
    SDL_FillRect(sfc, nullptr, col);
}

void RectangleShape::draw(RenderTarget& target, void* /*states*/) const {
    SDL_Surface* dest = target.getRenderTargetSurface();
    if (!dest)
        return;

    const View& view = target.getView();
    float viewportX = view.getViewport().position.x * dest->w;
    float viewportY = view.getViewport().position.y * dest->h;
    float viewportW = view.getViewport().size.x * dest->w;
    float viewportH = view.getViewport().size.y * dest->h;

    float scaleX = viewportW / view.getSize().x;
    float scaleY = viewportH / view.getSize().y;

    float offsetX =
        m_position.x - (view.getCenter().x - view.getSize().x / 2.f);
    float offsetY =
        m_position.y - (view.getCenter().y - view.getSize().y / 2.f);

    SDL_Rect rect = { static_cast<Sint16>(viewportX + offsetX * scaleX),
                      static_cast<Sint16>(viewportY + offsetY * scaleY),
                      static_cast<Uint16>(m_size.x * scaleX),
                      static_cast<Uint16>(m_size.y * scaleY) };

    SDL_Rect clipRect = { static_cast<Sint16>(viewportX),
                          static_cast<Sint16>(viewportY),
                          static_cast<Uint16>(viewportW),
                          static_cast<Uint16>(viewportH) };
    SDL_SetClipRect(dest, &clipRect);

    Uint32 col =
        SDL_MapRGB(dest->format, m_fillColor.r, m_fillColor.g, m_fillColor.b);
    SDL_FillRect(dest, &rect, col);
    SDL_SetClipRect(dest, nullptr); // Reset clip rect
}

Color Image::getPixel(const Vector2u& pos) const {
    if (!m_surface || pos.x >= (unsigned int)m_surface->w ||
        pos.y >= (unsigned int)m_surface->h)
        return Color::Transparent;

    Uint32 pixel = 0;
    Uint8 r, g, b, a;

    if (SDL_MUSTLOCK(m_surface))
        SDL_LockSurface(m_surface);

    int bpp = m_surface->format->BytesPerPixel;
    Uint8* p =
        (Uint8*)m_surface->pixels + pos.y * m_surface->pitch + pos.x * bpp;

    switch (bpp) {
        case 1:
            pixel = *p;
            break;
        case 2:
            pixel = *(Uint16*)p;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel = p[0] << 16 | p[1] << 8 | p[2];
            else
                pixel = p[0] | p[1] << 8 | p[2] << 16;
            break;
        case 4:
            pixel = *(Uint32*)p;
            break;
    }

    if (SDL_MUSTLOCK(m_surface))
        SDL_UnlockSurface(m_surface);

    SDL_GetRGBA(pixel, m_surface->format, &r, &g, &b, &a);
    return Color(r, g, b, a);
}

bool Keyboard::isKeyPressed(Key key) {
    const Uint8* state = SDL_GetKeyState(nullptr);
    // Miyoo Mini Button Mappings via SDL1.2:
    // A = Space
    // B = LCTRL
    // X = LSHIFT
    // Y = LALT
    // START = RETURN
    // SELECT = ESCAPE
    // L1 = E, R1 = T, L2 = TAB, R2 = BACKSPACE

    if (key == W || key == Up)
        return state[SDLK_w] || state[SDLK_UP];
    if (key == A || key == Left)
        return state[SDLK_a] || state[SDLK_LEFT];
    if (key == S || key == Down)
        return state[SDLK_s] || state[SDLK_DOWN];
    if (key == D || key == Right)
        return state[SDLK_d] || state[SDLK_RIGHT];
    // Miyoo Y (LALT) = attack; also map keyboard D for PC testing
    if (key == ButtonY)
        return state[SDLK_LALT] || state[SDLK_d];
    // Miyoo X (LSHIFT) = roll/counter; also map keyboard A for PC testing
    if (key == ButtonX)
        return state[SDLK_LSHIFT] || state[SDLK_a];

    // Miyoo A button (SPACE/LCTRL/RETURN) and B button
    if (key == Space || key == Enter)
        return state[SDLK_SPACE] || state[SDLK_LCTRL] || state[SDLK_RETURN];

    // Miyoo T button (interact, R1 = T on Miyoo SDL1.2)
    if (key == T)
        return state[SDLK_t];

    // Miyoo face button aliases
    if (key == ButtonA)
        return state[SDLK_SPACE] || state[SDLK_LCTRL];
    if (key == ButtonB)
        return state[SDLK_LCTRL];
    if (key == ButtonY)
        return state[SDLK_LALT]; // Miyoo Y = LALT
    if (key == ButtonX)
        return state[SDLK_LSHIFT]; // Miyoo X = LSHIFT

    // Miyoo Y button -> Sprint (Miyoo Y = LALT)
    if (key == LShift)
        return state[SDLK_LALT] || state[SDLK_LSHIFT];

    // Miyoo X button -> Apply
    if (key == E)
        return state[SDLK_LSHIFT] || state[SDLK_e];

    // Select/Start -> Pause / Map
    // On Miyoo+OnionOS: START (RETURN) = pause. MENU (ESCAPE) = system hotkey.
    // On PC: P or Escape = pause.
    if (key == Escape || key == P)
        return state[SDLK_RETURN] || state[SDLK_p];

    // Map Overview — TAB (L2) or keyboard M, or SELECT button (RCTRL on
    // Miyoo/OnionOS)
    if (key == M)
        return state[SDLK_TAB] || state[SDLK_m] || state[SDLK_RCTRL];

    // L1/R1 Bumper Buttons -> Inventory slots (L1=E, R1=T natively on Miyoo
    // SDL1.2)
    if (key == L)
        return state[SDLK_e] || state[SDLK_l];
    if (key == R)
        return state[SDLK_t] || state[SDLK_r];

    return false;
}

Vector2i Mouse::getPosition(const RenderWindow& relativeTo) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return transformCoords(Vector2i(x, y));
}

bool Mouse::isButtonPressed(Button button) {
    Uint32 state = SDL_GetMouseState(nullptr, nullptr);
    if (button == Left)
        return state & SDL_BUTTON(SDL_BUTTON_LEFT);
    if (button == Right)
        return state & SDL_BUTTON(SDL_BUTTON_RIGHT);
    return false;
}

} // namespace jo
#endif
