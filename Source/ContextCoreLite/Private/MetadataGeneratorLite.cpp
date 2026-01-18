// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetadataGeneratorLite.h"
#include "AnimGraphNode_StateMachine.h"
#include "Animation/AnimBlueprint.h"
#include "Components/ActorComponent.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "K2Node_Event.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/FileHelper.h"

bool FMetadataGeneratorLite::GenerateMetadataForBlueprint(
    UBlueprint *Blueprint, const FString &OutputDir) {
  if (!Blueprint) {
    return false;
  }

  TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();

  // Basic info
  RootObject->SetStringField(TEXT("name"), Blueprint->GetName());
  RootObject->SetStringField(TEXT("path"), Blueprint->GetOutermost()->GetName());

  // Type
  FString BlueprintType = TEXT("Blueprint");
  if (Blueprint->BlueprintType == BPTYPE_Interface) {
    BlueprintType = TEXT("BlueprintInterface");
  } else if (Blueprint->IsA<UAnimBlueprint>()) {
    BlueprintType = TEXT("AnimBlueprint");
  }
  RootObject->SetStringField(TEXT("type"), BlueprintType);

  // Parent class
  if (Blueprint->ParentClass) {
    RootObject->SetStringField(TEXT("parent"), Blueprint->ParentClass->GetName());
  }

  // C++ class chain (simplified - just names, no source paths)
  TArray<TSharedPtr<FJsonValue>> CppChainArray;
  if (Blueprint->ParentClass) {
    UClass *CurrentClass = Blueprint->ParentClass;
    while (CurrentClass) {
      if (!CurrentClass->ClassGeneratedBy) {
        TSharedPtr<FJsonObject> ClassObj = MakeShared<FJsonObject>();
        ClassObj->SetStringField(TEXT("name"), CurrentClass->GetName());
        CppChainArray.Add(MakeShared<FJsonValueObject>(ClassObj));
      }
      CurrentClass = CurrentClass->GetSuperClass();
    }
  }
  RootObject->SetArrayField(TEXT("cpp_chain"), CppChainArray);

  // Class settings
  TSharedPtr<FJsonObject> ClassSettings = GenerateClassSettingsJson(Blueprint);
  if (ClassSettings.IsValid()) {
    RootObject->SetObjectField(TEXT("class_settings"), ClassSettings);
  }

  // Interfaces
  TArray<TSharedPtr<FJsonValue>> Interfaces = GenerateInterfacesJson(Blueprint);
  RootObject->SetArrayField(TEXT("interfaces"), Interfaces);

  // Variables
  TSharedPtr<FJsonObject> Variables = GenerateVariablesJson(Blueprint);
  if (Variables.IsValid()) {
    RootObject->SetField(TEXT("variables"), MakeShared<FJsonValueObject>(Variables));
  }

  // Components
  TSharedPtr<FJsonObject> Components = GenerateComponentsJson(Blueprint);
  if (Components.IsValid()) {
    RootObject->SetField(TEXT("components"), MakeShared<FJsonValueObject>(Components));
  }

  // Functions
  TArray<TSharedPtr<FJsonValue>> Functions = GenerateFunctionsJson(Blueprint);
  RootObject->SetArrayField(TEXT("functions"), Functions);

  // Event Dispatchers
  TArray<TSharedPtr<FJsonValue>> Dispatchers = GenerateEventDispatchersJson(Blueprint);
  RootObject->SetArrayField(TEXT("event_dispatchers"), Dispatchers);

  // Events list
  TArray<TSharedPtr<FJsonValue>> Events;
  for (UEdGraph *Graph : Blueprint->UbergraphPages) {
    for (UEdGraphNode *Node : Graph->Nodes) {
      if (UK2Node_Event *EventNode = Cast<UK2Node_Event>(Node)) {
        Events.Add(MakeShared<FJsonValueString>(
            EventNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString()));
      }
    }
  }
  RootObject->SetArrayField(TEXT("events"), Events);

  // Graphs list
  TSharedRef<FJsonObject> Graphs = MakeShared<FJsonObject>();
  for (UEdGraph *Graph : Blueprint->UbergraphPages) {
    Graphs->SetStringField(Graph->GetName(), Graph->GetName() + TEXT(".txt"));
  }
  for (UEdGraph *Graph : Blueprint->FunctionGraphs) {
    Graphs->SetStringField(Graph->GetName(), TEXT("Function_") + Graph->GetName() + TEXT(".txt"));
  }
  RootObject->SetObjectField(TEXT("graphs"), Graphs);

  // AnimBlueprint specific
  if (UAnimBlueprint *AnimBlueprint = Cast<UAnimBlueprint>(Blueprint)) {
    GenerateAnimBlueprintMetadata(AnimBlueprint, RootObject);
  }

  // Note: NO dependencies in Lite version

  // Write to file
  FString OutputString;
  TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
      TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutputString);
  FJsonSerializer::Serialize(RootObject, Writer);

  FString MetaPath = OutputDir / TEXT("_meta.json");
  return FFileHelper::SaveStringToFile(
      OutputString, *MetaPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

TSharedPtr<FJsonObject> FMetadataGeneratorLite::GenerateVariablesJson(UBlueprint *Blueprint) {
  TSharedRef<FJsonObject> VarsObject = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> VarsArray;

  for (const FBPVariableDescription &Var : Blueprint->NewVariables) {
    TSharedRef<FJsonObject> VarObj = MakeShared<FJsonObject>();

    VarObj->SetStringField(TEXT("name"), Var.VarName.ToString());
    VarObj->SetStringField(TEXT("type"), PinTypeToString(Var.VarType));
    VarObj->SetStringField(TEXT("category"), Var.Category.ToString());

    if (Var.PropertyFlags & CPF_Net) {
      VarObj->SetBoolField(TEXT("replicated"), true);
    }
    if (Var.PropertyFlags & CPF_RepNotify) {
      VarObj->SetBoolField(TEXT("rep_notify"), true);
    }

    VarObj->SetBoolField(TEXT("instance_editable"), (Var.PropertyFlags & CPF_Edit) != 0);
    VarObj->SetBoolField(TEXT("blueprint_read_only"), (Var.PropertyFlags & CPF_BlueprintReadOnly) != 0);
    VarObj->SetBoolField(TEXT("expose_on_spawn"), (Var.PropertyFlags & CPF_ExposeOnSpawn) != 0);
    VarObj->SetBoolField(TEXT("private"), (Var.PropertyFlags & CPF_DisableEditOnInstance) != 0);

    if (!Var.DefaultValue.IsEmpty()) {
      VarObj->SetStringField(TEXT("default"), Var.DefaultValue);
    }

    if (Var.HasMetaData(FBlueprintMetadata::MD_Tooltip)) {
      VarObj->SetStringField(TEXT("tooltip"), Var.GetMetaData(FBlueprintMetadata::MD_Tooltip));
    }

    VarsArray.Add(MakeShared<FJsonValueObject>(VarObj));
  }

  VarsObject->SetArrayField(TEXT("list"), VarsArray);
  return VarsObject;
}

TSharedPtr<FJsonObject> FMetadataGeneratorLite::GenerateComponentsJson(UBlueprint *Blueprint) {
  TSharedRef<FJsonObject> ComponentsObject = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> ComponentsArray;

  if (Blueprint->SimpleConstructionScript) {
    TArray<USCS_Node *> AllNodes = Blueprint->SimpleConstructionScript->GetAllNodes();

    for (USCS_Node *Node : AllNodes) {
      if (Node && Node->ComponentTemplate) {
        TSharedRef<FJsonObject> CompObj = MakeShared<FJsonObject>();

        CompObj->SetStringField(TEXT("name"), Node->GetVariableName().ToString());
        CompObj->SetStringField(TEXT("type"), Node->ComponentTemplate->GetClass()->GetName());

        if (Node->ParentComponentOrVariableName != NAME_None) {
          CompObj->SetStringField(TEXT("parent"), Node->ParentComponentOrVariableName.ToString());
        } else {
          CompObj->SetStringField(TEXT("parent"), TEXT("root"));
        }

        ComponentsArray.Add(MakeShared<FJsonValueObject>(CompObj));
      }
    }
  }

  ComponentsObject->SetArrayField(TEXT("list"), ComponentsArray);
  return ComponentsObject;
}

TArray<TSharedPtr<FJsonValue>> FMetadataGeneratorLite::GenerateFunctionsJson(UBlueprint *Blueprint) {
  TArray<TSharedPtr<FJsonValue>> FunctionsArray;

  for (UEdGraph *Graph : Blueprint->FunctionGraphs) {
    TSharedRef<FJsonObject> FuncObj = MakeShared<FJsonObject>();

    FuncObj->SetStringField(TEXT("name"), Graph->GetName());
    FuncObj->SetStringField(TEXT("file"), TEXT("Function_") + Graph->GetName() + TEXT(".txt"));

    for (UEdGraphNode *Node : Graph->Nodes) {
      if (UK2Node_FunctionEntry *EntryNode = Cast<UK2Node_FunctionEntry>(Node)) {
        FString Access = TEXT("Public");
        if (EntryNode->GetFunctionFlags() & FUNC_Protected) {
          Access = TEXT("Protected");
        } else if (EntryNode->GetFunctionFlags() & FUNC_Private) {
          Access = TEXT("Private");
        }
        FuncObj->SetStringField(TEXT("access"), Access);
        FuncObj->SetBoolField(TEXT("pure"), (EntryNode->GetFunctionFlags() & FUNC_BlueprintPure) != 0);
        FuncObj->SetBoolField(TEXT("const"), (EntryNode->GetFunctionFlags() & FUNC_Const) != 0);

        TArray<TSharedPtr<FJsonValue>> Inputs;
        for (UEdGraphPin *Pin : EntryNode->Pins) {
          if (Pin->Direction == EGPD_Output && !Pin->PinType.PinCategory.IsNone() &&
              Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec) {
            TSharedRef<FJsonObject> InputObj = MakeShared<FJsonObject>();
            InputObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
            InputObj->SetStringField(TEXT("type"), PinTypeToString(Pin->PinType));
            Inputs.Add(MakeShared<FJsonValueObject>(InputObj));
          }
        }
        FuncObj->SetArrayField(TEXT("inputs"), Inputs);
        break;
      }
    }

    TArray<TSharedPtr<FJsonValue>> Outputs;
    for (UEdGraphNode *Node : Graph->Nodes) {
      if (UK2Node_FunctionResult *ResultNode = Cast<UK2Node_FunctionResult>(Node)) {
        for (UEdGraphPin *Pin : ResultNode->Pins) {
          if (Pin->Direction == EGPD_Input && !Pin->PinType.PinCategory.IsNone() &&
              Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec) {
            TSharedRef<FJsonObject> OutputObj = MakeShared<FJsonObject>();
            OutputObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
            OutputObj->SetStringField(TEXT("type"), PinTypeToString(Pin->PinType));
            Outputs.Add(MakeShared<FJsonValueObject>(OutputObj));
          }
        }
        break;
      }
    }
    FuncObj->SetArrayField(TEXT("outputs"), Outputs);

    FunctionsArray.Add(MakeShared<FJsonValueObject>(FuncObj));
  }

  return FunctionsArray;
}

TArray<TSharedPtr<FJsonValue>> FMetadataGeneratorLite::GenerateInterfacesJson(UBlueprint *Blueprint) {
  TArray<TSharedPtr<FJsonValue>> InterfacesArray;

  for (const FBPInterfaceDescription &Interface : Blueprint->ImplementedInterfaces) {
    if (Interface.Interface) {
      TSharedRef<FJsonObject> InterfaceObj = MakeShared<FJsonObject>();
      InterfaceObj->SetStringField(TEXT("name"), Interface.Interface->GetName());
      InterfacesArray.Add(MakeShared<FJsonValueObject>(InterfaceObj));
    }
  }

  return InterfacesArray;
}

TArray<TSharedPtr<FJsonValue>> FMetadataGeneratorLite::GenerateEventDispatchersJson(UBlueprint *Blueprint) {
  TArray<TSharedPtr<FJsonValue>> DispatchersArray;

  for (UEdGraph *Graph : Blueprint->DelegateSignatureGraphs) {
    TSharedRef<FJsonObject> DispObj = MakeShared<FJsonObject>();
    DispObj->SetStringField(TEXT("name"), Graph->GetName());

    TArray<TSharedPtr<FJsonValue>> Params;
    for (UEdGraphNode *Node : Graph->Nodes) {
      if (UK2Node_FunctionEntry *EntryNode = Cast<UK2Node_FunctionEntry>(Node)) {
        for (UEdGraphPin *Pin : EntryNode->Pins) {
          if (Pin->Direction == EGPD_Output && !Pin->PinType.PinCategory.IsNone() &&
              Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec) {
            TSharedRef<FJsonObject> ParamObj = MakeShared<FJsonObject>();
            ParamObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
            ParamObj->SetStringField(TEXT("type"), PinTypeToString(Pin->PinType));
            Params.Add(MakeShared<FJsonValueObject>(ParamObj));
          }
        }
        break;
      }
    }
    DispObj->SetArrayField(TEXT("params"), Params);

    DispatchersArray.Add(MakeShared<FJsonValueObject>(DispObj));
  }

  return DispatchersArray;
}

TSharedPtr<FJsonObject> FMetadataGeneratorLite::GenerateClassSettingsJson(UBlueprint *Blueprint) {
  TSharedRef<FJsonObject> SettingsObj = MakeShared<FJsonObject>();

  UClass *GeneratedClass = Blueprint->GeneratedClass;
  if (!GeneratedClass) {
    return SettingsObj;
  }

  UObject *CDO = GeneratedClass->GetDefaultObject();
  if (!CDO) {
    return SettingsObj;
  }

  if (AActor *ActorCDO = Cast<AActor>(CDO)) {
    SettingsObj->SetBoolField(TEXT("replicates"), ActorCDO->GetIsReplicated());
    SettingsObj->SetBoolField(TEXT("always_relevant"), ActorCDO->bAlwaysRelevant);
    SettingsObj->SetBoolField(TEXT("net_load_on_client"), ActorCDO->bNetLoadOnClient);
  }

  return SettingsObj;
}

void FMetadataGeneratorLite::GenerateAnimBlueprintMetadata(
    UAnimBlueprint *AnimBlueprint, TSharedRef<FJsonObject> &RootObject) {
  if (!AnimBlueprint) {
    return;
  }

  if (AnimBlueprint->TargetSkeleton) {
    RootObject->SetStringField(TEXT("skeleton"), AnimBlueprint->TargetSkeleton->GetName());
  }

  TArray<TSharedPtr<FJsonValue>> StateMachinesArray;
  for (UEdGraph *Graph : AnimBlueprint->FunctionGraphs) {
    if (Graph && Graph->GetFName().ToString().Contains(TEXT("AnimGraph"))) {
      for (UEdGraphNode *Node : Graph->Nodes) {
        if (UAnimGraphNode_StateMachine *SMNode = Cast<UAnimGraphNode_StateMachine>(Node)) {
          TSharedRef<FJsonObject> SMObj = MakeShared<FJsonObject>();
          FString SMName = SMNode->GetNodeTitle(ENodeTitleType::ListView).ToString();
          
          FString SanitizedName = SMName;
          SanitizedName.ReplaceInline(TEXT("/"), TEXT("_"));
          SanitizedName.ReplaceInline(TEXT("\\"), TEXT("_"));
          SanitizedName.ReplaceInline(TEXT(":"), TEXT("_"));

          SMObj->SetStringField(TEXT("name"), SMName);
          SMObj->SetStringField(TEXT("file"),
              FString::Printf(TEXT("StateMachine_%s.txt"), *SanitizedName));
          StateMachinesArray.Add(MakeShared<FJsonValueObject>(SMObj));
        }
      }
    }
  }
  RootObject->SetArrayField(TEXT("state_machines"), StateMachinesArray);
}

FString FMetadataGeneratorLite::PinTypeToString(const FEdGraphPinType &PinType) {
  FString TypeName = PinType.PinCategory.ToString();

  if (PinType.PinSubCategoryObject.IsValid()) {
    TypeName = PinType.PinSubCategoryObject->GetName();
  }

  if (PinType.PinCategory == UEdGraphSchema_K2::PC_Object ||
      PinType.PinCategory == UEdGraphSchema_K2::PC_Class) {
    TypeName += TEXT("*");
  }

  if (PinType.IsArray()) {
    TypeName = TEXT("TArray<") + TypeName + TEXT(">");
  }

  if (PinType.bIsReference) {
    TypeName += TEXT("&");
  }

  return TypeName;
}
