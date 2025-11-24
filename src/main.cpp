#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <cmath>
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>

class DrawableObject {
public:
    virtual ~DrawableObject() = default;
    virtual void draw(sf::RenderTarget& target) = 0;
};

class UpdatableObject {
public:
    virtual ~UpdatableObject() = default;
    virtual void update(float dt) = 0;
};

class TransformableObject {
public:
    virtual ~TransformableObject() = default;
    virtual void translate(float dx, float dy) = 0;
    virtual void rotate(float angleDeg) = 0;
    virtual void scale(float sx, float sy) = 0;
};

class GameObject : public virtual UpdatableObject, public virtual DrawableObject {
public:
    virtual ~GameObject() = default;
};

class ShapeObject : public virtual DrawableObject, public virtual TransformableObject {
public:
    virtual ~ShapeObject() = default;
};

class PrimitiveRenderer;

class Point2D : public ShapeObject {
    sf::Vector2f m_pos;
    sf::Color    m_color;
public:
    Point2D() : m_pos(0.f, 0.f), m_color(sf::Color::White) {}
    Point2D(float x, float y, sf::Color color = sf::Color::White)
        : m_pos(x, y), m_color(color) {
    }

    float x() const { return m_pos.x; }
    float y() const { return m_pos.y; }
    const sf::Vector2f& position() const { return m_pos; }
    void set(float x, float y) { m_pos = { x, y }; }

    void setColor(const sf::Color& c) { m_color = c; }
    sf::Color color() const { return m_color; }

    void translate(float dx, float dy) override {
        m_pos.x += dx;
        m_pos.y += dy;
    }

    void rotate(float angleDeg) override {
        float rad = angleDeg * 3.14159265359f / 180.f;
        float cs = std::cos(rad);
        float sn = std::sin(rad);
        float nx = m_pos.x * cs - m_pos.y * sn;
        float ny = m_pos.x * sn + m_pos.y * cs;
        m_pos.x = nx;
        m_pos.y = ny;
    }

    void scale(float sx, float sy) override {
        m_pos.x *= sx;
        m_pos.y *= sy;
    }

    void draw(sf::RenderTarget& target) override {
        sf::Vertex v(m_pos, m_color);
        target.draw(&v, 1, sf::Points);
    }
};

class LineSegment : public ShapeObject {
    Point2D m_a;
    Point2D m_b;
    sf::Color m_color;
    PrimitiveRenderer* m_renderer;
public:
    LineSegment()
        : m_a(), m_b(), m_color(sf::Color::White), m_renderer(nullptr) {
    }

    LineSegment(PrimitiveRenderer& renderer,
        const Point2D& a, const Point2D& b,
        sf::Color color = sf::Color::White)
        : m_a(a), m_b(b), m_color(color), m_renderer(&renderer) {
    }

    const Point2D& a() const { return m_a; }
    const Point2D& b() const { return m_b; }

    void setRenderer(PrimitiveRenderer& r) { m_renderer = &r; }

    void setColor(const sf::Color& c) { m_color = c; }

    void translate(float dx, float dy) override {
        m_a.translate(dx, dy);
        m_b.translate(dx, dy);
    }

    void rotate(float angleDeg) override {
        m_a.rotate(angleDeg);
        m_b.rotate(angleDeg);
    }

    void scale(float sx, float sy) override {
        m_a.scale(sx, sy);
        m_b.scale(sx, sy);
    }

    void draw(sf::RenderTarget& target) override;
};

class PrimitiveRenderer {
    static void putPixel(sf::RenderTarget& target, int x, int y, sf::Color color) {
        sf::Vertex v(sf::Vector2f(static_cast<float>(x),
            static_cast<float>(y)), color);
        target.draw(&v, 1, sf::Points);
    }

public:
    PrimitiveRenderer() = default;

    void drawLineDefault(sf::RenderTarget& target,
        const sf::Vector2f& a, const sf::Vector2f& b,
        sf::Color color) {
        sf::Vertex verts[2] = {
            sf::Vertex(a, color),
            sf::Vertex(b, color)
        };
        target.draw(verts, 2, sf::Lines);
    }

