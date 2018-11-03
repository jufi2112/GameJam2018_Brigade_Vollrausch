// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverTestGameModeBase.h"
#include "TrackObserver.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void AHoverTestGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		for (TActorIterator<ATrackObserver> ActorIter(GetWorld()); ActorIter; ++ActorIter)
		{
			TrackObserver = *ActorIter;
		}
	}

	if (!TrackObserver)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find a TrackObserver in %s. Did you forget to add one to the level?"), *GetName());
	}
}
