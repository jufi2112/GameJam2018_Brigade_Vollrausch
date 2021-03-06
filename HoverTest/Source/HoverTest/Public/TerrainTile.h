// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyStaticLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainGenerator.h"
#include "TerrainTile.generated.h"

class URuntimeMeshComponent;
class AProceduralCheckpoint;

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

	/**
	 * pointer to the checkpoint contained in this tile
	 */
	UPROPERTY()
	AProceduralCheckpoint* Checkpoint = nullptr;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SetCheckpointReference(AProceduralCheckpoint* CheckpointToSet);

	UFUNCTION()
	AProceduralCheckpoint* GetCheckpointReference() const;

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
	void UpdateMeshData(FTerrainSettings TerrainSettings, TArray<FMeshData>& MeshData);

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

	UFUNCTION()
	void SetVerticesLeftBorder(const TArray<FBorderVertex>& Vertices);

	UFUNCTION()
	void SetVerticesRightBorder(const TArray<FBorderVertex>& Vertices);

	UFUNCTION()
	void SetVerticesTopBorder(const TArray<FBorderVertex>& Vertices);

	UFUNCTION()
	void SetVerticesBottomBorder(const TArray<FBorderVertex>& Vertices);

	UFUNCTION()
	void GetVerticesLeftBorder(TArray<FBorderVertex>& OUTVertices) const;

	UFUNCTION()
	void GetVerticesRightBorder(TArray<FBorderVertex>& OUTVertices) const;

	UFUNCTION()
	void GetVerticesTopBorder(TArray<FBorderVertex>& OUTVertices) const;

	UFUNCTION()
	void GetVerticesBottomBorder(TArray<FBorderVertex>& OUTVertices) const;

	UFUNCTION()
	float GetVerticesLeftBorderNum() const;

	UFUNCTION()
	float GetVerticesRightBorderNum() const;

	UFUNCTION()
	float GetVerticesTopBorderNum() const;

	UFUNCTION()
	float GetVerticesBottomBorderNum() const;

	/**
	 * to be called after all vertices on border data have been set
	 */
	UFUNCTION()
	void AllVerticesOnBorderSet();

	UFUNCTION()
	bool GetVerticesOnBorderSet() const;

	UFUNCTION()
	FVector GetBottomLeftCorner() const;

	UFUNCTION()
	FVector GetBottomRightCorner() const;

	UFUNCTION()
	FVector GetTopRightCorner() const;

	UFUNCTION()
	FVector GetTopLeftCorner() const;

	UFUNCTION()
	void SetBottomLeftCorner(const FVector Vertex);

	UFUNCTION()
	void SetBottomRightCorner(const FVector Vertex);

	UFUNCTION()
	void SetTopRightCorner(const FVector Vertex);

	UFUNCTION()
	void SetTopLeftCorner(const FVector Vertex);

private:

	// component that is responsible for rendering the terrain
	UPROPERTY()
	URuntimeMeshComponent* RuntimeMesh = nullptr;


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

	UPROPERTY()
	TArray<int32> MeshSectionsCreated;

	// all vertices on the left border of the tile
	UPROPERTY()
	TArray<FBorderVertex> VerticesLeftBorder;

	// all vertices on the top border of the tile
	UPROPERTY()
	TArray<FBorderVertex> VerticesTopBorder;

	// all vertices on the right border of the tile
	UPROPERTY()
	TArray<FBorderVertex> VerticesRightBorder;

	// all vertices on the bottom border of the tile
	UPROPERTY()
	TArray<FBorderVertex> VerticesBottomBorder;

	// are all vertices on border data set?
	bool bVerticesOnBorderSet = false;

	// corner vertices of the terrain mesh
	UPROPERTY()
	FVector BottomLeftCorner;

	// corner vertices of the terrain mesh
	UPROPERTY()
	FVector BottomRightCorner;

	// corner vertices of the terrain mesh
	UPROPERTY()
	FVector TopRightCorner;

	// corner vertices of the terrain mesh
	UPROPERTY()
	FVector TopLeftCorner;
};