    void drawLineIncremental(sf::RenderTarget& target,
        const sf::Vector2f& a, const sf::Vector2f& b,
        sf::Color color) {
        float x0 = a.x;
        float y0 = a.y;
        float x1 = b.x;
        float y1 = b.y;

        float dx = x1 - x0;
        float dy = y1 - y0;

        if (dx == 0 && dy == 0) {
            putPixel(target, (int)std::round(x0), (int)std::round(y0), color);
            return;
        }

        bool steep = std::fabs(dy) > std::fabs(dx);
        if (steep) {
            std::swap(x0, y0);
            std::swap(x1, y1);
            std::swap(dx, dy);
        }

        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
            dx = x1 - x0;
            dy = y1 - y0;
        }

        float m = (dx == 0) ? 0.f : dy / dx;
        float y = y0;

        for (int x = (int)std::round(x0); x <= (int)std::round(x1); ++x) {
            int px = steep ? (int)std::round(y) : x;
            int py = steep ? x : (int)std::round(y);
            putPixel(target, px, py, color);
            y += m;
        }
    }

    void drawCircle(sf::RenderTarget& target,
        const sf::Vector2f& center,
        float R,
        sf::Color color,
        unsigned int steps = 64) {
        const float pi = 3.14159265359f;
        float x0 = center.x;
        float y0 = center.y;

        unsigned int localSteps = steps;
        for (unsigned int i = 0; i <= localSteps; ++i) {
            float alpha = (pi / 4.f) * (float)i / (float)localSteps;
            float x = R * std::cos(alpha);
            float y = R * std::sin(alpha);

            int px[8] = {
                (int)std::round(x0 + x), (int)std::round(x0 + y),
                (int)std::round(x0 - x), (int)std::round(x0 - y),
                (int)std::round(x0 - x), (int)std::round(x0 - y),
                (int)std::round(x0 + x), (int)std::round(x0 + y)
            };
            int py[8] = {
                (int)std::round(y0 + y), (int)std::round(y0 + x),
                (int)std::round(y0 + y), (int)std::round(y0 + x),
                (int)std::round(y0 - y), (int)std::round(y0 - x),
                (int)std::round(y0 - y), (int)std::round(y0 - x)
            };

            for (int k = 0; k < 8; ++k) {
                putPixel(target, px[k], py[k], color);
            }
        }
    }

    void drawEllipse(sf::RenderTarget& target,
        const sf::Vector2f& center,
        float Rx, float Ry,
        sf::Color color,
        unsigned int steps = 90) {
        const float pi = 3.14159265359f;
        float x0 = center.x;
        float y0 = center.y;

        for (unsigned int i = 0; i <= steps; ++i) {
            float alpha = (pi / 2.f) * (float)i / (float)steps;
            float x = Rx * std::cos(alpha);
            float y = Ry * std::sin(alpha);

            int px[4] = {
                (int)std::round(x0 + x),
                (int)std::round(x0 - x),
                (int)std::round(x0 + x),
                (int)std::round(x0 - x)
            };
            int py[4] = {
                (int)std::round(y0 + y),
                (int)std::round(y0 + y),
                (int)std::round(y0 - y),
                (int)std::round(y0 - y)
            };

            for (int k = 0; k < 4; ++k) {
                putPixel(target, px[k], py[k], color);
            }
        }
    }

    static float cross(const sf::Vector2f& a,
        const sf::Vector2f& b,
        const sf::Vector2f& c) {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    }

    static bool segmentsIntersect(const sf::Vector2f& a1, const sf::Vector2f& a2,
        const sf::Vector2f& b1, const sf::Vector2f& b2) {
        float d1 = cross(a1, a2, b1);
        float d2 = cross(a1, a2, b2);
        float d3 = cross(b1, b2, a1);
        float d4 = cross(b1, b2, a2);

        if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
            ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
            return true;
        }
        return false;
    }

    bool drawPolygon(sf::RenderTarget& target,
        const std::vector<sf::Vector2f>& pts,
        sf::Color color) {
        if (pts.size() < 3) return false;

        std::size_t n = pts.size();
        for (std::size_t i = 0; i < n; ++i) {
            sf::Vector2f a1 = pts[i];
            sf::Vector2f a2 = pts[(i + 1) % n];
            for (std::size_t j = i + 1; j < n; ++j) {
                if (j == i) continue;
                if ((j + 1) % n == i || (i + 1) % n == j) continue;

                sf::Vector2f b1 = pts[j];
                sf::Vector2f b2 = pts[(j + 1) % n];
                if (segmentsIntersect(a1, a2, b1, b2)) {
                    return false;
                }
            }
        }

        for (std::size_t i = 0; i < n; ++i) {
            sf::Vector2f a = pts[i];
            sf::Vector2f b = pts[(i + 1) % n];
            drawLineIncremental(target, a, b, color);
        }
        return true;
    }

    void boundaryFill(sf::Image& img,
        int x, int y,
        const sf::Color& fillColor,
        const sf::Color& boundaryColor) {
        sf::Vector2u size = img.getSize();
        if (x < 0 || y < 0 || (unsigned)x >= size.x || (unsigned)y >= size.y)
            return;

        sf::Color start = img.getPixel(x, y);
        if (start == boundaryColor || start == fillColor)
            return;

        std::queue<sf::Vector2i> q;
        q.push({ x, y });

        while (!q.empty()) {
            sf::Vector2i p = q.front();
            q.pop();

            if (p.x < 0 || p.y < 0 ||
                (unsigned)p.x >= size.x || (unsigned)p.y >= size.y)
                continue;

            sf::Color c = img.getPixel(p.x, p.y);
            if (c == boundaryColor || c == fillColor)
                continue;

            img.setPixel(p.x, p.y, fillColor);

            q.push({ p.x + 1, p.y });
            q.push({ p.x - 1, p.y });
            q.push({ p.x, p.y + 1 });
            q.push({ p.x, p.y - 1 });
        }
    }

    void floodFill(sf::Image& img,
        int x, int y,
        const sf::Color& fillColor) {
        sf::Vector2u size = img.getSize();
        if (x < 0 || y < 0 || (unsigned)x >= size.x || (unsigned)y >= size.y)
            return;

        sf::Color backgroundColor = img.getPixel(x, y);
        if (backgroundColor == fillColor)
            return;

        std::queue<sf::Vector2i> q;
        q.push({ x, y });

        while (!q.empty()) {
            sf::Vector2i p = q.front();
            q.pop();

            if (p.x < 0 || p.y < 0 ||
                (unsigned)p.x >= size.x || (unsigned)p.y >= size.y)
                continue;

            sf::Color c = img.getPixel(p.x, p.y);
            if (c != backgroundColor || c == fillColor)
                continue;

            img.setPixel(p.x, p.y, fillColor);

            q.push({ p.x + 1, p.y });
            q.push({ p.x - 1, p.y });
            q.push({ p.x, p.y + 1 });
            q.push({ p.x, p.y - 1 });
        }
    }
};

