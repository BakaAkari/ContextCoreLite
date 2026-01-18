// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FContextCoreLiteModule : public IModuleInterface {
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

private:
  void RegisterContextMenuExtension();
  void UnregisterContextMenuExtension();
  
  FDelegateHandle ContentBrowserExtenderDelegateHandle;
};
