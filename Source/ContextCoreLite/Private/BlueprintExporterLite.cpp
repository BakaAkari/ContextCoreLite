// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintExporterLite.h"
#include "Animation/AnimBlueprint.h"
#include "AnimationStateMachineGraph.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimStateNode.h"
#include "AnimStateTransitionNode.h"
#include "EdGraph/EdGraph.h"
#include "Engine/Blueprint.h"
#include "Exporters/Exporter.h"
#include "Misc/StringOutputDevice.h"
#include "UnrealExporter.h"
#include "Framework/Notifications/NotificationManager.h"
#include "MetadataGeneratorLite.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Widgets/Notifications/SNotificationList.h"

void FBlueprintExporterLite::ExportSelectedAssets(
    const TArray<FAssetData> &SelectedAssets) {
  
  int32 ExportedCount = 0;
  int32 FailedCount = 0;

  for (const FAssetData &AssetData : SelectedAssets) {
    UObject *Asset = AssetData.GetAsset();
    if (!Asset) {
      continue;
    }

    // Only handle Blueprints in Lite version
    if (UBlueprint *Blueprint = Cast<UBlueprint>(Asset)) {
      if (ExportBlueprint(Blueprint)) {
        ExportedCount++;
      } else {
        FailedCount++;
      }
    }
  }

  // Show notification
  FNotificationInfo Info(FText::Format(
      NSLOCTEXT("ContextCoreLite", "ExportComplete",
                "[ContextCore Lite] Exported {0} Blueprint(s). Upgrade to full version for dependency export."),
      FText::AsNumber(ExportedCount)));
  Info.ExpireDuration = 5.0f;
  Info.bUseLargeFont = false;
  FSlateNotificationManager::Get().AddNotification(Info);

  UE_LOG(LogTemp, Log, TEXT("[ContextCore Lite] Export complete: %d success, %d failed"),
         ExportedCount, FailedCount);
}

bool FBlueprintExporterLite::ExportBlueprint(UBlueprint *Blueprint) {
  if (!Blueprint) {
    return false;
  }

  FString OutputDir = GetBlueprintOutputPath(Blueprint);

  // Create output directory
  IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
  if (!PlatformFile.DirectoryExists(*OutputDir)) {
    PlatformFile.CreateDirectoryTree(*OutputDir);
  }

  bool bSuccess = true;

  // Export all graphs
  // UbergraphPages (EventGraph, etc.)
  for (UEdGraph *Graph : Blueprint->UbergraphPages) {
    if (!ExportGraph(Graph, OutputDir, Graph->GetName() + TEXT(".txt"))) {
      bSuccess = false;
    }
  }

  // Function graphs
  for (UEdGraph *Graph : Blueprint->FunctionGraphs) {
    if (!ExportGraph(Graph, OutputDir, TEXT("Function_") + Graph->GetName() + TEXT(".txt"))) {
      bSuccess = false;
    }
  }

  // Macro graphs
  for (UEdGraph *Graph : Blueprint->MacroGraphs) {
    if (!ExportGraph(Graph, OutputDir, TEXT("Macro_") + Graph->GetName() + TEXT(".txt"))) {
      bSuccess = false;
    }
  }

  // Generate metadata JSON
  if (!GenerateMetadata(Blueprint, OutputDir)) {
    bSuccess = false;
  }

  // AnimBlueprint specific exports
  if (UAnimBlueprint *AnimBlueprint = Cast<UAnimBlueprint>(Blueprint)) {
    for (UEdGraph *Graph : AnimBlueprint->FunctionGraphs) {
      if (Graph && Graph->GetFName().ToString().Contains(TEXT("AnimGraph"))) {
        for (UEdGraphNode *Node : Graph->Nodes) {
          if (UAnimGraphNode_StateMachine *SMNode = Cast<UAnimGraphNode_StateMachine>(Node)) {
            FString SMName = SMNode->GetNodeTitle(ENodeTitleType::ListView).ToString();
            FString SafeName = SanitizeFileName(SMName);
            
            if (SMNode->EditorStateMachineGraph) {
              ExportGraph(Cast<UEdGraph>(SMNode->EditorStateMachineGraph.Get()), OutputDir,
                         TEXT("StateMachine_") + SafeName + TEXT(".txt"));
            }
          }
        }
      }
    }
  }

  UE_LOG(LogTemp, Log, TEXT("[ContextCore Lite] Exported: %s"), *Blueprint->GetName());
  return bSuccess;
}

FString FBlueprintExporterLite::GetOutputDirectory() {
  return FPaths::ProjectDir() / TEXT("Docs") / TEXT(".context");
}

FString FBlueprintExporterLite::GetBlueprintOutputPath(UBlueprint *Blueprint) {
  FString PackagePath = Blueprint->GetOutermost()->GetName();
  PackagePath.RemoveFromStart(TEXT("/"));
  return GetOutputDirectory() / PackagePath;
}

bool FBlueprintExporterLite::ExportGraph(UEdGraph *Graph, const FString &OutputDir,
                                          const FString &FileName) {
  if (!Graph) {
    return false;
  }

  FStringOutputDevice Archive;
  const FExportObjectInnerContext Context;

  // Export header comment
  Archive.Logf(TEXT("// Graph: %s\n"), *Graph->GetName());
  Archive.Logf(TEXT("// Type: %s\n"), *Graph->GetClass()->GetName());
  Archive.Logf(TEXT("// Node Count: %d\n"), Graph->Nodes.Num());
  Archive.Logf(TEXT("// Exported: %s\n"), *FDateTime::Now().ToString(TEXT("%Y.%m.%d-%H.%M.%S")));
  Archive.Logf(TEXT("\n"));

  // Export each node in T3D format
  for (UEdGraphNode *Node : Graph->Nodes) {
    UExporter::ExportToOutputDevice(&Context, Node, nullptr, Archive, TEXT("copy"), 0,
                                    PPF_ExportsNotFullyQualified | PPF_Copy | PPF_Delimited);
  }

  FString OutputPath = OutputDir / FileName;
  return FFileHelper::SaveStringToFile(
      Archive, *OutputPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

bool FBlueprintExporterLite::GenerateMetadata(UBlueprint *Blueprint,
                                               const FString &OutputDir) {
  return FMetadataGeneratorLite::GenerateMetadataForBlueprint(Blueprint, OutputDir);
}

FString FBlueprintExporterLite::SanitizeFileName(const FString &Name) {
  FString SafeName = Name;
  SafeName.ReplaceInline(TEXT("/"), TEXT("_"));
  SafeName.ReplaceInline(TEXT("\\"), TEXT("_"));
  SafeName.ReplaceInline(TEXT(":"), TEXT("_"));
  SafeName.ReplaceInline(TEXT("*"), TEXT("_"));
  SafeName.ReplaceInline(TEXT("?"), TEXT("_"));
  SafeName.ReplaceInline(TEXT("\""), TEXT("_"));
  SafeName.ReplaceInline(TEXT("<"), TEXT("_"));
  SafeName.ReplaceInline(TEXT(">"), TEXT("_"));
  SafeName.ReplaceInline(TEXT("|"), TEXT("_"));
  return SafeName;
}
