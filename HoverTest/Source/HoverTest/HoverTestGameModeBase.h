// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HoverTestGameModeBase.generated.h"

class ACheckpoint;

/**
 * 
 */
UCLASS()
class HOVERTEST_API AHoverTestGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setup)
	TArray<ACheckpoint*> CheckPoints;
	
};
