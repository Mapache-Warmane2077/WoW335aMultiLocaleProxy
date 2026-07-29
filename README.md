# WoW 3.3.5a Proxy Multidioma
**Español** | [English](README_EN.md)

DLL proxy `dinput8.dll` que permite utilizar varios locales personalizados con un único `Wow.exe` de WoW 3.3.5a.

## Funcionamiento

La DLL lee el idioma configurado en:

```text
WTF\Config.wtf
```

Ejemplo:

```text
SET locale "ptBR"
```

Cuando detecta un locale personalizado, hace que el juego lo reconozca temporalmente sin modificar permanentemente `Wow.exe`.

Después delega DirectInput al `dinput8.dll` original de Windows, por lo que el teclado y el ratón continúan funcionando normalmente.

## Ventaja principal

Permite utilizar un único `Wow.exe` para varios idiomas. Solo es necesario incluir los archivos correspondientes al idioma y seleccionar el locale correcto en `Config.wtf`.

## Compatibilidad

- WoW 3.3.5a x86.
- Windows de 32 y 64 bits.
- DLL compilada en `Release | Win32`.
- Runtime estático `/MT`.
- Diseñada para el `Wow.exe` compatible documentado en este proyecto.

## Instalación

Copia `dinput8.dll` junto a `Wow.exe`:

```text
WoW 3.3.5a\
├── Wow.exe
├── dinput8.dll
└── WTF\
    └── Config.wtf
```

Después configura el idioma deseado en `Config.wtf`.

## Aviso

Este proyecto no incluye:

- `Wow.exe`.
- Archivos del juego.
- Traducciones.
- Paquetes de idioma.

La DLL no modifica permanentemente el ejecutable.

## Licencia

Este proyecto se distribuye bajo la licencia MIT.
