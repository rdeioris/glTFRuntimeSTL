// Copyright 2023, Roberto De Ioris.

#include "glTFRuntimeSTLAssetActorAsync.h"
#include "glTFRuntimeSTLFunctionLibrary.h"

// Sets default values
AglTFRuntimeSTLAssetActorAsync::AglTFRuntimeSTLAssetActorAsync()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AssetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("AssetRoot"));
	RootComponent = AssetRoot;
}

// Called when the game starts or when spawned
void AglTFRuntimeSTLAssetActorAsync::BeginPlay()
{
	Super::BeginPlay();

	if (!Asset)
	{
		return;
	}

	UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this, MakeUniqueObjectName(this, UStaticMeshComponent::StaticClass()));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	StaticMeshComponent->RegisterComponent();
	AddInstanceComponent(StaticMeshComponent);
	MeshesToLoad.Add(StaticMeshComponent);

	if (MeshesToLoad.Num() == 0)
	{
		ReceiveOnScenesLoaded();
	}
	else
	{
		LoadNextMeshAsync();
	}
}

// Called every frame
void AglTFRuntimeSTLAssetActorAsync::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AglTFRuntimeSTLAssetActorAsync::ReceiveOnStaticMeshComponentCreated_Implementation(UStaticMeshComponent* StaticMeshComponent)
{

}

void AglTFRuntimeSTLAssetActorAsync::ReceiveOnScenesLoaded_Implementation()
{

}

void AglTFRuntimeSTLAssetActorAsync::LoadNextMeshAsync()
{
	if (MeshesToLoad.Num() == 0)
	{
		return;
	}

	auto It = MeshesToLoad.CreateIterator();
	if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(*It))
	{
		CurrentPrimitiveComponent = StaticMeshComponent;
		FglTFRuntimeMeshLODAsync Delegate;
		Delegate.BindDynamic(this, &AglTFRuntimeSTLAssetActorAsync::LoadStaticMeshAsync);

		UglTFRuntimeSTLFunctionLibrary::LoadSTLAsRuntimeLODAsync(Asset, nullptr, EglTFRuntimeSTLFileMode::Auto, Delegate);
	}
}

void AglTFRuntimeSTLAssetActorAsync::LoadStaticMeshAsync(const bool bValid, const FglTFRuntimeMeshLOD& RuntimeLOD)
{
	if (bValid)
	{
		UStaticMesh* StaticMesh = Asset->LoadStaticMeshFromRuntimeLODs({ RuntimeLOD }, StaticMeshConfig);
		if (StaticMesh)
		{
			CurrentPrimitiveComponent->SetStaticMesh(StaticMesh);
		}

		ReceiveOnStaticMeshComponentCreated(CurrentPrimitiveComponent);
	}

	MeshesToLoad.Remove(CurrentPrimitiveComponent);
	if (MeshesToLoad.Num() > 0)
	{
		LoadNextMeshAsync();
	}
	// trigger event
	else
	{
		ReceiveOnScenesLoaded();
	}
}