void LineSegment::draw(sf::RenderTarget& target) {
    if (m_renderer) {
        m_renderer->drawLineIncremental(
            target,
            m_a.position(),
            m_b.position(),
            m_color
        );
    }
    else {
        sf::Vertex v[2] = {
            sf::Vertex(m_a.position(), m_color),
            sf::Vertex(m_b.position(), m_color)
        };
        target.draw(v, 2, sf::Lines);
    }
}

class BitmapHandler {
public:
    static sf::Image create(unsigned width, unsigned height,
        sf::Color color = sf::Color::Transparent) {
        sf::Image img;
        img.create(width, height, color);
        return img;
    }

    static bool loadFromFile(const std::string& filename, sf::Image& img) {
        return img.loadFromFile(filename);
    }

    static bool saveToFile(const std::string& filename, const sf::Image& img) {
        return img.saveToFile(filename);
    }

    static void copy(const sf::Image& src, sf::Image& dst,
        sf::Vector2u dstPos = { 0, 0 }) {
        dst.copy(src, dstPos.x, dstPos.y, sf::IntRect(), true);
    }
};

class BitmapObject : public virtual DrawableObject, public virtual TransformableObject {
protected:
    sf::Sprite m_sprite;
public:
    virtual ~BitmapObject() = default;

    void setTexture(const sf::Texture& tex) {
        m_sprite.setTexture(tex);
    }

