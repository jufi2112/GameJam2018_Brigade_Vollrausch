// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyStaticLibrary.h"
#include "Components/ActorComponent.h"
#include "TerrainTrackerComponent.generated.h"

class ATerrainManager;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTEST_API UTerrainTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTerrainTrackerComponent();

	/** called when the component gets destroyed (i.e. the owning actor gets destroyed)
	* removes the owning actor from the actors to be tracked in the terrain manager
	*/
	virtual void OnUnregister() override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	ATerrainManager* TerrainManager = nullptr;

	// the sector the actor is currently located in
	UPROPERTY()
	FIntVector2D CurrentSector;

	/**
	 * if the tracker component should track
	 */
	UPROPERTY()
	bool bShouldTrack = false;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * called from the gamemode
	 */
	UFUNCTION(BlueprintCallable)
	void SetTerrainManager(ATerrainManager* TerrainManagerToSet);

	/**
	 * activates tracking
	 */
	UFUNCTION(BlueprintCallable)
	void ActivateTracking();

	/**
	 * deactivates tracking
	 */
	UFUNCTION(BlueprintCallable)
	void DeactivateTracking();
};
