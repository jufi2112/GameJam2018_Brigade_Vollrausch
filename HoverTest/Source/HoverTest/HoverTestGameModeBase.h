// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HoverTestGameModeBase.generated.h"

class ATrackObserver;
class ACheckpoint;
class AHovercraft;
class AHovercraftPlayerController;
class AFinishLine;

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

	UPROPERTY(VisibleAnywhere, Category = Checkpoints)
	int32 NumberOfCheckpointsOnTrack = -1;

	UFUNCTION(BlueprintCallable)
	bool IsCorrectNextCheckpointForHovercraft(ACheckpoint* Checkpoint, AHovercraft* Hovercraft) const;

	UFUNCTION(BlueprintCallable)
	bool IsSameCheckpointForHovercraft(ACheckpoint* Checkpoint, AHovercraft* Hovercraft) const;
	
	
public:

	virtual void BeginPlay() override;

	// TODO: decent to allow this? maybe specific getters for needed information
	UFUNCTION(BlueprintCallable)
	ATrackObserver* GetTrackObserver() const;

	UFUNCTION()
	void RegisterTrackObserver(ATrackObserver* Observer);

	UFUNCTION(BlueprintCallable)
	void HandlePlayerHovercraftCheckpointOverlap(AHovercraft* Hovercraft, AHovercraftPlayerController* PlayerController, ACheckpoint* Checkpoint);

	UFUNCTION(BlueprintCallable)
	void HandleHovercraftFinishLineOverlap(AHovercraft* Hovercraft, AFinishLine* FinishLine);


};
