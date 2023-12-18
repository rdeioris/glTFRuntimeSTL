// Copyright 2023, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeSTLAssetActorAsync.generated.h"

UCLASS()
class GLTFRUNTIMESTL_API AglTFRuntimeSTLAssetActorAsync : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AglTFRuntimeSTLAssetActorAsync();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|STL")
	UglTFRuntimeAsset* Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|STL")
	FglTFRuntimeStaticMeshConfig StaticMeshConfig;

	UFUNCTION(BlueprintNativeEvent, Category = "glTFRuntime|STL", meta = (DisplayName = "On StaticMeshComponent Created"))
	void ReceiveOnStaticMeshComponentCreated(UStaticMeshComponent* StaticMeshComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "glTFRuntime|STL", meta = (DisplayName = "On Scenes Loaded"))
	void ReceiveOnScenesLoaded();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "glTFRuntime|STL")
	USceneComponent* AssetRoot;

	TArray<UStaticMeshComponent*> MeshesToLoad;

	void LoadNextMeshAsync();

	// this is safe to share between game and async threads because everything is sequential
	UStaticMeshComponent* CurrentPrimitiveComponent;

	UFUNCTION()
	void LoadStaticMeshAsync(const bool bValid, const FglTFRuntimeMeshLOD& RuntimeLOD);

};
