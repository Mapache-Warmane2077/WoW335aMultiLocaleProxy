#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <windows.h>

#include <fstream>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdint>

// ============================================================
// EXPORTACIÓN DE DIRECTINPUT8CREATE
//
// En Win32/x86, WINAPI usa __stdcall y el nombre interno es:
//
//     _DirectInput8Create@20
//
// Este pragma publica el nombre externo limpio:
//
//     DirectInput8Create
//
// No se necesita archivo .def.
// ============================================================

#pragma comment(linker, "/EXPORT:DirectInput8Create=_DirectInput8Create@20")

// ============================================================
// CONFIGURACIÓN FIJA PARA ESTE WOW.EXE 3.3.5A
//
// Ghidra confirma que los cuatro bytes "enUS" se encuentran
// exactamente en esta dirección del Wow.exe utilizado.
// ============================================================

constexpr std::uintptr_t ENUS_ADDRESS = 0x009E2744u;
constexpr std::size_t LOCALE_LENGTH = 4;

// WoW 3.3.5a es un proceso de 32 bits.
// Esto evita compilar accidentalmente una DLL x64.
static_assert(
    sizeof(void*) == 4,
    "Esta DLL debe compilarse como Win32/x86."
    );

// ============================================================
// DEFINICIÓN DE DIRECTINPUT8CREATE
// ============================================================

using DirectInput8Create_t = HRESULT(WINAPI*)(
    HINSTANCE,
    DWORD,
    REFIID,
    LPVOID*,
    LPUNKNOWN
    );

// Puntero a la función real del dinput8.dll de Windows.
static DirectInput8Create_t g_originalDirectInput8Create = nullptr;

// Conservamos el handle del módulo original durante toda
// la ejecución de Wow.exe.
static HMODULE g_originalDInput8 = nullptr;

// Garantiza que el dinput8.dll original se cargue una sola vez.
static INIT_ONCE g_dinputInitOnce = INIT_ONCE_STATIC_INIT;

// ============================================================
// VALIDACIÓN DEL LOCALE
// ============================================================

static bool EsLetraAscii(unsigned char caracter)
{
    return
        (caracter >= 'A' && caracter <= 'Z') ||
        (caracter >= 'a' && caracter <= 'z');
}

static bool EsLocaleValido(const std::string& idioma)
{
    // Debe contener exactamente cuatro letras:
    // enUS, ptBR, itIT, plPL, etc.
    if (idioma.size() != LOCALE_LENGTH) {
        return false;
    }

    return std::all_of(
        idioma.begin(),
        idioma.end(),
        [](unsigned char caracter) {
            return EsLetraAscii(caracter);
        }
    );
}

// Convierte únicamente letras ASCII a mayúsculas.
// No modifica el locale original que será escrito en memoria.
static char ConvertirAsciiAMayuscula(char caracter)
{
    if (caracter >= 'a' && caracter <= 'z') {
        return static_cast<char>(
            caracter - ('a' - 'A')
            );
    }

    return caracter;
}

static bool EsIdiomaOficial(const std::string& idioma)
{
    std::string idiomaMayusculas = idioma;

    std::transform(
        idiomaMayusculas.begin(),
        idiomaMayusculas.end(),
        idiomaMayusculas.begin(),
        ConvertirAsciiAMayuscula
    );

    static const char* const idiomasOficiales[] = {
        "ENUS",
        "ENGB",
        "KOKR",
        "FRFR",
        "DEDE",
        "ZHCN",
        "ZHTW",
        "ESES",
        "ESMX",
        "RURU"
    };

    for (const char* idiomaOficial : idiomasOficiales) {
        if (idiomaMayusculas == idiomaOficial) {
            return true;
        }
    }

    return false;
}

// ============================================================
// LEER EL LOCALE DESDE WTF\CONFIG.WTF
// ============================================================

static bool LeerLocaleConfigurado(std::string& idiomaEncontrado)
{
    // Se conserva la ubicación fija definida para este proyecto.
    std::ifstream configFile(
        "WTF\\Config.wtf",
        std::ios::in
    );

    if (!configFile.is_open()) {
        return false;
    }

    std::string linea;

    while (std::getline(configFile, linea)) {
        // Formato esperado:
        //
        // SET locale "ptBR"
        //
        const std::size_t posicionLocale =
            linea.find("SET locale");

        if (posicionLocale == std::string::npos) {
            continue;
        }

        const std::size_t primeraComilla =
            linea.find('"', posicionLocale);

        if (primeraComilla == std::string::npos) {
            continue;
        }

        const std::size_t segundaComilla =
            linea.find('"', primeraComilla + 1);

        if (segundaComilla == std::string::npos) {
            continue;
        }

        const std::string candidato = linea.substr(
            primeraComilla + 1,
            segundaComilla - primeraComilla - 1
        );

        if (!EsLocaleValido(candidato)) {
            continue;
        }

        idiomaEncontrado = candidato;
        return true;
    }

    return false;
}

