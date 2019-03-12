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
	* TODO to be implemented
	*/
	UFUNCTION(BlueprintCallable)
	void HandleTrackedActorChangedSector(AActor* TrackedActor, FIntVector2D PreviousSector, FIntVector2D NewSector);

	// queue where threads send their finished jobs to
	TQueue<FTerrainJob, EQueueMode::Mpsc> FinishedJobQueue;

	/**
	* TODO
	*/
};