    void translate(float dx, float dy) override {
        m_sprite.move(dx, dy);
    }

    void rotate(float angleDeg) override {
        m_sprite.rotate(angleDeg);
    }

    void scale(float sx, float sy) override {
        m_sprite.scale(sx, sy);
    }

    void draw(sf::RenderTarget& target) override {
        target.draw(m_sprite);
    }

    sf::Vector2f position() const {
        return m_sprite.getPosition();
    }
};

class AnimatedObject : public virtual UpdatableObject {
public:
    virtual ~AnimatedObject() = default;
    virtual void animate(float dt) = 0;
};

class SpriteObject : public BitmapObject, public AnimatedObject {
protected:
    std::vector<sf::Texture> m_frames;
    float  m_timePerFrame{ 0.15f };
    float  m_timeAccumulator{ 0.f };
    std::size_t m_currentFrame{ 0 };

public:
    virtual ~SpriteObject() = default;

    void setFrames(const std::vector<sf::Texture>& frames) {
        m_frames = frames;
        if (!m_frames.empty()) {
            m_currentFrame = 0;
            m_sprite.setTexture(m_frames[0], true);
        }
    }

    void setTimePerFrame(float t) { m_timePerFrame = t; }

    void animate(float dt) override {
        if (m_frames.empty()) return;
        m_timeAccumulator += dt;
        if (m_timeAccumulator >= m_timePerFrame) {
            m_timeAccumulator -= m_timePerFrame;
            m_currentFrame = (m_currentFrame + 1) % m_frames.size();
            m_sprite.setTexture(m_frames[m_currentFrame], true);
        }
    }

    void update(float dt) override {
        animate(dt);
    }
};

class Player : public SpriteObject, public GameObject {
    sf::Vector2f m_velocity{ 0.f, 0.f };
    float m_speed{ 150.f };
public:
    Player() = default;

    void setVelocity(const sf::Vector2f& v) {
        m_velocity = v;
    }

    void update(float dt) override {
        m_sprite.move(m_velocity * dt);
        SpriteObject::update(dt);
    }
};

class Engine {
    sf::RenderWindow m_window;
    PrimitiveRenderer m_renderer;

    std::vector<std::shared_ptr<GameObject>> m_objects;

    std::shared_ptr<Player> m_player;

    sf::Image  m_imgBoundary;
    sf::Texture m_texBoundary;
    sf::Sprite  m_sprBoundary;

    sf::Image  m_imgFlood;
    sf::Texture m_texFlood;
    sf::Sprite  m_sprFlood;

public:
    Engine()
        : m_window(sf::VideoMode(1000, 700), "Silnik 2D - demo PGK") {
        m_window.setFramerateLimit(60);

        initPlayer();
        initFillDemos();
    }

    bool isOpen() const { return m_window.isOpen(); }

    void initPlayer() {
        const unsigned W = 32;
        const unsigned H = 48;
        std::vector<sf::Texture> frames;

        for (int i = 0; i < 4; ++i) {
            sf::Image img;
            img.create(W, H, sf::Color(100 + 30 * i,
                100 + 20 * i,
                255 - 30 * i));
            for (unsigned x = 0; x < W; ++x) {
                img.setPixel(x, 0, sf::Color::Black);
                img.setPixel(x, H - 1, sf::Color::Black);
            }
            for (unsigned y = 0; y < H; ++y) {
                img.setPixel(0, y, sf::Color::Black);
                img.setPixel(W - 1, y, sf::Color::Black);
            }

            sf::Texture tex;
            tex.loadFromImage(img);
            frames.push_back(tex);
        }

        m_player = std::make_shared<Player>();
        m_player->setFrames(frames);
        m_player->setTimePerFrame(0.2f);
        m_player->translate(200.f, 400.f);

        m_objects.push_back(m_player);
    }

