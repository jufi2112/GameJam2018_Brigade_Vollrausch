// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HoverTestGameModeBase.h"
#include "HoverTestGameModeProceduralLevel.generated.h"

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
	 */
	UFUNCTION()
	void SpawnPlayerFromTerrainManager();

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


private:

	/**
	 * Transform where to spawn the player
	 */
	UPROPERTY()
	FTransform PlayerSpawn = FTransform();
	
};
