// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeSTLFunctionLibrary.generated.h"

UENUM()
enum class EglTFRuntimeSTLFileMode : uint8
{
	Auto,
	ASCII,
	Binary
};

/**
 * 
 */
UCLASS()
class GLTFRUNTIMESTL_API UglTFRuntimeSTLFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|STL")
	static bool LoadSTLAsRuntimeLOD(UglTFRuntimeAsset* Asset, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode, FglTFRuntimeMeshLOD& RuntimeLOD);

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|STL")
	static bool LoadSTLFromArchiveAsRuntimeLOD(UglTFRuntimeAsset* Asset, const FString& Name, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode, FglTFRuntimeMeshLOD& RuntimeLOD);

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|STL")
	static void LoadSTLAsRuntimeLODAsync(UglTFRuntimeAsset* Asset, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode, const FglTFRuntimeMeshLODAsync& AsyncCallback);

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|STL")
	static void LoadSTLFromArchiveAsRuntimeLODAsync(UglTFRuntimeAsset* Asset, const FString& Name, UMaterialInterface* Material, EglTFRuntimeSTLFileMode STLFileMode, const FglTFRuntimeMeshLODAsync& AsyncCallback);
};
