// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyStaticLibrary.h"
#include "Runtime/Core/Public/Containers/Queue.h"
#include "ProceduralCheckpoint.h"
#include "TerrainManager.generated.h"

class ATerrainTile;
class AHoverTestGameModeProceduralLevel;

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
	void CalculateSectorsNeededAroundGivenLocation(const FVector Location, TArray<FIntVector2D>& OUTSectorsNeeded);

	/**
	* returns an array containing all sectors around the given sector that should be covered with tiles according to TilesToBeCreatedAroundActorRadius in FTerrainSettings
	* the function does not check if sectors may already be coverd by tiles
	* the sector provided to the function will also get included in the return array
	*/
	UFUNCTION(BlueprintCallable)
	void CalculateSectorsNeededAroundGivenSector(const FIntVector2D Sector, TArray<FIntVector2D>& OUTNeededSectors);

	// array of queues, where each queue is an input queue for a thread
	TArray<TQueue<FTerrainJob, EQueueMode::Spsc>> TerrainCreationQueue;

	// queue for pending terrain jobs
	TQueue<FTerrainJob, EQueueMode::Spsc> PendingTerrainJobQueue;

	// queue for pending checkpoint spawns
	TQueue<FCheckpointSpawnJob, EQueueMode::Spsc> PendingCheckpointSpawnQueue;

	// array of all used threads
	TArray<FRunnableThread*> Threads;

	// hashmap that stores all already calculated track information for every processed sector
	//UPROPERTY()
	TMap<FIntVector2D, FSectorTrackInfo> TrackMap;

	/**
	 * calculates the global track path for all sectors in SectorsToCreateTileFor
	 */
	UFUNCTION()
	void CalculateTrackPath(const TArray<FIntVector2D> SectorsToCreate);

	/**
	 * the following 4 points define a quad that restricts the area, where new track parts can be created
	 * i.e. NextTrackSector cannot be located in the defined quad
	 * in the comments, this quad is often refered to as 'no-go quad'
	 */

	UPROPERTY()
	FIntVector2D TopLeftCorner;

	UPROPERTY()
	FIntVector2D TopRightCorner;

	UPROPERTY()
	FIntVector2D BottomLeftCorner;

	UPROPERTY()
	FIntVector2D BottomRightCorner;

	// the sector where the next track segment should be created (not yet created)
	UPROPERTY()
	FIntVector2D NextTrackSector = FIntVector2D();

	// the sector where the current last track segment is located (already created)
	UPROPERTY()
	FIntVector2D CurrentTrackSector = FIntVector2D();

	/**
	 * Next available check point ID
	 */
	UPROPERTY()
	uint32 NextAvailableCheckPointID = 1;

	/**
	 * array that contains sectors that need coverage before the player can be resetted
	 */
	UPROPERTY()
	TArray<FIntVector2D> SectorsNeedCoverageForReset;

	/**
	 * if the terrain manager should check if the SectorsNeedCoverageForReset array is empty after removing jobs from the FinishedJobQueue
	 */
	UPROPERTY()
	bool bShouldCheckSectorsNeedCoverageForReset = false;

	/**
	 * array that contains all currently processed tiles
	 */
	UPROPERTY()
	TArray<FIntVector2D> SectorsCurrentlyProcessed;


