// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HoverTestGameModeBase.h"
#include "HoverTestGameModeProceduralLevel.generated.h"

class ATerrainManager;
class AProceduralCheckpoint;
class AProceduralDefaultPawn;

/**
 * 
 */
UCLASS()
class HOVERTEST_API AHoverTestGameModeProceduralLevel : public AHoverTestGameModeBase
{
	GENERATED_BODY()
	
	
public:

	virtual void BeginPlay() override;

	UFUNCTION()
	void SetPlayerSpawn(const FTransform Transform);

	/**
	 * Called from the terrain manager when all necessary tiles are created and the player can be spawned
	 * the actual spawn functionality is provided by DefaultPawnFinishedTransition()
	 * this function initiates the transition of the default pawn to the Hovercraft pawn's location (for a smooth feeling)
	 */
	UFUNCTION()
	void SpawnPlayerFromTerrainManager(const float TransitionElevationOffset, const float TransitionSpeed, const float DeltaToStop);

	/**
	 * The player pawn class
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* PlayerPawnClass = nullptr;

	/**
	 * The player controller
	 */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//APlayerController* PC = nullptr;

	/**
	 * function called when the default pawn finished transition to the player pawn
	 */
	UFUNCTION()
	void DefaultPawnFinishedTransition();

	void HandlePlayerHovercraftCheckpointOverlap(AHovercraft * Hovercraft, AHovercraftPlayerController* PlayerController, AProceduralCheckpoint * Checkpoint);

	UFUNCTION()
	ATerrainManager* GetTerrainManager() const;

	/**
	 * Switches the player controller's controlled pawn from hovercraft to default pawn and sets default pawn's location to the location provided
	 */
	UFUNCTION()
	void SwitchToDefaultPawn(APawn* CurrentPawn, const FVector DefaultPawnLocation);

	/**
	 * Switches the player controller's controlled pawn from the default pawn to the hovercraft pawn
	 */
	UFUNCTION()
	void SwitchToPlayerPawn();


private:

	/**
	 * Transform where to spawn the player
	 */
	UPROPERTY()
	FTransform PlayerSpawn = FTransform();

	/**
	 * Reference to the terrain manager
	 */
	UPROPERTY()
	ATerrainManager* TerrainManager = nullptr;

	/**
	 * Reference to default pawn
	 */
	UPROPERTY()
	AProceduralDefaultPawn* DefaultPawnReference = nullptr;

	/**
	 * Reference to player pawn
	 */
	UPROPERTY()
	APawn* PlayerPawn = nullptr;

	/**
	 * bool that indicates if the player controller possesses the Hovercraft pawn
	 */
	UPROPERTY()
	bool bControllerPossessesHovercraftPawn = false;
	
};