// ============================================================
// REEMPLAZAR "enUS" EN LA MEMORIA DE WOW.EXE
// ============================================================

static void AplicarParcheUniversal()
{
    std::string idiomaEncontrado;

    if (!LeerLocaleConfigurado(idiomaEncontrado)) {
        return;
    }

    // Los idiomas oficiales ya funcionan nativamente.
    // En ese caso no se modifica la memoria.
    if (EsIdiomaOficial(idiomaEncontrado)) {
        return;
    }

    auto* direccionLocale =
        reinterpret_cast<char*>(ENUS_ADDRESS);

    // Comprobación defensiva:
    // solamente escribimos si la dirección todavía contiene
    // exactamente los cuatro bytes esperados.
    if (std::memcmp(
        direccionLocale,
        "enUS",
        LOCALE_LENGTH) != 0) {
        return;
    }

    DWORD proteccionAnterior = 0;

    // La dirección contiene datos, no instrucciones.
    // Se habilita temporalmente lectura y escritura.
    if (!VirtualProtect(
        direccionLocale,
        LOCALE_LENGTH,
        PAGE_READWRITE,
        &proteccionAnterior)) {
        return;
    }

    // Se escriben exactamente cuatro bytes.
    //
    // Ejemplo:
    //     enUS -> ptBR
    //
    // No se escribe un terminador nulo porque ya existe
    // después de los cuatro caracteres originales.
    std::memcpy(
        direccionLocale,
        idiomaEncontrado.data(),
        LOCALE_LENGTH
    );

    // Restauramos la protección original de la página.
    DWORD proteccionTemporal = 0;

    VirtualProtect(
        direccionLocale,
        LOCALE_LENGTH,
        proteccionAnterior,
        &proteccionTemporal
    );
}

// ============================================================
// CARGAR EL DINPUT8.DLL ORIGINAL DE WINDOWS
// ============================================================

static BOOL CALLBACK InicializarDInputOriginal(
    PINIT_ONCE,
    PVOID,
    PVOID*)
{
    char rutaSistema[MAX_PATH] = {};

    const UINT longitud = GetSystemDirectoryA(
        rutaSistema,
        MAX_PATH
    );

    if (longitud == 0 || longitud >= MAX_PATH) {
        return FALSE;
    }

    if (strcat_s(
        rutaSistema,
        MAX_PATH,
        "\\dinput8.dll") != 0) {
        return FALSE;
    }

    HMODULE moduloOriginal = LoadLibraryA(
        rutaSistema
    );

    if (!moduloOriginal) {
        return FALSE;
    }

    const auto funcionOriginal =
        reinterpret_cast<DirectInput8Create_t>(
            GetProcAddress(
                moduloOriginal,
                "DirectInput8Create"
            )
            );

    if (!funcionOriginal) {
        // Esta limpieza ocurre fuera de DllMain.
        // Solo se ejecuta si la carga no pudo completarse.
        FreeLibrary(moduloOriginal);
        return FALSE;
    }

    g_originalDInput8 = moduloOriginal;
    g_originalDirectInput8Create = funcionOriginal;

    return TRUE;
}

// ============================================================
// FUNCIÓN PROXY QUE WOW.EXE LLAMA
// ============================================================

extern "C"
HRESULT WINAPI DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID* ppvOut,
    LPUNKNOWN punkOuter)
{
    // El dinput8.dll original se carga aquí, fuera de DllMain,
    // y solamente una vez.
    if (!InitOnceExecuteOnce(
        &g_dinputInitOnce,
        InicializarDInputOriginal,
        nullptr,
        nullptr)) {
        return E_FAIL;
    }

    if (!g_originalDirectInput8Create) {
        return E_FAIL;
    }

    // Reenviamos sin modificar parámetros ni resultado.
    return g_originalDirectInput8Create(
        hinst,
        dwVersion,
        riidltf,
        ppvOut,
        punkOuter
    );
}

// ============================================================
// PUNTO DE ENTRADA DE LA DLL
// ============================================================

BOOL APIENTRY DllMain(
    HMODULE,
    DWORD motivo,
    LPVOID)
{
    if (motivo == DLL_PROCESS_ATTACH) {
        // Se aplica temprano porque WoW necesita encontrar
        // el locale personalizado durante su arranque.
        AplicarParcheUniversal();
    }

    // No se llama a FreeLibrary durante DLL_PROCESS_DETACH.
    // Windows libera automáticamente los módulos cuando
    // termina el proceso de Wow.exe.
    return TRUE;
}