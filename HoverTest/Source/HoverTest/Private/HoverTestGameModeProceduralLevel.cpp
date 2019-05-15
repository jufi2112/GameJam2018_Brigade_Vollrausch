// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverTestGameModeProceduralLevel.h"
#include "Engine/World.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TerrainManager.h"
#include "MyStaticLibrary.h"
#include "Hovercraft.h"
#include "TerrainTrackerComponent.h"
#include "HovercraftPlayerController.h"
#include "ProceduralDefaultPawn.h"

void AHoverTestGameModeProceduralLevel::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		for (TActorIterator<ATerrainManager> ActorIter(GetWorld()); ActorIter; ++ActorIter)
		{
			TerrainManager = *ActorIter;
		}
	}

	if (!TerrainManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find a TerrainManager in %s. Did you forget to add one to the level?"), *GetName());
		if (GetWorld())
		{
			UE_LOG(LogTemp, Warning, TEXT("Will be spawning a TerrainManager with default settings."));
			TerrainManager = GetWorld()->SpawnActor<ATerrainManager>(FActorSpawnParameters());

		}
		if (!TerrainManager)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not spawn a TerrainManager!"));
		}
	}

	if (TerrainManager)
	{
		TerrainManager->BuildTerrainAroundSector(FIntVector2D(0, 0));
	}
}

void AHoverTestGameModeProceduralLevel::SetPlayerSpawn(const FTransform Transform)
{
	PlayerSpawn = Transform;
}

void AHoverTestGameModeProceduralLevel::SpawnPlayerFromTerrainManager(const float TransitionElevationOffset, const float TransitionSpeed, const float DeltaToStop)
{
	if (!PlayerPawnClass || !TerrainManager) { return; }
	if (!bControllerPossessesHovercraftPawn)
	{
		//APlayerController* PC = UGameplayStatics::CreatePlayer(this, -1, true);
		//UE_LOG(LogTemp, Warning, TEXT("Created PlayerController %s"), *PC->GetName());
		UWorld* World = GetWorld();
		if (World)
		{
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(World->GetFirstPlayerController());
			if (!PC)
			{
				UE_LOG(LogTemp, Error, TEXT("Could not retrieve player controller as AHovercraftPlayerController in %s!"), *GetName());
				return;
			}

			AProceduralDefaultPawn* DefaultPawn = Cast<AProceduralDefaultPawn>(PC->GetPawn());
			if (DefaultPawn)
			{
				DefaultPawn->StartTransition(PlayerSpawn.GetLocation() + FVector(0.f, 0.f, TransitionElevationOffset), TransitionSpeed, DeltaToStop);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Could not cast player controller into AProceduralDefaultPawn in %s"), *GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not retrieve game world in GameMode %s"), *GetName());
		}
	}
}

void AHoverTestGameModeProceduralLevel::DefaultPawnFinishedTransition()
{
	if (!bControllerPossessesHovercraftPawn)
	{
		bControllerPossessesHovercraftPawn = true;
		if (!PlayerPawnClass || !TerrainManager) { return; }
		//APlayerController* PC = UGameplayStatics::CreatePlayer(this, -1, true);
		//UE_LOG(LogTemp, Warning, TEXT("Created PlayerController %s"), *PC->GetName());
		UWorld* World = GetWorld();
		if (World)
		{
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(World->GetFirstPlayerController());
			if (!PC)
			{
				UE_LOG(LogTemp, Error, TEXT("Could not retrieve player controller as AHovercraftPlayerController in %s!"), *GetName());
				return;
			}
			AActor* Actor = World->SpawnActor(PlayerPawnClass, &PlayerSpawn, FActorSpawnParameters());
			if (Actor)
			{
				APawn* Pawn = Cast<APawn>(Actor);
				if (Pawn)
				{
					PC->Possess(Pawn);
					PC->AfterDelay();
					TerrainManager->AddActorToTrack(Actor);
				}

				AHovercraft* HC = Cast<AHovercraft>(Actor);
				if (HC)
				{
					// link Hovercraft's TerrainTrackerComponent with TerrainManager
					UTerrainTrackerComponent* TTC = HC->GetTerrainTrackerComponent();
					if (TTC)
					{
						TTC->SetTerrainManager(TerrainManager);
						TTC->ActivateTracking();
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("TerrainTrackerComponent from %s is nullpointer in %s!"), *Actor->GetName(), *GetName());
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Could not cast spawned player to AHovercraft in %s"), *GetName());
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not retrieve game world in GameMode %s"), *GetName());
		}
	}
}
