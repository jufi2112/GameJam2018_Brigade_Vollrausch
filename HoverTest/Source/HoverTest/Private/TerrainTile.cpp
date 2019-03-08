// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainTile.h"
#include "RuntimeMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"

// Sets default values
ATerrainTile::ATerrainTile()
{
	// we don't want that every tile ticks, instead we iterate over all tiles in our terrain manager
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));


}

// Called when the game starts or when spawned
void ATerrainTile::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ATerrainTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATerrainTile::SetupTile(FTerrainSettings TerrainSettings, FIntVector2D Sector)
{
	// change tile location
	SetActorLocation(FVector(TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize * Sector.X, TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize * Sector.Y, 0.f));
	CurrentSector = Sector;
	// make sure newly created tile does not get destroyed immediately
	TimeSinceTileFreed = FPlatformTime::Seconds();
	if (!bIsInitialized)
	{
		TerrainMesh = NewObject<URuntimeMeshComponent>(this, URuntimeMeshComponent::StaticClass(), FName("Terrain Mesh"));
		TerrainMesh->SetRelativeTransform(FTransform());
		TerrainMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		TerrainMesh->BodyInstance.SetResponseToAllChannels(ECollisionResponse::ECR_Block);
		TerrainMesh->SetCollisionUseAsyncCooking(TerrainSettings.bUseAsyncCollisionCooking);
		TerrainMesh->RegisterComponent();
		TerrainMesh->SetVisibility(false);

		TrackMesh = NewObject<URuntimeMeshComponent>(this, URuntimeMeshComponent::StaticClass(), FName("Track Mesh"));
		TrackMesh->SetRelativeTransform(FTransform());
		TrackMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		TrackMesh->BodyInstance.SetResponseToAllChannels(ECollisionResponse::ECR_Block);
		TrackMesh->SetCollisionUseAsyncCooking(TerrainSettings.bUseAsyncCollisionCooking);
		TrackMesh->RegisterComponent();
		TrackMesh->SetVisibility(false);

		bIsInitialized = true;
	}
	SetActorHiddenInGame(true);
	// a tile may be created because an actor needs sector coverage, but at this moment the tile is not yet associated to this sector
	ActorsAssociatedWithThisTile = 0;
	TileStatus = ETileStatus::TILE_INITIALIZED;
}

void ATerrainTile::UpdateTilePosition(FTerrainSettings TerrainSettings, FIntVector2D Sector)
{
	if (!bIsInitialized || TileStatus == ETileStatus::TILE_UNDEFINED)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to call UpdateTilePosition on %s before calling SetupTile"), *GetName());
		return;
	}
	if (TileStatus == ETileStatus::TILE_FREE)
	{
		TileStatus = ETileStatus::TILE_TRANSITION;
	}
	SetActorLocation(FVector(TerrainSettings.TileSizeXUnits * TerrainSettings.UnitTileSize * Sector.X, TerrainSettings.TileSizeYUnits * TerrainSettings.UnitTileSize * Sector.Y, 0));
	CurrentSector = Sector;
	SetActorHiddenInGame(true);
}

void ATerrainTile::UpdateMeshData(FMeshData& TerrainMeshData, FMeshData& TrackMeshData)
{
	if (TerrainMesh == nullptr || TrackMesh == nullptr) { return; }
	if (!bIsInitialized || TileStatus == ETileStatus::TILE_UNDEFINED)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to call UpdateMeshData on %s before calling SetupTile"), *GetName());
		return;
	}

	if (TileStatus == ETileStatus::TILE_INITIALIZED || TileStatus == ETileStatus::TILE_TRANSITION)
	{
		// tile is initialized, but runtime mesh sections do not exist
		TerrainMesh->CreateMeshSection(0, TerrainMeshData.VertexBuffer, TerrainMeshData.TriangleBuffer, true, EUpdateFrequency::Infrequent, ESectionUpdateFlags::None);
		TrackMesh->CreateMeshSection(0, TrackMeshData.VertexBuffer, TrackMeshData.TriangleBuffer, true, EUpdateFrequency::Infrequent, ESectionUpdateFlags::None);
		TerrainMesh->RegisterComponent();
		TrackMesh->RegisterComponent();
		TileStatus = ETileStatus::TILE_FINISHED;
	}

	// runtime mesh sections exist, so only update them
	else if (TileStatus == ETileStatus::TILE_FINISHED)
	{
		TerrainMesh->UpdateMeshSection(0, TerrainMeshData.VertexBuffer, TerrainMeshData.TriangleBuffer, ESectionUpdateFlags::None);
		TrackMesh->UpdateMeshSection(0, TrackMeshData.VertexBuffer, TrackMeshData.TriangleBuffer, ESectionUpdateFlags::None);
		TileStatus = ETileStatus::TILE_FINISHED;
	}

	TerrainMesh->SetVisibility(true);
	TrackMesh->SetVisibility(true);
	SetActorHiddenInGame(false);
}

FIntVector2D ATerrainTile::GetCurrentSector() const
{
	return CurrentSector;
}

void ATerrainTile::AddAssociatedActor()
{
	ActorsAssociatedWithThisTile++;
}

int32 ATerrainTile::RemoveAssociatedActor()
{
	ActorsAssociatedWithThisTile--;
	return ActorsAssociatedWithThisTile;
}

void ATerrainTile::FreeTile()
{
	if (TileStatus == ETileStatus::TILE_FINISHED)
	{
		TerrainMesh->ClearMeshSection(0);
		TrackMesh->ClearMeshSection(0);
	}
	TileStatus = ETileStatus::TILE_FREE;
	SetActorHiddenInGame(true);
	// TODO check if visibility gets propagated to children
	ActorsAssociatedWithThisTile = 0;
	CurrentSector = FIntVector2D();
	SetActorLocation(FVector(0.f, 0.f, 0.f));
	TimeSinceTileFreed = GetWorld()->TimeSeconds;
}

float ATerrainTile::GetTimeSinceTileFreed() const
{
	return TimeSinceTileFreed;
}

ETileStatus ATerrainTile::GetTileStatus() const
{
	return TileStatus;
}

