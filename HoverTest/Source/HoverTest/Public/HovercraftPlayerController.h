// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HovercraftPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTEST_API AHovercraftPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = Reset)
	FVector ResetPosition = FVector(0.f, 0.f, 0.f);

	UPROPERTY(VisibleAnywhere, Category = Reset)
	float ResetYaw = 0.f;

public:

	UFUNCTION(BlueprintCallable)
	void SetResetPosition(FVector NewResetPosition);

	UFUNCTION(BlueprintCallable)
	FVector GetResetPosition();

	UFUNCTION(BlueprintCallable)
	float GetResetYaw();

	UFUNCTION(BlueprintCallable)
	void SetResetYaw(float Yaw);



	virtual void BeginPlay() override;


	
	
	
	
};
