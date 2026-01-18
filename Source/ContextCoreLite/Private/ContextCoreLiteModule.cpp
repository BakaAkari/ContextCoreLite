// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContextCoreLiteModule.h"
#include "Animation/AnimBlueprint.h"
#include "BlueprintExporterLite.h"
#include "ContentBrowserDelegates.h"
#include "ContentBrowserModule.h"
#include "Engine/Blueprint.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FContextCoreLiteModule"

void FContextCoreLiteModule::StartupModule() {
  UToolMenus::RegisterStartupCallback(
      FSimpleMulticastDelegate::FDelegate::CreateRaw(
          this, &FContextCoreLiteModule::RegisterContextMenuExtension));
  
  UE_LOG(LogTemp, Log, TEXT("[ContextCore Lite] Trial version loaded. Upgrade to full version for recursive exports and auto-update."));
}

void FContextCoreLiteModule::ShutdownModule() {
  UnregisterContextMenuExtension();
  UToolMenus::UnRegisterStartupCallback(this);
}

void FContextCoreLiteModule::RegisterContextMenuExtension() {
  FContentBrowserModule &ContentBrowserModule =
      FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

  TArray<FContentBrowserMenuExtender_SelectedAssets> &CBMenuExtenderDelegates =
      ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

  CBMenuExtenderDelegates.Add(
      FContentBrowserMenuExtender_SelectedAssets::CreateLambda(
          [](const TArray<FAssetData> &SelectedAssets) {
            TSharedRef<FExtender> Extender = MakeShared<FExtender>();

            // Check if any selected asset is a Blueprint
            bool bHasBlueprint = false;
            for (const FAssetData &Asset : SelectedAssets) {
              UClass *Class = Asset.GetClass();
              if (Class->IsChildOf(UBlueprint::StaticClass())) {
                bHasBlueprint = true;
                break;
              }
            }

            if (bHasBlueprint) {
              Extender->AddMenuExtension(
                  "GetAssetActions", EExtensionHook::After, nullptr,
                  FMenuExtensionDelegate::CreateLambda([SelectedAssets](
                                                           FMenuBuilder &MenuBuilder) {
                    MenuBuilder.AddMenuEntry(
                        LOCTEXT("ExportToContextLite", "Export to Context (Lite)"),
                        LOCTEXT("ExportToContextLiteTooltip",
                                "Export selected Blueprint to AI-readable format.\n"
                                "[Trial Version] Single asset only, no dependency export."),
                        FSlateIcon(),
                        FUIAction(FExecuteAction::CreateLambda(
                            [SelectedAssets]() {
                              FBlueprintExporterLite::ExportSelectedAssets(SelectedAssets);
                            })));
                  }));
            }

            return Extender;
          }));

  ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
}

void FContextCoreLiteModule::UnregisterContextMenuExtension() {
  if (ContentBrowserExtenderDelegateHandle.IsValid()) {
    FContentBrowserModule &ContentBrowserModule =
        FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    TArray<FContentBrowserMenuExtender_SelectedAssets> &CBMenuExtenderDelegates =
        ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
    CBMenuExtenderDelegates.RemoveAll(
        [this](const FContentBrowserMenuExtender_SelectedAssets &Delegate) {
          return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle;
        });
  }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FContextCoreLiteModule, ContextCoreLite)
