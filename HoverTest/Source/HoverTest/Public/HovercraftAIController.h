// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HovercraftAIController.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTEST_API AHovercraftAIController : public AAIController
{
	GENERATED_BODY()

public:

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = Navigation)
	float AcceptanceRadius = 100.f;


	
	
	
	
};
