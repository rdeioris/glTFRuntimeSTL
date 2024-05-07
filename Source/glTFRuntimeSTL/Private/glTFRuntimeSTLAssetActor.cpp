// Copyright 2023, Roberto De Ioris.

#include "glTFRuntimeSTLAssetActor.h"
#include "glTFRuntimeSTLFunctionLibrary.h"

// Sets default values
AglTFRuntimeSTLAssetActor::AglTFRuntimeSTLAssetActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AssetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("AssetRoot"));
	RootComponent = AssetRoot;
}

// Called when the game starts or when spawned
void AglTFRuntimeSTLAssetActor::BeginPlay()
{
	Super::BeginPlay();

	if (!Asset)
	{
		return;
	}

	FglTFRuntimeMeshLOD LOD;
	if (UglTFRuntimeSTLFunctionLibrary::LoadSTLAsRuntimeLOD(Asset, nullptr, EglTFRuntimeSTLFileMode::Auto, LOD))
	{
		UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this);
		StaticMeshComponent->SetupAttachment(GetRootComponent());
		StaticMeshComponent->RegisterComponent();
		AddInstanceComponent(StaticMeshComponent);

		StaticMeshComponent->ComponentTags.Add(TEXT("glTFRuntime:NodeName:solid"));
		StaticMeshComponent->ComponentTags.Add(TEXT("glTFRuntime:Format:STL"));

		UStaticMesh* StaticMesh = Asset->LoadStaticMeshFromRuntimeLODs({ LOD }, StaticMeshConfig);
		if (StaticMesh)
		{
			StaticMeshComponent->SetStaticMesh(StaticMesh);
		}

		ReceiveOnStaticMeshComponentCreated(StaticMeshComponent);
	}

}

// Called every frame
void AglTFRuntimeSTLAssetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AglTFRuntimeSTLAssetActor::ReceiveOnStaticMeshComponentCreated_Implementation(UStaticMeshComponent* StaticMeshComponent)
{
}