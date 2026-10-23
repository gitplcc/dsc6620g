#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <dlink_camera.hpp>

int main(int argc, char* argv[]) {
    // 1. Inicialización de SDL3 y Gráficos (igual que el esqueleto anterior)
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* window =
        SDL_CreateWindow("Panel Control - D-Link DCS-6620G", 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_Texture* videoTexture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 640, 480);

    // Instancia de tu controlador CGI de la cámara
    DLinkCamera camaraCasa("192.168.1.100", "admin", "password");

    // 2. Inicializar el Contexto de Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();  // Estilo oscuro moderno

    // Configurar los bindings oficiales para SDL3 y su renderizador por hardware
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    bool running = true;
    SDL_Event event;

    // --- BUCLE PRINCIPAL ---
    while (running) {
        // A. Gestión de Eventos (Se los pasamos primero a ImGui)
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        // B. Iniciar el fotograma de la Interfaz Gráfica
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // C. DISEÑO DE TU INTERFAZ (Ventana de control flotante)
        ImGui::Begin("Controles de la Segunda Residencia");

        ImGui::Text("Control de Movimiento PTZ:");
        if (ImGui::Button("Arriba"))
            camaraCasa.move(DLinkCamera::PTZDirection::UP);
        ImGui::SameLine();
        if (ImGui::Button("Abajo"))
            camaraCasa.move(DLinkCamera::PTZDirection::DOWN);
        ImGui::SameLine();
        if (ImGui::Button("Izquierda"))
            camaraCasa.move(DLinkCamera::PTZDirection::LEFT);
        ImGui::SameLine();
        if (ImGui::Button("Derecha"))
            camaraCasa.move(DLinkCamera::PTZDirection::RIGHT);

        ImGui::Separator();

        // Barra para ajustar parámetros sobre la marcha enviando comandos CGI
        static int brillo = 50;
        if (ImGui::SliderInt("Brillo Cámara", &brillo, 0, 100)) {
            camaraCasa.setVideoParams(brillo, 50, 50);  // Llama al CGI dinámicamente
        }

        ImGui::End();

        // D. RENDERIZADO FINAL
        SDL_RenderClear(renderer);

        // 1. Dibujamos el vídeo recibido de la cámara (FFmpeg actualiza esta
        // textura en su hilo) Lo posicionamos en una sección de la pantalla
        SDL_FRect dstRect{ 0, 0, 640, 480 };
        SDL_RenderTexture(renderer, videoTexture, nullptr, &dstRect);

        // 2. Dibujamos la interfaz de ImGui encima
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    // 3. Limpieza de ImGui antes de cerrar SDL3
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(videoTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
