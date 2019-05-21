// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyStaticLibrary.h"
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

	UPROPERTY(VisibleAnywhere, Category = Controls)
	EControllerType ControllerType = EControllerType::ECT_Keyboard;

	/**
	 * intermediate target point for the multipoint transition
	 */
	UPROPERTY()
	FVector MultipointTransitionIntermediatePointAtStartCoordinates = FVector();

	/**
	 * target end point for the multipoint transition
	 */
	UPROPERTY()
	FVector MultipointTransitionTargetEndPoint = FVector();

	/**
	 * intermediate point for the multipoint transition a target end coordinates but with spawn elevation
	 */
	UPROPERTY()
	FVector MultipointTransitionIntermediatePointAtFinalCoordinates = FVector();

	/**
	 * if there is a multipoint transition running currently
	 */
	UPROPERTY()
	bool bIsMultipointTransitionInProcess = false;

	/**
	 * if a currently running multipoint transition has already reached the intermediate transition point
	 */
	UPROPERTY()
	bool bHasMultipointTransitionReachedIntermediatePointAtSpawnElevation = false;

	/**
	 * if the currently running multipoint transition has reached the traget end point at the spawn's elevation
	 */
	UPROPERTY()
	bool bHasMultipointTransitionReachedTargetPointAtSpawnElevation = false;

	/**
	 * multipoint transition speed
	 */
	UPROPERTY()
	float MultipointTransitionSpeed = 0.f;

	/**
	 * delta between the target position and the current position where we can stop interpolation
	 */
	UPROPERTY()
	float MultipointTransitionDeltaToStop = 0.f;

	/**
	 * is the multipoint transition allowed to zoom in to target end point
	 */
	UPROPERTY()
	bool bIsMultipointTransitionAllowedToZoomToEndPoint = false;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * called to start the transition
	 * @param Target The transition target
	 * @param Speed The transition speed
	 * @param DeltaToStop The distance between current position and target position at which the transition is regarded as finished
	 */
	UFUNCTION(BlueprintCallable)
	void StartTransition(const FVector Target, const float Speed, const float DeltaToStop);

	// returns ECT_None if pawn is controlled by AI rather than by a player
	UFUNCTION(BlueprintCallable, Category = Controls)
	EControllerType GetControllerType() const;

	/**
	 * starts a transition from the target start location to the target end location
	 * in GTA V manner, an intermediate point above the target start location is targeted first
	 * then, the camera pans to a point above the target end location
	 * at the end, the target end location is targeted
	 * as elevation for the zooming out process, the Z component of the default spawn is used
	 * @param TargetStartLocation The location from where the transition should begin
	 * @param TargetEndLocation The location where to transition to
	 * @param Speed The transition speed
	 * @param DeltaToStop The distance between current position and target position at which the transition is regarded as finished
	 */
	UFUNCTION()
	void StartMultipointTransition(const FVector TargetStartLocation, const FVector TargetEndLocation, const float Speed, const float DeltaToStop);

	UFUNCTION()
	void AllowMultipointTransitionZoomToEndPoint();
};
