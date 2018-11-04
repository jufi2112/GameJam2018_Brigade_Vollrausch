// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Hovercraft.generated.h"

class UStaticMeshComponent;
class UHoverThruster;
class USceneComponent;
class UMomentumThruster;

UCLASS()
class HOVERTEST_API AHovercraft : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHovercraft();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = Setup)
	void SetStaticMeshReference(UStaticMeshComponent* MeshToSet);

	void CheckIsUpsideDown();

	void InitializeThrusters();

	TArray<UHoverThruster*> HoverThrusters;

	// Should the Hovercraft hover?
	UPROPERTY(VisibleAnywhere, Category = Movement)
	bool bShouldHover = true;

	// index of the last passed checkpoint, -1 when passed finish line but not yet passed a checkpoint, -2 if not yet passed the finish line
	UPROPERTY(VisibleAnywhere, Category = Checkpoints)
	int32 IndexOfLastCheckpoint = -2;

	UPROPERTY(VisibleAnywhere, Category = Time)
	float LapTime = 0.f;

	UPROPERTY(VisibleAnywhere, Category = Time)
	bool bShouldStopTime = false;


private:

	UStaticMeshComponent* StaticMesh = nullptr;

	TArray<bool> IsFallingArray;

	UPROPERTY(VisibleAnywhere, Category = Movement)
	bool bIsUpsideDown = false;

	bool bDrawDebugTraces = false;

	// unused
	UPROPERTY()
	UMomentumThruster* RightMomentumThruster = nullptr;

	// unused
	UPROPERTY()
	UMomentumThruster* LeftMomentumThruster = nullptr;

	// unused
	UPROPERTY()
	UMomentumThruster* BackMomentumThruster = nullptr;

	UPROPERTY()
	USceneComponent* RightRotationPoint = nullptr;

	UPROPERTY()
	USceneComponent* LeftRotationPoint = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = Input)
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = Input)
	void MoveRight(float Value);

	// already frame rate independent implemented
	UFUNCTION(BlueprintCallable, Category = Input)
	void RotateRight(float Value);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = Movement)
	float ForwardAcceleration = 10.f;

	UPROPERTY(EditAnywhere, Category = Movement)
	float SidewardAcceleration = 5.f;

	UPROPERTY(EditAnywhere, Category = Movement)
	float RotationSpeed = 30.f;

	// how much forward control does the user have while airborne? values from 0 to 1
	UPROPERTY(EditAnywhere, Category = Movement)
	float ForwardAirborneControl = 0.5f;

	// how much sidewards control does the user have while airborne? values from 0 to 1
	UPROPERTY(EditAnywhere, Category = Movement)
	float SidewardsAirborneControl = 0.05f;

	// calculates the combined mass of this hovercraft (sum of mass from all primitive components)
	float GetHovercraftMass();

	void SetIsFalling(bool IsFalling, int32 ThrustID);

	bool GetIsFalling();

	// gets calculated automatically
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Setup)
	int32 NumberOfThrusters = -1;

	// force that acts on the hovercraft when it is falling ("additional gravity")
	UPROPERTY(EditAnywhere, Category = Movement)
	float DownwardForce = 500000.f;
	
	// trace length in cm that determines how far up the trace for detecting upside down position of the hovercraft should go
	UPROPERTY(EditAnywhere, Category = Setup)
	float UpsideDownTraceLength = 200.f;

	// height in cm the hovercraft should be placed when resetting ! relative to the previous position !
	UPROPERTY(EditAnywhere, Category = Setup)
	float ResetHeightModificator = 100.f;

	UPROPERTY(EditAnywhere, Category = Movement)
	float RotationForce = 12000000.f;

	UFUNCTION(BlueprintCallable, Category = Movement)
	void ResetHovercraft(USceneComponent* AzimuthGimbal);

	UFUNCTION(BlueprintCallable, Category = Movement)
	void ToggleShouldHover();

	UFUNCTION(BlueprintCallable, Category = Setup)
	void ResetHoverHeight();

	UFUNCTION(BlueprintCallable, Category = Setup)
	void ChangeHoverHeightBySteps(int32 Steps);

	UFUNCTION(BlueprintCallable, Category = Setup)
	void ToggleDrawDebugTraces();

	UFUNCTION()
	bool GetStaticMeshLocation(FVector& Location);

	// returns the hovercrafts speed in km/h
	UFUNCTION(BlueprintCallable)
	int32 GetSpeed() const;

	// unused
	UFUNCTION(BlueprintCallable)
	void SetMomentumThrusterReferences(UMomentumThruster* RightMomentumReference, UMomentumThruster* LeftMomentumReference, UMomentumThruster* BackMomentumReference);

	UFUNCTION(BlueprintCallable)
	void SetRotationPointReferences(USceneComponent* RightRotationPointReference, USceneComponent* LeftRotationPointReference);

	UFUNCTION(BlueprintCallable)
	int32 GetIndexOfLastCheckpoint() const;

	UFUNCTION(BlueprintCallable)
	void SetIndexOfLastCheckpoint(int32 NewCheckpointID);

	UFUNCTION(BlueprintCallable)
	void SetStopLapTime(bool ShouldStopLapTime);

	UFUNCTION(BlueprintCallable)
	float ResetLapTimer();

	UFUNCTION(BlueprintCallable)
	float GetLapTime() const;

};
