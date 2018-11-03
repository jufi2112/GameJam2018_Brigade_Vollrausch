// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HoverTestGameModeBase.generated.h"

class ATrackObserver;

/**
 * 
 */
UCLASS()
class HOVERTEST_API AHoverTestGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category = Checkpoints)
	ATrackObserver* TrackObserver = nullptr;
	
	
public:

	virtual void BeginPlay() override;

	
};
