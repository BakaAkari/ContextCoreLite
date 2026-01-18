# ContextCore Lite

A lightweight Unreal Engine plugin that exports Blueprint assets to AI-readable text format.

## Features

- Export single Blueprint to structured text files
- Generates metadata JSON with class hierarchy, components, functions, and events
- Exports EventGraph and function graphs in T3D format
- Right-click context menu integration in Content Browser

## Installation

1. Copy the `ContextCoreLite` folder to your project's `Plugins` directory
2. Restart Unreal Editor
3. The plugin will be automatically enabled

## Usage

1. Right-click on any Blueprint asset in the Content Browser
2. Select **"Export for AI (Lite)"**
3. Exported files will be saved to `YourProject/Docs/.context/`

## Output Structure

```
Docs/.context/
└── [AssetPath]/
    ├── _meta.json           # Metadata (class info, components, functions)
    ├── EventGraph.txt       # Main event graph
    └── Function_*.txt       # Function graphs
```

## Limitations (Lite Version)

- Single asset export only (no recursive dependency export)
- No C++ class reflection
- No global index generation
- No auto-update on Blueprint save

## License

MIT License
