// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AssetRegistry/AssetData.h"
#include "CoreMinimal.h"

class UBlueprint;
class UEdGraph;

/**
 * Lite version exporter - single Blueprint only, no recursive dependencies
 */
class CONTEXTCORELITE_API FBlueprintExporterLite {
public:
  /** Export selected Blueprints (no dependency recursion) */
  static void ExportSelectedAssets(const TArray<FAssetData> &SelectedAssets);

  /** Export a single Blueprint */
  static bool ExportBlueprint(UBlueprint *Blueprint);

private:
  static FString GetOutputDirectory();
  static FString GetBlueprintOutputPath(UBlueprint *Blueprint);
  static bool ExportGraph(UEdGraph *Graph, const FString &OutputDir, const FString &FileName);
  static bool GenerateMetadata(UBlueprint *Blueprint, const FString &OutputDir);
  static FString SanitizeFileName(const FString &Name);
};
