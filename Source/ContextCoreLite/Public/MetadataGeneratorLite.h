// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UBlueprint;
class UAnimBlueprint;

/**
 * Generates _meta.json for Blueprints (Lite version)
 */
class CONTEXTCORELITE_API FMetadataGeneratorLite {
public:
  static bool GenerateMetadataForBlueprint(UBlueprint *Blueprint, const FString &OutputDir);

private:
  static TSharedPtr<FJsonObject> GenerateVariablesJson(UBlueprint *Blueprint);
  static TSharedPtr<FJsonObject> GenerateComponentsJson(UBlueprint *Blueprint);
  static TArray<TSharedPtr<FJsonValue>> GenerateFunctionsJson(UBlueprint *Blueprint);
  static TArray<TSharedPtr<FJsonValue>> GenerateInterfacesJson(UBlueprint *Blueprint);
  static TArray<TSharedPtr<FJsonValue>> GenerateEventDispatchersJson(UBlueprint *Blueprint);
  static TSharedPtr<FJsonObject> GenerateClassSettingsJson(UBlueprint *Blueprint);
  static void GenerateAnimBlueprintMetadata(UAnimBlueprint *AnimBlueprint, TSharedRef<FJsonObject> &RootObject);
  static FString PinTypeToString(const FEdGraphPinType &PinType);
};
