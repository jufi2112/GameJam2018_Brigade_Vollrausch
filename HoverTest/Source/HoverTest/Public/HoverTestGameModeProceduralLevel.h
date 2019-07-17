// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HoverTestGameModeBase.h"
#include "BiplanePawn.h"
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

	AHoverTestGameModeProceduralLevel();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

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
	 * The Biplane pawn class
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setup)
	TSubclassOf<ABiplanePawn> BiplaneClass;

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

	/**
	 * function called when the default pawn finished multipoint transition
	 */
	UFUNCTION()
	void DefaultPawnFinishedMultipointTransition();

	void HandlePlayerHovercraftCheckpointOverlap(AHovercraft * Hovercraft, AHovercraftPlayerController* PlayerController, AProceduralCheckpoint * Checkpoint);

	UFUNCTION()
	ATerrainManager* GetTerrainManager() const;

	/**
	 * Switches the player controller's controlled pawn from hovercraft to default pawn and sets default pawn's location to the location provided
	 * ! ONLY cass this function if the player hovercraft should be resetted !
	 */
	UFUNCTION()
	void SwitchToDefaultPawnAndStartMultipointTransition(APawn* CurrentPawn, const FVector DefaultPawnTargetLocation, const FRotator ResetRotation);

	/**
	 * Switches the player controller's controlled pawn from the default pawn to the hovercraft pawn
	 */
	UFUNCTION()
	void SwitchToPlayerPawn();

	/**
	 * called to allow the default pawn to transition to the multipoint target location
	 */
	UFUNCTION()
	void AllowDefaultPawnToTransitionToEndLocation();

	UFUNCTION()
	bool WasPlayerPawnCreated() const;

	UFUNCTION()
	void ToggleBetweenHovercraftAndBiplane(bool bIsBiplaneActive);

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
	 * Reference to biplane pawn
	 */
	UPROPERTY()
	ABiplanePawn* BiplanePawn = nullptr;

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

	/**
	 * player reset location to apply after multipoint transition finished
	 */
	UPROPERTY()
	FVector PlayerResetLocation = FVector();

	/**
	 * player reset rotation to apply after multipoint transition finished
	 */
	UPROPERTY()
	FRotator PlayerResetRotation = FRotator();

	UPROPERTY()
	bool bInformHovercraftAfterTicks = false;

	uint32 TicksAfterWhichToInformHovercraft = 2;

	
};
