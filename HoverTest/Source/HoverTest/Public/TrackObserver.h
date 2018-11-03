// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackObserver.generated.h"

class ACheckpoint;

UCLASS()
class HOVERTEST_API ATrackObserver : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrackObserver();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// list of all checkpoints on the track in this level
	// ordering corresponds to sequence of checkpoints in level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setup)
	TArray<ACheckpoint*> Checkpoints;

	
	
};
