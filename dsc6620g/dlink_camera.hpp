// Copyright (c) 2026 Pedro Luis Castedo Cepeda
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef DSC6620G_DLINK_CAMERA_HPP_
#define DSC6620G_DLINK_CAMERA_HPP_

#include <cstdint>
#include <string>
#include <vector>

namespace dsc6620g {

class DLinkCamera {
   public:
    enum class PTZDirection { UP, DOWN, LEFT, RIGHT, HOME };
    enum class ZoomAction { TELE, WIDE };
    enum class Resolution { LOW_160x120 = 1, MEDIUM_320x240 = 2, HIGH_640x480 = 4 };

    DLinkCamera(std::string ip, std::string user, std::string password);
    ~DLinkCamera();

    // --- Métodos de Control e Imagen existentes ---
    bool move(PTZDirection direction);
    bool zoom(ZoomAction action);
    bool setResolution(Resolution res);
    bool setVideoParams(int brightness, int contrast, int saturation);
    std::string getAsfStreamUrl()
        const;  // <-- El audio entrante (Escucha) viene dentro de este flujo
    std::string getMjpegStreamUrl() const;
    std::string getSnapshotUrl() const;

    // --- NUEVO: Métodos para Audio Bidireccional (Hablar) ---

    /**
     * @brief Envía una ráfaga o fragmento de audio capturado del micrófono
     * hacia el altavoz de la cámara.
     * @param pcmData Vector con los bytes de audio en bruto (PCM o formato
     * comprimido G.726 según la config).
     * @return true si el paquete fue transmitido y aceptado con éxito por el
     * firmware.
     */
    bool sendMicrophoneFrame(const std::vector<uint8_t>& pcmData);

   private:
    std::string m_baseUrl;

    bool sendCgiCommand(const std::string& cgiPath, const std::string& queryParams);
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);

    // Callback especial necesario para que cURL lea los datos binarios del
    // micrófono durante el POST
    static size_t readCallback(char* buffer, size_t size, size_t nmemb, void* userp);
};

}  // namespace dsc6620g
#endif  // DSC6620G_DLINK_CAMERA_HPP_
