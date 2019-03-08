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

	// the sector the actor is currently in
	UPROPERTY()
		FIntVector2D CurrentSector;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** gets the TerrainManager actor that is placed in the level
	* can return a nullptr if there is not exactly one TerrainManager in the level
	*/
	UFUNCTION()
		ATerrainManager* GetWorldTerrainManager();

};