    void initFillDemos() {
        m_imgBoundary.create(200, 150, sf::Color::White);
        sf::Color boundaryColor = sf::Color::Black;

        for (unsigned x = 10; x < 190; ++x) {
            m_imgBoundary.setPixel(x, 10, boundaryColor);
            m_imgBoundary.setPixel(x, 140, boundaryColor);
        }
        for (unsigned y = 10; y < 140; ++y) {
            m_imgBoundary.setPixel(10, y, boundaryColor);
            m_imgBoundary.setPixel(190, y, boundaryColor);
        }

        m_renderer.boundaryFill(m_imgBoundary, 50, 50,
            sf::Color(200, 255, 200),
            boundaryColor);
        m_texBoundary.loadFromImage(m_imgBoundary);
        m_sprBoundary.setTexture(m_texBoundary);
        m_sprBoundary.setPosition(700.f, 50.f);

        m_imgFlood.create(200, 150, sf::Color(240, 240, 255));
        for (unsigned x = 0; x < 200; ++x) {
            m_imgFlood.setPixel(x, 0, sf::Color::Black);
            m_imgFlood.setPixel(x, 149, sf::Color::Black);
        }
        for (unsigned y = 0; y < 150; ++y) {
            m_imgFlood.setPixel(0, y, sf::Color::Black);
            m_imgFlood.setPixel(199, y, sf::Color::Black);
        }

        m_renderer.floodFill(m_imgFlood, 100, 75,
            sf::Color(255, 220, 200));
        m_texFlood.loadFromImage(m_imgFlood);
        m_sprFlood.setTexture(m_texFlood);
        m_sprFlood.setPosition(700.f, 250.f);
    }

    void handleEvents() {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                m_window.close();
        }
    }

    void handleInput() {
        sf::Vector2f vel(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            vel.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            vel.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            vel.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            vel.y += 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            m_player->rotate(-1.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
            m_player->rotate(1.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
            m_player->scale(1.001f, 1.001f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
            m_player->scale(0.999f, 0.999f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            m_window.close();

        float speed = 150.f;
        m_player->setVelocity(vel * speed);
    }

    void update(float dt) {
        for (auto& obj : m_objects)
            obj->update(dt);
    }

    void render() {
        m_window.clear(sf::Color(220, 220, 220));

        sf::Vector2f p1(50.f, 50.f);
        sf::Vector2f p2(300.f, 100.f);
        m_renderer.drawLineDefault(m_window, p1, p2, sf::Color::Red);

        sf::Vector2f p3(50.f, 100.f);
        sf::Vector2f p4(300.f, 200.f);
        m_renderer.drawLineIncremental(m_window, p3, p4, sf::Color::Blue);

        m_renderer.drawCircle(m_window,
            sf::Vector2f(200.f, 300.f),
            60.f, sf::Color::Black);

        m_renderer.drawEllipse(m_window,
            sf::Vector2f(400.f, 300.f),
            80.f, 40.f, sf::Color::Black);

        std::vector<sf::Vector2f> polygon = {
            {100.f, 400.f},
            {200.f, 450.f},
            {180.f, 550.f},
            {60.f, 520.f}
        };
        m_renderer.drawPolygon(m_window, polygon, sf::Color::Magenta);

        m_window.draw(m_sprBoundary);
        m_window.draw(m_sprFlood);

        for (auto& obj : m_objects)
            obj->draw(m_window);

        m_window.display();
    }

    void run() {
        sf::Clock clock;
        while (m_window.isOpen()) {
            float dt = clock.restart().asSeconds();
            handleEvents();
            handleInput();
            update(dt);
            render();
        }
    }
};

int main() {
    Engine engine;
    engine.run();
    return 0;
}
