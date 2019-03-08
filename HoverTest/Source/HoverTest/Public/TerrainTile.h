// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyStaticLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainTile.generated.h"

class URuntimeMeshComponent;

UCLASS()
class HOVERTEST_API ATerrainTile : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ATerrainTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// to be called only once directly after terrain tile is created
	UFUNCTION()
		void SetupTile(FTerrainSettings TerrainSettings, FIntVector2D Sector);

	/**
	* to be called when the position of a tile is changed
	* hides the actor ingame
	*/
	UFUNCTION()
		void UpdateTilePosition(FTerrainSettings TerrainSettings, FIntVector2D Sector);

	/**
	* called when mesh data should be updated
	* unhides the actor
	*/
	UFUNCTION()
		void UpdateMeshData(FMeshData& TerrainMeshData, FMeshData& TrackMeshData);

	UFUNCTION(BlueprintCallable)
		FIntVector2D GetCurrentSector() const;

	UFUNCTION(BlueprintCallable)
		void AddAssociatedActor();

	UFUNCTION(BlueprintCallable)
		int32 RemoveAssociatedActor();

	/**
	* manages freeing the tile
	* hides the tile ingame
	*/
	UFUNCTION(BlueprintCallable)
		void FreeTile();

	// returns time in seconds since the tile was freed
	UFUNCTION(BlueprintCallable)
		float GetTimeSinceTileFreed() const;

	UFUNCTION(BlueprintCallable)
		ETileStatus GetTileStatus() const;

private:

	// component that is responsible for rendering the terrain
	UPROPERTY()
		URuntimeMeshComponent* TerrainMesh = nullptr;

	// component that is responsible for rendering the track
	UPROPERTY()
		URuntimeMeshComponent* TrackMesh = nullptr;

	// represents the state of the tile
	UPROPERTY()
		ETileStatus TileStatus = ETileStatus::TILE_UNDEFINED;

	// is the tile initialized
	bool bIsInitialized = false;

	/**
	* the sector the tile is currently located in
	* a sector has the same size as a tile but has a fixed position, whereas tiles can be moved around (i.e. tiles can be moved to different sectors)
	*/
	UPROPERTY()
		FIntVector2D CurrentSector;

	/**
	* number of actors that are associated with this tile (i.e. actors that this tile is relevant for)
	* if this number is zero, the tile can be freed
	*/
	UPROPERTY()
		int32 ActorsAssociatedWithThisTile = 0;

	UPROPERTY()
		float TimeSinceTileFreed = 0;

};
