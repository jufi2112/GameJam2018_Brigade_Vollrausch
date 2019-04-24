// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyStaticLibrary.h"
#include "Runtime/Core/Public/Containers/Queue.h"
#include "TerrainManager.generated.h"

class ATerrainTile;

UCLASS()
class HOVERTEST_API ATerrainManager : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ATerrainManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// actors that are tracked by the terrain manager, i.e. actors around which terrain is generated
	UPROPERTY()
	TArray<AActor*> TrackedActors;

	// terrain tiles that are currently not used (free to use)
	UPROPERTY()
	TArray<ATerrainTile*> FreeTiles;

	// terrain tiles that are currently used (not free to use)
	UPROPERTY()
	TArray<ATerrainTile*> TilesInUse;

	/**
	* returns an array containing all sectors around the input location that should be covered with tiles according to TilesToBeCreatedAroundActorRadius in FTerrainSettings
	* the function does not check if sectors may already be covered by tiles
	*/
	UFUNCTION(BlueprintCallable)
	TArray<FIntVector2D> CalculateSectorsNeededAroundGivenLocation(FVector Location);

	/**
	* returns an array containing all sectors around the given sector that should be covered with tiles according to TilesToBeCreatedAroundActorRadius in FTerrainSettings
	* the function does not check if sectors may already be coverd by tiles
	* the sector provided to the function will also get included in the return array
	*/
	UFUNCTION(BlueprintCallable)
	TArray<FIntVector2D> CalculateSectorsNeededAroundGivenSector(FIntVector2D Sector);

	// array of queues, where each queue is an input queue for a thread
	TArray<TQueue<FTerrainJob, EQueueMode::Spsc>> TerrainCreationQueue;

	// queue for pending terrain jobs
	TQueue<FTerrainJob, EQueueMode::Spsc> PendingTerrainJobQueue;

	// array of all used threads
	TArray<FRunnableThread*> Threads;

	// the sector where the next track segment should be created (not yet created)
	UPROPERTY()
	FIntVector2D NextTrackSector = FIntVector2D();

	// the sector where the current last track segment is located (already created)
	UPROPERTY()
	FIntVector2D CurrentTrackSector = FIntVector2D();

	// hashmap that stores all already calculated track information for every processed sector
	//UPROPERTY()
	TMap<FIntVector2D, FSectorTrackInfo> TrackMap;

	/**
	 * array that contains all sectors for which a tile should be created
	 * gets filled by the function CreateAndInitializeTiles
	 */
	UPROPERTY()
	TArray<FIntVector2D> SectorsToCreateTileFor;

	/**
	 * calculates the global track path for all sectors in SectorsToCreateTileFor
	 */
	UFUNCTION()
	void CalculateTrackPath();

	/**
	 * calculates the track entry and exit points for the NextTrackSector
	 */
	UFUNCTION()
	void CalculateTrackPoints(FVector2D& OUTTrackEntryPoint, FVector2D& OUTTrackExitPoint);









private:

	/**
	 * calculates all neighboring sectors for the given sector
	 */
	UFUNCTION()
	void GetAdjacentSectors(const FIntVector2D Sector, TArray<FIntVector2D>& OUTAdjacentSectors);

	/**
	 * calculates all neighboring sectors for the given sectors that are relevant for border constraint calculation
	 */
	UFUNCTION()
	void GetRelevantAdjacentSectors(const FIntVector2D Sector, TArray<FIntVector2D>& OUTAdjacentSectors);





public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// settings for terrain generation
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FTerrainSettings TerrainSettings;

	// should the terrain manager start generating terrain as soon as an actor registers itself
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bGenerateTerrainOnActorRegister = true;

	/**
	 * to make it possible to begin terrain generation not just when an actor registers itself but also on an input event,
	 * tile creation and initialization is outsourced to an own function
	 * called by default from the AddActorToTrack() method, but can also be called from for example blueprint when setting bGenerateTerrainOnActorRegister to false
	 */
	UFUNCTION(BlueprintCallable)
	void CreateAndInitializeTiles(int32 NumberOfTilesToCreate = 9);

	// adds the specified actor to the list of actors that are tracked, i.e. that terrain is generated around
	UFUNCTION(BlueprintCallable)
	void AddActorToTrack(AActor* ActorToTrack);

	// removes the specified actor from the list of actors that are tracked, i.e. no terrain is generated around this actor anymore
	UFUNCTION(BlueprintCallable)
	void RemoveTrackedActor(AActor* ActorToRemove);

	// used to calculate the sector an entity with the given world location is in
	UFUNCTION(BlueprintCallable)
	FIntVector2D CalculateSectorFromLocation(FVector CurrentWorldLocation);

	virtual void BeginDestroy() override;

	/**
	* function called from a TerrainTrackerComponent when its actor changes sector
	* the function handles moving of associated tiles
	*/
	UFUNCTION(BlueprintCallable)
	void HandleTrackedActorChangedSector(AActor* TrackedActor, FIntVector2D PreviousSector, FIntVector2D NewSector);

	// queue where threads send their finished jobs to
	TQueue<FTerrainJob, EQueueMode::Mpsc> FinishedJobQueue;

	/**
	 * calculates all tiles adjacent to the given sector (all neighboring tiles)
	 * only tiles in use are searched
	 * @param Sector The sector for which adjacent tiles should be searched
	 * @param OUTAdjacentTiles Out parameter containing all tiles adjacent to the given sector
	 * @param OnlyReturnRelevantTiles If only relevant tiles should be returned. Relevant tiles are only top, bottom, left and right of the given sector.
	 */
	UFUNCTION(BlueprintCallable)
	void GetAdjacentTiles(const FIntVector2D Sector, TArray<ATerrainTile*>& OUTAdjacentTiles, const bool OnlyReturnRelevantTiles = false);

	/**
	* returns (when existent) the track entry and track exit points for the given sector
	* @param Sector The sector for which the track points should be returned
	* @param OUTTrackEntryPoint The entry point of the track in the sector
	* @param OUTTrackExitPoint The exit point of the track in the sector
	* @return -1 if the provided sector has not yet been processed, 0 if the provided sector does not contain a track, 1 if the provided sector does contain a track
	*/
	UFUNCTION()
	int32 GetTrackPointsForSector(const FIntVector2D Sector, FVector2D& OUTTrackEntryPoint, FVector2D& OUTTrackExitPoint);
};
