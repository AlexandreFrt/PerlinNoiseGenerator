#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>

class PerlinNoise {
private:
    std::vector<int> p; // Permutation vector

public:
	// Initialize the permutation vector
    PerlinNoise(unsigned int seed) {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);
        p.insert(p.end(), p.begin(), p.end());
    }

	// Fade function
    double fade(double t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

	// Linear interpolation
    double lerp(double t, double a, double b) {
        return a + t * (b - a);
    }

	// Gradient function
    double grad(int hash, double x, double y) {
        int h = hash & 7;
        double u = h < 4 ? x : y;
        double v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0 * v : 2.0 * v);
    }

	// Calculate Perlin noise
    double perlinNoise(double x, double y) {
        int xi0 = static_cast<int>(std::floor(x)) % 256;
        int xi1 = (xi0 + 1) % 256;
        int yi0 = static_cast<int>(std::floor(y)) % 256;
        int yi1 = (yi0 + 1) % 256;

        double tx = x - std::floor(x);
        double ty = y - std::floor(y);

        double u = fade(tx);
        double v = fade(ty);

        double a = grad(p[p[xi0] + yi0], tx, ty);
        double b = grad(p[p[xi1] + yi0], tx - 1, ty);
        double c = grad(p[p[xi0] + yi1], tx, ty - 1);
        double d = grad(p[p[xi1] + yi1], tx - 1, ty - 1);

        double x1 = lerp(u, a, b);
        double x2 = lerp(u, c, d);

        return lerp(v, x1, x2);
    }

	// Calculate Perlin noise with octaves
    double perlinNoiseOctaves(double x, double y, int octaves, double persistence) {
        double total = 0;
        double frequency = 1;
        double amplitude = 1;
        double max_value = 0;

        for (int i = 0; i < octaves; ++i) {
            total += perlinNoise(x * frequency, y * frequency) * amplitude;
            max_value += amplitude;
            amplitude *= persistence;
            frequency *= 2;
        }

        return (total + max_value) / (2 * max_value); // Normalize to [0, 1]
    }
};

// Generate Perlin noise image
void generateNoiseImage(sf::Image& image, PerlinNoise& noise, double scale, int octaves, double persistence) {
    int width = image.getSize().x;
    int height = image.getSize().y;

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            double x = i * scale;
            double y = j * scale;
            double value = noise.perlinNoiseOctaves(x, y, octaves, persistence);
            int color_value = static_cast<int>(value * 255);
            image.setPixel(i, j, sf::Color(color_value, color_value, color_value));
        }
    }
}

int main() {
    // Noise parameters
    int width = 256;
    int height = 256;
    unsigned int seed = 1;
    double scale = 0.1;
    int octaves = 4;
    double persistence = 0.75;

    PerlinNoise noise(seed);

    sf::RenderWindow window(sf::VideoMode(width, height), "Perlin Noise Generator");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    sf::Image image;
    image.create(width, height, sf::Color::Black);

    generateNoiseImage(image, noise, scale, octaves, persistence);

    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Perlin Noise Parameters");

        // Cast to float for ImGui sliders
        float scale_f = static_cast<float>(scale);
        float persistence_f = static_cast<float>(persistence);

        int seed_i = static_cast<int>(seed);

        // Scale slider
        if (ImGui::SliderFloat("Scale", &scale_f, 0.001f, 1.0f)) {
            scale = static_cast<double>(scale_f);
            generateNoiseImage(image, noise, scale, octaves, persistence);
            texture.loadFromImage(image);
            sprite.setTexture(texture);
        }
        // Octave slider
        if (ImGui::SliderInt("Octaves", &octaves, 1, 10.f)) {
            generateNoiseImage(image, noise, scale, octaves, persistence);
            texture.loadFromImage(image);
            sprite.setTexture(texture);
        }
        // Persistence slider
        if (ImGui::SliderFloat("Persistence", &persistence_f, 0.1f, 5.0f)) {
            persistence = static_cast<double>(persistence_f);
            generateNoiseImage(image, noise, scale, octaves, persistence);
            texture.loadFromImage(image);
            sprite.setTexture(texture);
        }
        // Seed slider
        if (ImGui::SliderInt("Seed", &seed_i, 1, 16)) {
            seed = static_cast<unsigned int>(seed_i);
            noise = PerlinNoise(seed);
            generateNoiseImage(image, noise, scale, octaves, persistence);
            texture.loadFromImage(image);
            sprite.setTexture(texture);
        }

        // End the ImGui window
        ImGui::End();

        // Clear the window with the current clear color
        window.clear();

        // Draw the sprite
        window.draw(sprite);

        // Render the ImGui interface
        ImGui::SFML::Render(window);

        // Display the contents of the window
        window.display();
    }

    // Shutdown the ImGui-SFML
    ImGui::SFML::Shutdown();
    return 0;
}
