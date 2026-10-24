// Copyright (c) 2026 Pedro Luis Castedo Cepeda
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <dsc6620g/dlink_camera.hpp>

#include <curl/curl.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <utility>

// Estructura interna para gestionar la lectura de datos binarios del micrófono
struct AudioBufferState {
    const uint8_t* data;
    size_t bytesRemaining;
};

using dsc6620g::DLinkCamera;

DLinkCamera::DLinkCamera(std::string ip, std::string user, std::string password)
    : m_baseUrl("http://" + std::move(user) + ":" + std::move(password) + "@" +
        std::move(ip)) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

DLinkCamera::~DLinkCamera() { curl_global_cleanup(); }

size_t DLinkCamera::writeCallback(void* contents, size_t size, size_t nmemb,
    void* userp) {
    return size * nmemb;
}

// Callback intermitente: cURL lo llamará repetidamente para rellenar el cuerpo
// del POST HTTP
size_t DLinkCamera::readCallback(char* buffer, size_t size, size_t nmemb,
    void* userp) {
    auto* state = static_cast<AudioBufferState*>(userp);
    size_t maxBufferSize = size * nmemb;

    if (state->bytesRemaining == 0) {
        return 0;  // Fin del flujo de datos para este frame
    }

    size_t bytesToCopy = (std::min)(maxBufferSize, state->bytesRemaining);
    std::memcpy(buffer, state->data, bytesToCopy);

    // Avanzamos el puntero del buffer de audio externo
    state->data += bytesToCopy;
    state->bytesRemaining -= bytesToCopy;

    return bytesToCopy;
}

bool DLinkCamera::sendCgiCommand(const std::string& cgiPath,
    const std::string& queryParams) {
    CURL* curl = curl_easy_init();
    if (!curl)
        return false;

    std::string fullUrl = m_baseUrl + cgiPath + "?" + queryParams;
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    if (res == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    return (res == CURLE_OK && http_code == 200);
}

// --- Implementación de Transmisión de Audio (Hablar) ---
bool DLinkCamera::sendMicrophoneFrame(const std::vector<uint8_t>& pcmData) {
    if (pcmData.empty())
        return false;

    CURL* curl = curl_easy_init();
    if (!curl)
        return false;

    // Ruta CGI utilizada por la generación D-Link/Vivotek clásica para inyección
    // de audio
    std::string transmitUrl = m_baseUrl + "/cgi-bin/audio/transmit.cgi";

    AudioBufferState bufferState{ pcmData.data(), pcmData.size() };

    // Configuramos cURL para realizar un HTTP POST binario continuo
    curl_easy_setopt(curl, CURLOPT_URL, transmitUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // Configuramos los métodos de lectura
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &bufferState);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
        static_cast<long>(pcmData.size()));

    // Establecemos cabeceras para flujo de audio en tiempo real
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(
        headers, "Content-Type: audio/basic");  // Puede alternar a audio/x-raw-pcm
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
        writeCallback);  // Ignoramos respuesta del cuerpo
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,
        2L);  // Timeout ágil para no retrasar el hilo de audio

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    if (res == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && http_code == 200);
}

// --- Resto de funciones PTZ existentes ---
bool DLinkCamera::move(PTZDirection direction) {
    std::string dirStr;
    switch (direction) {
    case PTZDirection::UP:
        dirStr = "up";
        break;
    case PTZDirection::DOWN:
        dirStr = "down";
        break;
    case PTZDirection::LEFT:
        dirStr = "left";
        break;
    case PTZDirection::RIGHT:
        dirStr = "right";
        break;
    case PTZDirection::HOME:
        dirStr = "home";
        break;
    }
    return sendCgiCommand("/cgi-bin/ptctrl.cgi", "move=" + dirStr);
}

bool DLinkCamera::zoom(ZoomAction action) {
    return sendCgiCommand("/cgi-bin/ptctrl.cgi",
        std::string("zoom=") +
        ((action == ZoomAction::TELE) ? "tele" : "wide"));
}

bool DLinkCamera::setResolution(Resolution res) {
    return sendCgiCommand("/cgi-bin/camera.cgi",
        "resolution=" + std::to_string(static_cast<int>(res)));
}

bool DLinkCamera::setVideoParams(int brightness, int contrast, int saturation) {
    // Validar rangos rápidos de seguridad [0-100]
    if (brightness < 0 || brightness > 100 || contrast < 0 || contrast > 100)
        return false;

    std::string params = "brightness=" + std::to_string(brightness) +
        "&contrast=" + std::to_string(contrast) +
        "&saturation=" + std::to_string(saturation);
    return sendCgiCommand("/cgi-bin/camera.cgi", params);
}

std::string DLinkCamera::getMjpegStreamUrl() const {
    return m_baseUrl + "/cgi-bin/video.jpg";
}

std::string DLinkCamera::getAsfStreamUrl() const {
    return m_baseUrl + "/video.asf";
}

std::string DLinkCamera::getSnapshotUrl() const {
    return m_baseUrl + "/cgi-bin/image.jpg";
}
