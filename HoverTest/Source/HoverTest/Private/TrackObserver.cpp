// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackObserver.h"
#include "HoverTestGameModeBase.h"
#include "Engine/World.h"


// Sets default values
ATrackObserver::ATrackObserver()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATrackObserver::BeginPlay()
{
	Super::BeginPlay();

	// already done in the game mode
	/*AHoverTestGameModeBase* GameMode = Cast<AHoverTestGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve correct game mode in %s"), *GetName());
	}
	else
	{
		GameMode->RegisterTrackObserver(this);
	}*/
	
}

// Called every frame
void ATrackObserver::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

