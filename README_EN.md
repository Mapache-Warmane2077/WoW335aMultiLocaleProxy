# WoW 3.3.5a Multi-Locale Proxy

A `dinput8.dll` proxy that allows multiple custom locales to be used with a single WoW 3.3.5a `Wow.exe`.

## How It Works

The DLL reads the locale configured in:

```text
WTF\Config.wtf
```

Example:

```text
SET locale "ptBR"
```

When it detects a custom locale, it temporarily makes the game recognize it without permanently modifying `Wow.exe`.

It then forwards DirectInput calls to the original Windows `dinput8.dll`, so keyboard and mouse input continue to work normally.

## Main Advantage

It allows a single `Wow.exe` to support multiple languages. You only need to include the corresponding language files and select the correct locale in `Config.wtf`.

## Compatibility

- WoW 3.3.5a x86.
- 32-bit and 64-bit versions of Windows.
- DLL compiled in `Release | Win32`.
- Static runtime `/MT`.
- Designed for the compatible `Wow.exe` documented in this project.

## Installation

Copy `dinput8.dll` next to `Wow.exe`:

```text
WoW 3.3.5a\
├── Wow.exe
├── dinput8.dll
└── WTF\
    └── Config.wtf
```

Then configure the desired locale in `Config.wtf`.

## Disclaimer

This project does not include:

- `Wow.exe`.
- Game files.
- Translations.
- Language packs.

The DLL does not permanently modify the executable.

## License

This project is distributed under the MIT License.
