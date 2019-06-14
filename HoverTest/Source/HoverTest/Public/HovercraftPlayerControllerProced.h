// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HovercraftPlayerController.h"
#include "HovercraftPlayerControllerProced.generated.h"

/**
 * 
 */
UCLASS()
class HOVERTEST_API AHovercraftPlayerControllerProced : public AHovercraftPlayerController
{
	GENERATED_BODY()
	
protected:

	bool bIsBiplaneActive = false;

public:

	UFUNCTION(BlueprintCallable, Category = Controls)
	void ToggleBiplane();
	
	
};
