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

	MeshSectionsCreated.Empty();

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
		RuntimeMesh = NewObject<URuntimeMeshComponent>(this, URuntimeMeshComponent::StaticClass(), FName("Runtime Mesh"));
		RuntimeMesh->SetRelativeTransform(FTransform());
		RuntimeMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		RuntimeMesh->BodyInstance.SetResponseToAllChannels(ECollisionResponse::ECR_Block);
		RuntimeMesh->SetCollisionUseAsyncCooking(TerrainSettings.bUseAsyncCollisionCooking);
		RuntimeMesh->RegisterComponent();
		RuntimeMesh->SetVisibility(false);

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
	SetActorLocation(FVector((TerrainSettings.TileSizeXUnits -1) * TerrainSettings.UnitTileSize * Sector.X, (TerrainSettings.TileSizeYUnits -1) * TerrainSettings.UnitTileSize * Sector.Y, 0));
	CurrentSector = Sector;
	SetActorHiddenInGame(true);
}

void ATerrainTile::UpdateMeshData(FTerrainSettings TerrainSettings, TArray<FMeshData>& MeshData)
{
	if (RuntimeMesh == nullptr) { return; }
	if (!bIsInitialized || TileStatus == ETileStatus::TILE_UNDEFINED)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to call UpdateMeshData on %s before calling SetupTile"), *GetName());
		return;
	}

	if (TileStatus == ETileStatus::TILE_INITIALIZED || TileStatus == ETileStatus::TILE_TRANSITION)
	{
		// tile is initialized, but runtime mesh sections do not exist
		for (int32 i = 0; i < MeshData.Num(); ++i)
		{
			if (MeshData[i].VertexBuffer.Num() != 0)
			{
				RuntimeMesh->CreateMeshSection(i, MeshData[i].VertexBuffer, MeshData[i].TriangleBuffer, true, EUpdateFrequency::Infrequent, ESectionUpdateFlags::None);
				MeshSectionsCreated.Add(i);
			}
		}
		TileStatus = ETileStatus::TILE_FINISHED;
	}

	// runtime mesh sections exist, so only update them
	else if (TileStatus == ETileStatus::TILE_FINISHED)
	{
		for (int32 i = 0; i < MeshData.Num(); ++i)
		{
			if (MeshSectionsCreated.Find(i) == INDEX_NONE) { continue; }
			if (MeshData[i].VertexBuffer.Num() != 0)
			{
				RuntimeMesh->UpdateMeshSection(i, MeshData[i].VertexBuffer, MeshData[i].TriangleBuffer, ESectionUpdateFlags::None);
			}
		}

		TileStatus = ETileStatus::TILE_FINISHED;
	}
	else { return; }

	// apply materials
	for (int32 i = 0; i < MeshData.Num(); ++i)
	{
		if (MeshSectionsCreated.Find(i) == INDEX_NONE) { continue; }
		if (MeshData[i].VertexBuffer.Num() != 0)
		{
			if (TerrainSettings.Materials.IsValidIndex(i))
			{
				RuntimeMesh->SetMaterial(i, TerrainSettings.Materials[i]);
			}
		}
	}

	RuntimeMesh->SetVisibility(true);
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
		while (MeshSectionsCreated.Num() > 0)
		{
			RuntimeMesh->ClearMeshSection(MeshSectionsCreated.Pop());
		}
		/*TerrainMesh->ClearMeshSection(0);
		TrackMesh->ClearMeshSection(0);*/
	}
	// better safe than sorry
	MeshSectionsCreated.Empty();

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

