// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyStaticLibrary.h"
#include "BiplanePawn.generated.h"

class UPoseableMeshComponent;
class UStaticMeshComponent;
class UTerrainTrackerComponent;

UCLASS()
class HOVERTEST_API ABiplanePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABiplanePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	UPoseableMeshComponent* PoseableMesh = nullptr;

	UPROPERTY()
	UStaticMeshComponent* StaticMesh = nullptr;

	UPROPERTY()
	UTerrainTrackerComponent* TTC = nullptr;

	UPROPERTY()
	EControllerType ControllerType = EControllerType::ECT_Keyboard;

	// amount of throttle currently applied to the plane
	uint32 ThrottleAmount = 30.f;

	float AirControl = 2500.f;

	float LastRollAxisValue = 0.f;
	float LastPitchAxisValue = 0.f;
	float LastYawAxisValue = 0.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = Setup)
	void SetPoseableMeshReference(UPoseableMeshComponent* ReferenceToSet);

	UFUNCTION(BlueprintCallable, Category = Setup)
	void SetStaticMeshReference(UStaticMeshComponent* ReferenceToSet);

	UFUNCTION(BlueprintCallable, Category = Controls)
	void ThrottleInput(float Value, EControllerType InputControllerType);

	UFUNCTION(BlueprintCallable)
	EControllerType GetControllerType() const;

	UFUNCTION(BlueprintCallable, Category = Controls)
	void BiplanePitch(float Value, EControllerType InputControllerType);

	UFUNCTION(BlueprintCallable, Category = Controls)
	void BiplaneRoll(float Value, EControllerType InputControllerType);

	UFUNCTION(BlueprintCallable, Category = Controls)
	void BiplaneYaw(float Value, EControllerType InputControllerType);

	UFUNCTION(BlueprintCallable)
	void CalculateSpeed();

	UFUNCTION(BlueprintCallable, Category = Setup)
	void SetTerrainTrackerReference(UTerrainTrackerComponent* ReferenceToSet);

	UFUNCTION(BlueprintCallable)
	UTerrainTrackerComponent* GetTerrainTrackerComponent() const;

	UFUNCTION(BlueprintCallable)
	void AnimateBiplane(float DeltaTime);

	UFUNCTION()
	void SetSpawnSpeed(float Speed);

	
};
