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
#include "HovercraftPlayerControllerProced.h"

AHoverTestGameModeProceduralLevel::AHoverTestGameModeProceduralLevel()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = true;
}

void AHoverTestGameModeProceduralLevel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bInformHovercraftAfterTicks)
	{
		TicksAfterWhichToInformHovercraft--;
		if (TicksAfterWhichToInformHovercraft <= 0)
		{
			AHovercraft* HC = Cast<AHovercraft>(PlayerPawn);
			if (HC)
			{
				HC->OnMultipointTransitionResetComplete();
				bInformHovercraftAfterTicks = false;
				SetActorTickEnabled(false);
			}
		}
	}
}

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
				DefaultPawnReference = DefaultPawn;
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
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AActor* Actor = World->SpawnActor(PlayerPawnClass, &PlayerSpawn, SpawnParameters);
			if (Actor)
			{
				APawn* Pawn = Cast<APawn>(Actor);
				if (Pawn)
				{
					PC->Possess(Pawn);
					bControllerPossessesHovercraftPawn = true;
					PC->AfterDelay();
					PlayerPawn = Pawn;
				}

				AHovercraft* HC = Cast<AHovercraft>(Actor);
				if (HC)
				{
					HC->SetIsPlayerControlled(true);
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

void AHoverTestGameModeProceduralLevel::DefaultPawnFinishedMultipointTransition()
{
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid player pawn found in DefaultPawnFinishedMultipointTransition in %s"), *GetName());
		return;
	}
	if (!bControllerPossessesHovercraftPawn)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(World->GetFirstPlayerController());
			if (PC)
			{
				PC->Possess(PlayerPawn);
				bControllerPossessesHovercraftPawn = true;
				AHovercraft* HC = Cast<AHovercraft>(PlayerPawn);
				if (HC)
				{
					// apply reset values
					HC->ApplyResetValues(PlayerResetLocation, PlayerResetRotation);
					SetActorTickEnabled(true);
					bInformHovercraftAfterTicks = true;
					TicksAfterWhichToInformHovercraft = 2;
				}
			}
		}
	}
}

void AHoverTestGameModeProceduralLevel::HandlePlayerHovercraftCheckpointOverlap(AHovercraft * Hovercraft, AHovercraftPlayerController * PlayerController, AProceduralCheckpoint * Checkpoint)
{
	if (!Hovercraft || !PlayerController || !Checkpoint || !TerrainManager)
	{
		return;
	}

	if (Checkpoint->GetCheckpointID() > Hovercraft->GetCurrentProceduralCheckpointID())
	{
		Hovercraft->SetNewProceduralCheckpointID(Checkpoint->GetCheckpointID());
		PlayerController->SetResetPosition(Checkpoint->GetActorLocation());
		PlayerController->SetResetYaw(Checkpoint->GetActorRotation().Yaw);
	}
}

ATerrainManager * AHoverTestGameModeProceduralLevel::GetTerrainManager() const
{
	return TerrainManager;
}

void AHoverTestGameModeProceduralLevel::SwitchToDefaultPawnAndStartMultipointTransition(APawn* CurrentPawn, const FVector DefaultPawnTargetLocation, const FRotator ResetRotation)
{
	if (!DefaultPawnReference || !TerrainManager) { return; }
	if (bControllerPossessesHovercraftPawn)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(World->GetFirstPlayerController());
			if (PC)
			{
				// save reference to calling pawn so we can switch back to it later
				PlayerPawn = CurrentPawn;
				DefaultPawnReference->SetActorLocation(CurrentPawn->GetActorLocation());
				PC->Possess(DefaultPawnReference);
				// start transition in default pawn
				DefaultPawnReference->StartMultipointTransition(PlayerPawn->GetActorLocation(), DefaultPawnTargetLocation + FVector(0.f, 0.f, TerrainManager->GetTerrainSettingsTransitionElevationOffset()), TerrainManager->GetTerrainSettingsTransitionInterpolationSpeed(), TerrainManager->GetTerrainSettingsTransitionDeltaToStop());
				bControllerPossessesHovercraftPawn = false;

				PlayerResetLocation = DefaultPawnTargetLocation;
				PlayerResetRotation = ResetRotation;
			}
		}
	}
}

void AHoverTestGameModeProceduralLevel::SwitchToPlayerPawn()
{
	if (!PlayerPawn) { return; }
	if (!bControllerPossessesHovercraftPawn)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(World->GetFirstPlayerController());
			if (PC)
			{
				PC->Possess(PlayerPawn);
				bControllerPossessesHovercraftPawn = true;
			}
		}
	}
}

void AHoverTestGameModeProceduralLevel::AllowDefaultPawnToTransitionToEndLocation()
{
	if (!DefaultPawnReference) { return; }
	DefaultPawnReference->AllowMultipointTransitionZoomToEndPoint();
}

bool AHoverTestGameModeProceduralLevel::WasPlayerPawnCreated() const
{
	if (PlayerPawn && PlayerPawn->IsValidLowLevel())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AHoverTestGameModeProceduralLevel::ToggleBetweenHovercraftAndBiplane(bool bIsBiplaneActive)
{
	if (!PlayerPawn || !TerrainManager) { return; }
	UWorld* World = GetWorld();
	if (!World) { return; }
	AHovercraftPlayerControllerProced* PC = Cast<AHovercraftPlayerControllerProced>(World->GetFirstPlayerController());
	if (!PC) { return; }

	if (bIsBiplaneActive)
	{
		// switch from player pawn to biplane
		// check if Biplane already created
		if (!BiplanePawn)
		{
			// create it
			if (BiplaneClass)
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				FTransform PlayerTransform = PlayerPawn->GetActorTransform();
				FVector Location = PlayerTransform.GetLocation();
				Location.Z += 5000.f;
				PlayerTransform.SetLocation(Location);

				AActor* Actor = World->SpawnActor(BiplaneClass, &PlayerTransform, SpawnParameters);
				if (Actor)
				{
					ABiplanePawn* Biplane = Cast<ABiplanePawn>(Actor);
					if (Biplane)
					{
						BiplanePawn = Biplane;
						BiplanePawn->SetSpawnSpeed(5000.f);
						PC->Possess(Biplane);

						UTerrainTrackerComponent* TTC = BiplanePawn->GetTerrainTrackerComponent();
						if (TTC)
						{
							TTC->SetTerrainManager(TerrainManager);
							TTC->ActivateTracking();
						}
					}
				}
				else
				{
				}
			}
		}
		else
		{
			// biplane already exists -> possess it
			PC->Possess(BiplanePawn);
		}
	}
	else
	{
		// switch from biplane to player pawn
		PC->Possess(PlayerPawn);
		BiplanePawn->Destroy();
		BiplanePawn = nullptr;
	}
}

