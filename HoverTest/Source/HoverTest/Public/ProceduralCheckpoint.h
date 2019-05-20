// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralCheckpoint.generated.h"

class UShapeComponent;
class UPrimitiveComponent;

/**
 * base class for checkpoints that get spawned in the procedural level
 * this c++ code provides base functionality while child blueprints provide the collision boxes
 */
UCLASS()
class HOVERTEST_API AProceduralCheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralCheckpoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * The unique ID of this checkpoint, gets assigned by the terrain manager
	 */
	UPROPERTY()
	uint32 CheckpointID = 0;

	/**
	 * The collision shape
	 */
	UPROPERTY()
	UShapeComponent* CollisionShape = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SetCheckpointID(const uint32 NewCheckpointID);

	UFUNCTION()
	uint32 GetCheckpointID() const;

	UFUNCTION(BlueprintCallable, Category = "Setup")
	void SetCollisionShapeReference(UShapeComponent* ReferenceToSet);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	
	
};
