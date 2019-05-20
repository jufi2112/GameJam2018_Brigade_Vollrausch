// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyStaticLibrary.h"
#include "Hovercraft.generated.h"

class UStaticMeshComponent;
class UHoverThruster;
class USceneComponent;
class UMomentumThruster;
class UCurveFloat;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UCameraComponent;
class UTerrainTrackerComponent;

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

	UFUNCTION(BlueprintCallable)
	void SetResetCurveReference(UCurveFloat* CurveReference);

	UFUNCTION(BlueprintCallable, Category = Setup)
	void SetPostProcessMaterialReference(UMaterialInterface* MaterialInterface, UCameraComponent* CameraToAdd);

	UPROPERTY(VisibleAnywhere, Category = PostProcess)
	bool bIsRadialBlurApplied = false;

	//UFUNCTION(BlueprintCallable, Category = Setup)

	void CheckIsUpsideDown();

	void InitializeThrusters();

	UPROPERTY()
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

	// Length of the reset curve in seconds
	UPROPERTY(EditDefaultsOnly, Category = Reset)
	float ResetCurveLength = 1.f;

	UPROPERTY(VisibleAnywhere, Category = Reset)
	float ResetCurveTimer = 0.f;

	// when resetting, input is accepted
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Reset)
	bool bIsResetting = false;

	UPROPERTY(VisibleAnywhere, Category = Reset)
	UCurveFloat* ResetCurve = nullptr;

	// time needed for resetting in seconds
	UPROPERTY(EditDefaultsOnly, Category = Reset)
	float TimeNeededForReset = 3.f;

	UPROPERTY()
	FTimerHandle ResetTimerHandle;

	UPROPERTY(BlueprintReadOnly)
	UStaticMeshComponent* StaticMesh = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Controls)
	bool bIsPlayerControlled = false;

	UPROPERTY(VisibleAnywhere, Category = Controls)
	EControllerType ControllerType = EControllerType::ECT_Keyboard;

	UPROPERTY(VisibleAnywhere, Category = Controls)
	bool bShowControls = true;

	uint32 CurrentProceduralCheckpointID = 0;

private:
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

	UPROPERTY()
	UTerrainTrackerComponent* TerrainTrackerComponent = nullptr;

	UFUNCTION()
	void OnResetComplete();

	// material instance for standard pawn material
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialStandard = nullptr;

	// material instance for pawn while resetting
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialTranslucent = nullptr;

	// post process material instance
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialPostProcess = nullptr;

	UPROPERTY()
	UCameraComponent* CameraComponent = nullptr;

	UFUNCTION()
	bool HandleResetStuff(float DeltaTime);

	UFUNCTION()
	void HandlePostProcessStuff();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = PostProcess)
	float RadialBlurMaxStrength = .3f;

	UPROPERTY(EditDefaultsOnly, Category = PostProcess)
	float RadialBlurRadius = .25f;

	UFUNCTION(BlueprintCallable, Category = Input)
	void MoveForward(float Value, EControllerType InputControllerType);

	UFUNCTION(BlueprintCallable, Category = Input)
	void MoveRight(float Value, EControllerType InputControllerType);

	// already frame rate independent implemented
	UFUNCTION(BlueprintCallable, Category = Input)
	void RotateRight(float Value, EControllerType InputControllerType);

	// returns ECT_None if pawn is controlled by AI rather than by a player
	UFUNCTION(BlueprintCallable, Category = Controls)
	EControllerType GetControllerType() const;

	// applies the given reset values
	UFUNCTION(BlueprintCallable)
	void ApplyResetValues(const FVector ResetLocation, const FRotator ResetRotation);

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
	bool GetStaticMeshLocation(FVector& Location, float& Yaw);

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

	UFUNCTION(BlueprintCallable)
	UTerrainTrackerComponent* GetTerrainTrackerComponent() const;

	UFUNCTION(BlueprintCallable)
	void ToggleShowControls();

	UFUNCTION(BlueprintCallable)
	bool GetShowControls() const;

	UFUNCTION(BlueprintCallable)
	void SetIsPlayerControlled(bool IsPlayerControlled);

	UFUNCTION()
	void SetNewProceduralCheckpointID(const uint32 NewID);

	UFUNCTION()
	uint32 GetCurrentProceduralCheckpointID() const;

};
