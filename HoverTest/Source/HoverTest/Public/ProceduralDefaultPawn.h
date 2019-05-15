// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ProceduralDefaultPawn.generated.h"

UCLASS()
class HOVERTEST_API AProceduralDefaultPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AProceduralDefaultPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * bool to indicate if transition to target position should be started
	 */
	UPROPERTY()
	bool bTransitionInProcess = false;

	/**
	 * the target position for the transition
	 */
	UPROPERTY()
	FVector TransitionTarget = FVector(0.f, 0.f, 0.f);

	/**
	 * interpolation speed for the transition
	 */
	UPROPERTY()
	float TransitionSpeed = 0.f;

	/**
	 * the spawn position
	 */
	UPROPERTY()
	FTransform SpawnTransform = FTransform();

	/**
	 * delta between the target position and the current position where we can stop interpolation
	 */
	UPROPERTY()
	float TransitionDeltaToStop = 0.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * called to start the transition
	 */
	UFUNCTION(BlueprintCallable)
	void StartTransition(const FVector Target, const float Speed, const float DeltaToStop);
};
