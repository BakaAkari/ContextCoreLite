# ContextCore Lite (Trial Version)

[简体中文](README_CN.md)

> ⚠️ **This is the free trial version of ContextCore.** It provides basic Blueprint export functionality for evaluation purposes. For the full feature set, please purchase the complete version on [Fab](<!-- FAB_URL_PLACEHOLDER -->).

## What is ContextCore?

ContextCore is an Unreal Engine Editor plugin that exports Blueprint assets into an AI-readable text format, enabling AI coding assistants (ChatGPT, Claude, Gemini, etc.) to understand your project's visual logic and provide accurate assistance.

## Lite vs Full Version

| Feature | Lite (Free) | Full Version |
|---------|:-----------:|:------------:|
| **Blueprint Export** (EventGraph, Functions, Macros) | ✅ | ✅ |
| **AnimBlueprint Export** (State Machines) | ✅ | ✅ |
| **Metadata Generation** (`_meta.json`) | ✅ | ✅ |
| **Deep Dependency Export** (recursive asset tree) | ❌ | ✅ |
| **Live Auto-Sync** (auto re-export on save/compile) | ❌ | ✅ |
| **Native C++ Class Export** (reflection-based) | ❌ | ✅ |
| **Graph Structure Export** (JSON node connections) | ❌ | ✅ |
| **Class Defaults (CDO) Export** | ❌ | ✅ |
| **DataAsset / DataTable Export** | ❌ | ✅ |
| **UserDefinedStruct / Enum Export** | ❌ | ✅ |
| **Curve / Skeleton / BlendSpace Export** | ❌ | ✅ |
| **AnimMontage / AnimSequence Export** | ❌ | ✅ |
| **Material / MaterialFunction Export** | ❌ | ✅ |
| **MetaSound Export** | ❌ | ✅ |
| **Global Knowledge Base Index** (`_index.json`) | ❌ | ✅ |
| **Asset Tracking & Incremental Updates** | ❌ | ✅ |
| **AI Context Files** (`CONTEXT.md`, `llms.txt`) | ❌ | ✅ |
| **Source File Indexing** (class → header/cpp mapping) | ❌ | ✅ |

### Key Differences

- **Lite** exports only the **selected Blueprint itself** — one asset at a time, with no dependency scanning.
- **Full** performs **deep dependency analysis**, automatically exporting parent classes, components, interfaces, and all referenced assets to give AI the complete context of your project.
- **Full** includes **live auto-sync**: once initialized, any tracked asset is automatically re-exported when you save or compile — your AI context is never stale.
- **Full** supports **14 asset types** beyond Blueprints, including Materials, MetaSounds, DataTables, C++ classes, and more.

## Installation

1. Copy the `ContextCoreLite` folder to your project's `Plugins` directory
2. Restart the Unreal Editor
3. The plugin will load automatically (Editor-only plugin)

## Usage

1. In the Content Browser, right-click on any Blueprint asset(s)
2. Select **Export to Context (Lite)**
3. Exported files will be saved to `[ProjectDir]/Docs/.context/`

## Output Structure

```
Docs/.context/
└── [AssetPath]/
    ├── _meta.json           # Metadata (class info, components, functions)
    ├── EventGraph.txt       # Main event graph
    ├── Function_*.txt       # Function graphs
    ├── Macro_*.txt          # Macro graphs
    └── StateMachine_*.txt   # AnimBP state machines
```

## Requirements

- Unreal Engine 5.5+
- Editor-only (does not ship with packaged builds)

## Get the Full Version

The full version of **ContextCore** is available on **Fab**:

🔗 **[Get ContextCore on Fab](<!-- FAB_URL_PLACEHOLDER -->)**

Upgrade to unlock:
- 🔄 **Live Auto-Sync** — Never manually re-export again
- 🌳 **Deep Dependency Export** — AI sees your entire asset tree
- ⚙️ **C++ Reflection Export** — Bridge Blueprint and C++ logic
- 📦 **14 Asset Types** — Materials, DataTables, MetaSounds, and more
- 📋 **Knowledge Base Index** — Organized, searchable AI context

## License

This trial version is provided under the MIT License for evaluation purposes.
