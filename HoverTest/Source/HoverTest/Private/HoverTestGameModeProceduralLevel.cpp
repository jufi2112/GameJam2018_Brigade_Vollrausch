// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverTestGameModeProceduralLevel.h"
#include "Engine/World.h"
#include "Classes/Kismet/GameplayStatics.h"

void AHoverTestGameModeProceduralLevel::BeginPlay()
{
	Super::BeginPlay();
}

void AHoverTestGameModeProceduralLevel::SetPlayerSpawn(const FTransform Transform)
{
	PlayerSpawn = Transform;
}

void AHoverTestGameModeProceduralLevel::SpawnPlayerFromTerrainManager()
{
	if (!PlayerPawnClass) { return; }
	APlayerController* PC = UGameplayStatics::CreatePlayer(this, -1, true);
	UWorld* World = GetWorld();
	if (World)
	{
		AActor* Actor = World->SpawnActor(PlayerPawnClass, &PlayerSpawn, FActorSpawnParameters());
		if (Actor)
		{
			APawn* Pawn = Cast<APawn>(Actor);
			if (Pawn)
			{
				PC->Possess(Pawn);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve game world in GameMode %s"), *GetName());
	}
}