private:

	/**
	 * bool to check whether we already recalculated the tile 'behind' the track start point or not
	 */
	UPROPERTY()
	bool bRecalculatedTileBehindStartPoint = false;

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

	/**
	 * calculates the new NextTrackSector
	 * updates NextTrackSector and CurrentTrackSector
	 */
	UFUNCTION()
	FSectorTrackInfo CalculateNewNextTrackSector();

	/**
	 * checks if the given sector lies within the quad specified by TopLeftCorner, TopRightCorner, BottomLeftCorner, BottomRightCorner
	 */
	UFUNCTION()
	bool CheckSectorWithinQuad(const FIntVector2D Sector);

	/**
	 * checks if the given sector can become the new NextTrackSector
	 */
	UFUNCTION()
	bool CheckupSector(const FIntVector2D Sector);

	/**
	 * calculates the entry and exit points for the CurrentTrackSector with the given FSectorTrackInfo
	 */
	UFUNCTION()
	void CalculateEntryExitPoints(const FSectorTrackInfo TrackInfo, FVector2D& OUTEntryPoint, FVector2D& OUTExitPoint);

	/**
	 * adjusts the no-go quad's corner points to include the new CurrentTrackSector
	 */
	UFUNCTION()
	void AdjustQuad();

	/**
	 * creates a FRuntimeMeshVertexSimple struct from the given information
	 */
	FRuntimeMeshVertexSimple CreateRuntimeMeshVertexSimple(const FVector Vertex, const FVector Normal) const;

	/**
	 * calculates the TrackExitPoint's elevation for a track in the specified sector
	 * @param Sector The sector for which the exit point elevation should be calculated
	 * @param TrackInfo The current sector's track info
	 * @param OUTExitPointElevation Out parameter that contains the exit point's elevation
	 * @return True if exit point's elevation could be calculated successfully, false if an error occured
	 */
	UFUNCTION()
	bool CalculateTrackExitPointElevation(const FIntVector2D Sector, const FSectorTrackInfo TrackInfo, float& OUTExitPointElevation);

	/**
	 * calculates the two control points of the bezier curve for the given sector
	 * @param Sector The sector to calculate the control points for
	 * @param TrackEntryPoint The entry point of the track in the current sector, with elevation
	 * @param OUTControlPointOne Out parameter of the first control point
	 * @param OUTControlPointTwo Out parameter of the second control point
	 */
	UFUNCTION()
	void CalculateBezierControlPoints(const FIntVector2D Sector, const FSectorTrackInfo TrackInfo, FVector& OUTControlPointOne, FVector& OUTControlPointTwo);

	/**
	 * bool to check if at least one tile has been added to the terrain creation queue
	 * this is used to check if we can spawn the player (i.e. if all tiles that needed to be created at game start have been created and drawn)
	 */
	UPROPERTY()
	bool bHasTileBeenAddedToQueue = false;

	/**
	 * counter that keeps track on how many tiles were added to the terrain creation queue and how many tiles are already processed
	 * used in combination with bHasTileBeenAddedToQueue to check if all initially needed tiles have been created
	 */
	UPROPERTY()
	int32 TilesInProcessCounter = 0;

	/**
	 * bool to make sure only one player gets spawned
	 */
	UPROPERTY()
	int32 RemainingPlayersToSpawn = 1;

	UPROPERTY()
	AHoverTestGameModeProceduralLevel* GameMode = nullptr;




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

	/**
	 * creates terrain around the given sector, used on game start to build terrain before a player is spawned
	 */
	UFUNCTION(BlueprintCallable)
	void BuildTerrainAroundSector(const FIntVector2D Sector);

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
	int32 GetTrackPointsForSector(const FIntVector2D Sector, FVector& OUTTrackEntryPoint, FVector& OUTTrackExitPoint);

	/**
	 * calculates a Bézier curve given by the provided start and end point and two internal created control points
	 * @param Sector The sector for which the track gets generated, used to update its track sector info
	 * @param StartPoint The start point of the Bézier curve (the entry point of the track in the tile)
	 * @param EndPoint The end point of the Bézier curve (the exit point of the track in the tile)
	 * @param OUTVertexBuffer A vertex buffer where the generated triangle points should be stored
	 * @param OUTTriangleBuffer A triangle buffer where the generated triangle point order should be stored
	 */
	void GenerateTrackMesh(const FIntVector2D Sector, const FVector StartPoint, const FVector EndPoint, TArray<FRuntimeMeshVertexSimple>& OUTVertexBuffer, TArray<int32>& OUTTriangleBuffer, TArray<FTrackSegment>& TrackSegments);

	/**
	 * used to recalculate the tile for the given sector
	 * only recalculates the tile if it was already calculated (i.e. a tile with the given sector is in TilesInUse)
	 * @param Sector The sector for which the tile should be recalculated
	 */
	UFUNCTION()
	void RecalculateTileForSector(const FIntVector2D Sector);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<AProceduralCheckpoint> CheckpointClassToSpawn;

	/**
	 * checks if a tile is present at the given location
	 * @param Location The location to check for existence of a tile
	 * @return Whether a tile is present at the given location or not
	 */
	UFUNCTION(BlueprintCallable)
	bool IsLocationCoveredByTile(const FVector Location);

	UFUNCTION(BlueprintCallable)
	float GetTerrainSettingsTransitionElevationOffset() const;

	UFUNCTION(BlueprintCallable)
	float GetTerrainSettingsTransitionInterpolationSpeed() const;

	UFUNCTION(BlueprintCallable)
	float GetTerrainSettingsTransitionDeltaToStop() const;

	/**
	* called to begin tile generation around given location
	* notifies game mode if all necessary tiles are created
	* ! ONLY to be called when resetting the player pawn to the given sector !
	* @param Location The location to create tiles around
	*/
	UFUNCTION()
	void BeginTileGenerationForReset(const FVector Location);
};
