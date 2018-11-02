// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HoverThruster.generated.h"

class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOVERTEST_API UHoverThruster : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHoverThruster();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UStaticMeshComponent* StaticMesh = nullptr;

	bool bIsFalling = false;

	bool bShouldHover = true;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = Setup)
	void SetStaticMeshReference(UStaticMeshComponent* MeshToSet, int32 ThrustID);

	// Height in cm above ground the thruster should engage
	UPROPERTY(EditAnywhere, Category = Hover)
	float HoverHeight = 100.f;

	// offset in cm from hover height after which the thruster counts as falling
	UPROPERTY(EditAnywhere, Category = Hover)
	float HoverHeightFallingOffset = 10.f;

	UPROPERTY(EditAnywhere, Category = Hover)
	float TraceLength = 200.f;

	// The maximum force the thruster can push with
	UPROPERTY(EditAnywhere, Category = Hover)
	float MaxHoverForce = 1000000.f;

	// step size in cm
	UPROPERTY(EditAnywhere, Category = Hover)
	float HoverHeightStepSize = 100.f;

	bool GetIsFalling();

	void SetShouldHover(bool NewHoverState);

	void ResetHoverValues();

	// one step equals HoverHeightStepSize cm
	void ChangeHoverHeightBySteps(int32 Steps);

private:

	void HoverLogic();

	// ID is 1-based
	int32 ThrustID = 0;

	float HoverHeightReset = -1.f;
	float HoverHeightFallingOffsetReset = -1.f;
	float TraceLengthReset = -1.f;

		
	
};
